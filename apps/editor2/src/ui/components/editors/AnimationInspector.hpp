#pragma once
#include <gp/components/Animation.hpp>

namespace ed::ui::cmp {
   inline auto renderAnimationInspector(tr::gp::cmp::Animation* animation) -> void {
      if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
         static size_t itemCurrentIndex{};

         std::vector<const char*> items;
         items.push_back(animation->currentAnimationName.c_str());

         if (ImGui::BeginCombo("Animation", items[itemCurrentIndex])) {
            for (size_t n = 0; n < items.size(); ++n) {
               const bool isSelected = (itemCurrentIndex == n);
               if (ImGui::Selectable(items[n], isSelected)) {
                  itemCurrentIndex = n;
                  Log.debug("Set Current Animation Name to {0}",
                            std::string{items[itemCurrentIndex]});
               }
               if (isSelected) {
                  ImGui::SetItemDefaultFocus();
               }
            }
            ImGui::EndCombo();
         }

         ImGui::BeginDisabled(animation->playing);
         if (ImGui::Checkbox("Bind Pose", &animation->renderBindPose)) {}
         ImGui::EndDisabled();

         ImGui::BeginDisabled(animation->renderBindPose);

         ImGui::SameLine();

         if (ImGui::Checkbox("Play", &animation->playing)) {}

         ImGui::BeginDisabled(animation->playing);

         if (ImGui::SliderFloat("Time", &animation->timeRatio, 0.f, 1.f, "%.2f")) {}
         ImGui::EndDisabled();
         ImGui::EndDisabled();
      }
   }
}