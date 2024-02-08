#pragma once

#include "../events/Key.hpp"
#include "../events/Mouse.hpp"
#include "ActionType.hpp"

namespace Triton::Actions {

   /*
      TODO: Bind Sources to Actions

      Convert all Events into SourceEvents

      Consult current actionset to convert SourceEvents into ActionStates
      have an ActionState contain an ActionType as well as the optional 'value'
   */

   struct Source {
      Source(Events::MouseEvent me) : src{me} {
      }
      Source(Key key) : src(key) {
      }
      std::variant<Events::MouseEvent, Key> src;
   };

   struct SourceEvent : public Source {

      SourceEvent(const Source s) : Source{s} {
      }

      SourceEvent(const Source s, float value) : Source{s}, value{value} {
      }
      std::optional<float> value{};
   };

   struct ActionState {
      ActionState(ActionType aType) : actionType{aType} {
      }

      ActionState(ActionType aType, float value) : actionType(aType), value(value) {
      }
      ActionType actionType;
      std::optional<float> value;
   };

   using ActionStateMap = std::unordered_map<ActionType, ActionState>;

   class ActionSet {
    public:
      ActionSet() = default;
      ActionSet(const ActionSet&) = delete;
      ActionSet(ActionSet&&) = delete;
      ActionSet& operator=(const ActionSet&) = default;
      ActionSet& operator=(ActionSet&&) = delete;

      ~ActionSet() = default;

      void mapKey(Key key, ActionType actionType);
      [[nodiscard]] bool hasMapping(Key key) const;
      [[nodiscard]] ActionType getActionForKey(Key key) const;
      [[nodiscard]] ActionType getActionForMouse(Events::MouseEvent mouse) const;

      [[nodiscard]] ActionType getActionForSourceEvent(const SourceEvent source) const;

      void bindSource(Key key, ActionType actionType);
      void bindSource(Events::MouseEvent mouse, ActionType actionType);
      void bindSource(Source src, ActionType actionType);

    private:
      std::multimap<Key, ActionType> keyMap;
      std::multimap<Events::MouseEvent, ActionType> mouseMap;
   };
};
