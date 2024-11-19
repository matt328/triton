#include "Manager.hpp"

#include "ImGuiStyle.hpp"
#include "assets/RobotoRegular.h"
#include "assets/SourceCodePro.h"

#include "ui/components/AppLog.hpp"
#include "ui/components/AssetViewer.hpp"
#include "ui/components/ImGuiSink.hpp"
#include "ui/components/Dock.hpp"

namespace ed::ui {
   static constexpr float FontSize = 18.f;

   Manager::Manager(std::shared_ptr<cmp::Menu> newAppMenu,
                    std::shared_ptr<cmp::AssetViewer> newAssetViewer,
                    std::shared_ptr<cmp::DialogManager> newDialogManager,
                    std::shared_ptr<cmp::EntityEditor> newEntityEditor,
                    std::shared_ptr<TaskQueue> newTaskQueue)
       : appMenu{std::move(newAppMenu)},
         assetViewer{std::move(newAssetViewer)},
         dialogManager{std::move(newDialogManager)},
         entityEditor{std::move(newEntityEditor)},
         taskQueue{std::move(newTaskQueue)} {

      Log.trace("Constructing Manager");
      ImGuiEx::setupImGuiStyle();
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

      setupFonts();

      appLog = std::make_shared<cmp::AppLog>();
      const auto logFn = [weakLog =
                              std::weak_ptr<cmp::AppLog>(appLog)](const std::string& message) {
         if (auto sharedLog = weakLog.lock()) {
            sharedLog->AddLog("%s", message.c_str());
         }
      };
      Log.sinks().push_back(std::make_shared<my_sink_mt>(logFn));
   }

   Manager::~Manager() {
      Log.trace("Destroying Manager");
      appLog = nullptr;
   }

   void Manager::render() {
      ZoneNamedN(guiRender, "Gui Render", true);
      taskQueue->processCompleteTasks();
      cmp::Dock::render();

      appLog->font = sauce;
      appLog->Draw("Log");

      dialogManager->render();

      appMenu->render();
      entityEditor->render();
      assetViewer->render();

      dialogManager->update();
   }

   auto Manager::setupFonts() -> void {
      const auto& io = ImGui::GetIO();
      auto* const fontAtlas = io.Fonts;
      const auto* const ranges = io.Fonts->GetGlyphRangesDefault();

      auto config = ImFontConfig{};
      config.FontDataOwnedByAtlas = false;
      fontAtlas->AddFontFromMemoryTTF(RobotoFont, RobotoLength, FontSize, &config, ranges);

      sauce = fontAtlas->AddFontFromMemoryTTF(SourceCodePro_Regular_ttf,
                                              SourceCodePro_Regular_ttf_len,
                                              FontSize,
                                              &config,
                                              ranges);

      if (!ImGui_ImplVulkan_CreateFontsTexture()) {
         Log.warn("Error creating Fonts Texture");
      }
   }
}