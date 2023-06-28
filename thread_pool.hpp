#ifndef __THREAD_POOL__
#define __THREAD_POOL__

// Standard library headers
#include <thread>                   //std::thread
#include <functional>               // std::function
#include <condition_variable>       // std::condition_variable
#include <vector>                   // std::vector
#include <memory>                   // std::shared_ptr
#include <map>                      // std::map
#include <iostream>                 // for tests

// ILRD Headers
#include "priority_queue.hpp"       // priority_queue
#include "waitable_queue.hpp"       // waitable queue
#include "utils.hpp"                // noncopyable

class ThreadPool : private noncopyable 
{
public:
    class Task
    {
    public:
        virtual void Run() = 0;
        virtual ~Task() {}
    };

    class FunctionTask : public Task
    {
    public:
        FunctionTask(std::function<void(void)>& function);
        virtual void Run();
    private:
        std::function<void(void)> m_function;   
    };

    template <typename T>
    class FutureTask : public Task
    {
    public:
        FutureTask(std::function<T (void)>& function);
        virtual void Run();
        T Get() const;
    private:
        T m_future_obj;
        std::function<T (void)> m_function;
        mutable std::condition_variable future_task_cv;
        mutable std::mutex future_task_mutex;
        bool future_task_ready;
    };

    typedef struct TaskWithCounter
    {
        std::shared_ptr<Task> m_task;
        int m_priority;
        std::size_t m_counter;

        TaskWithCounter(std::shared_ptr<Task> task, int priority, std::size_t counter);
        TaskWithCounter() {}
    } task_t;

    struct Compare 
    {
        bool operator()(const task_t& lhs, const task_t& rhs) const;
    };

    ThreadPool(std::size_t initialNumThreads);
    ~ThreadPool();

    enum Priority
    {
        LOW = 1,
        MEDIUM = 2,
        HIGH = 3,
        PAUSE = 4
    };

    void Add(std::shared_ptr<Task> task, Priority priority);
    void Pause();
    void Resume();
    void SetNumOfThreads(std::size_t numThreads);


private:      
    WaitablePriorityQueue<PriorityQueue<task_t, std::vector<task_t>, Compare>> m_queue;
    std::map<std::thread::id, std::thread> m_threads_map;
    std::condition_variable m_dtor_cv;
    std::mutex m_dtor_mutex;
    std::condition_variable m_pause_task_cv;
    std::mutex m_pause_task_mutex;
    WaitablePriorityQueue<PriorityQueue<std::thread::id>> m_detach_threads;
    std::map<std::thread::id, bool> m_map;
    std::condition_variable m_cond_var;
    std::mutex m_mutex;
    void ThreadHandler();
    std::function<void(void)> m_end_thread;
    std::shared_ptr<FunctionTask> m_task_end_thread_shrd_ptr;
    bool m_is_pause;
    bool m_shouldWait;
    std::size_t m_counter;
    class PauseTask : public Task
    {
    public:
        PauseTask(std::condition_variable& cv, std::mutex& mutex, bool& shouldWait);
        void Run();
    private:
        std::condition_variable& m_cv;
        std::mutex& m_mutex;
        bool& m_shouldWait;
    };
};


template <typename T>
ThreadPool::FutureTask<T>::FutureTask(std::function<T (void)>& function)
    : m_function(function), future_task_ready(false)
{}

template <typename T>
void ThreadPool::FutureTask<T>::Run()
{
    m_future_obj = m_function();
    
    std::lock_guard<std::mutex> lock(future_task_mutex);
    future_task_ready = true;
    future_task_cv.notify_one();
}

template <typename T>
T ThreadPool::FutureTask<T>::Get() const
{
    std::unique_lock<std::mutex> lock(future_task_mutex);
    future_task_cv.wait(lock, [this] { return future_task_ready; });
    return m_future_obj;
}



#endif //__THREAD_POOL__
