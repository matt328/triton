# Debug/Editor Gui Design

## ImGui Current Use

- ImGuiHelper - just initializes and tears down imgui statically, has no actual behavor beyond that.
- Frame::renderOverlay - draws imgui stuff
- Window uses it to query and set mouse/kb interaction mode.

## System Design

- IGuiSystem needs IGraphicsDevice and IWindow injected into it in order to construct it.
- triton will own IGuiSystem, but expose a getter on the `Context` so it can be bound into Editor's di system as well.
- inject `std::optional<IGuiSystem>` and make sure to handle optional everywhere so it gracefully doesn't render anything if `.has_value()` is false.

- inject `IGuiSystem` into `Application`, if `.has_value()` call `setRenderFn([]() { imguiManager->render(); })`

- inject `IGuiSystem` into `DefaultRenderContext`, if `.has_value()`, call `render()` at the right point, probably between
`currentFrame.render()` and `currentFrame.present()`, adjusting the commandBuffer's `endRendering()` and `end()` placement.

```cxx
setRenderFn(const std::function<void(void)> newRenderFn);

render(const std::shared_ptr<vk::raii::CommandBuffer>& commandBuffer,
       const std::shared_ptr<vk::raii::ImageView>& swapchainImageView,
       const vk::Extent2D& swapchainExtent) {

   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   this->renderFn();

   ImGui::Render();

   ZoneNamedN(imguiZone, "Render ImGui", true);
   const auto colorAttachment = vk::RenderingAttachmentInfo{
         .imageView = *swapchainImageView,
         .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
         .loadOp = vk::AttachmentLoadOp::eLoad,
         .storeOp = vk::AttachmentStoreOp::eStore,
   };

   const auto renderInfo = vk::RenderingInfo{
         .renderArea = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent},
         .layerCount = 1,
         .colorAttachmentCount = 1,
         .pColorAttachments = &colorAttachment,
   };

   commandBuffer->beginRendering(renderInfo);

   ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), **commandBuffer);

   commandBuffer->endRendering();
}
```

## Synchronization

- Should be able to edit entities values from the editor such that access to them is synchronized by the EntityService.

- Editor side data store stores all the initial values that get stored in the project file.
- Can the editor just edit these initial values, and push them to the game world once per frame?
- Syncs back from the game world to the UI:
  - While a behavior component is present and active, component editor is read only, reads and displays values from the game world. IE Can't change an entity's position while it's being updated by a behavior.

Edit mode:

- Load data into store only.
- Keep a list of dirty store items.
- At the end of each frame, sync the store item with the game world, whether it means creating, updating, or deleting it.
  - Dirty List
  - Deleted List
  - Created List?
- Inspectors will work on store data only, marking edited entities dirty.
