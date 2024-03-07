#pragma once

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

      [[nodiscard]] std::optional<FsPath> getRecentFilePath() const {
         return rootJson.contains("recentFile")
                    ? std::optional<FsPath>{FsPath{rootJson["recentFile"]}}
                    : std::nullopt;
      }

      void setRecentFilePath(FsPath fsPath) {
         rootJson["recentFile"] = fsPath.string();
         save();
      }

      void load(const std::filesystem::path& filePath) {
         this->filePath = filePath;

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
      Properties() {
      }

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