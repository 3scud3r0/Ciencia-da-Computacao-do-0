/*
 * main.c -- CLI tool: build and parse QUIC packet headers
 *
 * Usage:
 *   ./quic_packet           # builds all four long header types + short header
 *   ./quic_packet -p <hex>  # parse hex bytes as QUIC header
 */
#include "quic_packet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_hex(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++)
        printf("%02x%s", buf[i], i + 1 < len ? " " : "");
}

static void print_cid(const uint8_t *cid, uint8_t len) {
    if (len == 0) {
        printf("(empty)");
        return;
    }
    for (uint8_t i = 0; i < len; i++)
        printf("%02x", cid[i]);
}

static void build_and_show(int pkt_type) {
    struct quic_long_hdr hdr = {0};
    hdr.first_byte =
        QUIC_HEADER_FORM_BIT | QUIC_FIXED_BIT | (uint8_t)(pkt_type << QUIC_LONG_TYPE_SHIFT);
    hdr.version = QUIC_VERSION_1;
    hdr.dcid_len = 8;
    memcpy(hdr.dcid, "\x83\x94\xc8\xf0\x3e\x51\x57\x08", 8);
    hdr.scid_len = 4;
    memcpy(hdr.scid, "\xbe\xef\xca\xfe", 4);

    uint8_t buf[64];
    size_t n = quic_long_hdr_build(&hdr, buf, sizeof(buf));

    printf("%-10s  ", quic_pkt_type_name(pkt_type));
    print_hex(buf, n);
    printf("\n");

    /* Parse it back */
    struct quic_long_hdr parsed;
    size_t consumed;
    if (quic_long_hdr_parse(buf, n, &parsed, &consumed) == 0) {
        printf("  → type=%s version=0x%08x DCID=",
               quic_pkt_type_name(quic_long_hdr_type(parsed.first_byte)), parsed.version);
        print_cid(parsed.dcid, parsed.dcid_len);
        printf(" SCID=");
        print_cid(parsed.scid, parsed.scid_len);
        printf(" (%zu bytes consumed)\n", consumed);
    }
}

static void show_short_header(void) {
    struct quic_short_hdr hdr = {0};
    hdr.first_byte = QUIC_FIXED_BIT | QUIC_SHORT_SPIN_BIT;
    hdr.dcid_len = 8;
    memcpy(hdr.dcid, "\x83\x94\xc8\xf0\x3e\x51\x57\x08", 8);

    uint8_t buf[64];
    size_t n = quic_short_hdr_build(&hdr, buf, sizeof(buf));

    printf("%-10s  ", "1-RTT");
    print_hex(buf, n);
    printf("\n");

    struct quic_short_hdr parsed;
    size_t consumed;
    if (quic_short_hdr_parse(buf, n, 8, &parsed, &consumed) == 0) {
        printf("  → short header DCID=");
        print_cid(parsed.dcid, parsed.dcid_len);
        printf(" spin=%d key_phase=%d (%zu bytes consumed)\n",
               (parsed.first_byte & QUIC_SHORT_SPIN_BIT) ? 1 : 0,
               (parsed.first_byte & QUIC_SHORT_KEY_PHASE) ? 1 : 0, consumed);
    }
}

static int hex_to_bytes(const char *hex, uint8_t *out, size_t max, size_t *len) {
    size_t slen = strlen(hex);
    if (slen % 2 != 0 || slen / 2 > max)
        return -1;
    for (size_t i = 0; i < slen; i += 2) {
        unsigned int b;
        if (sscanf(hex + i, "%2x", &b) != 1)
            return -1;
        out[i / 2] = (uint8_t)b;
    }
    *len = slen / 2;
    return 0;
}

static void parse_hex_input(const char *hex) {
    uint8_t buf[1500];
    size_t len;
    if (hex_to_bytes(hex, buf, sizeof(buf), &len) != 0) {
        fprintf(stderr, "Error: invalid hex '%s'\n", hex);
        return;
    }

    if (len == 0) {
        fprintf(stderr, "Error: empty input\n");
        return;
    }

    if (quic_is_long_header(buf[0])) {
        struct quic_long_hdr hdr;
        size_t consumed;
        if (quic_long_hdr_parse(buf, len, &hdr, &consumed) == 0) {
            printf("Long Header: type=%s version=0x%08x\n",
                   quic_pkt_type_name(quic_long_hdr_type(hdr.first_byte)), hdr.version);
            printf("  DCID (%u bytes): ", hdr.dcid_len);
            print_cid(hdr.dcid, hdr.dcid_len);
            printf("\n  SCID (%u bytes): ", hdr.scid_len);
            print_cid(hdr.scid, hdr.scid_len);
            printf("\n  Consumed: %zu bytes\n", consumed);
        } else {
            fprintf(stderr, "Error: failed to parse long header\n");
        }
    } else {
        printf("Short Header detected (need DCID length from connection state)\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
        for (int i = 2; i < argc; i++)
            parse_hex_input(argv[i]);
        return 0;
    }

    printf("Building QUIC packet headers:\n\n");
    build_and_show(QUIC_PKT_INITIAL);
    build_and_show(QUIC_PKT_0RTT);
    build_and_show(QUIC_PKT_HANDSHAKE);
    build_and_show(QUIC_PKT_RETRY);
    printf("\n");
    show_short_header();

    printf("\nFrame type names:\n");
    uint64_t types[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e};
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++)
        printf("  0x%02llx → %s\n", (unsigned long long)types[i], quic_frame_name(types[i]));

    return 0;
}
