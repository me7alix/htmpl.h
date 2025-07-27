#define HTMPL_IMPLEMENTATION
#include "../htmpl.h"

int main() {
	compile_template(
		"./demo/templates/index.tmpl",	// input file
		"./demo/templates/index.h",		// output file
		"index_tmpl",					// template name
		"name, cnt");					// template args

	return 0;
}
