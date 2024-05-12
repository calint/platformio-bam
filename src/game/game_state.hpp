#pragma once
// game state used by game objects and 'main.hpp'

// reviewed: 2023-12-11
// reviewed: 2024-05-01

class game_state final {
public:
  bool hero_is_alive = false;
} static game_state{};