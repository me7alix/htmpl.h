mkdir -p build
cc -Wall -std=c99 demo/tmpls.c -o build/tmpls && ./build/tmpls
cc -Wall -std=c99 demo/main.c -o build/main && ./build/main
