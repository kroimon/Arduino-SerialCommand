#ifndef _TRANSIENT_FUNCTION_
#define _TRANSIENT_FUNCTION_
// from http://brnz.org/hbr/?p=1767

#include <type_traits>

template<typename>
struct TransientFunction; // intentionally not defined

template<typename R, typename ...Args>
struct TransientFunction<R(Args...)>
{
  using Dispatcher = R(*)(void*, Args...);
  
  Dispatcher m_Dispatcher; // A pointer to the static function that will call the 
                           // wrapped invokable object
  void* m_Target;          // A pointer to the invokable object

  // Dispatch() is instantiated by the TransientFunction constructor,
  // which will store a pointer to the function in m_Dispatcher.
  template<typename S>
  static R Dispatch(void* target, Args... args)
  {
    return (*(S*)target)(args...);
  }
  
  template<typename T>
  TransientFunction(T&& target)
    : m_Dispatcher(&Dispatch<typename std::decay<T>::type>)
    , m_Target(&target)
  {
  }

  // Specialize for reference-to-function, to ensure that a valid pointer is 
  // stored.
  using TargetFunctionRef = R(Args...);
  TransientFunction(TargetFunctionRef target)
    : m_Dispatcher(Dispatch<TargetFunctionRef>)
  {
    static_assert(sizeof(void*) == sizeof target, 
    "It will not be possible to pass functions by reference on this platform. "
    "Please use explicit function pointers i.e. foo(target) -> foo(&target)");
    m_Target = (void*)target;
  }

  void operator=(const TransientFunction &lhs)
  {
    m_Dispatcher = lhs.m_Dispatcher;
    m_Target = lhs.m_Target;
  }

  R operator()(Args... args) const
  {
    Serial.println(m_Dispatcher == nullptr ? "Dispatcher is null" : "Dispatcher is not null");
    Serial.println(m_Target == nullptr ? "Target is null" : "Target is not null");
    return m_Dispatcher(m_Target, args...);
  }
};

#endif