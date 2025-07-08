#pragma once

namespace bk {
class RandomUtils {
public:
  static void seed(uint32_t s) {
    getEngine().seed(s);
  }

  static auto uint32InRange(uint32_t min, uint32_t max) -> uint32_t {
    std::uniform_int_distribution<uint32_t> dist(min, max);
    return dist(getEngine());
  }

  static auto uint64InRange(uint64_t min, uint64_t max) -> uint64_t {
    std::uniform_int_distribution<uint64_t> dist(min, max);
    return dist(getEngine());
  }

  static auto int32InRange(int32_t min, int32_t max) -> int32_t {
    std::uniform_int_distribution<int32_t> dist(min, max);
    return dist(getEngine());
  }

  static auto int64InRange(int64_t min, int64_t max) -> int64_t {
    std::uniform_int_distribution<int64_t> dist(min, max);
    return dist(getEngine());
  }

  static auto floatInRange(float min, float max) -> float {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(getEngine());
  }

  static auto vec2InRange(float min, float max) -> glm::vec2 {
    return {floatInRange(min, max), floatInRange(min, max)};
  }

  static auto vec3InRange(float min, float max) -> glm::vec3 {
    return {floatInRange(min, max), floatInRange(min, max), floatInRange(min, max)};
  }

  static auto vec4InRange(float min, float max) -> glm::vec4 {
    return {floatInRange(min, max),
            floatInRange(min, max),
            floatInRange(min, max),
            floatInRange(min, max)};
  }

private:
  static auto getEngine() -> std::mt19937& {
    static thread_local std::mt19937 engine{std::random_device{}()};
    return engine;
  }
};
}

namespace tr {

struct MapKey {

  MapKey()
      : imageGen{imageRandomDevice()},
        imageDistribution(1, 10000),
        geometryGen{geometryRandomDevice()},
        geometryDistribution(1, 10000) {
  }

  [[deprecated("Use type HandleGenerator/Handle instead")]] [[nodiscard]] auto getKey() {
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
