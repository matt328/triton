#pragma once

namespace tr {
template <typename T>
static auto toByteVector(const std::shared_ptr<std::vector<T>>& src)
    -> std::shared_ptr<std::vector<std::byte>> {
  auto byteVec = std::make_shared<std::vector<std::byte>>();
  byteVec->resize(src->size() * sizeof(T));
  std::memcpy(byteVec->data(), src->data(), byteVec->size());
  return byteVec;
}

template <typename T>
static auto fromByteVector(const std::shared_ptr<std::vector<std::byte>>& src)
    -> std::shared_ptr<std::vector<T>> {
  auto typedVec = std::make_shared<std::vector<T>>();
  const size_t count = src->size() / sizeof(T);
  typedVec->resize(count);
  std::memcpy(typedVec->data(), src->data(), src->size());
  return typedVec;
}
}
