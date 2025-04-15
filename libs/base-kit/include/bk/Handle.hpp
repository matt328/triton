#pragma once

namespace tr {

template <typename T>
struct Handle {
  size_t id;
  constexpr auto operator==(const Handle&) const -> bool = default;
};

}

namespace std {
template <typename T>
struct hash<tr::Handle<T>> {
  auto operator()(const tr::Handle<T>& h) const -> size_t {
    return h.id;
  }
};
}
