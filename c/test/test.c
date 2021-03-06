#include "unicode.h"
#include "hanzi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// #define DEBUG
#include "util.h"

static const int LINE_LEN_MAX = 255;

typedef void (*test_func)(const char *, const char *, Encoding);

static void test_hz2pinyin_codepoint(const char *s, const char *expected_pinyin,
        Encoding enc) {
    size_t n;
    UTF32 *cp;

    if (enc == ENCODING_UTF8)
        cp = hz_utf8_to_utf32(s, &n);
    else {
        printf("Not supported encoding\n");
        return;
    }

    if (!cp) {
        printf("conversion to codepoint failed\n");
        return;
    }
    DPRINTF("hanzi contains %lu characters", n);

    char pinyin[255] = { 0 };
    char *pinyin_start = pinyin;

    DPRINTF("str: %s ", s);
    for (int i = 0; i < n; i++) {
        const char *py = hz2pinyin_codepoint(cp[i]);
        if (!py) {
            printf("No pinyin found for: %s cp: %x\n", s, cp[i]);
            return;
        }
        int len = strlen(py);
        strncpy(pinyin_start, py, len);
        pinyin_start += len;

        DPRINTF("%x %s ", cp[i], py);
    }

    if (strcmp(pinyin, expected_pinyin) != 0) {
        printf("ERROR, str: %s, expected_pinyin: %s, got: %s END\n",
            s, expected_pinyin, pinyin);
    }
    free(cp);
}

static void test_hz2pinyin_string(const char *s, const char *expected_pinyin,
        Encoding enc) {
    char *pinyin = hz2pinyin(s, " ", enc);
    if (!pinyin) {
        printf("ERROR converting hanzi %s to string\n", s);
        exit(1);
    }
    if (strcmp(pinyin, expected_pinyin) != 0) {
        printf("ERROR, str: %s, expected_pinyin: %s, got: %sEND\n",
            s, expected_pinyin, pinyin);
    }
    free(pinyin);
}

static void test(const char *path, Encoding enc, test_func func) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "error opening file\n");
        return;
    }

    while (1) {
        char c[LINE_LEN_MAX];
        char pinyin[LINE_LEN_MAX];

        fgets(c, LINE_LEN_MAX, fp);
        if (feof(fp))
            break;
        fgets(pinyin, LINE_LEN_MAX, fp);

        /* Remove trailing "\n"
         * XXX strlen does not work with UTF-16 encoded byte sequence */
        c[strlen(c) - 1] = '\0';
        pinyin[strlen(pinyin) - 1] = '\0';

        func(c, pinyin, enc);
    }
    fclose(fp);
}

int main(int argc, char* argv[])
{
    test("./utf8-test.txt", ENCODING_UTF8, test_hz2pinyin_codepoint);
    test("./utf8-test-mixed.txt", ENCODING_UTF8, test_hz2pinyin_string);

    /* 0x4E06 has no pinyin */
    const char *pinyin = hz2pinyin_codepoint(0x4E06);
    assert(!pinyin);
    return 0;
}
