#pragma once

namespace tr {

template <class... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overload(Ts...) -> overload<Ts...>;

template <typename Variant, typename... Visitors>
auto match(Variant&& v, Visitors&&... visitors) -> decltype(auto) {
  return std::visit(overload{std::forward<Visitors>(visitors)...}, std::forward<Variant>(v));
}

}
