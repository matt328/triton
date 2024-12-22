#pragma once

namespace ed {
class Dock {
 public:
   static void render() {
      static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

      ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

      const ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

      if ((dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0) {
         windowFlags |= ImGuiWindowFlags_NoBackground;
      }

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace", nullptr, windowFlags);
      ImGui::PopStyleVar();
      ImGui::PopStyleVar(2);

      // DockSpace
      if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
         ImGuiID dockspaceId = ImGui::GetID("DockSpace");
         ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);

         static auto first_time = true;
         if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspaceId); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspaceId, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

            const auto dockIdLeft = ImGui::DockBuilderSplitNode(dockspaceId,
                                                                ImGuiDir_Left,
                                                                0.25f,
                                                                nullptr,
                                                                &dockspaceId);

            const auto dockIdRight = ImGui::DockBuilderSplitNode(dockspaceId,
                                                                 ImGuiDir_Down,
                                                                 0.2f,
                                                                 nullptr,
                                                                 &dockspaceId);

            ImGui::DockBuilderDockWindow("Entity Editor", dockIdLeft);
            ImGui::DockBuilderDockWindow("Asset Tree", dockIdLeft);
            ImGui::DockBuilderDockWindow("Log", dockIdRight);

            ImGui::DockBuilderFinish(dockspaceId);
         }
      }
      ImGui::End(); // Dockspace
   }
};
}
