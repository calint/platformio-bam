#pragma once
// first include engine
#include "../../engine.hpp"
// then objects
#include "game_object.hpp"

class dummy final : public game_object {
public:
  dummy() : game_object{dummy_cls} {}

  auto update() -> bool override {
    if (game_object::update()) {
      return true;
    }
    if (x >= display_width) {
      return true;
    }
    return false;
  }
};
