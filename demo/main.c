#define TMPL_IMPLEMENTATION
#include "../htmpl.h"
#include "templates/index.h"

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
