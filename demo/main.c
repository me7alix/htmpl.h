#define HTMPL_IMPLEMENTATION
#include "../htmpl.h"

//templates definition
char *index_tmpl(char *name, int cnt);

// templates implementation
#include "impls.c"

int main() {
	char name[16];
	int cnt;

	printf("Enter your name: "); scanf("%s", name);
	printf("Enter number of items: "); scanf("%i", &cnt);

	char *html = index_tmpl(name, cnt);
	file_write("index.html", html);
	free(html);
	return 0;
}
