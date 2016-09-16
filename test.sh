#!/bin/bash
# test the ppmrw program

make clean && make &&
echo testing 3 to 3 &&
./ppmrw 3 data/test_ascii.ppm out.ppm &&
echo testing 3 to 6 &&
./ppmrw 6 data/test_ascii.ppm out.ppm &&
echo testing 6 to 3 &&
./ppmrw 3 data/test_raw.ppm out.ppm &&
echo testing 6 to 6 &&
./ppmrw 6 data/test_raw.ppm out.ppm &&
echo done
