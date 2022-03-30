// Copyright 2022 Touca, Inc. Subject to Apache-2.0 License.

#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#if defined(TOUCA_HAS_CPP17) || defined(TUCA_HAS_CPP20)
#include <variant>
#else
#include "mpark/variant.hpp"
#endif

#include "touca/lib_api.hpp"

namespace touca {
namespace internal {

#if defined(TOUCA_HAS_CPP17) || defined(TUCA_HAS_CPP20)

using std::get;
using std::holds_alternative;
using std::monostate;
using std::variant;
using std::visit;

#else  // vvvv c++11/14 || c++17/20 ^^^

using mpark::get;
using mpark::holds_alternative;
using mpark::monostate;
using mpark::variant;
using mpark::visit;

#endif  // different variant implementations for different standards

template <typename T, typename U>
T exchange(T& rhs,
           U&& lhs) noexcept(std::is_nothrow_move_assignable<T>::value&&
                                 std::is_nothrow_move_assignable<T>::value) {
  auto tmp = std::move(rhs);
  rhs = std::forward<U>(lhs);
  return tmp;
}

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
      : ptr_(new value_type(std::forward<Args>(args)...)) {}

  template <typename T2 = T,
            typename std::enable_if<std::is_array<T2>::value &&
                                        std::extent<T2>::value == 0,
                                    bool>::type = true>
  deep_copy_ptr(const std::size_t size)
      : ptr_(new typename std::remove_extent<T>::type[size]) {}

  deep_copy_ptr(const deep_copy_ptr& other) : ptr_(new value_type(*other)) {}

  deep_copy_ptr(deep_copy_ptr&& other) noexcept = default;

  deep_copy_ptr& operator=(const deep_copy_ptr& other) noexcept(
      std::is_nothrow_copy_assignable<value_type>::value) {
    *ptr_ = *other.ptr_;
    return *this;
  }

  deep_copy_ptr& operator=(deep_copy_ptr&& other) noexcept = default;

  ~deep_copy_ptr() = default;

  void swap(deep_copy_ptr& other) noexcept(noexcept(std::swap(ptr_,
                                                              other.ptr_))) {
    std::swap(ptr_, other.ptr_);
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
    return ptr_[index];
  }

  const value_type& operator[](const std::size_t index) const& noexcept {
    return ptr_[index];
  }

  value_type&& operator[](const std::size_t index) && noexcept {
    return ptr_[index];
  }

  const value_type&& operator[](const std::size_t index) const&& noexcept {
    return ptr_[index];
  }

  value_type& operator*() & noexcept { return *ptr_; }
  const value_type& operator*() const& noexcept { return *ptr_; }
  value_type&& operator*() && noexcept { return *ptr_; }
  const value_type&& operator*() const&& noexcept { return *ptr_; }

  pointer operator->() noexcept { return ptr_.get(); }
  const pointer operator->() const noexcept { return ptr_.get(); }

  operator pointer() const
      noexcept(std::is_nothrow_copy_constructible<value_type>::value) {
    return ptr_.get();
  }

 private:
  std::unique_ptr<value_type> ptr_;
};

}  // namespace internal
}  // namespace touca
