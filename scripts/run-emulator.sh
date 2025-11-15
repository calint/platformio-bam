#/bin/sh
set -e
cd $(dirname "$0")

cd ..
emu/make.sh
emu/bam
