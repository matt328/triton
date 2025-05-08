#pragma once

namespace tr {

class QuitFlag {
public:
  void requestQuit() {
    {
      std::lock_guard<std::mutex> lock(mutex);
      quitRequested = true;
    }
    cv.notify_all();
  }

  void waitForQuit() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return quitRequested; });
  }

private:
  std::mutex mutex;
  std::condition_variable cv;
  bool quitRequested = false;
};

}
