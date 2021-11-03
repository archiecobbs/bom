/*
 * bom - Deals with Unicode byte order marks
 *
 * Copyright (C) 2021 Archie L. Cobbs. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Copyright character
#define COPYRIGHT       "\xc2\xa9"

// Special exit values
#define EX_EXPECT_FAIL      2
#define EX_ILLEGAL_BYTES    3

// Version string
extern const char *const bom_version;

// Command line options that only have long versions
#define FLAG_LIST       (-2)
#define FLAG_PREFER_32  (-3)

#define OPT(_letter, _name, _arg)                                                               \
    {                                                                                           \
        .name=      _name,                                                                      \
        .has_arg=   _arg,                                                                       \
        .flag=      NULL,                                                                       \
        .val=       _letter                                                                     \
    }
static const struct option long_options[] = {
    OPT('d',            "detect",      no_argument),
    OPT('e',            "expect",      required_argument),
    OPT('h',            "help",        no_argument),
    OPT(FLAG_LIST,      "list",        no_argument),
    OPT('l',            "lenient",     no_argument),
    OPT('p',            "print",       required_argument),
    OPT(FLAG_PREFER_32, "prefer32",    no_argument),
    OPT('s',            "strip",       no_argument),
    OPT('u',            "utf8",        no_argument),
    OPT('v',            "version",     no_argument),
    OPT(0, NULL, 0)
};

// Execution modes
#define MODE_STRIP      1
#define MODE_DETECT     2
#define MODE_LIST       3
#define MODE_PRINT      4
#define MODE_HELP       5
#define MODE_VERSION    6

// BOM types
struct bom_type {
    const char      *name;
    const char      *encoding;
    const char      *bytes;
    const int       len;
};
#define BOM_TYPE(_name, _encoding, _bytes)                                                  \
    {                                                                                       \
        .name=      _name,                                                                  \
        .encoding=  _encoding,                                                              \
        .bytes=     _bytes,                                                                 \
        .len=       sizeof(_bytes) - 1                                                      \
    }
static const struct bom_type bom_types[] = {
    BOM_TYPE("NONE",        NULL,       ""),
    BOM_TYPE("UTF-7",       "UTF-7",    "\x2b\x2f\x76"),
    BOM_TYPE("UTF-8",       "UTF-8",    "\xef\xbb\xbf"),
    BOM_TYPE("UTF-16BE",    "UTF-16BE", "\xfe\xff"),
    BOM_TYPE("UTF-16LE",    "UTF-16LE", "\xff\xfe"),
    BOM_TYPE("UTF-32BE",    "UTF-32BE", "\x00\x00\xfe\xff"),
    BOM_TYPE("UTF-32LE",    "UTF-32LE", "\xff\xfe\x00\x00"),
    BOM_TYPE("GB18030",     "GB18030",  "\x84\x31\x95\x33"),
};
#define BOM_TYPE_NONE       0
#define BOM_TYPE_UTF_7      1
#define BOM_TYPE_UTF_8      2
#define BOM_TYPE_UTF_16BE   3
#define BOM_TYPE_UTF_16LE   4
#define BOM_TYPE_UTF_32BE   5
#define BOM_TYPE_UTF_32LE   6
#define BOM_TYPE_GB18030    7
#define BOM_TYPE_MAX        8

// Input buffer
#define BUFFER_SIZE         1024
struct bom_input {
    char    buf[BUFFER_SIZE];
    int     len;
    int     num_complete;
    int     num_finished;
    int     match_state[BOM_TYPE_MAX];
};
#define MATCH_PREFIX        0
#define MATCH_COMPLETE      1
#define MATCH_FAILED        2

// Mode of execution functions
static void bom_detect(FILE *fp, long expect_types, int prefer32);
static void bom_strip(FILE *fp, long expect_types, int lenient, int prefer32, int utf8);
static void bom_list(void);
static void bom_print(int bom_type);

// Helper functions
static int read_bom(FILE *fp, struct bom_input *const input, long expect_types, int prefer32);
static int read_byte(FILE *fp, struct bom_input *input);
static int bom_type_from_name(const char *name);
static void init_bom_input(struct bom_input *const input);
static void set_mode(int *modep, int mode);
static void usage(void);

int
main(int argc, char **argv)
{
    const struct option *opt;
    char optstring[32];
    long expect_types = 0;
    int option_index;
    int bom_type = -1;
    int prefer32 = 0;
    int lenient = 0;
    FILE *fp = NULL;
    int mode = 0;
    int utf8 = 0;
    char *s;
    int ch;

    // Build optstring dynamically
    s = optstring;
    for (opt = long_options; opt->name != NULL; opt++) {
        if (opt->val > 0) {
            *s++ = (char)opt->val;
            if (opt->has_arg)
                *s++ = ':';
        }
    }
    *s = '\0';

    // Parse command line
    while ((ch = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
        switch (ch) {
        case 'd':
            set_mode(&mode, MODE_DETECT);
            break;
        case 'e':
            while ((s = strsep(&optarg, ",")) != NULL) {
                if ((bom_type = bom_type_from_name(s)) >= sizeof(expect_types) * 8)
                    errx(1, "internal error: %s", "too many BOM types");
                expect_types |= (1 << bom_type);
            }
            break;
        case 'h':
            set_mode(&mode, MODE_HELP);
            break;
        case 'l':
            lenient = 1;
            break;
        case 'p':
            bom_type = bom_type_from_name(optarg);
            set_mode(&mode, MODE_PRINT);
            break;
        case 's':
            set_mode(&mode, MODE_STRIP);
            break;
        case 'u':
            utf8 = 1;
            break;
        case 'v':
            set_mode(&mode, MODE_VERSION);
            break;
        case FLAG_PREFER_32:
            prefer32 = 1;
            break;
        case FLAG_LIST:
            set_mode(&mode, MODE_LIST);
            break;
        case '?':
        default:
            usage();
            return 1;
        }
    }
    argv += optind;
    argc -= optind;

    // Parse remainder of command line
    switch (mode) {
    case MODE_STRIP:
    case MODE_DETECT:
        switch (argc) {
        case 0:
            fp = stdin;
            break;
        case 1:
            if (strcmp(argv[0], "-") == 0) {
                fp = stdin;
                break;
            }
            if ((fp = fopen(argv[0], "r")) == NULL)
                err(1, "%s", argv[0]);
            break;
        default:
            usage();
            return 1;
        }
        break;
    default:
        switch (argc) {
        case 0:
            break;
        default:
            usage();
            return 1;
        }
        break;
    }

    // Execute
    switch (mode) {
    case MODE_STRIP:
        bom_strip(fp, expect_types, lenient, prefer32, utf8);
        break;
    case MODE_DETECT:
        bom_detect(fp, expect_types, prefer32);
        break;
    case MODE_LIST:
        bom_list();
        break;
    case MODE_PRINT:
        bom_print(bom_type);
        break;
    case MODE_HELP:
        usage();
        break;
    case MODE_VERSION:
        fprintf(stderr, "bom %s\n", bom_version);
        fprintf(stderr, "Copyright %s Archie L. Cobbs. All rights reserved.\n", COPYRIGHT);
        break;
    default:
        usage();
        return 1;
    }

    // Done
    return 0;
}

static void
bom_detect(FILE *fp, long expect_types, int prefer32)
{
    const struct bom_type *bt;
    struct bom_input input;
    int bom_type;

    // Read BOM
    init_bom_input(&input);
    bom_type = read_bom(fp, &input, expect_types, prefer32);
    bt = &bom_types[bom_type];

    // Print its name
    printf("%s\n", bt->name);
}

#if DEBUG_ICONV_OPS

#define BYTES_PER_ROW   20
static void
debug_buffer(const size_t base, const void *data, size_t len)
{
    size_t offset;
    size_t i;

    if (data == NULL) {
        fprintf(stderr, "    NULL\n");
        return;
    }
    for (offset = 0; offset < len; offset += BYTES_PER_ROW) {
        fprintf(stderr, "%08d: ", (unsigned int)(base + offset));
        for (i = 0; i < BYTES_PER_ROW; i++) {
            const int val = offset + i < len ? *((const char *)data + offset + i) & 0xff : -1;
            if (i == BYTES_PER_ROW / 2)
                fprintf(stderr, " ");
            if (val != -1)
                fprintf(stderr, " %02x", val);
            else
                fprintf(stderr, "   ");
        }
        fprintf(stderr, "  ");
        for (i = 0; i < BYTES_PER_ROW; i++) {
            const int val = offset + i < len ? *((const char *)data + offset + i) & 0xff : -1;
            if (val != -1)
                fprintf(stderr, "%c", isprint(val) ? val : '.');
            else
                fprintf(stderr, " ");
        }
        fprintf(stderr, "\n");
    }
}

#endif  /* DEBUG_ICONV_OPS */

static void
bom_strip(FILE *fp, long expect_types, int lenient, int prefer32, int utf8)
{
    const struct bom_type *bt;
    struct bom_input input;
    char ibuf[BUFFER_SIZE];
    char obuf[BUFFER_SIZE];
    char tocode[32];
    size_t offset;
    iconv_t icd = 0;
    int done = 0;
    int bom_type;
    int ilen;

    // Read BOM
    init_bom_input(&input);
    bom_type = read_bom(fp, &input, expect_types, prefer32);
    bt = &bom_types[bom_type];

    // If BOM type is NONE, then obviously we can't convert to UTF-8
    if (bom_type == BOM_TYPE_NONE)
        utf8 = 0;

    // Initialize iconv conversion engine
    if (utf8) {
        snprintf(tocode, sizeof(tocode), "%s%s", bom_types[BOM_TYPE_UTF_8].encoding, lenient ? "//IGNORE" : "");
        if ((icd = iconv_open(tocode, bt->encoding)) == (iconv_t)-1)
            err(1, "iconv: \"%s\" -> \"%s\"", bt->encoding, tocode);
    }

    // Copy over any bytes we read after the BOM into our input buffer
    ilen = input.len - bt->len;
    memcpy(ibuf, input.buf + bt->len, ilen);
    offset = bt->len;

    // Convert remainder of file
    while (!done) {
        size_t nread;
        size_t nwrit;
        char *iptr;
        char *optr;
        size_t iremain;
        size_t oremain;
        int eof = 0;
        size_t r;

        // Fill the input buffer
        while (ilen < sizeof(ibuf)) {
            if ((nread = fread(ibuf + ilen, 1, sizeof(ibuf) - ilen, fp)) == 0) {
                if (ferror(fp))
                    err(1, "read error");
                eof = 1;
                break;
            }
            ilen += nread;
        }

        // When the input buffer is empty and we couldn't add anything more, this is the last round
        done = ilen == 0;

        // Convert bytes (unless BOM_TYPE_NONE)
        iptr = ibuf;
        optr = obuf;
        iremain = ilen;
        oremain = sizeof(obuf);

        // Convert to UTF-8 or just pass through
        if (utf8) {
#if DEBUG_ICONV_OPS
            fprintf(stderr, "->iconv@%d: ilen=%d\n", (int)offset, (int)ilen);
            debug_buffer(offset, iptr, ilen);
#endif
            r = iconv(icd, !done ? &iptr : NULL, &iremain, &optr, &oremain);
#if DEBUG_ICONV_OPS
            {
                const int errno_save = errno;

                fprintf(stderr, "<-iconv@%d: r=%d errno=%d iptr@%d optr@%d\n",
                  (int)offset, (int)r, errno, (int)(iptr - ibuf), (int)(optr - obuf));
                debug_buffer(offset, obuf, optr - obuf);
                errno = errno_save;
            }
#endif
            if (r == (size_t)-1) {
                switch (errno) {
                case EINVAL:                    // incomplete multi-byte sequence at the end of the input buffer
                    if (!done && !eof)
                        break;
                    // FALLTHROUGH
                case EILSEQ:                    // an invalid byte sequence was detected
                    if (lenient) {
                        iptr += iremain;        // avoid an infinite loop on trailing partial multi-byte sequence
                        iremain = 0;
                        break;
                    }
                    errx(EX_ILLEGAL_BYTES, "invalid %s byte sequence at file offset %lu", bt->name, offset + (iptr - ibuf));
                default:
                    err(1, "iconv");
                }
            }
        } else {                                // behave like iconv() would but just copy the bytes
            memcpy(optr, iptr, ilen);
            if (!done)
                iptr += ilen;
            iremain = 0;
            optr += ilen;
            oremain -= ilen;
        }

        // Update file offset
        offset += ilen - iremain;

        // Shift unprocessed input for next time
        memmove(ibuf, iptr, iremain);
        ilen = iremain;

        // Write output
        oremain = optr - obuf;
        optr = obuf;
        while (oremain > 0 && (nwrit = fwrite(optr, 1, oremain, stdout)) > 0) {
            optr += nwrit;
            oremain -= nwrit;
        }
        if (ferror(stdout))
            err(1, "write error");
    }
    if (fflush(stdout) == EOF)
        err(1, "write error");

    // Close conversion
    if (utf8)
        (void)iconv_close(icd);
}

static void
bom_list(void)
{
    int bom_type;

    for (bom_type = 0; bom_type < BOM_TYPE_MAX; bom_type++) {
        const struct bom_type *const bt = &bom_types[bom_type];

        printf("%s\n", bt->name);
    }
}

static void
bom_print(int bom_type)
{
    const struct bom_type *const bt = &bom_types[bom_type];
    int i;

    for (i = 0; i < bt->len; i++) {
        if (putchar(bt->bytes[i] & 0xff) == EOF)
            err(1, "write error");
    }
}

static int
read_bom(FILE *fp, struct bom_input *const input, long expect_types, int prefer32)
{
    int bom_type;

    // Read bytes until all BOM's are either completely matched or have failed to match
    while (read_byte(fp, input)) {
        if (input->num_finished == BOM_TYPE_MAX)
            break;
    }

    // Handle the UTF-16LE vs. UTF-32LE ambiguity
    if (input->match_state[BOM_TYPE_UTF_16LE] == MATCH_COMPLETE
      && input->match_state[BOM_TYPE_UTF_32LE] == MATCH_COMPLETE) {
        input->match_state[prefer32 ? BOM_TYPE_UTF_16LE : BOM_TYPE_UTF_32LE] = MATCH_FAILED;
        input->num_complete--;
    }

    // At this point there should be BOM_TYPE_NONE and at most one other match
    assert(input->match_state[BOM_TYPE_NONE] == MATCH_COMPLETE);
    switch (input->num_complete) {
    case 1:
        bom_type = BOM_TYPE_NONE;
        break;
    case 2:
        for (bom_type = 0; bom_type < BOM_TYPE_MAX; bom_type++) {
            if (bom_type != BOM_TYPE_NONE && input->match_state[bom_type] == MATCH_COMPLETE)
                break;
        }
        if (bom_type < BOM_TYPE_MAX)
            break;
        // FALLTHROUGH
    default:
        errx(1, "internal error: %s", ">2 BOM type matches");
    }

    // Check expected BOM type
    if (expect_types != 0 && (expect_types & (1 << bom_type)) == 0)
        errx(EX_EXPECT_FAIL, "unexpected BOM type %s", bom_types[bom_type].name);

    // Done
    return bom_type;
}

static int
bom_type_from_name(const char *name)
{
    int bom_type;

    for (bom_type = 0; bom_type < BOM_TYPE_MAX; bom_type++) {
        if (strcmp(bom_types[bom_type].name, name) == 0)
            return bom_type;
    }
    errx(1, "unknown BOM type \"%s\"", name);
}

static int
read_byte(FILE *fp, struct bom_input *const input)
{
    int bom_type;
    int ch;

    // Read next byte
    if ((ch = getc(fp)) == EOF) {
        if (ferror(fp))
            err(1, "read error");
        return 0;
    }

    // Update state
    if (input->len >= sizeof(input->buf))
        errx(1, "internal error: %s", "input buffer overflow");
    for (bom_type = 0; bom_type < BOM_TYPE_MAX; bom_type++) {
        const struct bom_type *const bt = &bom_types[bom_type];

        switch (input->match_state[bom_type]) {
        case MATCH_PREFIX:
            if (bt->bytes[input->len] != (char)ch) {
                input->match_state[bom_type] = MATCH_FAILED;
                input->num_finished++;
            } else if (bt->len == input->len + 1) {
                input->match_state[bom_type] = MATCH_COMPLETE;
                input->num_finished++;
                input->num_complete++;
            }
            break;
        case MATCH_COMPLETE:
        case MATCH_FAILED:
            break;
        default:
            errx(1, "internal error: %s", "invalid match state");
        }
    }
    input->buf[input->len++] = (char)ch;
    return 1;
}

static void
init_bom_input(struct bom_input *const input)
{
    memset(input, 0, sizeof(*input));
    input->match_state[BOM_TYPE_NONE] = MATCH_COMPLETE;
    input->num_complete = 1;
    input->num_finished = 1;
}

static void
set_mode(int *modep, int mode)
{
    if (*modep != 0) {
        usage();
        exit(1);
    }
    *modep = mode;
}

static void
usage(void)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  bom --strip [--expect types] [--lenient] [--prefer32] [--utf8] [file]\n");
    fprintf(stderr, "  bom --detect [--expect types] [--prefer32] [file]\n");
    fprintf(stderr, "  bom --list\n");
    fprintf(stderr, "  bom --print type\n");
    fprintf(stderr, "  bom --help\n");
    fprintf(stderr, "  bom --version\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d, --detect        Report the detected BOM type and exit\n");
    fprintf(stderr, "  -e, --expect types  Expect the specified BOM type(s) (separated by commas)\n");
    fprintf(stderr, "  -h, --help          Output command line usage summary\n");
    fprintf(stderr, "  -l, --lenient       Skip invalid input byte sequences instead of failing\n");
    fprintf(stderr, "      --list          List the supported BOM types\n");
    fprintf(stderr, "  -p, --print type    Output the byte sequence corresponding to \"type\"\n");
    fprintf(stderr, "      --prefer32      Prefer UTF-32LE instead of UTF-16LE followed by NUL\n");
    fprintf(stderr, "  -s, --strip         Strip the BOM and output the remainder of the file\n");
    fprintf(stderr, "  -u, --utf8          Convert the remainder of the file to UTF-8\n");
    fprintf(stderr, "  -v, --version       Output program version and exit\n");
}
