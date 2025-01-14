#pragma once
#include <cstring>
// #include <iostream>
template<class T>
class SharedPtr{
private:
  T* raw_ptr_;
  size_t* count_;
public:
  SharedPtr();
  SharedPtr(T* ptr);
  ~SharedPtr();

  SharedPtr(SharedPtr& other);
  SharedPtr& operator=(SharedPtr&);

  size_t use_count();

/**
   * @brief get the raw pointer from SharedPtr
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
  * @brief have a input and make a new pointer with it after deleting the old pointer:
  * 
  * @param new_ptr 
  */
  void reset(T* new_ptr = nullptr);
  
};

template<class T>
T* make_shared(T t) {
  return new T(t);
}

template<class T>
SharedPtr<T>::SharedPtr():raw_ptr_(nullptr), count_(new size_t(0)){}

template<class T>
SharedPtr<T>::SharedPtr(T* ptr):raw_ptr_(ptr), count_(new size_t(1)){}

template<class T>
SharedPtr<T>::~SharedPtr() {
  if(raw_ptr_ && --(*count_) <= 0) {
    delete raw_ptr_;
    delete count_;
    raw_ptr_ = nullptr;
    count_ = nullptr;
  }
}

template <class T>
SharedPtr<T>::SharedPtr(SharedPtr<T>& other) {
  raw_ptr_ = other.raw_ptr_;
  count_ = other.count_;
  *(other.count_) += 1; 
}

template<class T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr& other) {
  if(this == &other) {
    return *this;
  }
  raw_ptr_ = other.raw_ptr_;
  count_ = other.count_;
  *(other.count_) += 1; 
  return *this;
}

template<class T>
size_t SharedPtr<T>::use_count() {
  return count_ != nullptr ? *count_ : 0;
}

template<class T>
T* SharedPtr<T>::get() {
  return raw_ptr_;
}

template<class T>
T SharedPtr<T>::operator*() {
  if(raw_ptr_ != nullptr) {
    return *raw_ptr_;
  }
  // TODO(LZY): 这个要分为平凡类型和非平凡类型
  return T();
}

template<class T>
T* SharedPtr<T>::operator->() {
  return raw_ptr_ != nullptr ? (raw_ptr_) : nullptr;
}

template<class T>
SharedPtr<T>::operator bool() {
  return raw_ptr_ != nullptr;
}
template<class T>
void SharedPtr<T>::reset(T* new_ptr) {
  if(raw_ptr_) {
    delete raw_ptr_;
  }
  raw_ptr_ = new_ptr;
  if(new_ptr == nullptr) {
    *count_ = 0;
  }
}









