#pragma once

namespace tr {
enum class ActionType : uint8_t {
  MoveForward = 0,
  MoveBackward,
  StrafeLeft,
  StrafeRight,
  LookHorizontal,
  LookVertical,
  SelectionForward,
  SelectionBack,
  Ok,
  Cancel
};

enum class StateType : uint8_t {
  Action = 0,
  State,
  Range
};

struct Action {
  ActionType actionType{};
  StateType stateType{};
  std::variant<bool, float> value{};
};
}

// Custom fmt formatter for ActionType
template <>
struct std::formatter<tr::ActionType> : std::formatter<std::string> {
  auto format(tr::ActionType actionType, std::format_context& ctx) const {
    std::string name;
    switch (actionType) {
      case tr::ActionType::MoveForward:
        name = "MoveForward";
        break;
      case tr::ActionType::MoveBackward:
        name = "MoveBackward";
        break;
      case tr::ActionType::StrafeLeft:
        name = "StrafeLeft";
        break;
      case tr::ActionType::StrafeRight:
        name = "StrafeRight";
        break;
      case tr::ActionType::LookHorizontal:
        name = "LookHorizontal";
        break;
      case tr::ActionType::LookVertical:
        name = "LookVertical";
        break;
      case tr::ActionType::SelectionForward:
        name = "SelectionForward";
        break;
      case tr::ActionType::SelectionBack:
        name = "SelectionBack";
        break;
      case tr::ActionType::Ok:
        name = "Ok";
        break;
      case tr::ActionType::Cancel:
        name = "Cancel";
        break;
      default:
        name = "Unknown";
        break;
    }
    return std::formatter<std::string>::format(name, ctx);
  }
};

// Custom fmt formatter for StateType
template <>
struct std::formatter<tr::StateType> : std::formatter<std::string_view> {
  auto format(tr::StateType stateType, std::format_context& ctx) const {
    std::string_view name;
    switch (stateType) {
      case tr::StateType::Action:
        name = "Action";
        break;
      case tr::StateType::State:
        name = "State";
        break;
      case tr::StateType::Range:
        name = "Range";
        break;
    }
    return std::formatter<std::string_view>::format(name, ctx);
  }
};

// Custom fmt formatter for Action
template <>
struct std::formatter<tr::Action> : std::formatter<std::string> {
  auto format(const tr::Action& action, std::format_context& ctx) const {
    std::string valueStr = std::visit(
        [](const auto& value) -> std::string {
          if constexpr (std::is_same_v<decltype(value), bool>) {
            return value ? "true" : "false";
          } else if constexpr (std::is_same_v<decltype(value), float>) {
            return std::format("{:.2f}", value);
          } else {
            return "unknown";
          }
        },
        action.value);

    return std::format_to(ctx.out(),
                          "Action {{ actionType: {}, stateType: {}, value: {} }}",
                          action.actionType,
                          action.stateType,
                          valueStr);
  }
};
