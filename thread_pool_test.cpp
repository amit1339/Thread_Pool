#include <unistd.h>
#include "thread_pool.hpp"

size_t counter1 = 0;
size_t counter2 = 0;

std::function<void(void)> test_function1 = []()
{
    int i = 20;
    while (i)
    {
        std::cout << "counter 1" << counter1 << std::endl;
        counter1++;
        sleep(1);
        i--;
    }
};

std::function<void(void)> test_function2 = []()
{
    int i = 20;
    while (i)
    {
        std::cout << "counter 2" << counter2 << std::endl;
        counter2++;
        sleep(1);

        i--;
    }
};

std::function<size_t(void)> test_future_function1 = []()
{
    return 999;
};

int main()
{
    ilrd::ThreadPool thread_pool(2);
  
    ilrd::ThreadPool::FunctionTask *test_function_task1 = new ilrd::ThreadPool::FunctionTask(test_function1);
    std::shared_ptr<ilrd::ThreadPool::FunctionTask> test_function_sp1(test_function_task1);
    thread_pool.Add(test_function_sp1, ilrd::ThreadPool::Priority::MEDIUM);

    sleep(3);

    thread_pool.Pause();
    thread_pool.Pause();
    std::cout << "Pause" << std::endl;

    ilrd::ThreadPool::FunctionTask *test_function_task2 = new ilrd::ThreadPool::FunctionTask(test_function2);
    std::shared_ptr<ilrd::ThreadPool::FunctionTask> test_function_sp2(test_function_task2);
    thread_pool.Add(test_function_sp2, ilrd::ThreadPool::Priority::MEDIUM);

    sleep(3);

    thread_pool.Resume();
    thread_pool.Resume();
    std::cout << "Resume" << std::endl;

    thread_pool.SetNumOfThreads(3);

    sleep(3);

    ilrd::ThreadPool::FutureTask<size_t> *test_future_function_task1 = new ilrd::ThreadPool::FutureTask<size_t>(test_future_function1);
    std::shared_ptr<ilrd::ThreadPool::FutureTask<size_t>> test_future_function_sp1(test_future_function_task1);
    thread_pool.Add(test_future_function_sp1, ilrd::ThreadPool::Priority::MEDIUM);

    std::cout << "first Get(): " << test_future_function_sp1->Get() << std::endl;
    std::cout << "second Get(): " << test_future_function_sp1->Get() << std::endl; 
  
    thread_pool.SetNumOfThreads(20);
    thread_pool.SetNumOfThreads(10);
    thread_pool.SetNumOfThreads(20);
    thread_pool.SetNumOfThreads(10);
    thread_pool.SetNumOfThreads(20);
    thread_pool.SetNumOfThreads(0);
    thread_pool.SetNumOfThreads(5);
    thread_pool.SetNumOfThreads(4);
    thread_pool.SetNumOfThreads(8);

    return 0;
}
