//
// Created by Marcelo Lima on 17/06/2018.
//

#pragma once

#include <string>
#include <optional>

/**
 * Possible actions to be taken given a certain packet
 */
struct Action {
  enum ActionType {
    DO_NOTHING,
    STORE,
    REMOVE,
    FILTER
  };

  const ActionType action;
  const std::optional<std::string> key;
  const std::optional<std::string> value;

  Action()
      : action{DO_NOTHING},
        key{},
        value{} {}

  Action(ActionType action, std::optional<std::string> && key, std::optional<std::string> && value)
      : action{action},
        key{key},
        value{value} {}
};

