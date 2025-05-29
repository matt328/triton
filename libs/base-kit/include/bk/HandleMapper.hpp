#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"

namespace tr {

template <typename PublicTag, typename InternalTag>
class HandleMapper {
public:
  using PublicHandle = Handle<PublicTag>;
  using InternalHandle = Handle<InternalTag>;

  auto toPublic(InternalHandle internal) -> PublicHandle {
    auto [it, inserted] = internalToPublic.emplace(internal, generator.requestHandle());
    if (inserted) {
      publicToInternal[it->second] = internal;
    }
    return it->second;
  }

  auto toInternal(PublicHandle publicHandle) const -> std::optional<InternalHandle> {
    auto it = publicToInternal.find(publicHandle);
    return it != publicToInternal.end() ? std::optional{it->second} : std::nullopt;
  }

private:
  uint32_t nextId = 0;
  HandleGenerator<PublicTag> generator;
  std::unordered_map<InternalHandle, PublicHandle> internalToPublic;
  std::unordered_map<PublicHandle, InternalHandle> publicToInternal;
};

}
