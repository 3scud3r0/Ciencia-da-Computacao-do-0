# 37. QUIC Streams

> By the end of this lesson you will classify any QUIC stream ID by its initiator and direction, build and parse STREAM frames with the exact wire format from RFC 9000, and reassemble out-of-order data into a contiguous byte stream -- the mechanism that eliminates head-of-line blocking.

## 1. Problem

Open two browser tabs to the same HTTPS site. Both tabs share a single TCP connection (HTTP/2 multiplexing). Now simulate 1% packet loss. Both tabs freeze, even though only one tab's data was affected. This is TCP head-of-line (HOL) blocking: a single lost segment stalls the entire byte stream, and every HTTP/2 stream behind it waits.

QUIC solves this with transport-layer streams. Each QUIC stream has its own receive buffer and its own reassembly state. A lost packet on stream 4 does not block data on streams 0, 1, 2, or 3. The receiver delivers data from unaffected streams immediately, while waiting only for the retransmission on the affected stream.

This lesson implements three things:
1. Stream ID classification -- extracting initiator and direction from the low 2 bits.
2. STREAM frame encoding and decoding -- the wire format that carries stream data.
3. A reassembly buffer -- accepting out-of-order chunks and producing contiguous output.

## 2. Theory

### Stream ID encoding

Every QUIC stream has a 62-bit ID (encoded as a varint). The two least significant bits encode two properties:

- **Bit 0**: who initiated the stream. `0` = client, `1` = server.
- **Bit 1**: directionality. `0` = bidirectional, `1` = unidirectional.

This gives four stream types, each with its own ID space:

```
Type 0x00: client-initiated, bidirectional   IDs: 0, 4, 8, 12, ...
Type 0x01: server-initiated, bidirectional   IDs: 1, 5, 9, 13, ...
Type 0x02: client-initiated, unidirectional  IDs: 2, 6, 10, 14, ...
Type 0x03: server-initiated, unidirectional  IDs: 3, 7, 11, 15, ...
```

The sequence number within each type is `stream_id >> 2`. So stream ID 8 is the third client-initiated bidirectional stream (sequence 2, zero-indexed).

This encoding means both endpoints can open streams concurrently without coordination -- the client uses even-numbered IDs, the server uses odd-numbered IDs. No negotiation needed.

### Head-of-line blocking elimination

In TCP, the kernel maintains a single reassembly buffer. If byte 1000 arrives but byte 999 is missing, the kernel holds byte 1000 and every subsequent byte until 999 is retransmitted and received. All HTTP/2 streams sharing that TCP connection are blocked.

QUIC maintains a separate reassembly buffer per stream:

```
Stream 0: [=====     =====]  ← gap at offset 5..10, waiting for retransmit
Stream 4: [===============]  ← fully received, delivered to application
Stream 8: [========        ]  ← partial, but contiguous from 0, delivered so far
```

Stream 4's data is delivered immediately. The loss on stream 0 blocks only stream 0. This is the single biggest latency win QUIC provides over TCP+HTTP/2, especially on lossy mobile networks.

### Stream state machines

RFC 9000 Section 3 defines two state machines per stream -- one for the sending side, one for the receiving side.

Sending states:
```
  +-------+
  | Ready |---(app opens stream)--->+------+
  +-------+                         | Send |
                                    +------+
                                       |
                              (all data sent)
                                       v
                                  +-----------+
                                  | Data Sent |
                                  +-----------+
                                       |
                              (all data ACKed)
                                       v
                                  +-----------+
                                  | Data Recvd|
                                  +-----------+
```

Receiving states:
```
  +------+
  | Recv |---(receive all data up to FIN)--->+------------+
  +------+                                   | Size Known |
                                             +------------+
                                                  |
                                        (all data received)
                                                  v
                                             +-----------+
                                             | Data Recvd|
                                             +-----------+
                                                  |
                                        (app reads all data)
                                                  v
                                             +-----------+
                                             | Data Read |
                                             +-----------+
```

A stream can also be cancelled at any point: the sender issues RESET_STREAM (type 0x04), and the receiver can request cancellation with STOP_SENDING (type 0x05).

### Flow control

QUIC enforces flow control at two levels:

1. **Stream-level**: each stream has a maximum offset the sender may reach. The receiver extends this with MAX_STREAM_DATA frames (type 0x11).
2. **Connection-level**: the sum of all data across all streams must not exceed a global limit. The receiver extends this with MAX_DATA frames (type 0x10).

The receiver also limits how many streams each side may open via MAX_STREAMS frames (type 0x12 for bidi, 0x13 for uni). This prevents a peer from opening millions of streams to exhaust memory.

## 3. Math / Spec

### Stream ID classification (RFC 9000, Section 2.1)

> "The least significant bit (0x01) of the stream ID identifies the initiator of the stream. Client-initiated streams have even-numbered stream IDs (with the bit set to 0), and server-initiated streams have odd-numbered stream IDs (with the bit set to 1)."
>
> "The second least significant bit (0x02) of the stream ID distinguishes between bidirectional streams (with the bit set to 0) and unidirectional streams (with the bit set to 1)."

The full classification table:

```
+------+-------+----------+------+-------------------+
| Bits | Init  | Dir      | Type | Example IDs       |
+------+-------+----------+------+-------------------+
| 0b00 | Client| Bidi     | 0x00 | 0, 4, 8, 12, ... |
| 0b01 | Server| Bidi     | 0x01 | 1, 5, 9, 13, ... |
| 0b10 | Client| Uni      | 0x02 | 2, 6, 10, 14, ...|
| 0b11 | Server| Uni      | 0x03 | 3, 7, 11, 15, ...|
+------+-------+----------+------+-------------------+
```

### STREAM frame format (RFC 9000, Section 19.8)

> "STREAM frames implicitly create a stream and carry stream data. The type for a STREAM frame is a value in the range 0x08 to 0x0f."

```
STREAM Frame {
  Type (8) = 0x08..0x0f,
  Stream ID (i),
  [Offset (i)],
  [Length (i)],
  Stream Data (..),
}
```

The three low bits of the type byte encode optional fields:

```
+-----+------+----------------------------------------------+
| Bit | Mask | Meaning                                      |
+-----+------+----------------------------------------------+
|  0  | 0x01 | FIN — marks end of stream data               |
|  1  | 0x02 | LEN — Length field is present                 |
|  2  | 0x04 | OFF — Offset field is present                |
+-----+------+----------------------------------------------+
```

All eight type values and their flags:

```
Type  Binary     OFF  LEN  FIN
0x08  0000 1000   0    0    0    Data at offset 0, extends to packet end
0x09  0000 1001   0    0    1    Data at offset 0, FIN, extends to packet end
0x0a  0000 1010   0    1    0    Data at offset 0, explicit length
0x0b  0000 1011   0    1    1    Data at offset 0, explicit length, FIN
0x0c  0000 1100   1    0    0    Data at offset, extends to packet end
0x0d  0000 1101   1    0    1    Data at offset, FIN, extends to packet end
0x0e  0000 1110   1    1    0    Data at offset, explicit length
0x0f  0000 1111   1    1    1    Data at offset, explicit length, FIN
```

When the LEN bit is not set, the stream data extends to the end of the QUIC packet. This saves 1-8 bytes per frame when there is only one STREAM frame in a packet (the common case).

### Wire example

A STREAM frame carrying "ABCDE" on stream 4 at offset 1000 with FIN:

```
0f                  type = 0x0f (OFF + LEN + FIN)
04                  stream ID = 4 (1-byte varint)
43 e8               offset = 1000 (2-byte varint: 0x40 | 0x03, 0xe8)
05                  length = 5 (1-byte varint)
41 42 43 44 45      "ABCDE"
```

Total: 10 bytes of framing for 5 bytes of payload.

### RESET_STREAM frame (RFC 9000, Section 19.4)

```
RESET_STREAM Frame {
  Type (i) = 0x04,
  Stream ID (i),
  Application Protocol Error Code (i),
  Final Size (i),
}
```

The sender uses RESET_STREAM to abruptly terminate a stream. The Final Size field tells the receiver how much data *would have been sent*, so the receiver can update its flow control accounting.

### STOP_SENDING frame (RFC 9000, Section 19.5)

```
STOP_SENDING Frame {
  Type (i) = 0x05,
  Stream ID (i),
  Application Protocol Error Code (i),
}
```

The receiver uses STOP_SENDING to tell the sender it is no longer interested in data on this stream. The sender should respond with RESET_STREAM.

### MAX_STREAMS frame (RFC 9000, Section 19.11)

```
MAX_STREAMS Frame {
  Type (i) = 0x12 or 0x13,
  Maximum Streams (i),
}
```

Type 0x12 limits bidirectional streams; 0x13 limits unidirectional streams. The value is the total number of streams the peer may open (cumulative, not incremental).

## 4. Code

The implementation is in three files:

- [`quic_stream.h`](quic_stream.h) -- public API: classification, frame codec, reassembly buffer.
- [`quic_stream.c`](quic_stream.c) -- implementation, depending on the varint codec from [Lesson 35](../35-quic-overview-rfc-9000/).
- [`main.c`](main.c) -- CLI tool that demos all three features.

### Stream classification

The classifier extracts the two low bits from the stream ID:

```c
void quic_stream_classify(uint64_t stream_id, struct quic_stream_info *info)
{
    info->initiator = (int)(stream_id & 0x01);       /* bit 0 */
    info->direction = (int)((stream_id >> 1) & 0x01); /* bit 1 */
    info->seq = stream_id >> 2;                        /* remaining bits */
}
```

The sequence number `stream_id >> 2` counts how many streams of that type have been opened. Stream 0 is the first client-bidi stream (seq 0). Stream 4 is the second (seq 1). Stream 8 is the third (seq 2).

### STREAM frame builder

The builder constructs the type byte by OR-ing the base type `0x08` with the caller's flags, then encodes the stream ID, optional offset, optional length, and data:

```c
int quic_stream_frame_build(uint64_t stream_id, uint64_t offset,
                            const uint8_t *data, size_t data_len,
                            int flags,
                            uint8_t *buf, size_t len, size_t *written)
{
    uint8_t type = 0x08 | (uint8_t)(flags & 0x07);
    buf[0] = type;
    /* ... encode stream_id, offset (if OFF), length (if LEN), data ... */
}
```

The flags argument uses the same bit constants as the wire format: `QUIC_STREAM_FIN_BIT` (0x01), `QUIC_STREAM_LEN_BIT` (0x02), `QUIC_STREAM_OFF_BIT` (0x04).

### STREAM frame parser

The parser reads the type byte, checks it falls in `0x08..0x0f`, then extracts optional fields based on the flag bits:

```c
int quic_stream_frame_parse(const uint8_t *buf, size_t len,
                            struct quic_stream_frame *out, size_t *consumed)
{
    uint8_t type = buf[0];
    if (type < 0x08 || type > 0x0f) return -1;

    out->fin = (type & 0x01) ? 1 : 0;
    /* decode stream_id (varint) */
    /* if OFF bit: decode offset (varint) */
    /* if LEN bit: decode length (varint), validate data available */
    /* else: data extends to end of buffer */
}
```

When the LEN bit is not set, `out->data` points to everything remaining in the buffer. The caller must know where the packet ends (from the packet's Length field or because the STREAM frame is the last frame in the packet).

### Stream reassembly buffer

The reassembly buffer stores received chunks as `(offset, length)` pairs and copies data into a flat byte array indexed by offset:

```c
int quic_stream_reasm_insert(struct quic_stream_reasm *r,
                             uint64_t offset, const uint8_t *data,
                             size_t len, int fin)
{
    /* Copy data into r->data[offset..offset+len) */
    memcpy(r->data + offset, data, len);

    /* Record the chunk, then sort and merge */
    r->chunks[r->num_chunks++] = (struct quic_stream_chunk){offset, len};
    merge_chunks(r);

    /* Track FIN */
    if (fin) { r->fin_received = 1; r->fin_offset = offset + len; }
    return 0;
}
```

`quic_stream_reasm_readable()` returns the size of the first contiguous chunk starting at offset 0. `quic_stream_reasm_complete()` returns 1 only when FIN has been received *and* all bytes from 0 to the FIN offset are contiguous.

### Building and running

```bash
make            # builds ./quic_stream
./quic_stream   # runs all demos: classify, build/parse, reassembly
make test       # runs the test suite (26 tests)
```

Subcommands:
```bash
./quic_stream classify 0 4 7 10     # classify specific stream IDs
./quic_stream build 4 1000 50 fin,len,off  # build a STREAM frame
./quic_stream reasm                  # demonstrate out-of-order reassembly
```

## 5. Tests

```bash
make test
```

The test suite ([`tests/test_stream.c`](tests/test_stream.c)) covers 26 cases:

| Group | Test | What it verifies |
|-------|------|-----------------|
| Classification | `test_classify_client_bidi` | IDs 0, 4, 8 are client-initiated bidirectional |
| | `test_classify_server_bidi` | IDs 1, 5, 9 are server-initiated bidirectional |
| | `test_classify_client_uni` | IDs 2, 6, 10 are client-initiated unidirectional |
| | `test_classify_server_uni` | IDs 3, 7, 11 are server-initiated unidirectional |
| | `test_classify_sequence_numbers` | Sequence = `stream_id >> 2` for IDs 0-11 |
| Frame codec | `test_frame_type_byte_no_flags` | No flags produces type byte 0x08 |
| | `test_frame_type_byte_all_flags` | OFF+LEN+FIN produces type byte 0x0f |
| | `test_frame_type_byte_fin_len` | FIN+LEN produces type byte 0x0b |
| | `test_frame_roundtrip_simple` | Build then parse returns identical fields |
| | `test_frame_roundtrip_with_offset_and_fin` | Stream 4, offset 1000, 50 bytes, FIN |
| | `test_frame_roundtrip_empty_fin` | Zero-length FIN on stream 16383 |
| | `test_frame_pin_bytes` | Exact wire bytes: `0f 04 43 e8 05 41 42 43 44 45` |
| | `test_frame_parse_without_len` | Data extends to buffer end when LEN not set |
| Errors | `test_frame_parse_reject_non_stream` | Type 0x07 rejected |
| | `test_frame_parse_reject_truncated` | Just type byte, no stream ID |
| | `test_frame_parse_reject_short_data` | LEN says 10 but only 3 bytes follow |
| | `test_frame_build_buffer_too_small` | 4-byte buffer for 50-byte payload |
| Reassembly | `test_reasm_in_order` | Sequential insert, complete after FIN |
| | `test_reasm_out_of_order` | 4 chunks at offsets 200, 0, 300, 100 reassemble correctly |
| | `test_reasm_overlapping` | Overlapping chunks merge without corruption |
| | `test_reasm_duplicate` | Duplicate insert is idempotent |
| | `test_reasm_empty_fin` | FIN with zero data bytes marks stream complete |
| | `test_reasm_fin_before_all_data` | FIN arrives before gap is filled |
| | `test_reasm_inconsistent_fin` | Two FINs at different offsets returns error |
| | `test_reasm_data_past_fin` | Data beyond FIN offset returns error |
| | `test_reasm_readable_with_gap` | Gap at offset 0 means readable = 0 |

## 6. Exercises

### 1. Classify stream IDs 0-11 against the RFC table (★)

Write `quic_stream_classify()` and test it with stream IDs 0 through 11. For each ID, verify:
- The initiator matches the RFC table (even = client, odd = server).
- The direction matches (bit 1 = 0 for bidi, 1 for uni).
- The sequence number equals `stream_id >> 2`.

Verify against `./quic_stream classify 0 1 2 3 4 5 6 7 8 9 10 11`.

### 2. Build a specific STREAM frame and pin the bytes (★)

Build a STREAM frame with stream ID 4, offset 1000, 50 bytes of payload, FIN set, LEN set, OFF set. Verify:
- The type byte is `0x0f` (all three flag bits set).
- The stream ID encodes as the single byte `0x04`.
- The offset 1000 encodes as the 2-byte varint `0x43 0xe8`.
- The total frame size is 55 bytes (1 + 1 + 2 + 1 + 50).

Run `./quic_stream build 4 1000 50 fin,len,off` and compare.

### 3. Simulate out-of-order delivery and verify reassembly (★★)

Send four 100-byte chunks at offsets 200, 0, 300, 100 (out of order). Set FIN on the last-arriving chunk (offset 300). After each insert, check `quic_stream_reasm_readable()`:
- After offset 200: readable = 0 (gap at 0).
- After offset 0: readable = 100.
- After offset 300 (FIN): readable = 100 (gap at 100..200).
- After offset 100: readable = 400, complete = true.

This demonstrates per-stream independent recovery: if these were four different streams, offsets 0 and 300 would be delivered immediately while waiting for the retransmit at 100.

### 4. Handle all eight STREAM type byte variants (★★)

Write a test that builds STREAM frames with every combination of OFF/LEN/FIN flags (type bytes 0x08 through 0x0f). For each:
1. Build the frame.
2. Parse it back.
3. Assert the parsed flags match the original.
4. Assert the data round-trips correctly.

Pay special attention to type 0x08 (no flags): the data extends to the end of the buffer, so the parser must handle this differently from types with the LEN bit set.

### 5. Enforce MAX_STREAMS limits (★★)

Implement a function `quic_streams_may_open(max_bidi, max_uni, stream_id)` that returns 1 if opening the given stream ID would not exceed the peer's MAX_STREAMS limits. The stream's sequence number (stream_id >> 2) must be less than the corresponding limit. Test:
- `max_bidi=3, max_uni=1, stream_id=8` (client-bidi seq 2) -> allowed (2 < 3).
- `max_bidi=3, max_uni=1, stream_id=12` (client-bidi seq 3) -> forbidden (3 >= 3).
- `max_bidi=3, max_uni=1, stream_id=6` (client-uni seq 1) -> forbidden (1 >= 1).

### 6. Implement stream-level flow control checking (★★★)

Add a `max_stream_data` field to the reassembly buffer. Before inserting data, check that `offset + len` does not exceed `max_stream_data`. If it does, return a FLOW_CONTROL_ERROR.

Then add connection-level flow control: track the sum of `max_stream_data` consumed across all streams, and reject inserts that would exceed a global `max_data` limit. Write tests showing:
- A single stream blocked at its per-stream limit.
- Multiple streams blocked at the connection-level limit even though each stream is within its own limit.
- Flow control window extended by simulating a MAX_STREAM_DATA update.

## References

- **RFC 9000, Section 2** -- *Streams* (stream types, bidirectional vs unidirectional, stream ID encoding).
- **RFC 9000, Section 3** -- *Stream States* (sending and receiving state machines, RESET_STREAM, STOP_SENDING).
- **RFC 9000, Section 4** -- *Flow Control* (stream-level and connection-level limits, MAX_DATA, MAX_STREAM_DATA).
- **RFC 9000, Section 19.4** -- *RESET_STREAM Frames* (abrupt stream termination).
- **RFC 9000, Section 19.5** -- *STOP_SENDING Frames* (receiver-initiated cancellation).
- **RFC 9000, Section 19.8** -- *STREAM Frames* (wire format, type byte encoding, OFF/LEN/FIN flags).
- **RFC 9000, Section 19.11** -- *MAX_STREAMS Frames* (concurrent stream limits).
- **Langley et al.** -- *"The QUIC Transport Protocol: Design and Internet-Scale Deployment"*, SIGCOMM 2017. Section 4.4 discusses head-of-line blocking measurements.
