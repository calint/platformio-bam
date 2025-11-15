#!/bin/bash
set -e

g++ -std=gnu++11 -O0 -g3 -o bam -Wfatal-errors -Werror -Wall -Wextra -Wpedantic \
    $(pkg-config --cflags --libs sdl3) \
    -Wconversion -Wsign-conversion \
    -Wno-unused-parameter \
    -flifetime-dse=1 \
    -DTOUCH_MIN_X=0 -DTOUCH_MAX_X=240 -DTOUCH_MIN_Y=0 -DTOUCH_MAX_Y=320 \
    -DBAM_TIME_STEP_MS=33 \
    main.cpp

# note: `-flifetime-dse=1` fixes o1store::alloc()...inst->alloc_ptr optimization issue
# note: matching g++ c++ standard to platformio
# note: TOUCH_x and BAM_x defines are usually defined in `platformio.ini`
