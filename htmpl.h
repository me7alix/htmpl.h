#ifndef HTMPL_H
#define HTMPL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <dlfcn.h>

#define tmpl_load(lib, name) ((name) = dlsym(lib, #name))
#define HTML(format, ...) { \
	size_t len = snprintf(NULL, 0, format, ##__VA_ARGS__); \
	htmpl_sb_ensure_capacity(&buf, len); \
	sprintf(buf.str, format, ##__VA_ARGS__); \
	htmpl_sb_append_str(&l_html, buf.str); \
}

typedef struct {
	char   *str;
	size_t	cnt;
	size_t	cap;
} HTMPL_StringBuilder;

HTMPL_StringBuilder htmpl_sb_create(size_t cap);
void htmpl_sb_ensure_capacity(HTMPL_StringBuilder *sb, size_t extra);
void htmpl_sb_append_str(HTMPL_StringBuilder *sb, const char *s);
void htmpl_sb_append_char(HTMPL_StringBuilder *sb, char ch);
void htmpl_sb_reset(HTMPL_StringBuilder *sb);
void htmpl_sb_destroy(HTMPL_StringBuilder *sb);

char *file_read(const char *filepath);
void file_write(const char *filepath, char *str);

void *tmpls_load(const char *libfile);
HTMPL_StringBuilder tmpls_builder_create();
void tmpls_builder_write(HTMPL_StringBuilder *sb, const char *filepath);
void tmpls_builder_destroy(HTMPL_StringBuilder *sb);
void tmpls_builder_compile_template(
		HTMPL_StringBuilder *tmpls_builder,
		const char *input_file
		);

#endif // HTMPL_H

#ifdef HTMPL_IMPLEMENTATION

HTMPL_StringBuilder htmpl_sb_create(size_t cap) {
	HTMPL_StringBuilder sb;
	sb.str = (char *) malloc(cap);
	sb.cnt = 0;
	sb.cap = cap;
	if (sb.str) sb.str[0] = '\0';
	return sb;
}

void htmpl_sb_ensure_capacity(HTMPL_StringBuilder *sb, size_t extra) {
	size_t required = sb->cnt + extra + 1;
	if (required <= sb->cap) return;
	while (sb->cap < required) {
		sb->cap *= 2;
	}
	sb->str = (char *) realloc(sb->str, sb->cap);
}

void htmpl_sb_append_str(HTMPL_StringBuilder *sb, const char *s) {
	size_t len = strlen(s);
	htmpl_sb_ensure_capacity(sb, len);
	memcpy(sb->str + sb->cnt, s, len);
	sb->cnt += len;
	sb->str[sb->cnt] = '\0';
}

void htmpl_sb_append_char(HTMPL_StringBuilder *sb, char ch) {
	htmpl_sb_ensure_capacity(sb, 1);
	sb->str[sb->cnt++] = ch;
	sb->str[sb->cnt] = '\0';
}

void htmpl_sb_reset(HTMPL_StringBuilder *sb) {
	sb->cnt = 0;
	if (sb->str) sb->str[0] = '\0';
}

void htmpl_sb_destroy(HTMPL_StringBuilder *sb) {
	free(sb->str);
	sb->str = NULL;
	sb->cnt = sb->cap = 0;
}


char *file_read(const char *filepath) {
	FILE *f = fopen(filepath, "rb");
	if (!f) return NULL;

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);

	char *buffer = (char *) malloc(size + 1);
	if (!buffer) {
		fclose(f);
		return NULL;
	}

	fread(buffer, 1, size, f);
	buffer[size] = '\0';

	fclose(f);
	return buffer;
}

void file_write(const char *filepath, char *str) {
	FILE *f = fopen(filepath, "wb");
	if (!f) return;

	fwrite(str, 1, strlen(str), f);
	fclose(f);
}

void tmpls_builder_compile_template(
	HTMPL_StringBuilder *tmpls_builder,
	const char *input_file
) {
	HTMPL_StringBuilder compressed_html = htmpl_sb_create(128);
	char *file_str = file_read(input_file);
	char *file_str_p = file_str;
	if (file_str == NULL) {
		fprintf(stderr, "Reading file error\n");
		exit(1);
	}

	HTMPL_StringBuilder tmpl_name = htmpl_sb_create(16);
	HTMPL_StringBuilder tmpl_args = htmpl_sb_create(32);

	bool br_fl = false;
	for (; *file_str != ')'; file_str++) {
		if (*file_str == '(') br_fl = true;
		else if (!br_fl) {
			htmpl_sb_append_char(&tmpl_name, *file_str);
		} else {
			htmpl_sb_append_char(&tmpl_args, *file_str);
		}
	}

	file_str++;

	bool ln_flag = false;
	for (; *file_str != '\0'; file_str++) {
		if (!(*file_str == '\t' || *file_str == ' ')) ln_flag = false;
		if (*file_str == '\n') ln_flag = true;
		if (!ln_flag) {
			htmpl_sb_append_char(&compressed_html, *file_str);
		}
	}

	free(file_str_p);

	char *tmpl_str = compressed_html.str;

	HTMPL_StringBuilder tmpl = htmpl_sb_create(128);

	htmpl_sb_append_str(&tmpl, "char *");
	htmpl_sb_append_str(&tmpl, tmpl_name.str);
	htmpl_sb_append_str(&tmpl, "(");
	htmpl_sb_append_str(&tmpl, tmpl_args.str);
	htmpl_sb_append_str(&tmpl, ") {");
	htmpl_sb_append_str(&tmpl, "HTMPL_StringBuilder buf = htmpl_sb_create(128),");
	htmpl_sb_append_str(&tmpl, "l_html = htmpl_sb_create(128);");

	bool c_code = false;
	bool bquotes = false;
	int br_cnt = 0;

	HTMPL_StringBuilder code = htmpl_sb_create(64);
	HTMPL_StringBuilder html = htmpl_sb_create(64);

	while (*tmpl_str != '\0') {
		if (c_code && *tmpl_str != '\n') {
			if (*tmpl_str == '\\' && *(tmpl_str + 1) == '`') {
				htmpl_sb_append_char(&code, '`');
				tmpl_str++;
			} else if (*tmpl_str == '`') {
				bquotes = !bquotes;
				htmpl_sb_append_char(&code, '"');
			} else {
				if (bquotes && *tmpl_str == '"') {
					htmpl_sb_append_char(&code, '\\');
					htmpl_sb_append_char(&code, '"');
				} else htmpl_sb_append_char(&code, *tmpl_str);
			}
		}

		switch (*tmpl_str) {
			case '$':
				c_code = true;
				br_cnt = 0;

				htmpl_sb_append_str(&tmpl, "htmpl_sb_append_str(&l_html, \"");
				htmpl_sb_append_str(&tmpl, html.str);	
				htmpl_sb_append_str(&tmpl, "\");");
				htmpl_sb_reset(&html);	
				break;

			case '{':
				br_cnt++;
				break;

			case '}':
				br_cnt--;
				break;
		}

		if (!c_code) {
			switch (*tmpl_str) {
				case '\"': case '\\':
					htmpl_sb_append_char(&html, '\\');
					htmpl_sb_append_char(&html, *tmpl_str);
					break;

				case '\n':
					break;

				default:
					htmpl_sb_append_char(&html, *tmpl_str);
					break;
			}
		}

		if (c_code && *tmpl_str == '}' && br_cnt == 0) {
			c_code = false;
			htmpl_sb_append_str(&tmpl, code.str);
			htmpl_sb_reset(&code);
		}

		tmpl_str++;
	}

	if (!c_code) {
		htmpl_sb_append_str(&tmpl, "htmpl_sb_append_str(&l_html, \"");
		htmpl_sb_append_str(&tmpl, html.str);	
		htmpl_sb_append_str(&tmpl, "\");");
		htmpl_sb_reset(&html);
	}

	htmpl_sb_append_str(&tmpl, "htmpl_sb_destroy(&buf);");
	htmpl_sb_append_str(&tmpl, "return l_html.str;");
	htmpl_sb_append_str(&tmpl, "}");

	htmpl_sb_destroy(&code);
	htmpl_sb_destroy(&html);

	//file_write(output_file, tmpl.str);
	htmpl_sb_append_str(tmpls_builder, tmpl.str);
	htmpl_sb_destroy(&tmpl);
}

HTMPL_StringBuilder tmpls_builder_create() {
	HTMPL_StringBuilder tmpls_builder = htmpl_sb_create(2048);
	htmpl_sb_append_str(&tmpls_builder, "#include <stdlib.h>\n");
	htmpl_sb_append_str(&tmpls_builder, "#include <stdio.h>\n");
	htmpl_sb_append_str(&tmpls_builder, "#include <string.h>\n");
	htmpl_sb_append_str(&tmpls_builder, "#include <stdbool.h>\n");
	htmpl_sb_append_str(&tmpls_builder, "#include <stdint.h>\n");
	htmpl_sb_append_str(&tmpls_builder, "#define HTML(format, ...) { \
			size_t len = snprintf(NULL, 0, format, ##__VA_ARGS__); \
			htmpl_sb_ensure_capacity(&buf, len); \
			sprintf(buf.str, format, ##__VA_ARGS__); \
			htmpl_sb_append_str(&l_html, buf.str); \
			}\n");
htmpl_sb_append_str(&tmpls_builder, "typedef struct {char *str;size_t cnt, cap;} HTMPL_StringBuilder;");
htmpl_sb_append_str(&tmpls_builder, "HTMPL_StringBuilder htmpl_sb_create(size_t cap) {HTMPL_StringBuilder sb;sb.str = (char *) malloc(cap);sb.cnt = 0;sb.cap = cap;if (sb.str) sb.str[0] = '\\0';return sb;}");
htmpl_sb_append_str(&tmpls_builder, "void htmpl_sb_ensure_capacity(HTMPL_StringBuilder *sb, size_t extra) {size_t required = sb->cnt + extra + 1;if (required <= sb->cap) return;while (sb->cap < required) {sb->cap *= 2;}sb->str = (char *) realloc(sb->str, sb->cap);}");
htmpl_sb_append_str(&tmpls_builder, "void htmpl_sb_append_str(HTMPL_StringBuilder *sb, const char *s) {size_t len = strlen(s);htmpl_sb_ensure_capacity(sb, len);memcpy(sb->str + sb->cnt, s, len);sb->cnt += len;sb->str[sb->cnt] = '\\0';}");
htmpl_sb_append_str(&tmpls_builder, "void htmpl_sb_reset(HTMPL_StringBuilder *sb) {sb->cnt = 0;if (sb->str) sb->str[0] = '\\0';}");
htmpl_sb_append_str(&tmpls_builder, "void htmpl_sb_destroy(HTMPL_StringBuilder *sb) {free(sb->str);sb->str = NULL;sb->cnt = sb->cap = 0;}");
return tmpls_builder;
}

void tmpls_builder_write(HTMPL_StringBuilder *sb, const char *filepath) {
	file_write(filepath, sb->str);
}

void tmpls_builder_destroy(HTMPL_StringBuilder *sb) {
	htmpl_sb_destroy(sb);
}

void *tmpls_load(const char *libfile) {
	void *lib = dlopen(libfile, RTLD_LAZY | RTLD_GLOBAL);
	if (!lib) {
		fprintf(stderr, "dlopen error: %s\n", dlerror());
		exit(1);
	}

	return lib;
}

#endif // HTMPL_IMPLEMENTATION
