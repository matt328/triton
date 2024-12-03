#pragma once

template <class T, class D = std::default_delete<T>>
class SimpleObjectPool {
 public:
   explicit SimpleObjectPool(const size_t n) {
      for (size_t i = 0; i < n; ++i) {
         pool.push(std::make_unique<T>());
      }
   }

 private:
   void add(std::unique_ptr<T, D> obj) {
      pool.push(std::move(obj));
   }

   struct ReturnToPoolDeleter {
      explicit ReturnToPoolDeleter(SimpleObjectPool* pool) : pool_(pool) {
      }

      void operator()(T* ptr) {
         pool_->add(std::unique_ptr<T>{ptr});
      }

    private:
      SimpleObjectPool* pool_;
   };

 public:
   std::unique_ptr<T, ReturnToPoolDeleter> acquire() {
      if (pool.empty()) {
         throw std::out_of_range("Cannot acquire object from an empty pool.");
      }

      auto obj = std::move(pool.top());
      pool.pop();

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
