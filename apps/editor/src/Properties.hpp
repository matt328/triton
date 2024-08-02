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
            const std::string recentFile = rootJson["recentFile"];
            return std::optional{std::filesystem::path{recentFile}};
         }
         return std::nullopt;
      }

      void setRecentFilePath(const FsPath& fsPath) {
         rootJson["recentFile"] = fsPath.string();
         save();
      }

      void load(const std::filesystem::path& filePath) {
         this->filePath = filePath;

         if (!exists(filePath.parent_path())) {
            create_directories(filePath.parent_path());
         }

         if (!exists(filePath)) {
            using nlohmann::ordered_json;
            ordered_json rootJson{};
            rootJson["version"] = "0.0.1";

            if (std::ofstream o{filePath}; o.is_open()) {
               o << std::setw(2) << rootJson << std::endl;
               o.close();
            } else {
               Log.warn("Could not open config file for writing: {0}", filePath.string());
            }
         }

         if (std::ifstream i{filePath}; i.is_open()) {
            i >> rootJson;
            i.close();
         } else {
            Log.warn("Error opening application configuration file: {0}", filePath.string());
         }
         Log.info(rootJson.dump(2));
      }

    private:
      Properties() = default;

      std::filesystem::path filePath;
      ordered_json rootJson{};

      void save() const {
         if (std::ofstream o{filePath}; o.is_open()) {
            o << std::setw(2) << rootJson << std::endl;
            o.close();
         } else {
            Log.warn("Error saving application config file: {0}", filePath.string());
         }
      }
   };
}