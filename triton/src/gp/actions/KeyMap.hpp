#pragma once

#include "Inputs.hpp"

namespace tr::gp {
   inline const static std::unordered_map<int, Key> keyMap = {
       {GLFW_KEY_COMMA, Key::Comma},
       {GLFW_KEY_DOWN, Key::Down},
       {GLFW_KEY_ENTER, Key::Enter},
       {GLFW_KEY_EQUAL, Key::Equal},
       {GLFW_KEY_ESCAPE, Key::Escape},
       {GLFW_KEY_HOME, Key::Home},
       {GLFW_KEY_PAGE_UP, Key::PageUp},
       {GLFW_KEY_DELETE, Key::Delete},

       {GLFW_KEY_END, Key::End},
       {GLFW_KEY_PAGE_DOWN, Key::PageDown},

       {GLFW_KEY_NUM_LOCK, Key::NumLock},
       {GLFW_KEY_KP_EQUAL, Key::KpEqual},
       {GLFW_KEY_KP_DIVIDE, Key::KpDivide},
       {GLFW_KEY_KP_MULTIPLY, Key::KpMultiply},
       {GLFW_KEY_KP_0, Key::Kp0},
       {GLFW_KEY_KP_1, Key::Kp1},
       {GLFW_KEY_KP_2, Key::Kp2},
       {GLFW_KEY_KP_3, Key::Kp3},
       {GLFW_KEY_KP_4, Key::Kp4},
       {GLFW_KEY_KP_5, Key::Kp5},
       {GLFW_KEY_KP_6, Key::Kp6},
       {GLFW_KEY_KP_7, Key::Kp7},
       {GLFW_KEY_KP_8, Key::Kp8},
       {GLFW_KEY_KP_9, Key::Kp9},
       {GLFW_KEY_KP_SUBTRACT, Key::KpSubtract},
       {GLFW_KEY_KP_ADD, Key::KpAdd},
       {GLFW_KEY_KP_ENTER, Key::KpEnter},

       {GLFW_KEY_GRAVE_ACCENT, Key::Tilde},
       {GLFW_KEY_CAPS_LOCK, Key::CapsLock},

       {GLFW_KEY_LEFT, Key::Left},
       {GLFW_KEY_LEFT_ALT, Key::LeftAlt},
       {GLFW_KEY_LEFT_BRACKET, Key::LeftBracket},
       {GLFW_KEY_LEFT_SHIFT, Key::LeftShift},
       {GLFW_KEY_LEFT_CONTROL, Key::LeftControl},
       {GLFW_KEY_LEFT_SUPER, Key::LeftSuper},
       {GLFW_KEY_MINUS, Key::Minus},
       {GLFW_KEY_RIGHT, Key::Right},
       {GLFW_KEY_RIGHT_ALT, Key::RightAlt},
       {GLFW_KEY_RIGHT_BRACKET, Key::RightBracket},
       {GLFW_KEY_RIGHT_CONTROL, Key::RightControl},
       {GLFW_KEY_RIGHT_SUPER, Key::RightSuper},
       {GLFW_KEY_SEMICOLON, Key::Semicolon},
       {GLFW_KEY_SPACE, Key::Space},
       {GLFW_KEY_TAB, Key::Tab},
       {GLFW_KEY_UP, Key::Up},
       {GLFW_KEY_F1, Key::F1},
       {GLFW_KEY_F2, Key::F2},
       {GLFW_KEY_F3, Key::F3},
       {GLFW_KEY_F4, Key::F4},
       {GLFW_KEY_F5, Key::F5},
       {GLFW_KEY_F6, Key::F6},
       {GLFW_KEY_F7, Key::F7},
       {GLFW_KEY_F8, Key::F8},
       {GLFW_KEY_F9, Key::F9},
       {GLFW_KEY_F10, Key::F10},
       {GLFW_KEY_F11, Key::F11},
       {GLFW_KEY_F12, Key::F12},
       {GLFW_KEY_F13, Key::F13},
       {GLFW_KEY_F14, Key::F14},
       {GLFW_KEY_F15, Key::F15},
       {GLFW_KEY_F16, Key::F16},
       {GLFW_KEY_F17, Key::F17},
       {GLFW_KEY_F18, Key::F18},
       {GLFW_KEY_F19, Key::F19},
       {GLFW_KEY_0, Key::D0},
       {GLFW_KEY_1, Key::D1},
       {GLFW_KEY_2, Key::D2},
       {GLFW_KEY_3, Key::D3},
       {GLFW_KEY_4, Key::D4},
       {GLFW_KEY_5, Key::D5},
       {GLFW_KEY_6, Key::D6},
       {GLFW_KEY_7, Key::D7},
       {GLFW_KEY_8, Key::D8},
       {GLFW_KEY_9, Key::D9},
       {GLFW_KEY_A, Key::A},
       {GLFW_KEY_B, Key::B},
       {GLFW_KEY_C, Key::C},
       {GLFW_KEY_D, Key::D},
       {GLFW_KEY_E, Key::E},
       {GLFW_KEY_F, Key::F},
       {GLFW_KEY_G, Key::G},
       {GLFW_KEY_H, Key::H},
       {GLFW_KEY_I, Key::I},
       {GLFW_KEY_J, Key::J},
       {GLFW_KEY_K, Key::K},
       {GLFW_KEY_L, Key::L},
       {GLFW_KEY_M, Key::M},
       {GLFW_KEY_N, Key::N},
       {GLFW_KEY_O, Key::O},
       {GLFW_KEY_P, Key::P},
       {GLFW_KEY_Q, Key::Q},
       {GLFW_KEY_R, Key::R},
       {GLFW_KEY_S, Key::S},
       {GLFW_KEY_T, Key::T},
       {GLFW_KEY_U, Key::U},
       {GLFW_KEY_V, Key::V},
       {GLFW_KEY_W, Key::W},
       {GLFW_KEY_X, Key::X},
       {GLFW_KEY_Y, Key::Y},
       {GLFW_KEY_Z, Key::Z}};
};