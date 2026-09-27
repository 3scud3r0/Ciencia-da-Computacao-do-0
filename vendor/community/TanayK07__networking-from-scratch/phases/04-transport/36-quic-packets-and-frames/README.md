# 36. QUIC packets and frames

> By the end of this lesson you will parse QUIC long headers and short headers from raw bytes, classify all 21 QUIC frame types, and understand the packet type hierarchy that carries the QUIC handshake and application data.

## 1. Problem

A single UDP datagram arriving on port 443 might contain one or more QUIC packets — each with its own header, packet number, and payload. Inside each packet is a sequence of frames: STREAM frames carry data, ACK frames acknowledge delivery, CRYPTO frames carry the TLS handshake. If your parser misreads the first byte, it will misidentify the header form, read the wrong fields, and fail silently.

This lesson builds a QUIC packet header parser and a frame type classifier. Combined with the varint codec from [Lesson 35](../35-quic-overview-rfc-9000/), these form the foundation for all subsequent QUIC work.

## 2. Theory

### Two header forms

QUIC uses two header formats, distinguished by the first bit of the first byte:

- **Long Header** (bit 7 = 1): used during connection establishment. Carries Version, both Connection IDs, and a Length field. Four subtypes: Initial, 0-RTT, Handshake, Retry.
- **Short Header** (bit 7 = 0): used after the handshake for all 1-RTT data. Omits Version and Source Connection ID — both sides already know them. Minimal overhead for the common case.

The Long Header exists because during the handshake, the receiver does not yet know which Connection ID length the peer will use, what version they agreed on, or which keys apply. After the handshake, all that is negotiated, so the Short Header drops the redundant fields.

### Why a Length field matters

Long Header packets include a Length field (encoded as a varint) that tells the receiver how many bytes the packet occupies. This enables **packet coalescing** (RFC 9000, §12.2): multiple QUIC packets can be packed into a single UDP datagram. The receiver uses Length to find where one packet ends and the next begins.

Short Header packets do *not* have a Length field. They must be the last (or only) QUIC packet in a UDP datagram. You cannot coalesce two Short Header packets — there is no way to know where the first one ends.

### Packet number encoding

QUIC packet numbers are strictly monotonically increasing (unlike TCP sequence numbers, which count bytes and can repeat on retransmission). The packet number is encoded in 1-4 bytes, with the length indicated by the Packet Number Length field in the header. The actual packet number is reconstructed by combining the truncated value with the largest acknowledged packet number:

> **RFC 9000, §17.1:** "The packet number is protected using header protection; the length of the packet number field is encoded in lower two bits of the first byte... the encoded packet number is decoded by finding the packet number value that is closest to the next expected packet number."

### Frames: the unit of information

A QUIC packet payload is a sequence of frames. Each frame begins with a type byte (encoded as a varint), followed by type-specific fields. A single packet may contain multiple frames of different types — for example, an ACK frame followed by several STREAM frames.

RFC 9000 §12.4 defines 21 frame types. The frame type determines:
1. What fields follow
2. Which packet types may carry it (Initial, Handshake, 0-RTT, 1-RTT)
3. Whether it is ack-eliciting (the receiver must send an ACK)

### Packet type restrictions

Not every frame can appear in every packet type. For example:

| Frame | Initial | Handshake | 0-RTT | 1-RTT |
|-------|---------|-----------|-------|-------|
| PADDING | ✓ | ✓ | ✓ | ✓ |
| ACK | ✓ | ✓ | ✗ | ✓ |
| CRYPTO | ✓ | ✓ | ✗ | ✓ |
| STREAM | ✗ | ✗ | ✓ | ✓ |
| NEW_CONNECTION_ID | ✗ | ✗ | ✗ | ✓ |
| HANDSHAKE_DONE | ✗ | ✗ | ✗ | ✓ |

STREAM frames are forbidden in Initial and Handshake packets because those packet types use Initial/Handshake encryption keys — application data must wait for 1-RTT keys (or use 0-RTT keys for early data).

## 3. Math / Spec

### Long Header format (RFC 9000, §17.2)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
|1|1|T T|R R|P P|   First byte
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Version (32)                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| DCID Len (8)  |      Destination Connection ID (0..160)       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| SCID Len (8)  |       Source Connection ID (0..160)           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

First byte breakdown:
- Bit 7: Header Form = 1 (long header)
- Bit 6: Fixed Bit = 1
- Bits 5-4: Long Packet Type (00=Initial, 01=0-RTT, 10=Handshake, 11=Retry)
- Bits 3-2: Reserved (set to 0 before header protection)
- Bits 1-0: Packet Number Length minus 1

After the common fields, each type has specific additions:
- **Initial**: Token Length (varint) + Token + Length (varint) + Packet Number
- **0-RTT**: Length (varint) + Packet Number
- **Handshake**: Length (varint) + Packet Number
- **Retry**: Retry Token + Retry Integrity Tag (16 bytes)

### Short Header format (RFC 9000, §17.3)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+
|0|1|S|R R|K|P P|   First byte
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      Destination Connection ID (0..160)                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      Packet Number (8/16/24/32)                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- Bit 7: Header Form = 0 (short header)
- Bit 6: Fixed Bit = 1
- Bit 5: Spin Bit (for passive RTT measurement — see RFC 9312)
- Bits 4-3: Reserved
- Bit 2: Key Phase (for key updates)
- Bits 1-0: Packet Number Length minus 1

### All 21 frame types (RFC 9000, §19)

| Type | Name | Purpose |
|------|------|---------|
| 0x00 | PADDING | Increase packet size (e.g., Initial must be ≥1200 bytes) |
| 0x01 | PING | Keep-alive; elicits ACK |
| 0x02 | ACK | Acknowledge received packets |
| 0x03 | ACK_ECN | ACK with ECN counts |
| 0x04 | RESET_STREAM | Abruptly terminate a stream's sending side |
| 0x05 | STOP_SENDING | Request peer to stop sending on a stream |
| 0x06 | CRYPTO | Carry TLS handshake messages |
| 0x07 | NEW_TOKEN | Provide token for future 0-RTT |
| 0x08–0x0f | STREAM | Carry application data (3 low bits = OFF/LEN/FIN) |
| 0x10 | MAX_DATA | Connection-level flow control |
| 0x11 | MAX_STREAM_DATA | Stream-level flow control |
| 0x12 | MAX_STREAMS (bidi) | Limit on bidirectional streams peer may open |
| 0x13 | MAX_STREAMS (uni) | Limit on unidirectional streams |
| 0x14 | DATA_BLOCKED | Sender is blocked by connection flow control |
| 0x15 | STREAM_DATA_BLOCKED | Sender is blocked by stream flow control |
| 0x16 | STREAMS_BLOCKED (bidi) | Cannot open more bidirectional streams |
| 0x17 | STREAMS_BLOCKED (uni) | Cannot open more unidirectional streams |
| 0x18 | NEW_CONNECTION_ID | Supply a new Connection ID to peer |
| 0x19 | RETIRE_CONNECTION_ID | Ask peer to stop using a Connection ID |
| 0x1a | PATH_CHALLENGE | Validate a network path (8 random bytes) |
| 0x1b | PATH_RESPONSE | Reply to PATH_CHALLENGE (echo 8 bytes) |
| 0x1c | CONNECTION_CLOSE (QUIC) | Close connection with QUIC error code |
| 0x1d | CONNECTION_CLOSE (app) | Close connection with application error code |
| 0x1e | HANDSHAKE_DONE | Server signals handshake complete |

## 4. Code

The implementation builds on the varint codec from Lesson 35. Three new files:

- [`quic_packet.h`](quic_packet.h) — structs for long/short headers, frame type constants, public API.
- [`quic_packet.c`](quic_packet.c) — parsing and building long/short headers, frame name lookup.
- [`main.c`](main.c) — CLI that constructs one of each packet type, prints the hex, then parses it back.

### Parsing a Long Header

The parser reads the first byte, extracts the header form bit and packet type, then reads the fixed fields (version, DCID length, DCID, SCID length, SCID):

```c
int quic_long_hdr_parse(const uint8_t *buf, size_t len,
                        struct quic_long_hdr *out, size_t *consumed)
{
    if (len < 7) return -1;
    if ((buf[0] & 0x80) == 0) return -1;  /* not a long header */

    out->first_byte = buf[0];
    out->version = (uint32_t)buf[1] << 24 | (uint32_t)buf[2] << 16 |
                   (uint32_t)buf[3] << 8  | buf[4];

    size_t pos = 5;
    out->dcid_len = buf[pos++];
    if (out->dcid_len > 20 || pos + out->dcid_len > len) return -1;
    memcpy(out->dcid, buf + pos, out->dcid_len);
    pos += out->dcid_len;

    if (pos >= len) return -1;
    out->scid_len = buf[pos++];
    if (out->scid_len > 20 || pos + out->scid_len > len) return -1;
    memcpy(out->scid, buf + pos, out->scid_len);
    pos += out->scid_len;

    *consumed = pos;
    return 0;
}
```

### Frame type classification

```c
const char *quic_frame_name(uint64_t frame_type)
{
    if (frame_type == 0x00) return "PADDING";
    if (frame_type == 0x01) return "PING";
    if (frame_type == 0x02) return "ACK";
    if (frame_type == 0x03) return "ACK_ECN";
    if (frame_type >= 0x08 && frame_type <= 0x0f) return "STREAM";
    /* ... all 21 types ... */
}
```

### Building and running

```bash
make            # builds ./quic_packet
./quic_packet   # constructs and parses all four long header types
make test       # runs the test suite
```

## 5. Tests

```bash
make test
```

The test suite ([`tests/test_packet.c`](tests/test_packet.c)) covers:

| Test | What it verifies |
|------|-----------------|
| `test_long_hdr_initial` | Initial packet round-trips: version, DCID, SCID, type bits |
| `test_long_hdr_0rtt` | 0-RTT packet type bits are `01` in first byte |
| `test_long_hdr_handshake` | Handshake packet type bits are `10` |
| `test_long_hdr_retry` | Retry packet type bits are `11` |
| `test_short_hdr_roundtrip` | Short header round-trips with spin bit, key phase |
| `test_reject_truncated_long` | Buffer too short for long header → error |
| `test_reject_truncated_short` | Buffer too short for DCID → error |
| `test_dcid_max_20` | DCID length > 20 → rejected |
| `test_frame_names` | All 21 frame types map to correct string names |
| `test_frame_allowed_initial` | ACK and CRYPTO allowed in Initial; STREAM forbidden |
| `test_frame_allowed_1rtt` | All frame types allowed in 1-RTT packets |
| `test_coalesce_long_headers` | Two long header packets coalesced into one datagram |
| `test_header_form_bit` | Bit 7 = 1 → long, bit 7 = 0 → short |

## 6. Exercises

### 1. Decode a real Initial packet by hand (★)

Given this hex dump of the first 30 bytes of a QUIC Initial packet (from RFC 9001, Appendix A):

```
c0 00 00 00 01 08 83 94 c8 f0 3e 51 57 08 00 41
03 07 43 5f 63 5f 65 5f 67 5f 69 5f 6b 5f 6d
```

Decode every field by hand:
1. First byte `0xc0`: Header Form bit? Fixed bit? Packet type? Packet Number Length?
2. Version `00 00 00 01` — which QUIC version?
3. DCID Length and DCID bytes
4. SCID Length and SCID bytes
5. Any remaining varint fields (Token Length, Packet Length)

Verify against `./quic_packet -p <hex>` output.

### 2. Build all four long header types (★)

Using the code, construct one packet of each type (Initial, 0-RTT, Handshake, Retry) with DCID = `{0xDE, 0xAD}` and SCID = `{0xBE, 0xEF}`. Print the raw hex. Verify that `quic_long_hdr_type()` extracts the correct type from each first byte. Why is the Retry packet special — what does it lack that the other three have?

### 3. Implement STREAM frame encoding/decoding (★★)

A STREAM frame (type 0x08–0x0f) has this layout (RFC 9000, §19.8):

```
Type (1 byte) | Stream ID (varint) | [Offset (varint)] | [Length (varint)] | Data
```

The low 3 bits of the type byte encode optional fields:
- Bit 0 (0x01): FIN — this frame marks the end of the stream
- Bit 1 (0x02): LEN — Length field is present
- Bit 2 (0x04): OFF — Offset field is present

Implement `quic_stream_frame_build()` and `quic_stream_frame_parse()`. Test with:
- Stream 0, no offset, no length, 100 bytes, no FIN (type = 0x08)
- Stream 4, offset 1000, length present, 50 bytes, FIN (type = 0x0f)
- Stream 16383, offset 0, length present, 0 bytes, FIN (type = 0x0b)

### 4. Implement packet coalescing and decoalescing (★★)

RFC 9000 §12.2 allows multiple QUIC packets in one UDP datagram. Long Header packets include a Length field, so the receiver can find packet boundaries. Short Header packets lack Length and must be last.

Implement `quic_coalesce()` (combine packets) and `quic_decoalesce()` (split them apart). Test by coalescing Initial + Handshake packets, then decoalescing and verifying both parse correctly. Also test that decoalescing rejects a datagram that tries to coalesce two short header packets.

### 5. Write a frame-allowed-in-packet-type validator (★★)

RFC 9000, Table 3 specifies which frames are allowed in which packet types. Implement `quic_frame_allowed(frame_type, packet_type)` that returns 1 if the frame is allowed, 0 otherwise.

Test these cases (from the RFC):
- STREAM in 0-RTT → allowed
- STREAM in Initial → forbidden
- ACK in Handshake → allowed
- ACK in 0-RTT → forbidden
- HANDSHAKE_DONE in Handshake → forbidden
- HANDSHAKE_DONE in 1-RTT → allowed

### 6. Parse a pcap capture into QUIC packets (★★★)

Capture a real QUIC connection:

```bash
SSLKEYLOGFILE=/tmp/keys.log curl --http3 https://quic.tech:8443 -o /dev/null
tcpdump -i any -w /tmp/quic.pcap 'udp port 8443'
```

Write a program that reads the pcap file, strips the Ethernet + IP + UDP headers, and feeds the UDP payload to your QUIC parser. For each UDP datagram, print:
- Number of coalesced QUIC packets
- For each packet: header form (long/short), type, version, DCID, SCID, packet number length
- Count of frames by type in the entire capture

Verify that the first datagram contains coalesced Initial + Handshake packets.

## References

- **RFC 9000, §17** — Packet formats (Long Header §17.2, Short Header §17.3)
- **RFC 9000, §12** — Packets and frames: general structure, coalescing rules
- **RFC 9000, §19** — Frame types and formats (all 21 types defined here)
- **RFC 9000, §12.4, Table 3** — Which frames are permitted in which packet types
- **RFC 9001, Appendix A** — Example Initial packet bytes used in test vectors
- **RFC 9312** — QUIC spin bit for passive RTT measurement
