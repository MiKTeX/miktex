/*
 * Utility program to convert utf8strings.h.in into utf8strings.h
 *
 * Copyright (C) 2015-2019 The Gregorio Project (see CONTRIBUTORS.md)
 *
 * This file is part of Gregorio.
 *
 * Gregorio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gregorio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gregorio.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
    char buf[5];
    FILE *input, *output;
    int c;

    if (argc != 3) {
        fprintf(stderr, "Incorrect arguments\n");
        return -1;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Error opening %s: %s\n", argv[1], strerror(errno));
        return -1;
    }

    output = fopen(argv[2], "wb");
    if (output == NULL) {
        fprintf(stderr, "Error creating %s: %s\n", argv[2], strerror(errno));
        return -1;
    }

    while ((c = fgetc(input)) != EOF) {
        if (c & 0x80) {
            snprintf(buf, sizeof(buf), "\\%03o", c);
            if (fwrite(buf, sizeof(buf) - sizeof(char), 1, output) != 1) {
                fprintf(stderr, "Error writing %s: %s\n", argv[2],
                        strerror(errno));
                return -1;
            }
        } else {
            if (fputc(c, output) != c) {
                fprintf(stderr, "Error writing %s: %s\n", argv[2],
                        strerror(errno));
                return -1;
            }
        }
    }

    if (fclose(output) != 0) {
        fprintf(stderr, "Error closing %s: %s\n", argv[2], strerror(errno));
        return -1;
    }
    if (fclose(input) != 0) {
        fprintf(stderr, "Error closing %s: %s\n", argv[1], strerror(errno));
        return -1;
    }
    return 0;
}
