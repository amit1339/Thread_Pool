#ifndef __UTILS__
#define __UTILS__


class noncopyable {
protected:
  constexpr noncopyable() = default;
  ~noncopyable() = default;
  noncopyable(const noncopyable &) = delete;
  noncopyable & operator=(const noncopyable &) = delete;
};
    
#endif //_UTILS__