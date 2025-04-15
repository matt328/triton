#pragma once

namespace tr {

struct MapKey {

  MapKey()
      : imageGen{imageRandomDevice()},
        imageDistribution(1, 10000),
        geometryGen{geometryRandomDevice()},
        geometryDistribution(1, 10000) {
  }

  // TODO(matt): someday uncomment this and fix all the warnings. I'd really like to, but it's just
  // too much right now.
  //[[deprecated("Use type HandleGenerator/Handle instead")]]
  [[nodiscard]] auto getKey() {
    return geometryDistribution(geometryGen);
  }

private:
  std::random_device imageRandomDevice;
  std::mt19937 imageGen;
  std::uniform_int_distribution<std::size_t> imageDistribution;

  std::random_device geometryRandomDevice;
  std::mt19937 geometryGen;
  std::uniform_int_distribution<std::size_t> geometryDistribution;
};

}
