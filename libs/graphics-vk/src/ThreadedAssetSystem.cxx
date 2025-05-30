#include "gfx/ThreadedAssetSystem.hpp"
#include "gfx/IAssetSystem.hpp"

namespace tr {

ThreadedAssetSystem::ThreadedAssetSystem(std::function<std::unique_ptr<IAssetSystem>()> factory)
    : thread([this, factory = std::move(factory)](std::stop_token token) mutable {
        pthread_setname_np(pthread_self(), "AssetSystem");
        system = factory();
        system->run(std::move(token));
      }) {
}

auto ThreadedAssetSystem::requestStop() -> void {
  Log.trace("Stopping AssetSystem Thread");
  thread.request_stop();
}

}
