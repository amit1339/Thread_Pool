#include "thread_pool.hpp"

    // ***************** ThreadPool Class Definitions ***************** //
    ThreadPool::ThreadPool(std::size_t initialNumThreads)
        : m_queue(),
        m_threads_map(),
        m_dtor_cv(),
        m_dtor_mutex(),
        m_pause_task_cv(),
        m_pause_task_mutex(),
        m_detach_threads(),
        m_map(),
        m_cond_var(),
        m_mutex(),
        m_end_thread([&]() 
        {
            std::unique_lock<std::mutex> unique_lock_mutex(m_mutex);
            m_map[std::this_thread::get_id()] = false;
        }
            ),
        m_task_end_thread_shrd_ptr(new FunctionTask(m_end_thread)),
        m_is_pause(false),
        m_counter(0)
    {
        for (size_t i = 0; i < initialNumThreads; ++i)
        {
            std::thread new_thread(&ThreadPool::ThreadHandler, std::ref(*this));
            m_threads_map[new_thread.get_id()] = std::move(new_thread);
        }
    }


    ThreadPool::~ThreadPool()
    {
        SetNumOfThreads(0);
    }


    void ThreadPool::Add(std::shared_ptr<Task> task, Priority priority)
    {
        m_queue.Enqueue(task_t(task, priority, ++m_counter));
    }


    void ThreadPool::Pause()
    {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_is_pause)
    {
        m_is_pause = true;
        m_shouldWait = true; // Set the flag to true
        size_t curr_num_of_threads = m_threads_map.size();
        for (size_t i = 0; i < curr_num_of_threads; ++i)
        {
            std::shared_ptr<Task> pause_task_shrd_ptr(new PauseTask(m_pause_task_cv, m_pause_task_mutex, m_shouldWait));
            m_queue.Enqueue(task_t(pause_task_shrd_ptr, Priority::PAUSE, ++m_counter));
        }
    }
    }



    void ThreadPool::Resume()
    {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_is_pause)
    {
        m_is_pause = false;
        m_shouldWait = false; // Set the flag to false
        m_pause_task_cv.notify_all(); // Notify all threads
    }
    }



    void ThreadPool::ThreadHandler()
    {
        std::thread::id this_thread_id = std::this_thread::get_id();

        {   // scope for RAII of unique_lock
            std::unique_lock<std::mutex> unique_lock_mutex(m_mutex);
            m_map[this_thread_id] = true;
        }

        while (m_map[this_thread_id])
        {
            task_t task = m_queue.Dequeue();
            task.m_task->Run();
        }

        std::unique_lock<std::mutex> unique_lock_mutex(m_mutex);
        m_detach_threads.Enqueue(this_thread_id);

        m_dtor_cv.notify_all();
    }


    void ThreadPool::SetNumOfThreads(std::size_t numThreads)
    {
        int threads_diff = 0;
        {   // scope for RAII of unique_lock
            std::unique_lock<std::mutex> unique_lock_mutex(m_mutex);
            threads_diff = numThreads - m_threads_map.size();
        }

        if (0 < threads_diff)
        {
            // add threads
            for (int i = 0; i < threads_diff; ++i)
            {
                std::thread new_thread(&ThreadPool::ThreadHandler, std::ref(*this));
                m_threads_map[new_thread.get_id()] = std::move(new_thread);
            }
        }
        else if (0 > threads_diff)
        {
            // add tasks of thread removing
            for (int i = 0; i < -threads_diff; ++i)
            {
                Add(m_task_end_thread_shrd_ptr, static_cast<Priority>(0));
            }

            {
                std::unique_lock<std::mutex> lock(m_dtor_mutex);
                m_dtor_cv.wait(lock, [&]() { return m_detach_threads.Size() == static_cast<std::size_t>(-threads_diff); });
            }

            for (int i = 0; i < -threads_diff; ++i)
            {
                std::thread::id thread_id_to_remove = m_detach_threads.Dequeue();
                std::thread thread_to_remove = std::move(m_threads_map[thread_id_to_remove]);
                m_threads_map.erase(thread_id_to_remove);
                thread_to_remove.join();
            }
        }

        // in situation that needs to delete threads it will enter the loop
        // and for m_task_end_thread_shrd_ptr will invoke the m_end_thread,
        // and all of the threads will be added to the m_detach_threads.
    }


    // ***************** FunctionTask Class Definitions ***************** //
    ThreadPool::FunctionTask::FunctionTask(std::function<void(void)>& function)
        : m_function(function)
    {}

    void ThreadPool::FunctionTask::Run()
    {
        m_function();
    }


    // ***************** PauseTask Class Definitions ***************** //
    ThreadPool::PauseTask::PauseTask(std::condition_variable& cv, std::mutex& mutex, bool& shouldWait)
        : m_cv(cv), m_mutex(mutex), m_shouldWait(shouldWait)
    {}

    void ThreadPool::PauseTask::Run()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_shouldWait)
        {
            m_cv.wait(lock);
        }
        
    }


    // ***************** TaskWithCounter Struct Definitions ***************** //
    ThreadPool::TaskWithCounter::TaskWithCounter(std::shared_ptr<Task> task, int priority, std::size_t counter)
        : m_task(task), m_priority(priority), m_counter(counter)
    {}


    // ***************** Nested Compare Functor ***************** //
    bool ThreadPool::Compare::operator()(const task_t& lhs, const task_t& rhs) const
    {
        if (lhs.m_priority == rhs.m_priority)
        {
            return lhs.m_counter > rhs.m_counter;
        }
        else
        {
            return lhs.m_priority < rhs.m_priority;
        }
    }
