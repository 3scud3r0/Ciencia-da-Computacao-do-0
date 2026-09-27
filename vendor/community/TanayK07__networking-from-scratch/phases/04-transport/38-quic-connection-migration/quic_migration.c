/*
 * quic_migration.c -- QUIC Connection Migration implementation
 *
 * Implements RFC 9000 §9 (Connection Migration), §19.15-19.18
 * (NEW_CONNECTION_ID, RETIRE_CONNECTION_ID, PATH_CHALLENGE,
 * PATH_RESPONSE frames), and §5.1 (Connection ID management).
 */
#include "quic_migration.h"
#include <string.h>

/* ---------------------------------------------------------------
 * PATH_CHALLENGE / PATH_RESPONSE frames
 *
 * Both frames have identical wire format:
 *   Type (1 byte) + Data (8 bytes)
 *
 * PATH_CHALLENGE uses type 0x1a, PATH_RESPONSE uses 0x1b.
 * The 8-byte data in PATH_RESPONSE must exactly echo the data
 * from the PATH_CHALLENGE it replies to (RFC 9000, §8.2.2).
 * --------------------------------------------------------------- */

size_t quic_path_challenge_build(const struct quic_path_challenge *ch, uint8_t *buf, size_t len) {
    size_t need = 1 + QUIC_PATH_CHALLENGE_LEN; /* 9 bytes */
    if (len < need)
        return 0;

    buf[0] = QUIC_FRAME_PATH_CHALLENGE;
    memcpy(buf + 1, ch->data, QUIC_PATH_CHALLENGE_LEN);
    return need;
}

int quic_path_challenge_parse(const uint8_t *buf, size_t len, struct quic_path_challenge *out,
                              size_t *consumed) {
    size_t need = 1 + QUIC_PATH_CHALLENGE_LEN;
    if (len < need)
        return -1;
    if (buf[0] != QUIC_FRAME_PATH_CHALLENGE)
        return -1;

    memcpy(out->data, buf + 1, QUIC_PATH_CHALLENGE_LEN);
    *consumed = need;
    return 0;
}

size_t quic_path_response_build(const struct quic_path_challenge *ch, uint8_t *buf, size_t len) {
    size_t need = 1 + QUIC_PATH_CHALLENGE_LEN; /* 9 bytes */
    if (len < need)
        return 0;

    buf[0] = QUIC_FRAME_PATH_RESPONSE;
    memcpy(buf + 1, ch->data, QUIC_PATH_CHALLENGE_LEN);
    return need;
}

int quic_path_response_parse(const uint8_t *buf, size_t len, struct quic_path_challenge *out,
                             size_t *consumed) {
    size_t need = 1 + QUIC_PATH_CHALLENGE_LEN;
    if (len < need)
        return -1;
    if (buf[0] != QUIC_FRAME_PATH_RESPONSE)
        return -1;

    memcpy(out->data, buf + 1, QUIC_PATH_CHALLENGE_LEN);
    *consumed = need;
    return 0;
}

int quic_path_validate(const struct quic_path_challenge *challenge,
                       const struct quic_path_challenge *response) {
    return memcmp(challenge->data, response->data, QUIC_PATH_CHALLENGE_LEN) == 0 ? 1 : 0;
}

/* ---------------------------------------------------------------
 * NEW_CONNECTION_ID frame (RFC 9000, §19.15)
 *
 * Wire layout:
 *   0x18              (1 byte)  frame type
 *   Sequence Number   (varint)  we use 1-byte encoding (0..63)
 *   Retire Prior To   (varint)  we use 1-byte encoding (0..63)
 *   Length            (1 byte)  CID length, 1..20
 *   Connection ID     (Length bytes)
 *   Stateless Reset Token (16 bytes)
 *
 * A real implementation would use full varint encoding.  We use
 * 1-byte varints here to keep the code focused on migration
 * semantics rather than varint codec mechanics (see Lesson 35).
 * --------------------------------------------------------------- */

size_t quic_new_cid_build(const struct quic_new_cid_frame *f, uint8_t *buf, size_t len) {
    if (f->cid_len == 0 || f->cid_len > QUIC_MAX_CID_LEN)
        return 0;
    if (f->seq_num > 63 || f->retire_prior_to > 63)
        return 0;

    /* 1 (type) + 1 (seq) + 1 (retire) + 1 (len) + cid + 16 (token) */
    size_t need = 4 + f->cid_len + QUIC_STATELESS_RESET_LEN;
    if (len < need)
        return 0;

    size_t pos = 0;
    buf[pos++] = QUIC_FRAME_NEW_CONNECTION_ID;
    buf[pos++] = (uint8_t)f->seq_num;
    buf[pos++] = (uint8_t)f->retire_prior_to;
    buf[pos++] = f->cid_len;
    memcpy(buf + pos, f->cid, f->cid_len);
    pos += f->cid_len;
    memcpy(buf + pos, f->stateless_reset_token, QUIC_STATELESS_RESET_LEN);
    pos += QUIC_STATELESS_RESET_LEN;

    return pos;
}

int quic_new_cid_parse(const uint8_t *buf, size_t len, struct quic_new_cid_frame *out,
                       size_t *consumed) {
    /* Minimum: type(1) + seq(1) + retire(1) + len(1) + cid(1) + token(16) */
    if (len < 4 + 1 + QUIC_STATELESS_RESET_LEN)
        return -1;
    if (buf[0] != QUIC_FRAME_NEW_CONNECTION_ID)
        return -1;

    size_t pos = 1;
    out->seq_num = buf[pos++];
    out->retire_prior_to = buf[pos++];
    out->cid_len = buf[pos++];

    if (out->cid_len == 0 || out->cid_len > QUIC_MAX_CID_LEN)
        return -1;
    if (pos + out->cid_len + QUIC_STATELESS_RESET_LEN > len)
        return -1;

    /* RFC 9000, §19.15: retire_prior_to <= seq_num */
    if (out->retire_prior_to > out->seq_num)
        return -1;

    memcpy(out->cid, buf + pos, out->cid_len);
    pos += out->cid_len;
    memcpy(out->stateless_reset_token, buf + pos, QUIC_STATELESS_RESET_LEN);
    pos += QUIC_STATELESS_RESET_LEN;

    *consumed = pos;
    return 0;
}

/* ---------------------------------------------------------------
 * RETIRE_CONNECTION_ID frame (RFC 9000, §19.16)
 *
 * Wire layout:
 *   0x19              (1 byte)  frame type
 *   Sequence Number   (varint)  we use 1-byte encoding (0..63)
 * --------------------------------------------------------------- */

size_t quic_retire_cid_build(const struct quic_retire_cid_frame *f, uint8_t *buf, size_t len) {
    if (f->seq_num > 63)
        return 0;
    if (len < 2)
        return 0;

    buf[0] = QUIC_FRAME_RETIRE_CONNECTION_ID;
    buf[1] = (uint8_t)f->seq_num;
    return 2;
}

int quic_retire_cid_parse(const uint8_t *buf, size_t len, struct quic_retire_cid_frame *out,
                          size_t *consumed) {
    if (len < 2)
        return -1;
    if (buf[0] != QUIC_FRAME_RETIRE_CONNECTION_ID)
        return -1;

    out->seq_num = buf[1];
    *consumed = 2;
    return 0;
}

/* ---------------------------------------------------------------
 * Connection table
 *
 * The central insight of QUIC connection migration: we look up
 * connections by Connection ID, not by the 4-tuple.  A TCP stack
 * would match (src IP, src port, dst IP, dst port).  QUIC matches
 * the Destination Connection ID in the packet header.  When a
 * client moves from WiFi to cellular, the 4-tuple changes, but
 * the Connection ID stays the same — so the connection survives.
 * --------------------------------------------------------------- */

void quic_conn_table_init(struct quic_conn_table *table) {
    memset(table, 0, sizeof(*table));
}

int quic_conn_table_add(struct quic_conn_table *table, const uint8_t *initial_cid, uint8_t cid_len,
                        struct quic_peer_addr addr) {
    if (table->count >= QUIC_MAX_CONNECTIONS)
        return -1;
    if (cid_len == 0 || cid_len > QUIC_MAX_CID_LEN)
        return -1;

    /* Find empty slot */
    int idx = -1;
    for (int i = 0; i < QUIC_MAX_CONNECTIONS; i++) {
        if (!table->conns[i].active) {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return -1;

    struct quic_connection *conn = &table->conns[idx];
    memset(conn, 0, sizeof(*conn));
    conn->active = 1;
    conn->peer_addr = addr;
    conn->path_validated = 1; /* initial path is considered valid */

    /* Install the initial CID */
    struct quic_cid_entry *e = &conn->local_cids[0];
    e->seq_num = 0;
    e->cid_len = cid_len;
    memcpy(e->cid, initial_cid, cid_len);
    /* Stateless reset token is zero for the initial CID (RFC 9000, §5.1.1) */
    conn->local_cid_count = 1;
    conn->next_cid_seq = 1;

    table->count++;
    return idx;
}

int quic_match_connection(const struct quic_conn_table *table, const uint8_t *dcid,
                          uint8_t dcid_len) {
    /*
     * Scan all connections and all their active (non-retired) CIDs.
     * In production you would use a hash table.  For a teaching
     * implementation with <=64 connections and <=8 CIDs each,
     * a linear scan is clear and correct.
     */
    for (int i = 0; i < QUIC_MAX_CONNECTIONS; i++) {
        const struct quic_connection *conn = &table->conns[i];
        if (!conn->active)
            continue;

        for (int j = 0; j < conn->local_cid_count; j++) {
            const struct quic_cid_entry *e = &conn->local_cids[j];
            if (e->retired)
                continue;
            if (e->cid_len == dcid_len && memcmp(e->cid, dcid, dcid_len) == 0)
                return i;
        }
    }
    return -1;
}

/* ---------------------------------------------------------------
 * Connection ID manager
 * --------------------------------------------------------------- */

int quic_issue_cid(struct quic_connection *conn, const uint8_t *cid, uint8_t cid_len,
                   const uint8_t *reset_token, struct quic_cid_entry *entry) {
    if (conn->local_cid_count >= QUIC_MAX_CIDS_PER_CONN)
        return -1;
    if (cid_len == 0 || cid_len > QUIC_MAX_CID_LEN)
        return -1;

    struct quic_cid_entry *e = &conn->local_cids[conn->local_cid_count];
    e->seq_num = conn->next_cid_seq++;
    e->cid_len = cid_len;
    memcpy(e->cid, cid, cid_len);
    if (reset_token)
        memcpy(e->stateless_reset_token, reset_token, QUIC_STATELESS_RESET_LEN);
    else
        memset(e->stateless_reset_token, 0, QUIC_STATELESS_RESET_LEN);
    e->retired = 0;
    conn->local_cid_count++;

    if (entry)
        *entry = *e;
    return 0;
}

int quic_retire_cids_prior_to(struct quic_connection *conn, uint64_t retire_prior_to) {
    int retired = 0;
    for (int i = 0; i < conn->local_cid_count; i++) {
        struct quic_cid_entry *e = &conn->local_cids[i];
        if (!e->retired && e->seq_num < retire_prior_to) {
            e->retired = 1;
            retired++;
        }
    }
    return retired;
}

const struct quic_cid_entry *quic_active_cid(const struct quic_connection *conn) {
    const struct quic_cid_entry *best = NULL;
    for (int i = 0; i < conn->local_cid_count; i++) {
        const struct quic_cid_entry *e = &conn->local_cids[i];
        if (e->retired)
            continue;
        if (!best || e->seq_num < best->seq_num)
            best = e;
    }
    return best;
}

/* ---------------------------------------------------------------
 * Migration detection and path validation
 *
 * When a packet arrives from an address different from the last
 * known peer address, we:
 *   1. Record the new address as a candidate
 *   2. Send a PATH_CHALLENGE to the new address
 *   3. Wait for the PATH_RESPONSE
 *   4. If it matches, accept the new path
 *
 * Until path validation succeeds, the old path remains the
 * primary path.  This prevents an attacker from redirecting
 * traffic by spoofing the source address (RFC 9000, §9.3).
 * --------------------------------------------------------------- */

int quic_detect_migration(struct quic_connection *conn, struct quic_peer_addr new_addr) {
    if (conn->peer_addr.ip == new_addr.ip && conn->peer_addr.port == new_addr.port)
        return 0; /* same address, no migration */

    /*
     * Store the new address.  In a real implementation, you would
     * keep both old and new addresses until validation completes.
     * Here we update immediately but mark the path as unvalidated.
     */
    conn->peer_addr = new_addr;
    conn->path_validated = 0;
    conn->challenge_pending = 1;
    conn->migration_count++;

    /*
     * Fill the challenge with deterministic data derived from the
     * migration count.  A real implementation would use CSPRNG.
     * We use a simple counter-based fill for testability.
     */
    for (int i = 0; i < QUIC_PATH_CHALLENGE_LEN; i++)
        conn->pending_challenge.data[i] = (uint8_t)((conn->migration_count * 37 + i * 13) & 0xFF);

    return 1;
}

int quic_complete_migration(struct quic_connection *conn,
                            const struct quic_path_challenge *response) {
    if (!conn->challenge_pending)
        return -1;

    if (!quic_path_validate(&conn->pending_challenge, response))
        return -1;

    conn->path_validated = 1;
    conn->challenge_pending = 0;
    return 0;
}
