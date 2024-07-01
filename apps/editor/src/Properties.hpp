#pragma once

#include <filesystem>
namespace ed::pr {
   using nlohmann::ordered_json;
   using FsPath = std::filesystem::path;

   class Properties {
    public:
      static Properties& getInstance() {
         static Properties instance;
         return instance;
      }

      ~Properties() = default;
      Properties(const Properties&) = delete;
      Properties(Properties&&) = delete;
      Properties& operator=(const Properties&) = delete;
      Properties& operator=(Properties&&) = delete;

      [[nodiscard]] std::optional<std::filesystem::path> getRecentFilePath() const {
         if (rootJson.contains("recentFile")) {
            std::string recentFile = rootJson["recentFile"];
            return std::optional<FsPath>{std::filesystem::path{recentFile}};
         } else {
            return std::nullopt;
         }
      }

      void setRecentFilePath(FsPath fsPath) {
         rootJson["recentFile"] = fsPath.string();
         save();
      }

      void load(const std::filesystem::path& filePath) {
         this->filePath = filePath;

         if (!std::filesystem::exists(filePath.parent_path())) {
            std::filesystem::create_directories(filePath.parent_path());
         }

         if (!std::filesystem::exists(filePath)) {
            using nlohmann::ordered_json;
            ordered_json rootJson{};
            rootJson["version"] = "0.0.1";

            std::ofstream o{filePath};
            if (o.is_open()) {
               o << std::setw(2) << rootJson << std::endl;
               o.close();
            } else {
               Log::warn << "Could not open config file for writing: " << filePath << std::endl;
            }
         }

         std::ifstream i{filePath};
         if (i.is_open()) {
            i >> rootJson;
            i.close();
         } else {
            Log::warn << "Error opening application config file: " << filePath << std::endl;
         }

         Log::info << rootJson.dump(2) << std::endl;
      }

    private:
      Properties() = default;

      std::filesystem::path filePath;
      ordered_json rootJson{};

      void save() {
         std::ofstream o{filePath};
         if (o.is_open()) {
            o << std::setw(2) << rootJson << std::endl;
            o.close();
         } else {
            Log::warn << "Error saving application config file: " << filePath << std::endl;
         }
      }
   };
}