# anatomy of a game object

* `game_object` extends `object` defined in `engine.hpp`
* contains attributes and behavior common to most objects
* provides overridable functions for custom logic at every step in the game loop

## attributes

### related to run time information
* object class: `cls` is mandatory to initiate a game object and is defined in `defs.hpp` by game code, where each game object class has an entry

### related to position and motion
* position: `x`, `y`
* velocity: `dx`, `dy`
* acceleration: `ddx`, `ddy`

### related to display
* sprite: `spr`

### related to collisions
* health: `health`
* damage inflicted on collision: `damage`
* engine performs collision detection between sprites on screen if a bitwise AND operation involving `col_bits` from an object and `col_mask` from another object is non-zero
* example:
  - if `col_mask` of object A bitwise AND with `col_bits` of object B is non-zero then object A `col_with` pointer is set to object B
  - same procedure is done with A and B swapped
* the definition of bits and their meaning is custom depending on the game
* example:
  - bit 1 - _'enemy fire'_ - meaning that all classes representing _'enemy fire'_ enable bit 1 in `col_bits`
  - hero `col_mask` would enable bit 1 to get notified when collision with any _'enemy fire'_ object occurs
* this scheme enables:
  - objects to collide with each other without triggering collision detection, such as enemy ships rendered overlapping each other
  - allows to react to collisions with a set of object classes such as _'enemy fire'_ simplifying the design

## overridable functions

### constructor
* base constructor sets mandatory `cls` to provide run time information
  - object classes are defined in `enum object_class` in `defs.hpp`
* user code must allocate and initiate sprite `spr`
  - set `spr->obj` to current object
  - set `spr->img` to image data, usually defined in `sprite_imgs[...]`
* object may be composed of several sprites
  - declare additional sprite pointers as class attributes
  - initiate in the same manner as `spr`

### destructor
* object deallocates the default sprite `spr`
* user code might do additional clean up such as deallocating additional sprites

### pre_render
* game loop calls `pre_render` on allocated objects before rendering the sprites
* default implementation sets sprite screen position using object position
* objects composed of several sprites override this function to set screen position on the additional sprites

### update
* game loop calls `update` on allocated objects after the frame has been rendered and collisions detected
* default implementation is:
  - if collision occurred call `on_collision`
  - update position and motion attributes
* user code might implement custom collision handling
  - if `col_with` is not `nullptr`, handle collision and then set it to `nullptr`
* return `true` if object has died and should be deallocated by the engine

### on_collision
* called from `update` if game object is in collision
* returns `true` if object has died
* default implementation is to reduce `health` with the `damage` caused by the colliding object
* if `health` is zero or less then calls `on_death_by_collision` and returns `true`

### on_death_by_collision
* called from `on_collision` if game object has died due to collision damage

## examples
* `ship1.hpp` basic object with typical implementation
* `ship2.hpp` ad-hoc implementation of animated sprite
* `hero.hpp` composed of several sprites, spawns objects
* `ufo2.hpp` 2 x 2 sprites using helper class, spawns objects
