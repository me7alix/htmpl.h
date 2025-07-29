mkdir -p build
cc -Wall -std=c99 demo/tmpls.c -o build/tmpls && ./build/tmpls
cc -Wall -std=c99 -fPIC -shared build/tmpls.c -o ./build/tmpls.so
cc -Wall -std=c99 demo/main.c -o build/main -ldl && ./build/main
