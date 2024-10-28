#pragma once

namespace tr::cm {
   constexpr double frameTimeNumerator = 10.0;

   class Timer {
    public:
      /// Supply the desired max FPS and the maximum number of physics steps per frame
      Timer(int targetFps, int maxUpdatesPerFrame)
          : startTime{std::chrono::steady_clock::now()},
            maxFrameTime(1.0 / targetFps),
            fixedTimeStep(1.0 / targetFps / maxUpdatesPerFrame) {
      }

      /// Call this method once per 'frame' and call your Render function afterward
      void tick(const std::function<void()>& update) {
         currentInstant = getTime();

         // Calculate elapsed time since the last frame
         auto elapsed = currentInstant - previousInstant;
         previousInstant = currentInstant;

         // Clamp elapsed to avoid huge jumps in update calls
         if (elapsed > maxFrameTime) {
            elapsed = maxFrameTime;
         }

         accumulatedTime += elapsed;

         // Cap accumulated time to avoid excessive updates in a single tick
         if (accumulatedTime > fixedTimeStep) {
            accumulatedTime = fixedTimeStep;
         }

         TracyPlot("fixedTimeStep", fixedTimeStep);

         // Run updates as long as accumulated time allows
         while (accumulatedTime >= fixedTimeStep) {
            accumulatedTime -= fixedTimeStep;
            TracyPlot("accumulatedTime", accumulatedTime);
            update();
         }

         // Calculate the blending factor for interpolation
         blendingFactor = accumulatedTime / fixedTimeStep;
      }

      [[nodiscard]] auto getBlendingFactor() const -> double {
         return blendingFactor;
      }

    private:
      std::chrono::steady_clock::time_point startTime;
      double maxFrameTime;
      double fixedTimeStep;
      double currentInstant = 0.0, previousInstant = 0.0, accumulatedTime = 0.0,
             blendingFactor = 0.0;

      auto getTime() -> double {
         auto currentTime = std::chrono::steady_clock::now();
         auto elapsedTime = currentTime - startTime;
         return std::chrono::duration<double>(elapsedTime).count();
      }
   };
}
