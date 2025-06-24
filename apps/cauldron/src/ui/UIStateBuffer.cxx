#include "UIStateBuffer.hpp"

namespace ed {

auto UIStateBuffer::getStates([[maybe_unused]] tr::Timestamp t) -> std::optional<UIState> {
}

auto UIStateBuffer::pushState([[maybe_unused]] const UIState& newState,
                              [[maybe_unused]] tr::Timestamp t) -> void {
}

}
