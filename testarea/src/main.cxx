int main() {
   tf::Executor executor{4};

   auto future = executor.async("sleeper", []() {
      std::this_thread::sleep_for(std::chrono::seconds(3));
      return 42;
   });
   auto future2 = executor.async("sleeper2", []() {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return 42;
   });
   auto future3 = executor.async("sleeper3", []() {
      std::this_thread::sleep_for(std::chrono::seconds(4));
      return 42;
   });
   auto future4 = executor.async("sleeper4", []() {
      std::this_thread::sleep_for(std::chrono::seconds(6));
      return 42;
   });
   auto future5 = executor.async("sleeper5", []() {
      std::this_thread::sleep_for(std::chrono::seconds(3));
      return 42;
   });
   auto future6 = executor.async("sleeper6", []() {
      std::this_thread::sleep_for(std::chrono::seconds(7));
      return 42;
   });

   auto status = future.wait_for(std::chrono::milliseconds(0));

   if (status == std::future_status::ready) {
      std::cout << "10s went by awfully fast..." << std::endl;
   } else if (status == std::future_status::timeout) {
      std::cout << "future isn't ready yet so we'll just hit it up next frame" << std::endl;
   }

   const auto result = future.get();
   future2.get();
   future3.get();
   future4.get();
   future5.get();
   future6.get();
   std::cout << "result: " << result << std::endl;

   return 0;
}
