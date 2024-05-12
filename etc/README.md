# troubleshooting
device does not show up as '/dev/ttyUSB0' on linux
* service 'brltty' might be the problem
* disable it and related services or uninstall it

# clipboard.txt
* various command lines and notes

# todo.txt
* todo list and suggestions

# platformio.ini
* additional flags
```
;
; compile c++ 23
;
;build_flags = -std=gnu++2a
;build_unflags = -std=gnu++11

;
; flags below generate an excessive number of warnings from included dependencies
;
;build_src_flags = -Wunused-variable -Wuninitialized -Wshadow -Wsign-conversion -Wconversion
;build_src_flags =
;    -Wall -Wextra -Wpedantic
;    -Wunused-variable -Wuninitialized -Wsign-conversion -Wconversion -Wold-style-cast
;    -Wshadow -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wlogical-op
;    -Wmissing-declarations -Woverloaded-virtual -Wredundant-decls -Wsign-promo
;    -Wstrict-null-sentinel -Wswitch-default -Wfloat-equal -Wnoexcept
;    -Wno-unused-function
```
