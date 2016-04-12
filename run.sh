#!/bin/bash
valgrind --tool=callgrind ./luaio ./test/test.lua
for file in callgrind.out.*
do
  mv callgrind.out.* ./valgrind/callgrind.out
done
./valgrind/gprof2dot.py -f callgrind ./valgrind/callgrind.out | dot -Tpng -o report.png
