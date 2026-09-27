/*
 * main.c -- CLI tool for QUIC variable-length integer encoding/decoding
 *
 * Usage:
 *   ./quic_varint 37 15293 494878333    # encode decimal values
 *   ./quic_varint -d c2197c5eff14e88c   # decode hex bytes
 */
#include "quic_varint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hex_to_bytes(const char *hex, uint8_t *out, size_t max, size_t *len) {
    size_t slen = strlen(hex);
    if (slen % 2 != 0 || slen / 2 > max)
        return -1;
    for (size_t i = 0; i < slen; i += 2) {
        unsigned int byte;
        if (sscanf(hex + i, "%2x", &byte) != 1)
            return -1;
        out[i / 2] = (uint8_t)byte;
    }
    *len = slen / 2;
    return 0;
}

static void encode_and_print(uint64_t val) {
    uint8_t buf[8];
    size_t written;

    if (quic_varint_encode(val, buf, sizeof(buf), &written) != 0) {
        fprintf(stderr, "Error: %llu exceeds QUIC varint maximum (%llu)\n", (unsigned long long)val,
                (unsigned long long)QUIC_VARINT_MAX);
        return;
    }

    printf("Encoding %llu → ", (unsigned long long)val);
    for (size_t i = 0; i < written; i++)
        printf("%02x%s", buf[i], i + 1 < written ? " " : "");
    printf(" (%zu byte%s)\n", written, written == 1 ? "" : "s");
}

static void decode_and_print(const uint8_t *buf, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        uint64_t val;
        size_t consumed;

        if (quic_varint_decode(buf + offset, len - offset, &val, &consumed) != 0) {
            fprintf(stderr, "Error: truncated varint at offset %zu\n", offset);
            return;
        }

        printf("Decoding ");
        for (size_t i = 0; i < consumed; i++)
            printf("%02x%s", buf[offset + i], i + 1 < consumed ? " " : "");
        printf(" → %llu (%zu byte%s)\n", (unsigned long long)val, consumed,
               consumed == 1 ? "" : "s");

        offset += consumed;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <decimal>...       encode values\n", argv[0]);
        fprintf(stderr, "       %s -d <hex>...        decode hex bytes\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-d") == 0) {
        /* Decode mode */
        for (int i = 2; i < argc; i++) {
            uint8_t buf[8];
            size_t len;
            if (hex_to_bytes(argv[i], buf, sizeof(buf), &len) != 0) {
                fprintf(stderr, "Error: invalid hex '%s'\n", argv[i]);
                continue;
            }
            decode_and_print(buf, len);
        }
    } else {
        /* Encode mode */
        for (int i = 1; i < argc; i++) {
            char *end;
            unsigned long long val = strtoull(argv[i], &end, 10);
            if (*end != '\0') {
                fprintf(stderr, "Error: invalid number '%s'\n", argv[i]);
                continue;
            }
            encode_and_print((uint64_t)val);
        }
    }

    return 0;
}
