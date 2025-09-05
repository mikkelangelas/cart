#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

uint8_t *read_file_to_array(const char *filename, uint8_t is_binary) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        fprintf(stderr, "read_file_to_array(): Failed to open file: %s", filename);
        return NULL;
    }

    fseek(file, 0l, SEEK_END);

    long buf_size = ftell(file);

    if (is_binary == 0) {
        fclose(file);
        file = fopen(filename, "r");
        buf_size++;
    }
    else rewind(file);

    uint8_t *buf = (uint8_t*)malloc(buf_size);

    memset(buf, 0, buf_size);

    fread(buf, 1, buf_size, file);

    fclose(file);

    return buf;
}

