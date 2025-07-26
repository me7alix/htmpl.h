#ifndef _HTMPL_H_ 
#define _HTMPL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define HTML(format, ...) { \
	size_t len = snprintf(NULL, 0, format, ##__VA_ARGS__); \
	sb_ensure_capacity(&buf, len); \
	sprintf(buf.str, format, ##__VA_ARGS__); \
	sb_append_str(&l_html, buf.str); \
}

typedef struct {
	char   *str;
	size_t	cnt;
	size_t	cap;
} StringBuilder;


StringBuilder sb_create(size_t cap);
static void sb_ensure_capacity(StringBuilder *sb, size_t extra);
void sb_append_str(StringBuilder *sb, const char *s);
void sb_append_char(StringBuilder *sb, char ch);
void sb_reset(StringBuilder *sb);
void sb_destroy(StringBuilder *sb);

char *file_read(const char *filepath);
void file_write(const char *filepath, char *str);

void compile_template(
	const char *input_file,
	const char *output_file,
	char *tmpl_name,
	char *tmpl_args
);

#endif //_HTMPL_H_

#ifdef TMPL_IMPLEMENTATION

StringBuilder sb_create(size_t cap) {
	StringBuilder sb;
	sb.str = (char *) malloc(cap);
	sb.cnt = 0;
	sb.cap = cap;
	if (sb.str) sb.str[0] = '\0';
	return sb;
}

static void sb_ensure_capacity(StringBuilder *sb, size_t extra) {
	size_t required = sb->cnt + extra + 1;
	if (required <= sb->cap) return;
	while (sb->cap < required) {
		sb->cap *= 2;
	}
	sb->str = (char *) realloc(sb->str, sb->cap);
}

void sb_append_str(StringBuilder *sb, const char *s) {
	size_t len = strlen(s);
	sb_ensure_capacity(sb, len);
	memcpy(sb->str + sb->cnt, s, len);
	sb->cnt += len;
	sb->str[sb->cnt] = '\0';
}

void sb_append_char(StringBuilder *sb, char ch) {
	sb_ensure_capacity(sb, 1);
	sb->str[sb->cnt++] = ch;
	sb->str[sb->cnt] = '\0';
}

void sb_reset(StringBuilder *sb) {
	sb->cnt = 0;
	if (sb->str) sb->str[0] = '\0';
}

void sb_destroy(StringBuilder *sb) {
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

void compile_template(
	const char *input_file,
	const char *output_file,
	char *tmpl_name,
	char *tmpl_args
) {
	StringBuilder compressed_html = sb_create(128);
	char *file_str = file_read(input_file);
	if (file_str == NULL) {
		fprintf(stderr, "Reading file error\n");
		exit(1);
	}

	bool ln_flag = false;
	for (; *file_str != '\0'; file_str++) {
		if (!(*file_str == '\t' || *file_str == ' ')) ln_flag = false;
		if (*file_str == '\n') ln_flag = true;
		if (!ln_flag) {
			sb_append_char(&compressed_html, *file_str);
		}
	}

	//free(file_str);

	char *tmpl_str = compressed_html.str;

	StringBuilder tmpl = sb_create(128);

	sb_append_str(&tmpl, "#define ");
	sb_append_str(&tmpl, tmpl_name);
	sb_append_str(&tmpl, "(");
	sb_append_str(&tmpl, tmpl_args);
	sb_append_str(&tmpl, ") ({");
	sb_append_str(&tmpl, "StringBuilder buf = sb_create(128),");
	sb_append_str(&tmpl, "l_html = sb_create(128);");

	bool c_code = false;
	int br_cnt = 0;

	StringBuilder code = sb_create(64);
	StringBuilder html = sb_create(64);

	while (*tmpl_str != '\0') {
		if (c_code && *tmpl_str != '\n')
			sb_append_char(&code, *tmpl_str);
	
		switch (*tmpl_str) {
			case '$':
				c_code = true;
				br_cnt = 0;

				sb_append_str(&tmpl, "sb_append_str(&l_html, \"");
				sb_append_str(&tmpl, html.str);	
				sb_append_str(&tmpl, "\");");
				sb_reset(&html);	
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
					sb_append_char(&html, '\\');
					sb_append_char(&html, *tmpl_str);
					break;

				case '\n':
					break;

				default:
					sb_append_char(&html, *tmpl_str);
					break;
			}
		}

		if (*tmpl_str == '}' && br_cnt == 0) {
			c_code = false;
			sb_append_str(&tmpl, code.str);
			sb_reset(&code);
		}

		tmpl_str++;
	}

	if (!c_code) {
		sb_append_str(&tmpl, "sb_append_str(&l_html, \"");
		sb_append_str(&tmpl, html.str);	
		sb_append_str(&tmpl, "\");");
		sb_reset(&html);
	}

	sb_append_str(&tmpl, "sb_destroy(&buf);");
	sb_append_str(&tmpl, "l_html.str;");
	sb_append_str(&tmpl, "})");

	sb_destroy(&code);
	sb_destroy(&html);

	file_write(output_file, tmpl.str);
	sb_destroy(&tmpl);
}

#endif // TMPL_IMPLEMENTATION
