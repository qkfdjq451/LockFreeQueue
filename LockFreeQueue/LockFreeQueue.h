#pragma once
#include <atomic>
#include <array>
#include <mutex>

template<typename T, size_t _size>
class LockFreeQueue
{
public :
	bool push(const T& newElement);
	bool pop(T& returnedElement);
private:
	static constexpr size_t getPositionAftter(size_t pos) noexcept
	{
		return ++pos == ringBufferSize ? 0 : pos;
	}
	static constexpr size_t getPosition(size_t pos) noexcept
	{
		return pos >= ringBufferSize ? pos - ringBufferSize : pos;
	}

	static constexpr size_t ringBufferSize = _size + 1;
	std::array<std::atomic<T>, ringBufferSize> ringBuffer;
	std::atomic<size_t> readPos = { 0 }, writePos = { 0 };
	std::atomic<size_t> size = 0;
	std::recursive_mutex readMutex;
	std::recursive_mutex writeMutex;
};

template<typename T, size_t _size>
inline bool LockFreeQueue<T, _size>::push(const T& newElement)
{	
	auto beforeSize = size.fetch_add(1);
	if (beforeSize >= ringBufferSize)
	{
		size.fetch_sub(1);
		return false;
	}
	auto beforePos = writePos.fetch_add(1);
	if (beforePos >= ringBufferSize)
	{
		{
			//buffer 오버플로가 나면 잠깐 락걸고 buffer size만큼 뺀다.
			std::lock_guard<std::recursive_mutex> m(writeMutex);
			auto current = writePos.load();
			if (current >= ringBufferSize)
			{
				writePos.fetch_sub(ringBufferSize);
			}
		}
		beforePos = getPosition(beforePos);
	}
	ringBuffer[beforePos].store(newElement);	
	return true;
}

template<typename T, size_t _size>
inline bool LockFreeQueue<T, _size>::pop(T& returnedElement)
{
	auto beforeSize = size.fetch_sub(1);
	if (beforeSize <= 0)
	{
		size.fetch_add(1);
		return false;
	}

	auto beforePos = readPos.fetch_add(1);
	if (beforePos >= ringBufferSize)
	{
		{
			//buffer 오버플로가 나면 잠깐 락걸고 buffer size만큼 뺀다.
			std::lock_guard<std::recursive_mutex> m(readMutex);
			auto current = readPos.load();
			if (current >= ringBufferSize)
			{
				readPos.fetch_sub(ringBufferSize);
			}
		}
		beforePos = getPosition(beforePos);
	}
	returnedElement = ringBuffer[beforePos].load();
	return true;
}
