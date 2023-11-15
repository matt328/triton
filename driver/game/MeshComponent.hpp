#pragma once

#include "Component.hpp"

namespace Game {

   class MeshComponent : public Component {
    public:
      MeshComponent() = default;
      MeshComponent(const MeshComponent&) = default;
      MeshComponent(MeshComponent&&) = delete;
      MeshComponent& operator=(const MeshComponent&) = default;
      MeshComponent& operator=(MeshComponent&&) = delete;
      ~MeshComponent() override = default;

      void update() override{};
   };

}