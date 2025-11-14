#/bin/sh
set -e
cd $(dirname "$0")

DEV=JC4827W543C

cd ..
pio run -e $DEV -t upload
pio device monitor -e $DEV
