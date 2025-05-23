#pragma once

/*
  These utilities never worked out as nicely as I would have hoped. Leaving them here for posterity
  though.
*/

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

template <typename ResultT, typename... Fs>
class optionalMatchVisitor : public Fs... {
public:
  using Fs::operator()...;

  template <typename T>
  auto operator()(T&&) const -> std::optional<ResultT> {
    return std::nullopt; // fallback
  }
};

template <typename ResultT, typename... Fs>
auto matchOptional(Fs&&... fs) {
  return optionalMatchVisitor<ResultT, Fs...>{std::forward<Fs>(fs)...};
}

}
