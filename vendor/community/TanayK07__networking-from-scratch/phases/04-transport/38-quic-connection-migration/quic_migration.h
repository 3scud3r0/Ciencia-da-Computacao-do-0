#ifndef NFS_QUIC_MIGRATION_H
#define NFS_QUIC_MIGRATION_H

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------
 * QUIC Connection Migration (RFC 9000, Section 9)
 *
 * A QUIC connection is identified by Connection IDs, not by the
 * 4-tuple (src IP, src port, dst IP, dst port).  When a peer
 * migrates to a new network path, the connection survives because
 * the Connection ID in the packet header still matches.
 *
 * This module implements:
 *   - Connection table keyed by Connection ID
 *   - PATH_CHALLENGE / PATH_RESPONSE frame builder & validator
 *   - Connection ID manager (issue, retire, rotate)
 *   - NEW_CONNECTION_ID / RETIRE_CONNECTION_ID frame builder/parser
 *   - Migration simulation: detect address change, validate path
 * --------------------------------------------------------------- */

#define QUIC_MAX_CID_LEN         20
#define QUIC_PATH_CHALLENGE_LEN  8  /* RFC 9000, §19.17 */
#define QUIC_STATELESS_RESET_LEN 16 /* RFC 9000, §19.15 */
#define QUIC_MAX_CONNECTIONS     64
#define QUIC_MAX_CIDS_PER_CONN   8

/* Frame type constants (RFC 9000, §19) */
#define QUIC_FRAME_NEW_CONNECTION_ID    0x18
#define QUIC_FRAME_RETIRE_CONNECTION_ID 0x19
#define QUIC_FRAME_PATH_CHALLENGE       0x1a
#define QUIC_FRAME_PATH_RESPONSE        0x1b

/* ---------------------------------------------------------------
 * PATH_CHALLENGE / PATH_RESPONSE frames (RFC 9000, §19.17-19.18)
 *
 * PATH_CHALLENGE: type(0x1a) + 8 bytes random data
 * PATH_RESPONSE:  type(0x1b) + 8 bytes echoed data
 * --------------------------------------------------------------- */

struct quic_path_challenge {
    uint8_t data[QUIC_PATH_CHALLENGE_LEN];
};

/* Build a PATH_CHALLENGE frame into buf.
 * Returns bytes written (9), or 0 on error. */
size_t quic_path_challenge_build(const struct quic_path_challenge *ch, uint8_t *buf, size_t len);

/* Parse a PATH_CHALLENGE frame from buf.
 * Returns 0 on success, -1 on error. */
int quic_path_challenge_parse(const uint8_t *buf, size_t len, struct quic_path_challenge *out,
                              size_t *consumed);

/* Build a PATH_RESPONSE frame from a challenge.
 * Returns bytes written (9), or 0 on error. */
size_t quic_path_response_build(const struct quic_path_challenge *ch, uint8_t *buf, size_t len);

/* Parse a PATH_RESPONSE frame from buf.
 * Returns 0 on success, -1 on error. */
int quic_path_response_parse(const uint8_t *buf, size_t len, struct quic_path_challenge *out,
                             size_t *consumed);

/* Validate that a PATH_RESPONSE matches a PATH_CHALLENGE.
 * Returns 1 if they match, 0 otherwise. */
int quic_path_validate(const struct quic_path_challenge *challenge,
                       const struct quic_path_challenge *response);

/* ---------------------------------------------------------------
 * Connection ID entry (RFC 9000, §5.1)
 * --------------------------------------------------------------- */

struct quic_cid_entry {
    uint64_t seq_num; /* sequence number */
    uint8_t cid_len;
    uint8_t cid[QUIC_MAX_CID_LEN];
    uint8_t stateless_reset_token[QUIC_STATELESS_RESET_LEN];
    int retired; /* 1 = retired */
};

/* ---------------------------------------------------------------
 * NEW_CONNECTION_ID frame (RFC 9000, §19.15)
 *
 * Wire format:
 *   Type (0x18)
 *   Sequence Number (varint)
 *   Retire Prior To (varint)
 *   Length (8 bits)
 *   Connection ID (0..20 bytes)
 *   Stateless Reset Token (128 bits / 16 bytes)
 *
 * For simplicity we encode Sequence Number and Retire Prior To
 * as 1-byte varints (values 0-63) in this teaching implementation.
 * --------------------------------------------------------------- */

struct quic_new_cid_frame {
    uint64_t seq_num;
    uint64_t retire_prior_to;
    uint8_t cid_len;
    uint8_t cid[QUIC_MAX_CID_LEN];
    uint8_t stateless_reset_token[QUIC_STATELESS_RESET_LEN];
};

/* Build a NEW_CONNECTION_ID frame into buf.
 * Returns bytes written, or 0 on error. */
size_t quic_new_cid_build(const struct quic_new_cid_frame *f, uint8_t *buf, size_t len);

/* Parse a NEW_CONNECTION_ID frame from buf.
 * Returns 0 on success, -1 on error. */
int quic_new_cid_parse(const uint8_t *buf, size_t len, struct quic_new_cid_frame *out,
                       size_t *consumed);

/* ---------------------------------------------------------------
 * RETIRE_CONNECTION_ID frame (RFC 9000, §19.16)
 *
 * Wire format:
 *   Type (0x19)
 *   Sequence Number (varint)
 * --------------------------------------------------------------- */

struct quic_retire_cid_frame {
    uint64_t seq_num;
};

/* Build a RETIRE_CONNECTION_ID frame into buf.
 * Returns bytes written (2 for seq < 64), or 0 on error. */
size_t quic_retire_cid_build(const struct quic_retire_cid_frame *f, uint8_t *buf, size_t len);

/* Parse a RETIRE_CONNECTION_ID frame from buf.
 * Returns 0 on success, -1 on error. */
int quic_retire_cid_parse(const uint8_t *buf, size_t len, struct quic_retire_cid_frame *out,
                          size_t *consumed);

/* ---------------------------------------------------------------
 * Peer address — simplified to IPv4 for this teaching code
 * --------------------------------------------------------------- */

struct quic_peer_addr {
    uint32_t ip; /* host byte order for simplicity */
    uint16_t port;
};

/* ---------------------------------------------------------------
 * Connection entry in the connection table
 * --------------------------------------------------------------- */

struct quic_connection {
    int active;

    /* CIDs we issued to the peer (peer puts these as DCID) */
    struct quic_cid_entry local_cids[QUIC_MAX_CIDS_PER_CONN];
    int local_cid_count;
    uint64_t next_cid_seq;

    /* The CID we use as DCID when sending to the peer */
    uint8_t peer_cid_len;
    uint8_t peer_cid[QUIC_MAX_CID_LEN];

    /* Current peer address */
    struct quic_peer_addr peer_addr;

    /* Path validation state */
    int path_validated;
    struct quic_path_challenge pending_challenge;
    int challenge_pending;

    /* Migration tracking */
    int migration_count;
};

/* ---------------------------------------------------------------
 * Connection table — lookup by Connection ID
 * --------------------------------------------------------------- */

struct quic_conn_table {
    struct quic_connection conns[QUIC_MAX_CONNECTIONS];
    int count;
};

/* Initialize an empty connection table. */
void quic_conn_table_init(struct quic_conn_table *table);

/* Add a connection, returning its index, or -1 on error. */
int quic_conn_table_add(struct quic_conn_table *table, const uint8_t *initial_cid, uint8_t cid_len,
                        struct quic_peer_addr addr);

/* Find a connection by Destination Connection ID.
 * Returns connection index, or -1 if not found.
 * This is the key operation: match by CID, not by address. */
int quic_match_connection(const struct quic_conn_table *table, const uint8_t *dcid,
                          uint8_t dcid_len);

/* ---------------------------------------------------------------
 * Connection ID manager
 * --------------------------------------------------------------- */

/* Issue a new CID for a connection. Fills in *entry with the new CID.
 * Returns 0 on success, -1 if too many CIDs. */
int quic_issue_cid(struct quic_connection *conn, const uint8_t *cid, uint8_t cid_len,
                   const uint8_t *reset_token, struct quic_cid_entry *entry);

/* Retire all CIDs with sequence number < retire_prior_to.
 * Returns the number of CIDs retired. */
int quic_retire_cids_prior_to(struct quic_connection *conn, uint64_t retire_prior_to);

/* Find the active (non-retired) CID with the lowest sequence number.
 * Returns pointer to the entry, or NULL if none. */
const struct quic_cid_entry *quic_active_cid(const struct quic_connection *conn);

/* ---------------------------------------------------------------
 * Migration simulation
 * --------------------------------------------------------------- */

/* Detect that a packet arrived from a new address and initiate
 * migration.  Sets challenge_pending and stores the challenge.
 * Returns 1 if migration detected, 0 if same address. */
int quic_detect_migration(struct quic_connection *conn, struct quic_peer_addr new_addr);

/* Complete path validation after a matching PATH_RESPONSE.
 * Updates peer_addr and marks the path as validated.
 * Returns 0 on success, -1 if no challenge was pending or
 * the response does not match. */
int quic_complete_migration(struct quic_connection *conn,
                            const struct quic_path_challenge *response);

#endif /* NFS_QUIC_MIGRATION_H */
