#pragma once

namespace tr::util {
   constexpr double frameTimeNumerator = 10.0f;
   class Timer {
    public:
      /// Supply the desired max FPS and the maximum number of physics steps to take during a frame
      Timer(int targetFps, int maxUpdatesPerFrame)
          : maxFrameTime(frameTimeNumerator / targetFps),
            fixedTimeStep(1.f / static_cast<float>(maxUpdatesPerFrame * targetFps)){};
      ~Timer() = default;

      Timer(const Timer&) = default;
      Timer(Timer&&) = delete;
      Timer& operator=(const Timer&) = default;
      Timer& operator=(Timer&&) = delete;

      /// Call this method once per 'frame' and following it, call your Render function
      /// This method will get as close to
      void tick(std::function<void()> update) {
         currentInstant = glfwGetTime();

         auto elapsed = currentInstant - previousInstant;
         previousInstant = currentInstant;

         if (elapsed > maxFrameTime) {
            elapsed = maxFrameTime;
         }

         accumulatedTime += elapsed;

         while (accumulatedTime >= fixedTimeStep) {
            accumulatedTime -= fixedTimeStep;
            update();
         }

         blendingFactor = accumulatedTime / fixedTimeStep;
      }

      [[nodiscard]] double getBlendingFactor() const {
         return blendingFactor;
      }

    private:
      double maxFrameTime{};
      double fixedTimeStep{};
      double currentInstant{}, previousInstant{}, accumulatedTime{}, blendingFactor{};
   };
}
