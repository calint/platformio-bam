# bam game platform

table of contents:
* `main.cpp` platform-dependent code for booting and rendering
* `platform.hpp` platform-dependent constants used by main, engine and game
* `device.hpp` abstract device definition
* `devices/*` device implementations
* `engine.hpp` platform-independent game engine
* `game/*` platform-independent game implementation using `engine.hpp`
