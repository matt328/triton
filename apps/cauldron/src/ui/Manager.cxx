#include "Manager.hpp"

#include "ImGuiStyle.hpp"
#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "assets/RobotoRegular.h"
#include "assets/JetBrainsMono.hpp"
#include "assets/IconsLucide.hpp"
#include "assets/Lucide.hpp"

#include "imgui_impl_vulkan.h"
#include "ui/components/AppLog.hpp"
#include "ui/components/AssetViewer.hpp"
#include "ui/components/ImGuiSink.hpp"
#include "ui/components/Dock.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/Menu.hpp"

namespace ed {

static constexpr float FontSize = 18.f;

Manager::Manager(std::shared_ptr<Menu> newAppMenu,
                 std::shared_ptr<AssetViewer> newAssetViewer,
                 std::shared_ptr<DialogManager> newDialogManager,
                 std::shared_ptr<EntityEditor> newEntityEditor,
                 std::shared_ptr<Properties> newProperties,
                 std::shared_ptr<DataFacade> newDataFacade,
                 std::shared_ptr<AssetTool> newAssetTool,
                 std::shared_ptr<tr::IGuiCallbackRegistrar> newGuiCallbackRegistrar)
    : appMenu{std::move(newAppMenu)},
      assetViewer{std::move(newAssetViewer)},
      dialogManager{std::move(newDialogManager)},
      entityEditor{std::move(newEntityEditor)},
      properties{std::move(newProperties)},
      dataFacade{std::move(newDataFacade)},
      assetTool{std::move(newAssetTool)},
      guiCallbackRegistrar{std::move(newGuiCallbackRegistrar)} {

  Log.trace("Constructing Manager");

  guiCallbackRegistrar->setReadyCallback([&] {
    ImGuiEx::setupImGuiStyle();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    setupFonts();

    appLog = std::make_shared<AppLog>();
    const auto logFn = [weakLog = std::weak_ptr<AppLog>(appLog)](const std::string& message) {
      if (auto sharedLog = weakLog.lock()) {
        sharedLog->AddLog("%s", message.c_str());
      }
    };
    Log.sinks().push_back(std::make_shared<my_sink_mt>(logFn));
    isReady = true;
  });
}

Manager::~Manager() {
  Log.trace("Destroying Manager");
  appLog = nullptr;
}

void Manager::render() {
  if (!isReady) {
    return;
  }
  ZoneNamedN(guiRender, "Gui Render", true);
  Dock::render();

  appLog->font = sauce;
  appLog->Draw("Log");

  dialogManager->render();

  appMenu->render();
  entityEditor->render();
  assetViewer->render();
  assetTool->render();

  dialogManager->update();
}

auto Manager::setupFonts() -> void {
  const auto& io = ImGui::GetIO();
  auto* const fontAtlas = io.Fonts;
  const auto* const ranges = io.Fonts->GetGlyphRangesDefault();

  auto config = ImFontConfig{};
  config.FontDataOwnedByAtlas = false;
  fontAtlas->AddFontFromMemoryTTF(RobotoFont, RobotoLength, FontSize, &config, ranges);

  ImFontConfig lucideConfig;
  lucideConfig.MergeMode = true;
  lucideConfig.GlyphMinAdvanceX = 18.f;
  lucideConfig.GlyphOffset.y = 3.f;
  static const ImWchar iconRanges[] = {ICON_MIN_LC, ICON_MAX_LC, 0};
  fontAtlas->AddFontFromMemoryTTF(lucide_ttf, lucide_ttf_len, 18.f, &lucideConfig, iconRanges);

  sauce = fontAtlas->AddFontFromMemoryTTF(JetBrainsMono,
                                          JetBrainsMonoLength,
                                          FontSize,
                                          &config,
                                          ranges);

  if (!ImGui_ImplVulkan_CreateFontsTexture()) {
    Log.warn("Error creating Fonts Texture");
  }
}
}
