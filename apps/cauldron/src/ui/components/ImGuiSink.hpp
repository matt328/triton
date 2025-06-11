#pragma once

template <typename Mutex>
class my_sink final : public spdlog::sinks::base_sink<Mutex> {

public:
  explicit my_sink(const std::function<void(std::string)>& sinkFn) : fn{sinkFn} {
    spdlog::sinks::base_sink<Mutex>::set_pattern("%I:%M:%S %-8l %v%$");
  }

protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    fn(formatted);
  }

  void flush_() override {
    std::cout << std::flush;
  }

private:
  std::function<void(std::string)> fn;
};

using my_sink_mt = my_sink<std::mutex>;
using my_sink_st = my_sink<spdlog::details::null_mutex>;
