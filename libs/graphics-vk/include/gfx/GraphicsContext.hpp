#pragma once

namespace tr {

class IEventQueue;

class GraphicsContext {
public:
  explicit GraphicsContext(std::shared_ptr<IEventQueue> newEventQueue);
  ~GraphicsContext() = default;

  GraphicsContext(const GraphicsContext&) = default;
  GraphicsContext(GraphicsContext&&) = delete;
  auto operator=(const GraphicsContext&) -> GraphicsContext& = default;
  auto operator=(GraphicsContext&&) -> GraphicsContext& = delete;

  static auto create(std::shared_ptr<IEventQueue> newEventQueue)
      -> std::shared_ptr<GraphicsContext>;

  auto run() -> void;
  auto stop() -> void;

private:
  std::shared_ptr<IEventQueue> eventQueue;
};

}
