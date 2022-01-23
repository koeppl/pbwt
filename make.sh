#!/bin/zsh
set -x
set -e
# gcc -Wall -Wextra -pedantic -std=c99 -O0 -ggdb build.c -o travis.x &&
# g++ -Wall -Wextra -pedantic -O0 -ggdb convert.cpp -o convert.x
# gcc -O0 -ggdb build.c -o travis.x
# g++ -O0 -ggdb convert.cpp -o convert.x
gcc -O3 build.c -o travis.x
g++ -O3 convert.cpp -o convert.x
#./convert.x /yotta/pbwt/durbin.txt >! /yotta/pbwt/travisformat.txt
