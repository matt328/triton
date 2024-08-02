#pragma once

namespace ed::data {

   class FutureWrapperBase {
    public:
      FutureWrapperBase() = default;
      FutureWrapperBase(const FutureWrapperBase&) = delete;
      FutureWrapperBase& operator=(const FutureWrapperBase&) = delete;

      FutureWrapperBase(FutureWrapperBase&&) = delete;
      FutureWrapperBase& operator=(FutureWrapperBase&&) = delete;

      virtual ~FutureWrapperBase() = default;
      virtual auto check() -> bool = 0;
      virtual void execute() = 0;
   };

   template <typename T>
   class FutureWrapper final : public FutureWrapperBase {
    public:
      FutureWrapper(futures::cfuture<T>&& future, std::function<void(T)> fn)
          : FutureWrapperBase{}, future(std::move(future)), fn(fn) {
      }

      auto check() -> bool override {
         return future.valid() && future.is_ready();
      }

      void execute() override {
         if (future.valid()) {
            try {
               auto result = future.get();
               fn(result);
            } catch (const std::exception& ex) { Log.error("Exception {0}", ex.what()); }
         }
      }

    private:
      futures::cfuture<T> future;
      std::function<void(T)> fn;
   };

   class FutureMonitor {
    public:
      FutureMonitor() = default;
      ~FutureMonitor() { // NOLINT(*-use-equals-default)
      }

      FutureMonitor(const FutureMonitor&) = delete;
      FutureMonitor& operator=(const FutureMonitor&) = delete;

      FutureMonitor(FutureMonitor&&) = delete;
      FutureMonitor& operator=(FutureMonitor&&) = delete;

      void update() {
         for (auto it = futures.begin(); it != futures.end();) {
            if ((*it)->check()) {
               (*it)->execute();
               it = futures.erase(it);
            } else {
               ++it;
            }
         }
      }

      // TODO: Add an error handler function here.
      // catch the exception thrown by .get on L32 and call the error function
      template <typename T>
      void monitorFuture(futures::cfuture<T> future, const std::function<void(T)>& fn) {
         futures.emplace_back(std::make_unique<FutureWrapper<T>>(std::move(future), fn));
      }

    private:
      std::vector<std::unique_ptr<FutureWrapperBase>> futures;
   };
}