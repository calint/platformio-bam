#!/bin/bash
set -e
cd $(dirname "$0")

g++ -std=gnu++11 -O3 -g3 -o bam $(pkg-config --cflags --libs sdl3) \
    -Wfatal-errors -Werror -Wall -Wextra -Wpedantic \
    -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wcast-align \
    -Wold-style-cast -Wconversion -Wsign-conversion -Wmisleading-indentation \
    -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast \
    -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough -Wpointer-arith \
    -Wswitch-enum -Wvla -Wsuggest-final-types -Wsuggest-final-methods \
    -Wsuggest-override -Wno-unused-parameter \
    -flifetime-dse=1 \
    -DTOUCH_MIN_X=0 -DTOUCH_MAX_X=240 -DTOUCH_MIN_Y=0 -DTOUCH_MAX_Y=320 \
    -DTFT_WIDTH=240 -DTFT_HEIGHT=320 \
    -DBAM_TIME_STEP_MS=33 \
    main.cpp

# note: `-flifetime-dse=1` fixes o1store::alloc()...inst->alloc_ptr optimization issue
#       see: https://github.com/espressif/crosstool-NG/issues/55
# note: matching g++ c++ standard to platformio
# note: TOUCH_x, TFT_x and BAM_x defines are usually defined in `platformio.ini`
# note: extensive warnings recommendation from amongst other sources:
#       https://github.com/cpp-best-practices/cppbestpractices/
# note: -Wnull-dereference omitted
