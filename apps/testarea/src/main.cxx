auto main() -> int {

   initLogger(spdlog::level::trace, spdlog::level::trace);

   Log.debug("Hello World!");

   return 0;
}
