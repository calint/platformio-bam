#pragma once
// game state used by game objects and 'application.hpp'

// reviewed: 2023-12-11
// reviewed: 2024-05-01
// reviewed: 2024-05-22

class state final {
  public:
    bool hero_is_alive = false;
} static state{};