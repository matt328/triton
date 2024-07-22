
class Immutable {
 public:
   Immutable() = delete;
   ~Immutable() = default;

   Immutable(const Immutable&) = delete;
   Immutable(Immutable&&) noexcept = default;
   Immutable& operator=(const Immutable&) = delete;
   Immutable& operator=(Immutable&&) = delete;

   explicit Immutable(std::unique_ptr<int>&& intPointer) : intPointer(std::move(intPointer)) {
   }

 private:
   std::unique_ptr<int> intPointer;
};

auto produceUniquePtr() -> std::unique_ptr<int> {
   return std::make_unique<int>(42);
}

int main() {

   std::vector<Immutable> list{};

   list.emplace_back(produceUniquePtr());

   return 0;
}
