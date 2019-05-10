#include "LockFreeQueue.h"
#include <thread>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <iostream>
#include <concurrent_queue.h>
#include <queue>
#include <mutex>

int main()
{
	constexpr size_t bufferSize = 10000;
	constexpr size_t threadSize = 8;
	LockFreeQueue<int, bufferSize> lockFreeQueueTest;
	HANDLE hEvent;
	hEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	std::chrono::time_point<std::chrono::system_clock> now;
	
	//Ǫ�� �׽�Ʈ
	std::cout << "Push �ӵ� �׽�Ʈ " << std::endl;
	std::cout << "���� ������(���� ��) : " << bufferSize << std::endl;
	std::cout << "������ �� : " << threadSize << std::endl << std::endl;

	for (int j = 0; j < threadSize; ++j)
	{
		std::vector<std::thread> threads;
		for (int i = 0; i < threadSize; ++i)
		{
			threads.emplace_back(
				[&] {
					WaitForSingleObject(hEvent, INFINITE);
					for (int i = 0; i < bufferSize / threadSize; ++i)
					{
						lockFreeQueueTest.push(i);
					}
				});
		}
		now = std::chrono::system_clock::now();
		SetEvent(hEvent);
		for (auto& thread : threads)
		{
			thread.join();
		}
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - now);
		std::cout << "���� ���� ������ Ǫ�� �Ҹ�ð� : " << duration.count() << std::endl;
		
		ResetEvent(hEvent);
		threads.clear();
		for (int i = 0; i < threadSize; ++i)
		{
			threads.emplace_back(
				[&] {
					WaitForSingleObject(hEvent, INFINITE);
					int temp;
					for (int i = 0; i < bufferSize / threadSize; ++i)
					{
						while(lockFreeQueueTest.pop(temp)==false);
					}
				});
		}
		now = std::chrono::system_clock::now();
		SetEvent(hEvent);
		for (auto& thread : threads)
		{
			thread.join();
		}
		duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - now);
		std::cout << "���� ���� ������ �� �Ҹ�ð� : " << duration.count() << std::endl << std::endl;
	}

	for (int j = 0; j < threadSize; ++j)
	{
		concurrency::concurrent_queue<int> t;
		std::vector<std::thread> threads;
		for (int i = 0; i < threadSize; ++i)
		{
			threads.emplace_back(
				[&] {
					WaitForSingleObject(hEvent, INFINITE);
					for (int i = 0; i < bufferSize / threadSize; ++i)
					{
						t.push(i);
					}
				});
		}
		now = std::chrono::system_clock::now();
		SetEvent(hEvent);
		for (auto& thread : threads)
		{
			thread.join();
		}
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - now);
		std::cout <<"��Ŀ���� Ǫ�� �Ҹ�ð�: "<< duration.count() << std::endl;

		ResetEvent(hEvent);
		threads.clear();
		for (int i = 0; i < threadSize; ++i)
		{
			threads.emplace_back(
				[&] {
					WaitForSingleObject(hEvent, INFINITE);
					int temp;
					for (int i = 0; i < bufferSize / threadSize; ++i)
					{
						while (t.try_pop(temp) == false);
					}
				});
		}
		now = std::chrono::system_clock::now();
		SetEvent(hEvent);
		for (auto& thread : threads)
		{
			thread.join();
		}
		duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - now);
		std::cout << "��Ŀ���� �� �Ҹ�ð� : " << duration.count() << std::endl << std::endl;
	}

	for (int j = 0; j < threadSize; ++j)
	{
		std::queue<int> t;
		std::recursive_mutex lock;
		std::vector<std::thread> threads;
		for (int i = 0; i < threadSize; ++i)
		{
			threads.emplace_back(
				[&] {
					WaitForSingleObject(hEvent, INFINITE);
					for (int i = 0; i < bufferSize / threadSize; ++i)
					{
						std::lock_guard<std::recursive_mutex> m(lock);
						t.push(i);
					}
				});
		}
		now = std::chrono::system_clock::now();
		SetEvent(hEvent);
		for (auto& thread : threads)
		{
			thread.join();
		}
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - now);
		std::cout << "�� ť Ǫ�� �Ҹ�ð�: " << duration.count() << std::endl;

		ResetEvent(hEvent);
		threads.clear();
		for (int i = 0; i < threadSize; ++i)
		{
			threads.emplace_back(
				[&] {
					WaitForSingleObject(hEvent, INFINITE);
					int temp;
					for (int i = 0; i < bufferSize / threadSize; ++i)
					{
						std::lock_guard<std::recursive_mutex> m(lock);
						temp = t.front();
						t.pop();
					}
				});
		}
		now = std::chrono::system_clock::now();
		SetEvent(hEvent);
		for (auto& thread : threads)
		{
			thread.join();
		}
		duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - now);
		std::cout << "�� ť �� �Ҹ�ð� : " << duration.count() << std::endl << std::endl;

	}
	system("pause");
}