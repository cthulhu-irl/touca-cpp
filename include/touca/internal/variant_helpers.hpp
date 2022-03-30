// Copyright 2022 Touca, Inc. Subject to Apache-2.0 License.

#pragma once

#include <type_traits>
#include <utility>

namespace touca {
namespace internal {

/** a type that combines multiple callable types into one overloaded of them.
 *
 * it's an implementation of famous overloaded but in c++11:
 *  https://www.cppstories.com/2019/02/2lines3featuresoverload.html/
 *
 * it comes very handy and useful if you want to combine lambdas
 * to visit a variant.
 */
template <typename ...Fs>
struct overloaded {
  static_assert(sizeof...(Fs) > 0, "must provide at least one callable type.");
};

template <typename F>
struct overloaded<F> : F {
  template <typename Arg>
  constexpr explicit overloaded(Arg&& arg) : F(std::forward<Arg>(arg)) {}

  using F::operator();
};

template <typename F1, typename F2, typename ...Rest>
struct overloaded<F1, F2, Rest...> : F1, overloaded<F2, Rest...> {
  template <typename Arg, typename ...Args>
  constexpr explicit overloaded(Arg&& arg, Args&& ...args)
    : F1(std::forward<Arg>(arg))
    , overloaded<F2, Rest...>(std::forward<Args>(args)...)
  {}

  using F1::operator();
  using overloaded<F2, Rest...>::operator();
};

/** makes an `overloaded` by given callables. (it's a helper to deduce types)
 */
template <typename ...Fs>
constexpr overloaded<Fs...> make_overloaded(Fs&& ...fs) {
  return overloaded<Fs...>{std::forward<Fs>(fs)...};
}

/** a type to use instead of `auto` when you don't need given type or value.
 *
 * it is implicitly constructible (useful or sinking argument)
 * and doesn't hold any value.
 */
struct sink {
  template <typename ...Args>
  constexpr sink(Args&& ...) noexcept {}
};


/** a generic (templated) callable to forward a single argument
 * to given callable.
 *
 * useful when sinking the argument, since `auto` argument doesn't exist in
 * c++11, specially combined with sink or another implicitly constructible
 * type.
 *
 * NOTE: single argument since it's designed for visiting a variant.
 *
 * usage example:
 * visit(
 *  make_overloaded(
 *    [](int) { return "numeric"; },
 *    [](const std::string&) { return "string"; },
 *    make_generic_overload([](sink) { return "unknown"; })
 *  ), my_variant);
 */
template <typename F>
struct generic_overload {
  F callable_;

  constexpr explicit generic_overload(const F& callable)
    : callable_(callable) {}

  constexpr explicit generic_overload(F&& callable)
    : callable_(std::move(callable)) {}

  template <typename Arg>
  constexpr auto operator()(Arg&& arg) const
      -> typename std::result_of<F(Arg)>::type {
    return callable_(std::forward<Arg>(arg));
  }
};

/** makes a `generic_overload` by given callables.
 * 
 * it's a helper function to deduce type of given callable.
 */
template <typename F>
generic_overload<F> make_generic_overload(F&& callable) {
  return generic_overload<F>{std::forward<F>(callable)};
}

}  // namespace internal
}  // namespace touca
