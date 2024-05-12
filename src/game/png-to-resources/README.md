# resources from png files

## editing resources
* edit gimp files "sprites.xcf" and "tiles.xcf"
* suggestion is to create a layer for each sprite / tile
* "sprites.xcf" and "tiles.xcf" can use different palettes
* "sprites.xcf" background color is index 0 and represents transparent pixel

## exporting to png
* enable all layers

![layers](readme-1.png)

* File -> Export As...

![export to png](readme-2.png)

## extracting resources
script `extract.sh` will overwrite files in `game/resources/`

if necessary, change size of tiles and sprites in `extract.sh`

note. check that transparency pixel is index 0

## current resources
### tiles
![tiles](tiles.png)

### sprites
![sprites](sprites.png)
