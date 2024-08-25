#pragma once

namespace cereal {
   template <class Archive>
   void serialize(Archive& ar, std::filesystem::path& path) {
      std::string pathStr = path.string();
      ar(pathStr);
      if (Archive::is_loading::value) {
         path = std::filesystem::path(pathStr);
      }
   }
}

namespace ed {

   struct PropertiesData {
      std::filesystem::path recentFile{};

      template <class T>
      void serialize(T& archive) {
         archive(recentFile);
      }
   };

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

      [[nodiscard]] std::optional<std::filesystem::path> getRecentFile() const {
         if (propertiesData.recentFile.empty()) {
            return std::nullopt;
         } else {
            return std::optional{propertiesData.recentFile};
         }
      }

      auto setRecentFile(const std::filesystem::path& value) {
         auto lock = std::lock_guard(mtx);
         propertiesData.recentFile = value;
         save();
      }

      void load(const std::filesystem::path& filePath) {
         if (loaded) {
            return;
         }
         path = filePath;

         if (!exists(filePath.parent_path())) {
            create_directories(filePath.parent_path());
         }

         // Create the file if it doesn't exist
         if (!exists(filePath)) {
            if (std::ofstream o{filePath, std::ios::binary}; o.is_open()) {
               cereal::BinaryOutputArchive output(o);
               output(propertiesData);
            } else {
               Log.warn("Failed to create properties file");
            }
         }

         if (std::ifstream i{filePath, std::ios::binary}; i.is_open()) {
            cereal::BinaryInputArchive input(i);
            input(propertiesData);
            loaded = true;
         } else {
            Log.warn("Error reading application configuration file: {0}", filePath.string());
         }
      }

    private:
      Properties() = default;
      PropertiesData propertiesData;
      std::filesystem::path path;
      std::mutex mtx;
      bool loaded{};

      void save() const {
         if (std::ofstream o{path, std::ios::binary}; o.is_open()) {
            cereal::BinaryOutputArchive output{o};
            output(propertiesData);
         } else {
            Log.warn("Error saving application configuration file: {0}", path.string());
         }
      }
   };
}

CEREAL_CLASS_VERSION(ed::PropertiesData, 1);
