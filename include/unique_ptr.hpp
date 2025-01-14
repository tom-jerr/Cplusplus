#pragma once
#include <memory>

#include <memory>
#include <pthread.h>

template <class T>
class UniquePtr {
private:
  T* ptr_;

public:
  /**
   * @brief Construct a new Unique Ptr object
   * 
   */
  UniquePtr();
  /**
   * @brief Construct a new Unique Ptr object
   * 
   * @param t 
   */
  UniquePtr(T* t);
  /**
   * @brief Construct a new Unique Ptr object
   * 
   * @param ptr 
   */
  UniquePtr(std::unique_ptr<T> ptr);
  /**
   * @brief Destroy the Unique Ptr object
   * 
   */
  ~UniquePtr();
  /**
   * @brief delete copy constructor and copy assignment
   * 
   */
  UniquePtr(UniquePtr&) = delete;
  UniquePtr& operator=(UniquePtr&) = delete;
  /**
   * @brief Move Construct 
   * 
   * @param other 
   */
  UniquePtr(UniquePtr&& other);
  /**
   * @brief Move assignment
   * 
   * @param other 
   * @return UniquePtr& 
   */
  UniquePtr& operator=(UniquePtr&& other);
  /**
   * @brief get the raw pointer from UniquePtr
   * 
   * @return T*
   */
  T* get();
  /**
   * @brief get the raw data from raw pointer T*
   * 
   * @param unique_ptr 
   * @return T 
   */
  T operator*();
  /**
   * @brief get this raw pointer
   * 
   * @return T* 
   */
  T* operator->();
  /**
   * @brief covert unique ptr to bool
   * 
   * @return true 
   * @return false 
   */
  operator bool();
  /**
   * @brief get the raw pointer size 
   * 
   * @return size_t 
   */
  size_t length();
  /**
   * @brief return raw pointer
   * 
   * @return T* 
   */
  T* release();
  /**
  * @brief have a input and make a new pointer with it after deleting the old pointer:
  * 
  * @param new_ptr 
  */
  void reset(T* new_ptr = nullptr);



};


template<class T>
T* make_unique(T t);
template<class T>
UniquePtr<T>::UniquePtr():ptr_(nullptr){}

template<class T>
UniquePtr<T>::UniquePtr(T* t):ptr_(t) {

} 

template<class T>
UniquePtr<T>::UniquePtr(std::unique_ptr<T> other):ptr_(other.get()) {
  other.reset(nullptr);
}

template<class T>
UniquePtr<T>::~UniquePtr() {
  if(ptr_) {
    delete ptr_;
  }
}

template<class T>
UniquePtr<T>::UniquePtr(UniquePtr && other) {
  ptr_ = other.get();
  other.ptr_ = nullptr;
}

template<class T>
UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr && other) {
  ptr_ = other.get();
  other.ptr_ = nullptr;
}

template<class T>
T* UniquePtr<T>::get() {
  return ptr_;
}

template<class T>
T UniquePtr<T>::operator*() {
  if(ptr_ != nullptr) {
    return *ptr_;
  }
  // TODO(LZY): 这个要分为平凡类型和非平凡类型
  return T();
}

template<class T>
T* UniquePtr<T>::operator->() {
  return ptr_ != nullptr ? (ptr_) : nullptr;
}

template<class T>
UniquePtr<T>::operator bool() {
  return ptr_ != nullptr;
}
template<class T>
void UniquePtr<T>::reset(T* new_ptr) {
  if(ptr_) {
    delete ptr_;
  }
  ptr_ = new_ptr;
}

template<class T>
T* UniquePtr<T>::release() {
  T* tmp = ptr_;
  ptr_ = nullptr;
  return tmp;
}

template<class T>
T* make_unique(T t) {
  T* ptr_t = new T(t);
  
  return ptr_t;
}








