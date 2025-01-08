#pragma once

namespace tr {
enum class ActionType : uint32_t {
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

enum class StateType : uint32_t {
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
struct fmt::formatter<tr::ActionType> : fmt::formatter<std::string> {
  auto format(tr::ActionType actionType, fmt::format_context& ctx) {
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
    }
    return fmt::formatter<std::string>::format(name, ctx);
  }
};

// Custom fmt formatter for StateType
template <>
struct fmt::formatter<tr::StateType> : fmt::formatter<std::string> {
  auto format(tr::StateType stateType, fmt::format_context& ctx) {
    std::string name;
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
    return fmt::formatter<std::string>::format(name, ctx);
  }
};

// Custom fmt formatter for Action
template <>
struct fmt::formatter<tr::Action> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  auto format(const tr::Action& action, fmt::format_context& ctx) {
    std::string valueStr = std::visit(
        [](const auto& value) -> std::string {
          if constexpr (std::is_same_v<decltype(value), bool>) {
            return value ? "true" : "false";
          } else if constexpr (std::is_same_v<decltype(value), float>) {
            return fmt::format("{:.2f}", value);
          }
          return "unknown";
        },
        action.value);

    return fmt::format_to(ctx.out(),
                          "Action {{ actionType: {}, stateType: {}, value: {} }}",
                          action.actionType,
                          action.stateType,
                          valueStr);
  }
};
