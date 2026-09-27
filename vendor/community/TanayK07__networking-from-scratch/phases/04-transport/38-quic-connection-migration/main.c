/*
 * main.c -- CLI demo: QUIC Connection Migration
 *
 * Demonstrates the key migration operations:
 *   1. Connection table lookup by Connection ID (not by address)
 *   2. PATH_CHALLENGE / PATH_RESPONSE round-trip
 *   3. NEW_CONNECTION_ID frame build/parse
 *   4. CID rotation on migration
 *   5. Full migration simulation: WiFi → Cellular
 *
 * Usage:
 *   ./quic_migration           # run the full demo
 *   ./quic_migration challenge  # PATH_CHALLENGE/RESPONSE only
 *   ./quic_migration newcid     # NEW_CONNECTION_ID frame only
 *   ./quic_migration migrate    # migration simulation only
 */
#include "quic_migration.h"
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

static void print_addr(struct quic_peer_addr addr) {
    printf("%u.%u.%u.%u:%u", (addr.ip >> 24) & 0xFF, (addr.ip >> 16) & 0xFF, (addr.ip >> 8) & 0xFF,
           addr.ip & 0xFF, addr.port);
}

/* ---- Demo: PATH_CHALLENGE / PATH_RESPONSE ----------------------------- */

static void demo_path_challenge(void) {
    printf("=== PATH_CHALLENGE / PATH_RESPONSE ===\n\n");

    /* Build a PATH_CHALLENGE */
    struct quic_path_challenge ch;
    memcpy(ch.data, "\xaa\xbb\xcc\xdd\x11\x22\x33\x44", 8);

    uint8_t buf[64];
    size_t n = quic_path_challenge_build(&ch, buf, sizeof(buf));
    printf("PATH_CHALLENGE frame (%zu bytes): ", n);
    print_hex(buf, n);
    printf("\n");

    /* Parse it back */
    struct quic_path_challenge parsed;
    size_t consumed;
    if (quic_path_challenge_parse(buf, n, &parsed, &consumed) == 0) {
        printf("  Parsed data: ");
        print_hex(parsed.data, QUIC_PATH_CHALLENGE_LEN);
        printf("  (%zu bytes consumed)\n", consumed);
    }

    /* Build a matching PATH_RESPONSE */
    n = quic_path_response_build(&parsed, buf, sizeof(buf));
    printf("\nPATH_RESPONSE frame (%zu bytes): ", n);
    print_hex(buf, n);
    printf("\n");

    /* Parse and validate */
    struct quic_path_challenge resp;
    if (quic_path_response_parse(buf, n, &resp, &consumed) == 0) {
        printf("  Parsed data: ");
        print_hex(resp.data, QUIC_PATH_CHALLENGE_LEN);
        printf("\n");
        printf("  Validates against challenge: %s\n",
               quic_path_validate(&ch, &resp) ? "YES" : "NO");
    }

    /* Try with wrong data */
    struct quic_path_challenge wrong;
    memcpy(wrong.data, "\xff\xff\xff\xff\xff\xff\xff\xff", 8);
    printf("  Wrong response validates:     %s\n\n",
           quic_path_validate(&ch, &wrong) ? "YES" : "NO");
}

/* ---- Demo: NEW_CONNECTION_ID frame ------------------------------------ */

static void demo_new_cid(void) {
    printf("=== NEW_CONNECTION_ID frame ===\n\n");

    struct quic_new_cid_frame f = {0};
    f.seq_num = 3;
    f.retire_prior_to = 1;
    f.cid_len = 8;
    memcpy(f.cid, "\x01\x02\x03\x04\x05\x06\x07\x08", 8);
    memcpy(f.stateless_reset_token,
           "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7"
           "\xa8\xa9\xaa\xab\xac\xad\xae\xaf",
           16);

    uint8_t buf[64];
    size_t n = quic_new_cid_build(&f, buf, sizeof(buf));
    printf("NEW_CONNECTION_ID frame (%zu bytes):\n  ", n);
    print_hex(buf, n);
    printf("\n");

    /* Parse it back */
    struct quic_new_cid_frame parsed;
    size_t consumed;
    if (quic_new_cid_parse(buf, n, &parsed, &consumed) == 0) {
        printf("  Seq=%llu  Retire Prior To=%llu  CID=", (unsigned long long)parsed.seq_num,
               (unsigned long long)parsed.retire_prior_to);
        print_cid(parsed.cid, parsed.cid_len);
        printf("\n  Stateless Reset Token: ");
        print_hex(parsed.stateless_reset_token, QUIC_STATELESS_RESET_LEN);
        printf("\n  (%zu bytes consumed)\n", consumed);
    }
    printf("\n");
}

/* ---- Demo: Migration simulation --------------------------------------- */

static void demo_migration(void) {
    printf("=== Connection Migration Simulation ===\n\n");

    /* Set up connection table */
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    /* Client connects from WiFi: 192.168.1.50:12345 */
    uint8_t cid1[] = {0xDE, 0xAD, 0xBE, 0xEF};
    struct quic_peer_addr wifi = {.ip = (192u << 24) | (168u << 16) | (1u << 8) | 50u,
                                  .port = 12345};

    int idx = quic_conn_table_add(&table, cid1, sizeof(cid1), wifi);
    printf("1. Connection established from WiFi\n");
    printf("   CID: ");
    print_cid(cid1, sizeof(cid1));
    printf("  Address: ");
    print_addr(wifi);
    printf("  (conn index %d)\n", idx);

    /* Issue a second CID for post-migration use */
    uint8_t cid2[] = {0xCA, 0xFE, 0xBA, 0xBE};
    uint8_t token2[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                          0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
    struct quic_cid_entry new_entry;
    quic_issue_cid(&table.conns[idx], cid2, sizeof(cid2), token2, &new_entry);
    printf("\n2. Issued new CID (seq=%llu): ", (unsigned long long)new_entry.seq_num);
    print_cid(cid2, sizeof(cid2));
    printf("\n   Active CIDs: %d\n", table.conns[idx].local_cid_count);

    /* Look up by CID — both should find the same connection */
    int found1 = quic_match_connection(&table, cid1, sizeof(cid1));
    int found2 = quic_match_connection(&table, cid2, sizeof(cid2));
    printf("\n3. Lookup by CID:\n");
    printf("   DCID=deadbeef → conn %d\n", found1);
    printf("   DCID=cafebabe → conn %d\n", found2);

    /* Compare to TCP: a 4-tuple lookup would fail after migration */
    printf("\n   (TCP would match by 4-tuple — if the IP changes,\n");
    printf("    TCP loses the connection.  QUIC matches by CID.)\n");

    /* Client switches to cellular: 10.0.0.1:54321 */
    struct quic_peer_addr cellular = {.ip = (10u << 24) | 1u, .port = 54321};

    printf("\n4. Client migrates to cellular network\n");
    printf("   New address: ");
    print_addr(cellular);
    printf("\n");

    /* Detect migration */
    struct quic_connection *conn = &table.conns[idx];
    int migrated = quic_detect_migration(conn, cellular);
    printf("   Migration detected: %s\n", migrated ? "YES" : "NO");
    printf("   Path validated: %s\n", conn->path_validated ? "YES" : "NO");

    /* Send PATH_CHALLENGE and receive PATH_RESPONSE */
    printf("\n5. Path validation\n");
    uint8_t frame_buf[64];
    size_t frame_len =
        quic_path_challenge_build(&conn->pending_challenge, frame_buf, sizeof(frame_buf));
    printf("   Sending PATH_CHALLENGE: ");
    print_hex(frame_buf, frame_len);
    printf("\n");

    /* Peer echoes the challenge data back */
    struct quic_path_challenge response = conn->pending_challenge;
    int rc = quic_complete_migration(conn, &response);
    printf("   PATH_RESPONSE received: ");
    print_hex(response.data, QUIC_PATH_CHALLENGE_LEN);
    printf("\n");
    printf("   Migration complete: %s\n", rc == 0 ? "YES" : "NO");
    printf("   Path validated: %s\n", conn->path_validated ? "YES" : "NO");

    /* Retire old CID for unlinkability */
    printf("\n6. Retire old CID for unlinkability (RFC 9000, §9.5)\n");
    int retired = quic_retire_cids_prior_to(conn, 1);
    printf("   Retired %d CID(s) with seq < 1\n", retired);
    const struct quic_cid_entry *active = quic_active_cid(conn);
    if (active) {
        printf("   Active CID now: ");
        print_cid(active->cid, active->cid_len);
        printf(" (seq=%llu)\n", (unsigned long long)active->seq_num);
    }

    /* Verify connection is still reachable by new CID */
    found2 = quic_match_connection(&table, cid2, sizeof(cid2));
    printf("\n7. Connection still reachable by new CID: %s (conn %d)\n", found2 >= 0 ? "YES" : "NO",
           found2);

    /* Old CID retired — should not match */
    found1 = quic_match_connection(&table, cid1, sizeof(cid1));
    printf("   Old CID retired — lookup returns: %d\n", found1);

    printf("\n   Migration count: %d\n", conn->migration_count);
    printf("   Peer address: ");
    print_addr(conn->peer_addr);
    printf("\n\n");
}

/* ---- Main ------------------------------------------------------------- */

int main(int argc, char *argv[]) {
    if (argc >= 2 && strcmp(argv[1], "challenge") == 0) {
        demo_path_challenge();
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "newcid") == 0) {
        demo_new_cid();
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "migrate") == 0) {
        demo_migration();
        return 0;
    }

    /* Default: run all demos */
    demo_path_challenge();
    demo_new_cid();
    demo_migration();
    return 0;
}
