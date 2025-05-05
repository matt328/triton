#pragma once

#include "IBufferStrategy.hpp"

namespace tr {

class HostVisibleStrategy : public IBufferStrategy {
public:
  HostVisibleStrategy() = default;
  ~HostVisibleStrategy() override = default;

  HostVisibleStrategy(const HostVisibleStrategy&) = default;
  HostVisibleStrategy(HostVisibleStrategy&&) = delete;
  auto operator=(const HostVisibleStrategy&) -> HostVisibleStrategy& = default;
  auto operator=(HostVisibleStrategy&&) -> HostVisibleStrategy& = delete;

  void rewrite(ManagedBuffer& buffer, const void* data, size_t size) override;
};

}
