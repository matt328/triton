#include "bk/Logger2.hpp"

spdlog::logger Log("basic");

static auto getCurrentThreadName() -> std::string {
  std::array<char, 16> name{};
  pthread_getname_np(pthread_self(), name.data(), name.size());
  return std::string{name.data()};
}

#include "spdlog/pattern_formatter.h"
class my_formatter_flag : public spdlog::custom_flag_formatter {
public:
  void format(const spdlog::details::log_msg&,
              const std::tm&,
              spdlog::memory_buf_t& dest) override {
    std::string some_txt = getCurrentThreadName();
    some_txt = some_txt + std::string(padinfo_.width_ - some_txt.length(), ' ');
    dest.append(some_txt.data(), some_txt.data() + some_txt.size());
  }

  [[nodiscard]] auto clone() const -> std::unique_ptr<custom_flag_formatter> override {
    return spdlog::details::make_unique<my_formatter_flag>();
  }
};

const std::string LOG_PATTERN = "%I:%M:%S [%10*] %^%-8l%$ %v";

void initLogger([[maybe_unused]] spdlog::level::level_enum debugLevel,
                [[maybe_unused]] spdlog::level::level_enum releaseLevel) {

  auto formatter = std::make_unique<spdlog::pattern_formatter>();
  formatter->add_flag<my_formatter_flag>('*').set_pattern(LOG_PATTERN);
  // spdlog::set_formatter(std::move(formatter));

  const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  // console_sink->set_pattern(LOG_PATTERN);
  console_sink->set_formatter(std::move(formatter));

  spdlog::init_thread_pool(5, 1);

  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/my_log.txt", true);

  Log = spdlog::async_logger("core",
                             {console_sink, file_sink},
                             spdlog::thread_pool(),
                             spdlog::async_overflow_policy::block);
#ifdef _DEBUG
  Log.set_level(debugLevel);
#else
  Log.set_level(releaseLevel);
#endif
}
