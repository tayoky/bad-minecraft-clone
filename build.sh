set -e

cc -pedantic -Wall -Werror -c main.c -o bin/main.o -g
cc -pedantic -Wall -Werror -c render.c -o bin/render.o -g
cc bin/main.o bin/render.o -o out -g -fno-pie -no-pie -lraylib
