#ifndef __WAITABLE_QUEUE__
#define __WAITABLE_QUEUE__

#include <mutex>
#include <condition_variable>
#include <chrono>
#include "utils.hpp"



// ***************** WaitablePriorityQueue Class Declaration ***************** //
template <class Container>
class WaitablePriorityQueue : private noncopyable
{
public:
    WaitablePriorityQueue();
    void Enqueue(typename Container::value_type value);
    typename Container::value_type Dequeue();
    bool Dequeue(std::chrono::milliseconds timeout, typename Container::reference value);
    bool IsEmpty() const;
    size_t Size();

private:
    Container m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
};

// ***************** WaitablePriorityQueue Class Methods Definitions ***************** //

template <class Container>
WaitablePriorityQueue<Container>::WaitablePriorityQueue()
    : m_queue(), m_mutex(), m_conditionVariable()
{}

template <class Container>
void WaitablePriorityQueue<Container>::Enqueue(typename Container::value_type value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(value);
    m_conditionVariable.notify_all();
}

template <class Container>
typename Container::value_type WaitablePriorityQueue<Container>::Dequeue()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_conditionVariable.wait(lock, [this]() { return !m_queue.empty(); });

    typename Container::value_type value = m_queue.front();
    m_queue.pop();

    return value;
}

template <class Container>
bool WaitablePriorityQueue<Container>::Dequeue(std::chrono::milliseconds timeout, typename Container::reference value)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_conditionVariable.wait_for(lock, timeout, [this]() { return !m_queue.empty(); }))
    {
        return false;
    }

    value = m_queue.front();
    m_queue.pop();

    return true;
}

template <class Container>
bool WaitablePriorityQueue<Container>::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template <class Container>
size_t WaitablePriorityQueue<Container>::Size()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}



#endif // __WAITABLE_QUEUE__
