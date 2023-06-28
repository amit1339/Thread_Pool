#ifndef __PRIORITY_QUEUE__
#define __PRIORITY_QUEUE__

#include <vector> //vector
#include <functional> // less
#include <queue> // priority_queue


template <typename T,
          typename CONTAINER = std::vector<T>,
          typename COMPARE_FUNC = std::less<typename CONTAINER::value_type>>
class PriorityQueue : private std::priority_queue<T, CONTAINER, COMPARE_FUNC>
{
public:
  using std::priority_queue<T, CONTAINER, COMPARE_FUNC>::empty; // OK
  using std::priority_queue<T, CONTAINER, COMPARE_FUNC>::push; // OK
  using std::priority_queue<T, CONTAINER, COMPARE_FUNC>::pop; // OK
  using std::priority_queue<T, CONTAINER, COMPARE_FUNC>::size; // OK
  using typename std::priority_queue<T, CONTAINER, COMPARE_FUNC>::value_type;
  using typename std::priority_queue<T, CONTAINER, COMPARE_FUNC>::reference;
  
  const T& front() const;
};


template <typename T,
          typename CONTAINER,
          typename COMPARE_FUNC>
const T& PriorityQueue<T, CONTAINER, COMPARE_FUNC>::front() const
{
  return this->top();
}

#endif //__PRIORITY_QUEUE__