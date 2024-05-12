## intention
* developing using arduino framework with visual code and platformio
* exploring the devices by developing a toy game
* developing a platform-independent toy game engine featuring:
  - smooth scrolling tile map
  - sprites in layers with pixel precision on-screen collision detection
  - intuitive definition of game objects and logic

## device ESP32-2432S028R

a.k.a. cheap-yellow-display (CYD)

* display: 240 x 320
* performance: ~30 frames per second

* [purchased at](https://www.aliexpress.com/item/1005004502250619.html)
* [community](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)

## device JC4827W543R

a.k.a. cheap-chonky-display (CCD)

* display: 272 x 480
* performance: ~40 frames per second

* [purchased at](https://www.aliexpress.com/item/1005006729377800.html)
* [manufacturer](https://www.guition.com)
* [documentation](https://github.com/lsdlsd88/JC4827W543)
* [download](http://pan.jczn1688.com/s/zyojx8)
* [community](https://discord.com/channels/630078152038809611/1199730744424153109)

## development environment
* Visual Code 1.89.0
* PlatformIO 6.1.15
* Espressif 32 (6.6.0) > Espressif ESP32 Dev Module
* esp-idf 4.4 patch 6
* packages:
  - framework-arduinoespressif32 @ 3.20014.231204 (2.0.14) 
  - tool-esptoolpy @ 1.40501.0 (4.5.1) 
  - toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5
* dependencies:
  - https://github.com/PaulStoffregen/XPT2046_Touchscreen#v1.4
  - https://github.com/Bodmer/TFT_eSPI#V2.5.43

## videos

<video width="240" height="320" src="https://github.com/calint/JC4827W543R/assets/1920811/8b5c0ccf-73d2-4392-b81a-12377bdd7c0d"></video>
