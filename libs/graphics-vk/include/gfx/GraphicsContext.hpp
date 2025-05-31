#pragma once

namespace tr {

class IEventQueue;
class IRenderContext;
class IStateBuffer;
class IWindow;
class Device;
class IAssetSystem;
class IAssetService;

class GraphicsContext {
public:
  GraphicsContext(std::shared_ptr<IEventQueue> newEventQueue,
                  std::shared_ptr<IRenderContext> newRenderContext,
                  std::shared_ptr<IStateBuffer> newStateBuffer,
                  std::shared_ptr<Device> newDevice,
                  std::shared_ptr<IAssetSystem> newAssetSystem);
  ~GraphicsContext();

  GraphicsContext(const GraphicsContext&) = default;
  GraphicsContext(GraphicsContext&&) = delete;
  auto operator=(const GraphicsContext&) -> GraphicsContext& = default;
  auto operator=(GraphicsContext&&) -> GraphicsContext& = delete;

  static auto create(std::shared_ptr<IEventQueue> newEventQueue,
                     std::shared_ptr<IStateBuffer> newStateBuffer,
                     std::shared_ptr<IWindow> newWindow,
                     std::shared_ptr<IAssetService> newAssetService)
      -> std::shared_ptr<GraphicsContext>;

  auto run(std::stop_token token) -> void;

private:
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IRenderContext> renderContext;
  std::shared_ptr<IStateBuffer> stateBuffer;
  std::shared_ptr<IWindow> window;
  std::shared_ptr<Device> device;
  std::shared_ptr<IAssetSystem> assetSystem;

  std::chrono::steady_clock clock;
};

}
