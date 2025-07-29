#define HTMPL_IMPLEMENTATION
#include "../htmpl.h"

int main() {
	StringBuilder tb = tmpls_builder_create();
	tmpls_builder_compile_template(&tb, "./demo/templates/index.htmpl");
	tmpls_builder_write(&tb, "./build/tmpls.c");
	tmpls_builder_destroy(&tb);
	return 0;
}
