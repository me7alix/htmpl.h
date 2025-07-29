#define HTMPL_IMPLEMENTATION
#include "../htmpl.h"

char *(*index_tmpl)(char *name, int cnt);

int main() {
	void *tmpls = tmpls_load("./build/tmpls.so");
	TMPL_LOAD(tmpls, index_tmpl);
	
	char name[16];
	int cnt;

	printf("Enter your name: "); scanf("%s", name);
	printf("Enter number of items: "); scanf("%i", &cnt);

	char *html = index_tmpl(name, cnt);
	file_write("index.html", html);
	free(html);
	return 0;
}
