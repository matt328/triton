#pragma once

namespace tr {
   class Context {
    public:
      Context();
      ~Context();

      Context(const Context&) = delete;
      Context(Context&&) = delete;
      auto operator=(const Context&) -> Context& = delete;
      auto operator=(Context&&) -> Context& = delete;

      [[nodiscard]] auto getVkContext() const -> const vk::raii::Context&;
      [[nodiscard]] auto getExtensionProperties() const -> std::vector<vk::ExtensionProperties>;
      [[nodiscard]] auto getLayerProperties() const -> std::vector<vk::LayerProperties>;

      [[nodiscard]] auto createInstance(const vk::InstanceCreateInfo& createInfo) const -> std::unique_ptr<vk::raii::Instance>;

    private:
      std::unique_ptr<vk::raii::Context> context;
   };
}
