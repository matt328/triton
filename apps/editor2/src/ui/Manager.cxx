#include "Manager.hpp"

#include "ImGuiStyle.hpp"
#include "assets/RobotoRegular.h"
#include "assets/SourceCodePro.h"
#include "ui/components/ImGuiSink.hpp"

namespace ed::ui {
   static constexpr float FontSize = 18.f;

   Manager::Manager() {
      Log.trace("Constructing Manager");
      ImGuiEx::setupImGuiStyle();

      setupFonts();

      appLog = std::make_unique<cmp::AppLog>();
      const auto logFn = [this](const std::string& message) {
         appLog->AddLog("%s", message.c_str());
      };
      Log.sinks().push_back(std::make_shared<my_sink_mt>(logFn));
   }

   void Manager::render() {
      ZoneNamedN(guiRender, "Gui Render", true);
      appLog->font = sauce;
      appLog->Draw("Log");
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