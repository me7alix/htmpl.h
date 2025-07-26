mkdir -p build
cc -Wall -std=c99 demo/templates.c -o build/templates && ./build/templates
cc -Wall -std=c99 demo/main.c -o build/main && ./build/main
