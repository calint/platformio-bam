#/bin/sh
set -e
cd $(dirname "$0")

DEV=ESP32_2432S028R

cd ..
pio run -e $DEV -t upload
pio device monitor -e $DEV
