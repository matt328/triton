#pragma once

namespace tr {

template <typename T>
class LockableResource {
public:
  LockableResource(T& resource, tracy::Lockable<std::mutex>& mutex)
      : resource(resource), lock(mutex) {
  }

  auto get() const -> T& {
    return resource;
  }

private:
  T& resource;
  std::lock_guard<tracy::Lockable<std::mutex>> lock;
};

}
