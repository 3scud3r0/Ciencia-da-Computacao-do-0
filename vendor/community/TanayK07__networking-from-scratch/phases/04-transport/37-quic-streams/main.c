/*
 * main.c -- CLI tool for QUIC stream operations
 *
 * Usage:
 *   ./quic_stream                 # demo: classify IDs, build/parse frames, reassembly
 *   ./quic_stream classify 0 4 7  # classify stream IDs
 *   ./quic_stream build <id> <offset> <len> <flags>
 *                                 # build a STREAM frame (flags: fin,len,off)
 *   ./quic_stream reasm           # demonstrate out-of-order reassembly
 */
#include "quic_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_hex(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++)
        printf("%02x%s", buf[i], i + 1 < len ? " " : "");
}

static void demo_classify(void) {
    printf("Stream ID classification (RFC 9000, Section 2.1):\n\n");
    printf("  %-6s %-12s %-12s %-6s\n", "ID", "Initiator", "Direction", "Seq#");
    printf("  %-6s %-12s %-12s %-6s\n", "------", "----------", "----------", "----");

    for (uint64_t id = 0; id < 12; id++) {
        struct quic_stream_info info;
        quic_stream_classify(id, &info);
        printf("  %-6llu %-12s %-12s %-6llu  (%s)\n", (unsigned long long)id,
               info.initiator == QUIC_STREAM_CLIENT ? "client" : "server",
               info.direction == QUIC_STREAM_BIDI ? "bidi" : "uni", (unsigned long long)info.seq,
               quic_stream_type_name(id));
    }
}

static void demo_build_parse(void) {
    printf("\nSTREAM frame build/parse (RFC 9000, Section 19.8):\n\n");

    /* Example 1: stream 0, no offset, no length, no FIN */
    {
        uint8_t payload[] = "Hello, QUIC!";
        uint8_t buf[64];
        size_t written;
        int flags = 0;

        int rc = quic_stream_frame_build(0, 0, payload, sizeof(payload) - 1, flags, buf,
                                         sizeof(buf), &written);
        printf("  1. Stream 0, no offset, no FIN (type=0x08)\n");
        printf("     Built %zu bytes: ", written);
        print_hex(buf, written);
        printf("\n");

        if (rc == 0) {
            struct quic_stream_frame parsed;
            size_t consumed;
            if (quic_stream_frame_parse(buf, written, &parsed, &consumed) == 0) {
                printf("     Parsed: type=0x%02x stream_id=%llu offset=%llu "
                       "len=%llu fin=%d\n",
                       parsed.type, (unsigned long long)parsed.stream_id,
                       (unsigned long long)parsed.offset, (unsigned long long)parsed.length,
                       parsed.fin);
                printf("     Data: \"%.*s\"\n", (int)parsed.data_len, parsed.data);
            }
        }
    }

    /* Example 2: stream 4, offset 1000, length present, FIN (type=0x0f) */
    {
        uint8_t payload[50];
        memset(payload, 'X', sizeof(payload));
        uint8_t buf[128];
        size_t written;
        int flags = QUIC_STREAM_FIN_BIT | QUIC_STREAM_LEN_BIT | QUIC_STREAM_OFF_BIT;

        int rc = quic_stream_frame_build(4, 1000, payload, sizeof(payload), flags, buf, sizeof(buf),
                                         &written);
        printf("\n  2. Stream 4, offset=1000, len=50, FIN (type=0x0f)\n");
        printf("     Built %zu bytes: ", written);
        print_hex(buf, written);
        printf("\n");

        if (rc == 0) {
            struct quic_stream_frame parsed;
            size_t consumed;
            if (quic_stream_frame_parse(buf, written, &parsed, &consumed) == 0) {
                printf("     Parsed: type=0x%02x stream_id=%llu offset=%llu "
                       "len=%llu fin=%d\n",
                       parsed.type, (unsigned long long)parsed.stream_id,
                       (unsigned long long)parsed.offset, (unsigned long long)parsed.length,
                       parsed.fin);
                printf("     Type byte 0x%02x confirms: OFF=%d LEN=%d FIN=%d\n", parsed.type,
                       (parsed.type & QUIC_STREAM_OFF_BIT) ? 1 : 0,
                       (parsed.type & QUIC_STREAM_LEN_BIT) ? 1 : 0,
                       (parsed.type & QUIC_STREAM_FIN_BIT) ? 1 : 0);
            }
        }
    }

    /* Example 3: stream 16383, offset 0, LEN+FIN (type=0x0b) */
    {
        uint8_t buf[32];
        size_t written;
        int flags = QUIC_STREAM_FIN_BIT | QUIC_STREAM_LEN_BIT;

        int rc = quic_stream_frame_build(16383, 0, NULL, 0, flags, buf, sizeof(buf), &written);
        printf("\n  3. Stream 16383, offset=0, len=0, FIN (type=0x0b)\n");
        printf("     Built %zu bytes: ", written);
        print_hex(buf, written);
        printf("\n");

        if (rc == 0) {
            struct quic_stream_frame parsed;
            size_t consumed;
            if (quic_stream_frame_parse(buf, written, &parsed, &consumed) == 0) {
                printf("     Parsed: type=0x%02x stream_id=%llu offset=%llu "
                       "len=%llu fin=%d\n",
                       parsed.type, (unsigned long long)parsed.stream_id,
                       (unsigned long long)parsed.offset, (unsigned long long)parsed.length,
                       parsed.fin);
            }
        }
    }
}

static void demo_reassembly(void) {
    printf("\nOut-of-order stream reassembly:\n\n");

    struct quic_stream_reasm r;
    quic_stream_reasm_init(&r);

    /* Simulate out-of-order delivery of "Hello, QUIC streams!" */
    /* Deliver chunks out of order: [7..11], [0..7], [15..20+FIN], [5..7], [11..15] */
    struct {
        uint64_t offset;
        const char *data;
        int fin;
        const char *desc;
    } chunks[] = {
        {7, "QUIC", 0, "offset=7,  len=4  \"QUIC\""},
        {0, "Hello, ", 0, "offset=0,  len=7  \"Hello, \""},
        {15, "eams!", 1, "offset=15, len=5  \"eams!\" (FIN)"},
        {5, ", ", 0, "offset=5,  len=2  \", \" (overlap with chunk 2)"},
        {11, " str", 0, "offset=11, len=4  \" str\""},
    };

    for (size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); i++) {
        int rc = quic_stream_reasm_insert(&r, chunks[i].offset, (const uint8_t *)chunks[i].data,
                                          strlen(chunks[i].data), chunks[i].fin);
        uint64_t readable = quic_stream_reasm_readable(&r);
        printf("  Inserted: %-42s → readable=%llu complete=%d %s\n", chunks[i].desc,
               (unsigned long long)readable, quic_stream_reasm_complete(&r),
               rc == 0 ? "OK" : "ERROR");
    }

    uint64_t readable = quic_stream_reasm_readable(&r);
    printf("\n  Final reassembled data (%llu bytes): \"%.*s\"\n", (unsigned long long)readable,
           (int)readable, r.data);
    printf("  Complete: %s\n", quic_stream_reasm_complete(&r) ? "yes" : "no");
}

static void classify_ids(int argc, char *argv[]) {
    printf("%-6s %-12s %-12s %-6s  %s\n", "ID", "Initiator", "Direction", "Seq#", "Type");
    for (int i = 2; i < argc; i++) {
        uint64_t id = strtoull(argv[i], NULL, 10);
        struct quic_stream_info info;
        quic_stream_classify(id, &info);
        printf("%-6llu %-12s %-12s %-6llu  %s\n", (unsigned long long)id,
               info.initiator == QUIC_STREAM_CLIENT ? "client" : "server",
               info.direction == QUIC_STREAM_BIDI ? "bidi" : "uni", (unsigned long long)info.seq,
               quic_stream_type_name(id));
    }
}

static void build_frame(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s build <stream_id> <offset> <data_len> <flags>\n", argv[0]);
        fprintf(stderr, "  flags: comma-separated: fin,len,off\n");
        return;
    }

    uint64_t stream_id = strtoull(argv[2], NULL, 10);
    uint64_t offset = strtoull(argv[3], NULL, 10);
    size_t data_len = (size_t)strtoull(argv[4], NULL, 10);
    int flags = 0;

    if (strstr(argv[5], "fin"))
        flags |= QUIC_STREAM_FIN_BIT;
    if (strstr(argv[5], "len"))
        flags |= QUIC_STREAM_LEN_BIT;
    if (strstr(argv[5], "off"))
        flags |= QUIC_STREAM_OFF_BIT;

    uint8_t *payload = NULL;
    if (data_len > 0) {
        payload = malloc(data_len);
        if (!payload) {
            fprintf(stderr, "Error: allocation failed\n");
            return;
        }
        memset(payload, 'A', data_len);
    }

    uint8_t buf[1024];
    size_t written;
    int rc = quic_stream_frame_build(stream_id, offset, payload, data_len, flags, buf, sizeof(buf),
                                     &written);
    if (rc == 0) {
        printf("Type byte: 0x%02x\n", buf[0]);
        printf("Frame (%zu bytes): ", written);
        print_hex(buf, written);
        printf("\n");

        struct quic_stream_frame parsed;
        size_t consumed;
        if (quic_stream_frame_parse(buf, written, &parsed, &consumed) == 0) {
            printf("Parsed: stream_id=%llu offset=%llu length=%llu fin=%d\n",
                   (unsigned long long)parsed.stream_id, (unsigned long long)parsed.offset,
                   (unsigned long long)parsed.length, parsed.fin);
        }
    } else {
        fprintf(stderr, "Error: failed to build frame\n");
    }

    free(payload);
}

int main(int argc, char *argv[]) {
    if (argc >= 2 && strcmp(argv[1], "classify") == 0) {
        classify_ids(argc, argv);
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "build") == 0) {
        build_frame(argc, argv);
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "reasm") == 0) {
        demo_reassembly();
        return 0;
    }

    /* Default: run all demos */
    demo_classify();
    demo_build_parse();
    demo_reassembly();

    return 0;
}
