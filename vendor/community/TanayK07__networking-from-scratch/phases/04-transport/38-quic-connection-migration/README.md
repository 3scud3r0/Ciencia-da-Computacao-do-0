# 38. QUIC Connection Migration

> By the end of this lesson you will understand why TCP connections break on network change, implement a connection table that matches packets by Connection ID instead of address, build and validate PATH_CHALLENGE/PATH_RESPONSE frames, and simulate a seamless migration from WiFi to cellular.

## 1. Problem

You are on a video call over WiFi.  You walk out the door and your phone switches to cellular.  Your IP address changes from 192.168.1.50 to 10.0.0.1.  With TCP, the connection is dead.  TCP identifies connections by a 5-tuple: source IP, source port, destination IP, destination port, and protocol.  Change any one of those five values and the kernel cannot match the incoming packet to the socket.  The connection resets.  Your video freezes.  You reconnect, re-authenticate, re-negotiate TLS, and wait several RTTs before anything works again.

QUIC solves this.  A QUIC connection is identified by a Connection ID (CID) in the packet header, not by the 4-tuple.  When your phone's address changes, the CID stays the same.  The server matches the incoming packet to the existing connection, validates the new path with a PATH_CHALLENGE/PATH_RESPONSE exchange, and data resumes — often within a single RTT.

This lesson builds the machinery that makes this work: a connection table keyed by CID, the PATH_CHALLENGE/PATH_RESPONSE frame pair, a Connection ID manager that issues and retires CIDs, and the NEW_CONNECTION_ID frame that supplies fresh CIDs for unlinkability across paths.

## 2. Theory

### Why the 5-tuple fails

TCP and UDP demultiplex packets by (src IP, src port, dst IP, dst port).  This works when endpoints stay put.  It fails when they move.  NAT rebinding, WiFi-to-cellular handoffs, and VPN reconnections all change at least one element of the tuple.  The kernel sees an unknown tuple and drops the packet or sends a RST.

### Connection IDs: decoupling identity from address

QUIC places a Connection ID in every packet header (RFC 9000, §5.1).  The server allocates one or more CIDs for each connection and gives them to the client via NEW_CONNECTION_ID frames.  The client puts a CID in the Destination Connection ID field of every packet it sends.  When the server receives a packet, it looks up the connection by CID, not by source address.  If the CID matches, the packet belongs to that connection — regardless of where it came from.

```
TCP lookup:  (src_ip, src_port, dst_ip, dst_port) → connection
QUIC lookup: Destination Connection ID             → connection
```

### Path validation

An attacker could forge a packet with a valid CID but a spoofed source address, tricking the server into sending data to the wrong place.  QUIC prevents this with path validation (RFC 9000, §8.2).  When the server sees a packet from a new address, it sends a PATH_CHALLENGE frame containing 8 random bytes.  The client must echo those bytes back in a PATH_RESPONSE frame.  Only someone actually at the new address can receive the challenge and reply.

```
Server detects new source address
  │
  ├──► Send PATH_CHALLENGE (8 random bytes) to new address
  │
  │    Client at new address receives challenge
  │
  ◄──── Client sends PATH_RESPONSE (echo 8 bytes)
  │
  └──► Path validated.  Accept new address.
```

Until validation succeeds, the server limits data sent to the new address (anti-amplification, RFC 9000, §9.3).

### Connection ID rotation for unlinkability

If the client uses the same CID on both the WiFi and cellular paths, a network observer can link the two paths to the same connection.  To prevent this, RFC 9000 §9.5 requires using a new CID after migration.  The server pre-provisions spare CIDs via NEW_CONNECTION_ID frames.  After migration, the client retires the old CID (RETIRE_CONNECTION_ID frame) and switches to a fresh one.  A passive observer sees two different CIDs on two different paths and cannot correlate them.

### Congestion state reset

After migration, the network path may have completely different characteristics — different bandwidth, different RTT, different bottleneck.  RFC 9000 §9.4 requires the sender to reset its congestion controller and RTT estimates.  The old congestion window is meaningless on the new path.

## 3. Math / Spec

### PATH_CHALLENGE frame (RFC 9000, §19.17)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
|  Type (0x1a)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                         Data (64)                             +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Total: 9 bytes.  The 8-byte Data field must be unpredictable (RFC 9000, §8.2.1):

> "The endpoint MUST use unpredictable data in every PATH_CHALLENGE frame so that it can associate the peer's response with the corresponding challenge."

### PATH_RESPONSE frame (RFC 9000, §19.18)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
|  Type (0x1b)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                         Data (64)                             +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Total: 9 bytes.  The Data field must exactly echo the PATH_CHALLENGE data.

### NEW_CONNECTION_ID frame (RFC 9000, §19.15)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
|  Type (0x18)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Sequence Number (i)                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Retire Prior To (i)                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Length  (8)   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Connection ID (8..160)                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                Stateless Reset Token (128)                    |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Fields marked `(i)` use QUIC variable-length integer encoding.  The Retire Prior To field tells the peer to retire all CIDs with sequence number less than this value.  A constraint from the spec:

> "The value in the Retire Prior To field MUST be less than or equal to the value in the Sequence Number field." (RFC 9000, §19.15)

### RETIRE_CONNECTION_ID frame (RFC 9000, §19.16)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
|  Type (0x19)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Sequence Number (i)                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Sent by a peer to indicate it will no longer use a particular CID.

### disable_active_migration transport parameter (RFC 9000, §18.2)

A server can set `disable_active_migration` (0x0c) during the handshake to forbid client-initiated migration.  The client must not migrate unless the server explicitly allows it.  This is common for servers behind load balancers that route by source address.

## 4. Code

Three files:

- [`quic_migration.h`](quic_migration.h) — structs, frame constants, public API.
- [`quic_migration.c`](quic_migration.c) — frame builders/parsers, connection table, CID manager, migration logic.
- [`main.c`](main.c) — CLI demo that demonstrates each subsystem.

### Connection table: lookup by CID, not by address

The central data structure is a connection table where each connection owns one or more CIDs.  To find which connection a packet belongs to, we scan all active CIDs:

```c
int quic_match_connection(const struct quic_conn_table *table,
                          const uint8_t *dcid, uint8_t dcid_len)
{
    for (int i = 0; i < QUIC_MAX_CONNECTIONS; i++) {
        const struct quic_connection *conn = &table->conns[i];
        if (!conn->active)
            continue;
        for (int j = 0; j < conn->local_cid_count; j++) {
            const struct quic_cid_entry *e = &conn->local_cids[j];
            if (e->retired)
                continue;
            if (e->cid_len == dcid_len &&
                memcmp(e->cid, dcid, dcid_len) == 0)
                return i;
        }
    }
    return -1;
}
```

A production implementation would use a hash table.  The linear scan here is deliberately simple — the point is the lookup key.  TCP would match `(src_ip, src_port, dst_ip, dst_port)`.  QUIC matches only the Destination Connection ID.

### PATH_CHALLENGE / PATH_RESPONSE

Both frames have identical layout: 1-byte type + 8 bytes of data.  The builder is straightforward:

```c
size_t quic_path_challenge_build(const struct quic_path_challenge *ch,
                                 uint8_t *buf, size_t len)
{
    size_t need = 1 + QUIC_PATH_CHALLENGE_LEN;  /* 9 bytes */
    if (len < need)
        return 0;
    buf[0] = QUIC_FRAME_PATH_CHALLENGE;  /* 0x1a */
    memcpy(buf + 1, ch->data, QUIC_PATH_CHALLENGE_LEN);
    return need;
}
```

Validation compares the 8 bytes:

```c
int quic_path_validate(const struct quic_path_challenge *challenge,
                       const struct quic_path_challenge *response)
{
    return memcmp(challenge->data, response->data,
                  QUIC_PATH_CHALLENGE_LEN) == 0 ? 1 : 0;
}
```

### NEW_CONNECTION_ID frame

The builder serializes sequence number, retire-prior-to, CID length, the CID itself, and a 16-byte stateless reset token.  We use 1-byte varint encoding (values 0-63) to keep the code focused on migration semantics rather than varint mechanics (see [Lesson 35](../35-quic-overview-rfc-9000/)):

```c
size_t quic_new_cid_build(const struct quic_new_cid_frame *f,
                          uint8_t *buf, size_t len)
{
    if (f->cid_len == 0 || f->cid_len > QUIC_MAX_CID_LEN)
        return 0;
    size_t need = 4 + f->cid_len + QUIC_STATELESS_RESET_LEN;
    if (len < need)
        return 0;
    size_t pos = 0;
    buf[pos++] = QUIC_FRAME_NEW_CONNECTION_ID;   /* 0x18 */
    buf[pos++] = (uint8_t)f->seq_num;
    buf[pos++] = (uint8_t)f->retire_prior_to;
    buf[pos++] = f->cid_len;
    memcpy(buf + pos, f->cid, f->cid_len);
    pos += f->cid_len;
    memcpy(buf + pos, f->stateless_reset_token, QUIC_STATELESS_RESET_LEN);
    pos += QUIC_STATELESS_RESET_LEN;
    return pos;
}
```

The parser enforces the RFC constraint that `retire_prior_to <= seq_num`.

### Migration detection

When a packet arrives from an unexpected address, `quic_detect_migration()` marks the path as unvalidated and sets up a challenge:

```c
int quic_detect_migration(struct quic_connection *conn,
                          struct quic_peer_addr new_addr)
{
    if (conn->peer_addr.ip == new_addr.ip &&
        conn->peer_addr.port == new_addr.port)
        return 0;  /* same address, no migration */

    conn->peer_addr = new_addr;
    conn->path_validated = 0;
    conn->challenge_pending = 1;
    conn->migration_count++;
    /* fill pending_challenge with unpredictable data */
    ...
    return 1;
}
```

After the peer echoes the challenge in a PATH_RESPONSE, `quic_complete_migration()` validates the response and marks the path as validated.

### Building and running

```bash
make              # builds ./quic_migration
./quic_migration  # runs all demos
make test         # runs the test suite (29 tests)
```

Subcommands:

```bash
./quic_migration challenge  # PATH_CHALLENGE/RESPONSE demo only
./quic_migration newcid     # NEW_CONNECTION_ID frame demo only
./quic_migration migrate    # migration simulation only
```

## 5. Tests

```bash
make test
```

The test suite ([`tests/test_migration.c`](tests/test_migration.c)) covers:

| Test | What it verifies |
|------|-----------------|
| `test_path_challenge_roundtrip` | Build + parse PATH_CHALLENGE; type byte is 0x1a; data round-trips |
| `test_path_response_roundtrip` | Build + parse PATH_RESPONSE; type byte is 0x1b; data round-trips |
| `test_path_validate_match` | Identical 8-byte data validates successfully |
| `test_path_validate_mismatch` | One byte different causes validation failure |
| `test_path_challenge_wrong_type` | 0x1b frame rejected by PATH_CHALLENGE parser |
| `test_path_response_wrong_type` | 0x1a frame rejected by PATH_RESPONSE parser |
| `test_path_challenge_truncated` | Buffer < 9 bytes rejected |
| `test_path_challenge_exact_bytes` | Pin exact wire bytes: `1a 01 02 03 04 05 06 07 08` |
| `test_new_cid_roundtrip` | NEW_CONNECTION_ID frame round-trips all fields |
| `test_new_cid_exact_wire_format` | Pin exact wire layout: type + seq + retire + len + CID + token |
| `test_new_cid_reject_retire_gt_seq` | retire_prior_to > seq_num rejected per RFC 9000 §19.15 |
| `test_new_cid_reject_zero_cid_len` | CID length 0 rejected |
| `test_new_cid_reject_truncated` | Short buffer rejected |
| `test_new_cid_reject_wrong_type` | 0x19 frame rejected by NEW_CONNECTION_ID parser |
| `test_retire_cid_roundtrip` | RETIRE_CONNECTION_ID round-trips; type is 0x19 |
| `test_retire_cid_truncated` | 1-byte buffer rejected |
| `test_retire_cid_wrong_type` | 0x18 frame rejected by RETIRE parser |
| `test_match_by_cid` | Two connections found by their respective CIDs; unknown CID returns -1 |
| `test_match_multiple_cids` | Connection with 2 CIDs findable by either |
| `test_match_after_retire` | Retired CID no longer matches; non-retired CID still matches |
| `test_conn_table_full` | Table at capacity rejects new connections |
| `test_issue_and_active_cid` | Issue CIDs, retire old ones, active CID shifts correctly |
| `test_issue_cid_overflow` | Exceeding QUIC_MAX_CIDS_PER_CONN returns error |
| `test_detect_migration_new_addr` | Same address returns 0; different address returns 1 and clears validation |
| `test_complete_migration_success` | Correct PATH_RESPONSE completes migration |
| `test_complete_migration_wrong_response` | Wrong data leaves path unvalidated |
| `test_complete_migration_no_pending` | Completing without pending challenge returns error |
| `test_full_migration_wifi_to_cellular` | End-to-end: WiFi → cellular, CID rotation, path validation |
| `test_build_buffer_too_small` | All builders return 0 when buffer is too small |

## 6. Exercises

### 1. Decode PATH_CHALLENGE and PATH_RESPONSE by hand (★)

Given this 18-byte hex dump containing a PATH_CHALLENGE followed by a PATH_RESPONSE:

```
1a de ad be ef ca fe 00 01 1b de ad be ef ca fe 00 01
```

Decode every field:
1. First frame: type byte? Data bytes? How many bytes total?
2. Second frame: type byte? Data bytes?
3. Does the response match the challenge?
4. What would happen if the response data were `de ad be ef ca fe 00 02` instead?

Verify by building the same frames with `./quic_migration challenge`.

### 2. Implement `quic_match_connection()` and compare to TCP (★)

Write a program that creates a connection table with 3 connections, each with a unique CID.  Look up each connection by DCID and verify the correct one is returned.  Then explain in a comment: what fields would a TCP stack use for the same lookup?  Why does the CID-based lookup survive a NAT rebinding event while the TCP lookup does not?

### 3. Build and validate a PATH_CHALLENGE/PATH_RESPONSE pair (★★)

Using the code, build a PATH_CHALLENGE frame with data `{0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48}`.  Parse it, build a PATH_RESPONSE from the parsed data, and verify validation succeeds.  Then tamper with one byte in the response data and verify validation fails.  Print the exact hex of both frames.

### 4. Simulate migration from WiFi to cellular (★★)

Create a connection from 192.168.1.50:12345.  Issue two CIDs (seq 0 and seq 1).  Migrate the peer to 10.0.0.1:54321.  Walk through the full sequence:
1. Detect migration
2. Build PATH_CHALLENGE frame
3. Parse it, build PATH_RESPONSE
4. Complete migration
5. Retire the old CID (seq 0) for unlinkability
6. Verify the connection is still reachable by the new CID but not the old one

Print the peer address before and after migration.  Print the active CID before and after retirement.

### 5. Implement CID rotation with NEW_CONNECTION_ID frames (★★)

Extend the migration demo:  before migration, the server sends a NEW_CONNECTION_ID frame with `seq_num=2`, `retire_prior_to=1`, CID = `{0xFE, 0xED, 0xFA, 0xCE}`, and a 16-byte stateless reset token.  Build and parse the frame.  Apply the retire_prior_to field by calling `quic_retire_cids_prior_to()`.  Verify that after processing, the old CID (seq 0) is retired and the new CID is the active one.  What is the purpose of the stateless reset token?

### 6. Implement `disable_active_migration` enforcement (★★★)

RFC 9000, §18.2 defines the `disable_active_migration` transport parameter.  Add a `migration_disabled` flag to `struct quic_connection`.  Modify `quic_detect_migration()` to check this flag and return -1 (error) if migration is disabled.  Write tests that:
1. A connection with `migration_disabled = 0` migrates successfully
2. A connection with `migration_disabled = 1` rejects migration
3. The flag can be set during connection setup (simulate receiving the transport parameter)

Why would a server set this flag?  (Hint: consider servers behind a load balancer that routes by client IP.)

## References

- **RFC 9000, §9** — Connection Migration: the complete migration procedure
- **RFC 9000, §5.1** — Connection IDs: issuance, retirement, and usage rules
- **RFC 9000, §8.2** — Path Validation: PATH_CHALLENGE / PATH_RESPONSE exchange
- **RFC 9000, §9.3** — Anti-amplification limits during migration
- **RFC 9000, §9.4** — Congestion state after migration: reset required
- **RFC 9000, §9.5** — Privacy implications of connection migration: CID rotation
- **RFC 9000, §18.2** — `disable_active_migration` transport parameter
- **RFC 9000, §19.15** — NEW_CONNECTION_ID frame format
- **RFC 9000, §19.16** — RETIRE_CONNECTION_ID frame format
- **RFC 9000, §19.17** — PATH_CHALLENGE frame format
- **RFC 9000, §19.18** — PATH_RESPONSE frame format
