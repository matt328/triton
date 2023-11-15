#pragma once

namespace Game {

   class Component;

   class GameObject {
    public:
      void update();

      template <typename T, typename... Args>
      std::shared_ptr<T> addComponent(Args&&... args) {
         static_assert(std::is_base_of<Component, T>::value, "T Must derive from Component");

         for (auto& existingComponent : allComponents) {
            if (std::dynamic_pointer_cast<T>(existingComponent)) {
               return std::dynamic_pointer_cast<T>(existingComponent);
            }
         }

         std::shared_ptr<T> newComponent = std::make_shared<T>(std::forward<Args>(args)...);
         allComponents.push_back(newComponent);

         return newComponent;
      }

      template <typename T>
      std::shared_ptr<T> getComponent() {
         static_assert(std::is_base_of<Component, T>::value, "T Must derive from Component");
         for (auto& existingComponent : allComponents) {
            if (std::dynamic_pointer_cast<T>(existingComponent)) {
               return std::dynamic_pointer_cast<T>(existingComponent);
            }
         }
         return nullptr;
      }

    private:
      std::vector<std::shared_ptr<Component>> allComponents;
   };
}
