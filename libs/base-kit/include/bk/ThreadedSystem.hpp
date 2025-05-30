#pragma once

namespace tr {

template <typename T>
concept Runnable = requires(T t, std::stop_token token) {
  { t.run(token) } -> std::same_as<void>;
};

template <Runnable T>
class ThreadedSystem {
public:
  template <typename... Args>
  explicit ThreadedSystem(std::shared_ptr<Args>... args) {
    thread =
        std::jthread([args = std::make_tuple(std::move(args)...)](std::stop_token token) mutable {
          std::apply(
              [&](auto&&... unpackedArgs) {
                T system(std::move(unpackedArgs)...);
                system.run(token);
              },
              std::move(args));
        });
  }

  void requestStop() {
    thread.request_stop();
  }

private:
  std::jthread thread;
};

}
