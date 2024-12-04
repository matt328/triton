template <class T, class D = std::default_delete<T>>
class CommandBufferPool {
 public:
   explicit CommandBufferPool(const size_t n) {
      for (size_t i = 0; i < n; ++i) {
         pool.push(std::make_unique<T>(i));
      }
   }

 private:
   void add(std::unique_ptr<T, D> obj) {
      Log.debug("Returning object to the pool. {}", static_cast<T>(*obj));
      pool.push(std::move(obj));
   }

   struct ReturnToPoolDeleter {
      explicit ReturnToPoolDeleter(CommandBufferPool* pool) : pool(pool) {
      }

      void operator()(T* ptr) {
         pool->add(std::unique_ptr<T>{ptr});
      }

    private:
      CommandBufferPool* pool;
   };

 public:
   std::unique_ptr<T, ReturnToPoolDeleter> acquire() {
      if (pool.empty()) {
         throw std::out_of_range("Cannot acquire object from an empty pool.");
      }

      auto obj = std::move(pool.top());
      pool.pop();

      Log.debug("Acquired object from pool. {}", static_cast<T>(*obj));

      return std::unique_ptr<T, ReturnToPoolDeleter>(obj.release(), ReturnToPoolDeleter{this});
   }

   bool empty() const {
      return pool.empty();
   }

   size_t size() const {
      return pool.size();
   }

 private:
   std::stack<std::unique_ptr<T>> pool;
};

auto main() -> int {

   initLogger(spdlog::level::trace, spdlog::level::trace);

   const auto pool = std::make_unique<CommandBufferPool<int>>(10);

   { auto one = pool->acquire(); }
   {
      auto two = pool->acquire();
      auto three = pool->acquire();
      auto four = pool->acquire();
      auto five = pool->acquire();
   }

   return 0;
}
