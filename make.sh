#!/bin/zsh
set -x
set -e
# gcc -Wall -Wextra -pedantic -std=c99 -O0 -ggdb build.c -o travis.x &&
# g++ -Wall -Wextra -pedantic -O0 -ggdb convert.cpp -o convert.x
# gcc -O0 -ggdb build.c -o travis.x
# g++ -O0 -ggdb convert.cpp -o convert.x
#./convert.x /yotta/pbwt/durbin.txt >! /yotta/pbwt/travisformat.txt

gcc -Wall -Wextra -pedantic -O3 build.c -o travis.x
g++ -Wall -Wextra -pedantic -O3 convert.cpp -o convert.x
g++ -Wall -Wextra -pedantic -O3 togrammar.cpp -o togrammar.x

gcc -O0 -ggdb build.c -o travis.x
