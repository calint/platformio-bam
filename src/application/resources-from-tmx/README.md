# resources from tmx files

## edit resources

* use 'Tiled Map Editor 1.11.2' from <https://www.mapeditor.org> to edit
  `project.tiled-project` and generate file `map.tmx`

## export resources

* script `update.sh` overwrites files in `../resources/`
  * `tile_map.hpp`
  * `tile_map_flags.hpp`
* __note__: `update.sh` generates a new line every 30'th row by default or
  specified as first argument, e.g., `update.sh 60`
