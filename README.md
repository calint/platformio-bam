| . | . |
| - | - |
| <video width="100%" height="100%" controls src="https://github.com/calint/platformio-bam/assets/1920811/51fba3c0-08a6-4c6c-8f55-ef64c6cfcafe"></video> | <video width="100%" height="100%" controls src="https://github.com/calint/platformio-bam/assets/1920811/c6bc8561-14d7-4e2f-bc48-a28126c2fc9c"></video> |

## intention
* developing using arduino framework with visual code and platformio
* exploring the devices by developing a toy game
* developing a platform-independent toy game engine featuring:
  - smooth scrolling tile map
  - sprites in layers with pixel precision on-screen collision detection
  - intuitive definition of game objects and logic
  - decent performance

## device ESP32-2432S028R

a.k.a. cheap-yellow-display (CYD)

* display: 240 x 320
* performance: ~30 frames per second

* [purchased at](https://www.aliexpress.com/item/1005004502250619.html)
* [community](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)

## device JC4827W543R

a.k.a. cheap-black-device (CBD)

* display: 272 x 480
* performance: ~45 frames per second

* [purchased at](https://www.aliexpress.com/item/1005006729377800.html)
* [manufacturer](https://www.guition.com)
* [documentation](https://github.com/lsdlsd88/JC4827W543)
* [download](http://pan.jczn1688.com/s/zyojx8)
* [community](https://discord.com/channels/630078152038809611/1199730744424153109)

## development environment
* Visual Code 1.89.1
* PlatformIO 6.1.15
* Espressif 32 (6.6.0) > Espressif ESP32 Dev Module
* esp-idf 4.4 patch 6
* packages:
  - framework-arduinoespressif32 @ 3.20014.231204 (2.0.14) 
  - tool-esptoolpy @ 1.40501.0 (4.5.1) 
  - toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5
* dependencies:
  - https://github.com/Bodmer/TFT_eSPI/releases/tag/V2.5.43
  - https://github.com/PaulStoffregen/XPT2046_Touchscreen/releases/tag/v1.4
  - https://github.com/TheNitek/XPT2046_Bitbang_Arduino_Library/releases/tag/v2.0.1

