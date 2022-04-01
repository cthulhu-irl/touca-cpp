// Copyright 2022 Touca, Inc. Subject to Apache-2.0 License.

#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#include "touca/lib_api.hpp"

namespace touca {
namespace detail {

// doesn't support copy/move of T with other types of Allocator...
template <typename T>
class deep_copy_ptr {
 public:
  using value_type = typename std::remove_reference<
      typename std::remove_pointer<T>::type>::type;

  using pointer = typename std::add_pointer<value_type>::type;

  template <typename... Args,
            typename std::enable_if<
                (!std::is_array<T>::value || std::extent<T>::value != 0) &&
                    std::is_constructible<T, Args...>::value,
                bool>::type = true>
  deep_copy_ptr(Args&&... args)
      : _ptr(new value_type(std::forward<Args>(args)...)) {}

  template <typename T2 = T,
            typename std::enable_if<std::is_array<T2>::value &&
                                        std::extent<T2>::value == 0,
                                    bool>::type = true>
  deep_copy_ptr(const std::size_t size)
      : _ptr(new typename std::remove_extent<T>::type[size]) {}

  deep_copy_ptr(const deep_copy_ptr& other) : _ptr(new value_type(*other)) {}

  deep_copy_ptr(deep_copy_ptr&& other) noexcept = default;

  deep_copy_ptr& operator=(const deep_copy_ptr& other) noexcept(
      std::is_nothrow_copy_assignable<value_type>::value) {
    *_ptr = *other._ptr;
    return *this;
  }

  deep_copy_ptr& operator=(deep_copy_ptr&& other) noexcept = default;

  ~deep_copy_ptr() = default;

  void swap(deep_copy_ptr& other) noexcept(noexcept(std::swap(_ptr,
                                                              other._ptr))) {
    std::swap(_ptr, other._ptr);
  }

  friend bool operator==(const deep_copy_ptr<T>& lhs, const T& rhs) noexcept {
    return *lhs == rhs;
  }

  friend bool operator==(const T& lhs, const deep_copy_ptr<T>& rhs) noexcept {
    return lhs == *rhs;
  }

  friend bool operator==(const deep_copy_ptr<T>& lhs,
                         const deep_copy_ptr<T>& rhs) noexcept {
    return std::addressof(*lhs) == std::addressof(*rhs) || *lhs == *rhs;
  }

  friend bool operator!=(const deep_copy_ptr<T>& lhs, const T& rhs) noexcept {
    return *lhs != rhs;
  }

  friend bool operator!=(const T& lhs, const deep_copy_ptr<T>& rhs) noexcept {
    return lhs != *rhs;
  }

  friend bool operator!=(const deep_copy_ptr<T>& lhs,
                         const deep_copy_ptr<T>& rhs) noexcept {
    return std::addressof(*lhs) != std::addressof(*rhs) || *lhs != *rhs;
  }

  value_type& operator[](const std::size_t index) & noexcept {
    return _ptr[index];
  }

  const value_type& operator[](const std::size_t index) const& noexcept {
    return _ptr[index];
  }

  value_type&& operator[](const std::size_t index) && noexcept {
    return _ptr[index];
  }

  const value_type&& operator[](const std::size_t index) const&& noexcept {
    return _ptr[index];
  }

  value_type& operator*() & noexcept { return *_ptr; }
  const value_type& operator*() const& noexcept { return *_ptr; }
  value_type&& operator*() && noexcept { return *_ptr; }
  const value_type&& operator*() const&& noexcept { return *_ptr; }

  pointer operator->() noexcept { return _ptr.get(); }
  const pointer operator->() const noexcept { return _ptr.get(); }

  operator pointer() const
      noexcept(std::is_nothrow_copy_constructible<value_type>::value) {
    return _ptr.get();
  }

 private:
  std::unique_ptr<value_type> _ptr;
};

}  // namespace detail
}  // namespace touca
