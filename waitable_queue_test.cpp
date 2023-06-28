#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <queue>

#include "waitable_queue.hpp"
#include "priority_queue.hpp"

WaitablePriorityQueue<PriorityQueue<int>> queue;

void TestWithoutTimeout(void);
void TestTimeout(void);
void *Consumer(void);
void *Producer(void);

void *Consumer(void)
{
  for (size_t i = 0; i < 10000; ++i)
  {
    std::cout << queue.Dequeue() << std::endl;
  }
  return NULL;
}

void *ConsumerTimeout(void)
{
  int val = -1;
  for (size_t i = 0; i < 1000; ++i)
  {
  if (queue.Dequeue(std::chrono::milliseconds(1), val))
    {
      std::cout << "Succeded when should fail" << std::endl;
    }
    if (-1 != val)
    {
      std::cout << "Expected -1 but got " << val << std::endl;
    }
  }
  return NULL;
}

void *Producer(void)
{
  for (size_t i = 0; i < 10000; ++i)
  {
    queue.Enqueue(i);
  }
  return NULL;
}

void TestWithoutTimeout(void)
{
  const size_t num_consumers = 5;
  const size_t num_producers = 5;

  std::thread threads_arr[num_consumers + num_producers];

  for (size_t i = 0; i < num_producers + num_consumers; ++i)
  {
    if (i % 2)
    {
      threads_arr[i] = std::thread(Consumer);
    }
    else
    {
      threads_arr[i] = std::thread(Producer);
    }
  }
  for(size_t i = 0; i < num_consumers + num_producers; ++i)
  {
    threads_arr[i].join();
  }
}

void TestTimeout(void)
{
  const size_t num_consumers = 5;
  const size_t num_producers = 5;

  std::thread threads_arr[num_consumers + num_producers];

  for (size_t i = 0; i < num_producers + num_consumers; ++i)
  {
    threads_arr[i] = std::thread(ConsumerTimeout);
  }
  for(size_t i = 0; i < num_consumers + num_producers; ++i)
  {
    threads_arr[i].join();
  }
}

int main()
{
  TestWithoutTimeout();
  TestTimeout();
  std::cout << "Is empty" << queue.IsEmpty() << std::endl;
  queue.Enqueue(5);
  std::cout << "Is empty" << queue.IsEmpty() << std::endl;
  return 0;
}
