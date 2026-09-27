/*
 * test_migration.c -- Tests for QUIC Connection Migration
 *
 * Covers PATH_CHALLENGE/RESPONSE frames, NEW_CONNECTION_ID and
 * RETIRE_CONNECTION_ID frames, connection table lookup by CID,
 * CID lifecycle, and migration simulation.
 */
#include "quic_migration.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- PATH_CHALLENGE / PATH_RESPONSE ----------------------------------- */

static void test_path_challenge_roundtrip(void) {
    struct quic_path_challenge ch;
    memcpy(ch.data, "\xaa\xbb\xcc\xdd\x11\x22\x33\x44", 8);

    uint8_t buf[64];
    size_t n = quic_path_challenge_build(&ch, buf, sizeof(buf));
    assert(n == 9);
    assert(buf[0] == 0x1a); /* frame type */
    assert(memcmp(buf + 1, ch.data, 8) == 0);

    struct quic_path_challenge parsed;
    size_t consumed;
    assert(quic_path_challenge_parse(buf, n, &parsed, &consumed) == 0);
    assert(consumed == 9);
    assert(memcmp(parsed.data, ch.data, 8) == 0);
}

static void test_path_response_roundtrip(void) {
    struct quic_path_challenge ch;
    memcpy(ch.data, "\x01\x02\x03\x04\x05\x06\x07\x08", 8);

    uint8_t buf[64];
    size_t n = quic_path_response_build(&ch, buf, sizeof(buf));
    assert(n == 9);
    assert(buf[0] == 0x1b); /* frame type */
    assert(memcmp(buf + 1, ch.data, 8) == 0);

    struct quic_path_challenge parsed;
    size_t consumed;
    assert(quic_path_response_parse(buf, n, &parsed, &consumed) == 0);
    assert(consumed == 9);
    assert(memcmp(parsed.data, ch.data, 8) == 0);
}

static void test_path_validate_match(void) {
    struct quic_path_challenge ch, resp;
    memcpy(ch.data, "\xde\xad\xbe\xef\xca\xfe\x00\x01", 8);
    memcpy(resp.data, "\xde\xad\xbe\xef\xca\xfe\x00\x01", 8);
    assert(quic_path_validate(&ch, &resp) == 1);
}

static void test_path_validate_mismatch(void) {
    struct quic_path_challenge ch, resp;
    memcpy(ch.data, "\xde\xad\xbe\xef\xca\xfe\x00\x01", 8);
    memcpy(resp.data, "\xde\xad\xbe\xef\xca\xfe\x00\x02", 8);
    assert(quic_path_validate(&ch, &resp) == 0);
}

static void test_path_challenge_wrong_type(void) {
    /* PATH_RESPONSE frame (0x1b) should not parse as PATH_CHALLENGE */
    uint8_t buf[9] = {0x1b, 0, 0, 0, 0, 0, 0, 0, 0};
    struct quic_path_challenge out;
    size_t consumed;
    assert(quic_path_challenge_parse(buf, 9, &out, &consumed) == -1);
}

static void test_path_response_wrong_type(void) {
    /* PATH_CHALLENGE frame (0x1a) should not parse as PATH_RESPONSE */
    uint8_t buf[9] = {0x1a, 0, 0, 0, 0, 0, 0, 0, 0};
    struct quic_path_challenge out;
    size_t consumed;
    assert(quic_path_response_parse(buf, 9, &out, &consumed) == -1);
}

static void test_path_challenge_truncated(void) {
    uint8_t buf[4] = {0x1a, 0xaa, 0xbb, 0xcc};
    struct quic_path_challenge out;
    size_t consumed;
    assert(quic_path_challenge_parse(buf, sizeof(buf), &out, &consumed) == -1);
}

static void test_path_challenge_exact_bytes(void) {
    /* Pin specific bytes: type 0x1a followed by 8 data bytes */
    uint8_t expected[] = {0x1a, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    struct quic_path_challenge ch;
    memcpy(ch.data, "\x01\x02\x03\x04\x05\x06\x07\x08", 8);

    uint8_t buf[9];
    size_t n = quic_path_challenge_build(&ch, buf, sizeof(buf));
    assert(n == 9);
    assert(memcmp(buf, expected, 9) == 0);
}

/* ---- NEW_CONNECTION_ID frame ------------------------------------------ */

static void test_new_cid_roundtrip(void) {
    struct quic_new_cid_frame f = {0};
    f.seq_num = 5;
    f.retire_prior_to = 2;
    f.cid_len = 4;
    memcpy(f.cid, "\xca\xfe\xba\xbe", 4);
    memcpy(f.stateless_reset_token,
           "\x10\x11\x12\x13\x14\x15\x16\x17"
           "\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f",
           16);

    uint8_t buf[64];
    size_t n = quic_new_cid_build(&f, buf, sizeof(buf));
    assert(n > 0);
    assert(buf[0] == 0x18); /* frame type */

    struct quic_new_cid_frame parsed;
    size_t consumed;
    assert(quic_new_cid_parse(buf, n, &parsed, &consumed) == 0);
    assert(consumed == n);
    assert(parsed.seq_num == 5);
    assert(parsed.retire_prior_to == 2);
    assert(parsed.cid_len == 4);
    assert(memcmp(parsed.cid, f.cid, 4) == 0);
    assert(memcmp(parsed.stateless_reset_token, f.stateless_reset_token, 16) == 0);
}

static void test_new_cid_exact_wire_format(void) {
    /* Pin wire format:
     * 0x18  type
     * 0x03  seq_num = 3
     * 0x01  retire_prior_to = 1
     * 0x02  cid_len = 2
     * 0xAB 0xCD  cid
     * 16 bytes of token (all 0xFF)
     */
    struct quic_new_cid_frame f = {0};
    f.seq_num = 3;
    f.retire_prior_to = 1;
    f.cid_len = 2;
    f.cid[0] = 0xAB;
    f.cid[1] = 0xCD;
    memset(f.stateless_reset_token, 0xFF, 16);

    uint8_t buf[64];
    size_t n = quic_new_cid_build(&f, buf, sizeof(buf));
    assert(n == 4 + 2 + 16); /* 22 bytes total */
    assert(buf[0] == 0x18);
    assert(buf[1] == 0x03);
    assert(buf[2] == 0x01);
    assert(buf[3] == 0x02);
    assert(buf[4] == 0xAB);
    assert(buf[5] == 0xCD);
    for (int i = 0; i < 16; i++)
        assert(buf[6 + i] == 0xFF);
}

static void test_new_cid_reject_retire_gt_seq(void) {
    /* RFC 9000, §19.15: retire_prior_to must be <= seq_num */
    uint8_t buf[] = {0x18, 0x02, 0x05, 0x04, /* seq=2, retire=5, len=4 */
                     0x01, 0x02, 0x03, 0x04, /* CID */
                     0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /* token */
    struct quic_new_cid_frame out;
    size_t consumed;
    assert(quic_new_cid_parse(buf, sizeof(buf), &out, &consumed) == -1);
}

static void test_new_cid_reject_zero_cid_len(void) {
    uint8_t buf[] = {0x18, 0x01, 0x00, 0x00, /* seq=1, retire=0, len=0 */
                     0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /* token */
    struct quic_new_cid_frame out;
    size_t consumed;
    assert(quic_new_cid_parse(buf, sizeof(buf), &out, &consumed) == -1);
}

static void test_new_cid_reject_truncated(void) {
    uint8_t buf[] = {0x18, 0x01, 0x00, 0x04, 0x01, 0x02}; /* too short */
    struct quic_new_cid_frame out;
    size_t consumed;
    assert(quic_new_cid_parse(buf, sizeof(buf), &out, &consumed) == -1);
}

static void test_new_cid_reject_wrong_type(void) {
    uint8_t buf[32] = {0x19}; /* RETIRE_CONNECTION_ID, not NEW */
    struct quic_new_cid_frame out;
    size_t consumed;
    assert(quic_new_cid_parse(buf, sizeof(buf), &out, &consumed) == -1);
}

/* ---- RETIRE_CONNECTION_ID frame --------------------------------------- */

static void test_retire_cid_roundtrip(void) {
    struct quic_retire_cid_frame f = {.seq_num = 7};

    uint8_t buf[8];
    size_t n = quic_retire_cid_build(&f, buf, sizeof(buf));
    assert(n == 2);
    assert(buf[0] == 0x19);
    assert(buf[1] == 0x07);

    struct quic_retire_cid_frame parsed;
    size_t consumed;
    assert(quic_retire_cid_parse(buf, n, &parsed, &consumed) == 0);
    assert(consumed == 2);
    assert(parsed.seq_num == 7);
}

static void test_retire_cid_truncated(void) {
    uint8_t buf[] = {0x19}; /* need 2 bytes */
    struct quic_retire_cid_frame out;
    size_t consumed;
    assert(quic_retire_cid_parse(buf, sizeof(buf), &out, &consumed) == -1);
}

static void test_retire_cid_wrong_type(void) {
    uint8_t buf[] = {0x18, 0x00}; /* NEW_CONNECTION_ID, not RETIRE */
    struct quic_retire_cid_frame out;
    size_t consumed;
    assert(quic_retire_cid_parse(buf, sizeof(buf), &out, &consumed) == -1);
}

/* ---- Connection table lookup by CID ----------------------------------- */

static void test_match_by_cid(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid_a[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t cid_b[] = {0x11, 0x22, 0x33, 0x44};
    struct quic_peer_addr addr_a = {.ip = 0xC0A80132, .port = 1000}; /* 192.168.1.50 */
    struct quic_peer_addr addr_b = {.ip = 0x0A000001, .port = 2000}; /* 10.0.0.1 */

    int ia = quic_conn_table_add(&table, cid_a, sizeof(cid_a), addr_a);
    int ib = quic_conn_table_add(&table, cid_b, sizeof(cid_b), addr_b);
    assert(ia >= 0 && ib >= 0 && ia != ib);

    /* Lookup by CID */
    assert(quic_match_connection(&table, cid_a, sizeof(cid_a)) == ia);
    assert(quic_match_connection(&table, cid_b, sizeof(cid_b)) == ib);

    /* Unknown CID */
    uint8_t unknown[] = {0xFF, 0xFF};
    assert(quic_match_connection(&table, unknown, sizeof(unknown)) == -1);
}

static void test_match_multiple_cids(void) {
    /* A connection with two CIDs should be findable by either */
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid1[] = {0x01, 0x02, 0x03, 0x04};
    struct quic_peer_addr addr = {.ip = 0xC0A80101, .port = 443};
    int idx = quic_conn_table_add(&table, cid1, sizeof(cid1), addr);
    assert(idx >= 0);

    uint8_t cid2[] = {0x05, 0x06, 0x07, 0x08};
    uint8_t token[16] = {0};
    struct quic_cid_entry entry;
    assert(quic_issue_cid(&table.conns[idx], cid2, sizeof(cid2), token, &entry) == 0);

    /* Both CIDs find the same connection */
    assert(quic_match_connection(&table, cid1, sizeof(cid1)) == idx);
    assert(quic_match_connection(&table, cid2, sizeof(cid2)) == idx);
}

static void test_match_after_retire(void) {
    /* After retiring CID seq 0, only CID seq 1 should match */
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid1[] = {0xAA, 0xBB};
    struct quic_peer_addr addr = {.ip = 0x01020304, .port = 443};
    int idx = quic_conn_table_add(&table, cid1, sizeof(cid1), addr);
    assert(idx >= 0);

    uint8_t cid2[] = {0xCC, 0xDD};
    assert(quic_issue_cid(&table.conns[idx], cid2, sizeof(cid2), NULL, NULL) == 0);

    /* Retire seq < 1 (retires seq 0 = cid1) */
    int retired = quic_retire_cids_prior_to(&table.conns[idx], 1);
    assert(retired == 1);

    /* cid1 should no longer match */
    assert(quic_match_connection(&table, cid1, sizeof(cid1)) == -1);
    /* cid2 still matches */
    assert(quic_match_connection(&table, cid2, sizeof(cid2)) == idx);
}

static void test_conn_table_full(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid[4] = {0};
    struct quic_peer_addr addr = {.ip = 0, .port = 0};

    /* Fill the table */
    for (int i = 0; i < QUIC_MAX_CONNECTIONS; i++) {
        cid[0] = (uint8_t)i;
        assert(quic_conn_table_add(&table, cid, sizeof(cid), addr) >= 0);
    }

    /* One more should fail */
    cid[0] = 0xFF;
    assert(quic_conn_table_add(&table, cid, sizeof(cid), addr) == -1);
}

/* ---- CID lifecycle ---------------------------------------------------- */

static void test_issue_and_active_cid(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid0[] = {0x10, 0x20};
    struct quic_peer_addr addr = {.ip = 0xC0A80164, .port = 443};
    int idx = quic_conn_table_add(&table, cid0, sizeof(cid0), addr);
    struct quic_connection *conn = &table.conns[idx];

    /* Active CID should be the initial one (seq 0) */
    const struct quic_cid_entry *active = quic_active_cid(conn);
    assert(active != NULL);
    assert(active->seq_num == 0);
    assert(active->cid_len == 2);
    assert(memcmp(active->cid, cid0, 2) == 0);

    /* Issue two more */
    uint8_t cid1[] = {0x30, 0x40};
    uint8_t cid2[] = {0x50, 0x60};
    assert(quic_issue_cid(conn, cid1, sizeof(cid1), NULL, NULL) == 0);
    assert(quic_issue_cid(conn, cid2, sizeof(cid2), NULL, NULL) == 0);
    assert(conn->local_cid_count == 3);

    /* Active should still be seq 0 (lowest) */
    active = quic_active_cid(conn);
    assert(active->seq_num == 0);

    /* Retire seq 0 and 1 */
    int retired = quic_retire_cids_prior_to(conn, 2);
    assert(retired == 2);

    /* Active should now be seq 2 */
    active = quic_active_cid(conn);
    assert(active != NULL);
    assert(active->seq_num == 2);
    assert(memcmp(active->cid, cid2, 2) == 0);
}

static void test_issue_cid_overflow(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid[] = {0x01};
    struct quic_peer_addr addr = {.ip = 0, .port = 0};
    int idx = quic_conn_table_add(&table, cid, sizeof(cid), addr);
    struct quic_connection *conn = &table.conns[idx];

    /* Already has 1 CID (initial).  Issue up to max. */
    uint8_t cid_n[1];
    for (int i = 1; i < QUIC_MAX_CIDS_PER_CONN; i++) {
        cid_n[0] = (uint8_t)(i + 0x10);
        assert(quic_issue_cid(conn, cid_n, 1, NULL, NULL) == 0);
    }
    assert(conn->local_cid_count == QUIC_MAX_CIDS_PER_CONN);

    /* One more should fail */
    cid_n[0] = 0xFF;
    assert(quic_issue_cid(conn, cid_n, 1, NULL, NULL) == -1);
}

/* ---- Migration simulation --------------------------------------------- */

static void test_detect_migration_new_addr(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid[] = {0xDE, 0xAD, 0xBE, 0xEF};
    struct quic_peer_addr wifi = {.ip = (192u << 24) | (168u << 16) | (1u << 8) | 50u,
                                  .port = 12345};
    int idx = quic_conn_table_add(&table, cid, sizeof(cid), wifi);
    struct quic_connection *conn = &table.conns[idx];

    /* Same address: no migration */
    assert(quic_detect_migration(conn, wifi) == 0);
    assert(conn->path_validated == 1);

    /* Different address: migration detected */
    struct quic_peer_addr cell = {.ip = (10u << 24) | 1u, .port = 54321};
    assert(quic_detect_migration(conn, cell) == 1);
    assert(conn->path_validated == 0);
    assert(conn->challenge_pending == 1);
    assert(conn->migration_count == 1);
}

static void test_complete_migration_success(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid[] = {0x01, 0x02, 0x03, 0x04};
    struct quic_peer_addr addr1 = {.ip = 0xC0A80132, .port = 1000};
    int idx = quic_conn_table_add(&table, cid, sizeof(cid), addr1);
    struct quic_connection *conn = &table.conns[idx];

    struct quic_peer_addr addr2 = {.ip = 0x0A000001, .port = 2000};
    quic_detect_migration(conn, addr2);

    /* Correct response */
    struct quic_path_challenge resp = conn->pending_challenge;
    assert(quic_complete_migration(conn, &resp) == 0);
    assert(conn->path_validated == 1);
    assert(conn->challenge_pending == 0);
}

static void test_complete_migration_wrong_response(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid[] = {0x01, 0x02, 0x03, 0x04};
    struct quic_peer_addr addr1 = {.ip = 0xC0A80132, .port = 1000};
    int idx = quic_conn_table_add(&table, cid, sizeof(cid), addr1);
    struct quic_connection *conn = &table.conns[idx];

    struct quic_peer_addr addr2 = {.ip = 0x0A000001, .port = 2000};
    quic_detect_migration(conn, addr2);

    /* Wrong response */
    struct quic_path_challenge wrong;
    memset(wrong.data, 0xFF, QUIC_PATH_CHALLENGE_LEN);
    assert(quic_complete_migration(conn, &wrong) == -1);
    assert(conn->path_validated == 0);
    assert(conn->challenge_pending == 1);
}

static void test_complete_migration_no_pending(void) {
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid[] = {0x01, 0x02};
    struct quic_peer_addr addr = {.ip = 0xC0A80132, .port = 443};
    int idx = quic_conn_table_add(&table, cid, sizeof(cid), addr);
    struct quic_connection *conn = &table.conns[idx];

    /* No migration pending */
    struct quic_path_challenge resp;
    memset(resp.data, 0, QUIC_PATH_CHALLENGE_LEN);
    assert(quic_complete_migration(conn, &resp) == -1);
}

static void test_full_migration_wifi_to_cellular(void) {
    /*
     * End-to-end test: simulate migration from WiFi (192.168.1.50:12345)
     * to cellular (10.0.0.1:54321).  Verify the connection survives.
     */
    struct quic_conn_table table;
    quic_conn_table_init(&table);

    uint8_t cid1[] = {0xDE, 0xAD, 0xBE, 0xEF};
    struct quic_peer_addr wifi = {.ip = (192u << 24) | (168u << 16) | (1u << 8) | 50u,
                                  .port = 12345};
    int idx = quic_conn_table_add(&table, cid1, sizeof(cid1), wifi);
    assert(idx >= 0);

    /* Issue a second CID for post-migration use */
    uint8_t cid2[] = {0xCA, 0xFE, 0xBA, 0xBE};
    uint8_t token[16];
    memset(token, 0xAA, 16);
    assert(quic_issue_cid(&table.conns[idx], cid2, sizeof(cid2), token, NULL) == 0);

    /* Client migrates to cellular */
    struct quic_peer_addr cell = {.ip = (10u << 24) | 1u, .port = 54321};
    struct quic_connection *conn = &table.conns[idx];
    assert(quic_detect_migration(conn, cell) == 1);

    /* Build PATH_CHALLENGE frame */
    uint8_t frame_buf[64];
    size_t frame_n =
        quic_path_challenge_build(&conn->pending_challenge, frame_buf, sizeof(frame_buf));
    assert(frame_n == 9);

    /* Parse it on the peer side and build PATH_RESPONSE */
    struct quic_path_challenge peer_ch;
    size_t consumed;
    assert(quic_path_challenge_parse(frame_buf, frame_n, &peer_ch, &consumed) == 0);

    uint8_t resp_buf[64];
    size_t resp_n = quic_path_response_build(&peer_ch, resp_buf, sizeof(resp_buf));
    assert(resp_n == 9);

    /* Parse PATH_RESPONSE and complete migration */
    struct quic_path_challenge resp_parsed;
    assert(quic_path_response_parse(resp_buf, resp_n, &resp_parsed, &consumed) == 0);
    assert(quic_complete_migration(conn, &resp_parsed) == 0);

    /* Connection survives */
    assert(conn->path_validated == 1);
    assert(conn->peer_addr.ip == cell.ip);
    assert(conn->peer_addr.port == cell.port);

    /* Retire old CID for unlinkability */
    assert(quic_retire_cids_prior_to(conn, 1) == 1);

    /* Connection is reachable by new CID */
    assert(quic_match_connection(&table, cid2, sizeof(cid2)) == idx);
    /* Old CID retired */
    assert(quic_match_connection(&table, cid1, sizeof(cid1)) == -1);
}

static void test_build_buffer_too_small(void) {
    /* PATH_CHALLENGE needs 9 bytes; give it 5 */
    struct quic_path_challenge ch;
    memset(ch.data, 0, 8);
    uint8_t buf[5];
    assert(quic_path_challenge_build(&ch, buf, sizeof(buf)) == 0);

    /* PATH_RESPONSE same */
    assert(quic_path_response_build(&ch, buf, sizeof(buf)) == 0);

    /* NEW_CONNECTION_ID */
    struct quic_new_cid_frame f = {0};
    f.seq_num = 0;
    f.retire_prior_to = 0;
    f.cid_len = 4;
    assert(quic_new_cid_build(&f, buf, sizeof(buf)) == 0);

    /* RETIRE_CONNECTION_ID needs 2 bytes; give it 1 */
    struct quic_retire_cid_frame rf = {.seq_num = 0};
    uint8_t tiny[1];
    assert(quic_retire_cid_build(&rf, tiny, sizeof(tiny)) == 0);
}

/* ---- Main ------------------------------------------------------------- */

int main(void) {
    /* PATH_CHALLENGE / PATH_RESPONSE */
    test_path_challenge_roundtrip();
    test_path_response_roundtrip();
    test_path_validate_match();
    test_path_validate_mismatch();
    test_path_challenge_wrong_type();
    test_path_response_wrong_type();
    test_path_challenge_truncated();
    test_path_challenge_exact_bytes();

    /* NEW_CONNECTION_ID */
    test_new_cid_roundtrip();
    test_new_cid_exact_wire_format();
    test_new_cid_reject_retire_gt_seq();
    test_new_cid_reject_zero_cid_len();
    test_new_cid_reject_truncated();
    test_new_cid_reject_wrong_type();

    /* RETIRE_CONNECTION_ID */
    test_retire_cid_roundtrip();
    test_retire_cid_truncated();
    test_retire_cid_wrong_type();

    /* Connection table lookup */
    test_match_by_cid();
    test_match_multiple_cids();
    test_match_after_retire();
    test_conn_table_full();

    /* CID lifecycle */
    test_issue_and_active_cid();
    test_issue_cid_overflow();

    /* Migration simulation */
    test_detect_migration_new_addr();
    test_complete_migration_success();
    test_complete_migration_wrong_response();
    test_complete_migration_no_pending();
    test_full_migration_wifi_to_cellular();

    /* Buffer edge cases */
    test_build_buffer_too_small();

    printf("All 29 QUIC connection migration tests passed.\n");
    return 0;
}
