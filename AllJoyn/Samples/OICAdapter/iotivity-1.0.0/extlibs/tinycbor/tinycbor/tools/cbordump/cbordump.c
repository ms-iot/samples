/****************************************************************************
**
** Copyright (C) 2015 Intel Corporation
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#define _POSIX_C_SOURCE 200809L
#include "cbor.h"
#include "cborjson.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum Mode {
    PrintCborDump = 0,
    PrintJson = 0x80000000
};

void *xrealloc(void *old, size_t size, const char *fname)
{
    old = realloc(old, size);
    if (old == NULL) {
        fprintf(stderr, "%s: %s\n", fname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return old;
}

void printerror(CborError err, const char *fname)
{
    fprintf(stderr, "%s: %s\n", fname, cbor_error_string(err));
    exit(EXIT_FAILURE);
}

void dumpFile(FILE *in, const char *fname, int mode)
{
    static const size_t chunklen = 16 * 1024;
    static size_t bufsize = 0;
    static uint8_t *buffer = NULL;

    size_t buflen = 0;
    do {
        if (bufsize == buflen)
            buffer = xrealloc(buffer, bufsize += chunklen, fname);

        size_t n = fread(buffer + buflen, 1, bufsize - buflen, in);
        buflen += n;
        if (n == 0) {
            if (!ferror(in))
                continue;
            fprintf(stderr, "%s: %s\n", fname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } while (!feof(in));

    CborParser parser;
    CborValue value;
    CborError err = cbor_parser_init(buffer, buflen, 0, &parser, &value);
    if (!err) {
        if (mode)
            err = cbor_value_to_json_advance(stdout, &value, mode & ~PrintJson);
        else
            err = cbor_value_to_pretty_advance(stdout, &value);
        if (!err)
            puts("");
    }
    if (!err && value.ptr != buffer + buflen)
        err = CborErrorGarbageAtEnd;
    if (err)
        printerror(err, fname);
}

int main(int argc, char **argv)
{
    int mode = PrintCborDump;
    int c;
    while ((c = getopt(argc, argv, "MOSUcjh")) != -1) {
        switch (c) {
        case 'c':
            mode = PrintCborDump;
            break;
        case 'j':
            mode &= ~PrintCborDump;
            mode |= PrintJson;
            break;

        case 'M':
            mode |= CborConvertAddMetadata;
            break;
        case 'O':
            mode |= CborConvertTagsToObjects;
            break;
        case 'S':
            mode |= CborConvertStringifyMapKeys;
            break;
        case 'U':
            mode |= CborConvertByteStringsToBase64Url;
            break;

        case '?':
            fprintf(stderr, "Unknown option -%c.\n", optopt);
            // fall through
        case 'h':
            puts("Usage: cbordump [OPTION]... [FILE]...\n"
                 "Interprets FILEs as CBOR binary data and dumps the content to stdout.\n"
                 "\n"
                 "Options:\n"
                 " -c       Print a CBOR dump (see RFC 7049) (default)\n"
                 " -j       Print a JSON equivalent version\n"
                 " -h       Print this help output and exit\n"
                 "When JSON output is active, the following options are recognized:\n"
                 " -M       Add metadata so converting back to CBOR is possible\n"
                 " -O       Convert CBOR tags to JSON objects\n"
                 " -S       Stringify non-text string map keys\n"
                 " -U       Convert all CBOR byte strings to Base64url regardless of tags"
                 "");
            return c == '?' ? EXIT_FAILURE : EXIT_SUCCESS;
        }
    }

    char **fname = argv + optind;
    if (!*fname) {
        dumpFile(stdin, "-", mode);
    } else {
        for ( ; *fname; ++fname) {
            FILE *in = fopen(*fname, "rb");
            if (!in) {
                perror("open");
                return EXIT_FAILURE;
            }

            dumpFile(in, *fname, mode);
            fclose(in);
        }
    }

    return EXIT_SUCCESS;
}
