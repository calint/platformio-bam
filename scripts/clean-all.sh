#/bin/sh
set -e
cd $(dirname "$0")

cd ..
pio run -t clean
