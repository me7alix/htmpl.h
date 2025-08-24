#define HTMPL_IMPLEMENTATION
#include "../htmpl.h"

int main() {
	HTMPL_StringBuilder tb = {0};
	tmpls_builder_compile_template(&tb, "./demo/templates/index.htmpl");
	tmpls_builder_write(&tb, "./demo/impls.c");
	tmpls_builder_destroy(&tb);
	return 0;
}
