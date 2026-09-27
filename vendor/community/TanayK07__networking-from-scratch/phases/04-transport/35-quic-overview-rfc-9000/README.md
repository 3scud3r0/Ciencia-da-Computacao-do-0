# 35. QUIC overview (RFC 9000)

> By the end of this lesson you will understand why QUIC exists, how it differs from TCP+TLS at the wire level, and you will have implemented QUIC's variable-length integer encoding — the primitive that every QUIC parser begins with.

## 1. Problem

Open a browser and visit any Google service. Check the protocol column in DevTools → Network: it says `h3`, meaning HTTP/3 over QUIC. As of 2024, QUIC carries roughly 30% of global web traffic (W3Techs, Google Transparency Report). YouTube, Google Search, Meta, Cloudflare, and Akamai all serve QUIC in production.

Yet QUIC runs on UDP, has its own framing, its own congestion control, its own TLS integration, and its own connection identifier scheme. If you only know TCP, you are blind to how a third of the internet actually works.

This lesson covers the *why* and *what* of QUIC. The four lessons that follow cover packets and frames (36), streams (37), connection migration (38), and congestion control (39).

## 2. Theory

### Why TCP couldn't evolve

TCP was designed in 1981 (RFC 793). By the 2010s, decades of middlebox deployment — firewalls, NATs, load balancers, DPI boxes — had created a problem the IETF calls **ossification**: any new TCP option or behavior change gets mangled or dropped by at least one middlebox somewhere on the path.

> "We found that approximately 6.5% of paths actively strip unknown TCP options, and a further 1.7% of paths cause connections to fail entirely when unknown options are present."
> — Trammell & Kühlewind, *"Measuring the Ossification of the Protocol Stack"*, 2017

Google engineers discovered this when trying to deploy TCP Fast Open (RFC 7413) and Multipath TCP (RFC 8684). Both saw high failure rates because middleboxes interfered with the new option bytes. The conclusion: **TCP's wire format is effectively frozen.**

### The QUIC design bet

Jim Roskind at Google proposed the solution in 2012: build a new transport protocol *on top of UDP*. Middleboxes know how to forward UDP datagrams. They cannot inspect or modify what's inside if it's encrypted. QUIC encrypts almost everything — including transport headers that TCP leaves in the clear.

The key design decisions:

1. **UDP as substrate.** Middleboxes forward UDP without inspecting transport state. This lets QUIC deploy without permission from every middlebox vendor on the internet.

2. **TLS 1.3 integrated, not layered.** TCP+TLS requires two separate handshakes: TCP's three-way handshake (1 RTT), then TLS (1 RTT). Total: 2 RTTs before the first byte of application data. QUIC merges the transport and cryptographic handshakes into a single 1-RTT exchange. For resumed connections, QUIC supports 0-RTT: the client sends data in its very first packet.

3. **Connection IDs replace 5-tuples.** TCP identifies a connection by (src IP, src port, dst IP, dst port). When your phone switches from WiFi to cellular, the IP changes and every TCP connection breaks. QUIC identifies connections by a *Connection ID* — an opaque token chosen by each endpoint. The connection survives address changes.

4. **Streams at the transport layer.** TCP provides a single, ordered byte stream per connection. HTTP/2 multiplexes streams over that single TCP connection, but a single lost TCP segment blocks *all* streams (head-of-line blocking). QUIC builds multiplexed streams into the transport itself. A lost packet on stream 4 does not block data on streams 0, 1, 2, 3.

5. **Everything is encrypted.** In TCP, any on-path observer can read sequence numbers, window sizes, and option fields. In QUIC, only the first byte's header form bit, the fixed bit, and the connection ID (for routing) are visible in the clear. Packet numbers, frame types, and payload are all encrypted. This prevents future ossification.

### The handshake: 1-RTT vs. 2-RTT

```
       TCP + TLS 1.3                         QUIC
  Client           Server           Client           Server
    |                 |                |                 |
    |--- SYN -------->|                |--- Initial ---->|
    |<-- SYN-ACK -----|    1 RTT       |  (ClientHello)  |
    |--- ACK -------->|                |                 |
    |                 |                |<-- Initial -----|
    |--- ClientHello->|               |  (ServerHello,  |    1 RTT
    |<-- ServerHello--|    2 RTTs      |   EncExtns,     |
    |<-- Finished ----|                |   Cert, Fin)    |
    |--- Finished --->|                |                 |
    |                 |                |--- Handshake -->|
    | App data flows  |                |--- 1-RTT ----->|
    |                 |                | App data flows  |
```

For 0-RTT (resumed connections), the client sends application data in the *first* packet. The server can process it before the handshake completes. This is critical for latency-sensitive applications — it turns a cold page load into effectively zero transport overhead.

### Measured results

Google published deployment data in 2017 (Langley et al., SIGCOMM). Key numbers:

- **Google Search**: 8% latency reduction on desktop, 3.6% on mobile
- **YouTube**: 18% reduction in rebuffer rate on desktop, 15.3% on mobile
- **Tail latency**: improvements were "disproportionately strong in the long tail" — the worst-case connections improved the most

Meta reported similar results (Engineering at Meta, 2020): 6% fewer request errors, 20% reduction in tail latency, and up to 22% improvement in video streaming rebuffer intervals.

These gains come mostly from the 1-RTT handshake and head-of-line blocking elimination — not from raw throughput improvements. QUIC's advantage is largest on high-latency, lossy networks (mobile, developing regions).

### The cost

QUIC is not free:

- **CPU cost.** Every packet is encrypted and authenticated individually (AEAD). TCP offloads checksumming to the NIC; QUIC cannot offload per-packet encryption without hardware support. Google reported QUIC was approximately 2x more CPU-expensive than TCP+TLS at deployment (Langley et al., SIGCOMM 2017).
- **UDP amplification.** UDP has no built-in handshake, so QUIC's Initial packets must be at least 1200 bytes (padded) to prevent amplification attacks (RFC 9000, §14.1). Servers must not send more than 3x the data they've received before the client's address is validated.
- **Middlebox hostility.** Some corporate firewalls block UDP/443. Browsers fall back to TCP+TLS when this happens. QUIC must coexist with TCP, not replace it.

### Timeline

| Year | Event |
|------|-------|
| 2012 | Jim Roskind proposes QUIC at Google |
| 2013 | Google deploys gQUIC in Chrome + Google servers |
| 2016 | IETF QUIC Working Group formed; fresh design based on gQUIC |
| 2018 | HTTP/3 name adopted (draft-ietf-quic-http) |
| 2021 | RFC 9000 (transport), RFC 9001 (TLS), RFC 9002 (loss detection) published |
| 2022 | RFC 9369 (QUIC-LB), RFC 9221 (Unreliable Datagrams) |
| 2024 | ~30% of global web traffic uses QUIC (W3Techs); Meta reports 75% of its traffic on QUIC |

## 3. Math / Spec

### QUIC variable-length integer encoding (RFC 9000, §16)

Every length, stream ID, offset, and frame type in QUIC is encoded as a variable-length integer. The encoding uses the two most significant bits of the first byte to indicate the total length:

```
+------+--------+-------------+-----------------------+
| 2MSB | Length  | Usable Bits | Range                 |
+------+--------+-------------+-----------------------+
| 00   | 1 byte |  6 bits     | 0 .. 63               |
| 01   | 2 bytes| 14 bits     | 0 .. 16383            |
| 10   | 4 bytes| 30 bits     | 0 .. 1073741823       |
| 11   | 8 bytes| 62 bits     | 0 .. 4611686018427387903 |
+------+--------+-------------+-----------------------+
```

> **RFC 9000, §16:** "Variable-length integer encoding reserves the two most significant bits of the first byte to encode the base-2 logarithm of the integer encoding length in bytes. The integer value is encoded on the remaining bits, in network byte order."

Examples from the RFC:

| Encoded (hex) | Decoded | Why |
|---------------|---------|-----|
| `c2 19 7c 5e ff 14 e8 8c` | 151288809941952652 | `11` prefix → 8 bytes, mask off top 2 bits |
| `9d 7f 3e 7d` | 494878333 | `10` prefix → 4 bytes |
| `7b bd` | 15293 | `01` prefix → 2 bytes |
| `25` | 37 | `00` prefix → 1 byte |

### QUIC packet types (RFC 9000, §17)

QUIC has two header forms, distinguished by the first bit:

```
Header Form (1 bit) = 1 → Long Header (used during handshake)
Header Form (1 bit) = 0 → Short Header (used after handshake, 1-RTT)
```

**Long Header** (RFC 9000, §17.2):

```
Long Header Packet {
  Header Form (1) = 1,
  Fixed Bit (1) = 1,
  Long Packet Type (2),
  Type-Specific Bits (4),
  Version (32),
  Dest Connection ID Length (8),
  Dest Connection ID (0..160),
  Source Connection ID Length (8),
  Source Connection ID (0..160),
  Type-Specific Payload (..),
}
```

Long Packet Types: `00` = Initial, `01` = 0-RTT, `10` = Handshake, `11` = Retry.

**Short Header** (RFC 9000, §17.3) — used for all post-handshake data:

```
1-RTT Packet {
  Header Form (1) = 0,
  Fixed Bit (1) = 1,
  Spin Bit (1),
  Reserved Bits (2),
  Key Phase (1),
  Packet Number Length (2),
  Dest Connection ID (..),
  Packet Number (8..32),
  Packet Payload (..),
}
```

The short header omits Version, Source Connection ID Length, and Source Connection ID. The receiver already knows these from the handshake. This minimizes per-packet overhead for data transfer.

### Anti-amplification (RFC 9000, §14.1)

> "Prior to validating the client address, servers MUST NOT send more than three times as many bytes as the number of bytes they have received."

This 3x limit prevents attackers from using QUIC servers as traffic amplifiers. The client's Initial packet is padded to at least 1200 bytes to give the server enough "budget" to respond with its ServerHello and certificate chain.

## 4. Code

This lesson implements the QUIC variable-length integer encoder/decoder — the first building block any QUIC implementation needs. Every subsequent QUIC lesson builds on these functions.

The implementation is in three files:

- [`quic_varint.h`](quic_varint.h) — public API: `quic_varint_decode`, `quic_varint_encode`, `quic_varint_len`.
- [`quic_varint.c`](quic_varint.c) — the implementation, following RFC 9000 §16 exactly.
- [`main.c`](main.c) — a CLI tool that encodes/decodes varints from command-line arguments.

### Variable-length integer decoding

The decoder reads the 2-bit prefix, determines the length, then masks off the prefix bits and reads the remaining bytes in network order:

```c
int quic_varint_decode(const uint8_t *buf, size_t len, uint64_t *out,
                       size_t *consumed)
{
    if (len == 0) return -1;

    uint8_t prefix = buf[0] >> 6;
    size_t need = 1u << prefix;     /* 1, 2, 4, or 8 bytes */
    if (len < need) return -1;

    uint64_t val = buf[0] & 0x3F;   /* mask off the 2-bit prefix */
    for (size_t i = 1; i < need; i++)
        val = (val << 8) | buf[i];

    *out = val;
    *consumed = need;
    return 0;
}
```

The expression `1u << prefix` maps `00→1`, `01→2`, `10→4`, `11→8` — exactly the byte counts from the RFC table.

### Variable-length integer encoding

The encoder picks the smallest encoding that fits the value:

```c
int quic_varint_encode(uint64_t val, uint8_t *buf, size_t len,
                       size_t *written)
{
    size_t need = quic_varint_len(val);
    if (need == 0 || len < need) return -1;

    /* Determine the 2-bit prefix */
    uint8_t prefix;
    switch (need) {
        case 1: prefix = 0x00; break;
        case 2: prefix = 0x40; break;
        case 4: prefix = 0x80; break;
        case 8: prefix = 0xC0; break;
        default: return -1;
    }

    /* Write big-endian, then OR the prefix into the first byte */
    for (size_t i = need; i > 0; i--) {
        buf[i - 1] = (uint8_t)(val & 0xFF);
        val >>= 8;
    }
    buf[0] |= prefix;

    *written = need;
    return 0;
}
```

### Building and running

```bash
make            # builds ./quic_varint
./quic_varint 37 15293 494878333 151288809941952652
make test       # runs the test suite
```

Example output:

```
Encoding 37 → 25 (1 byte)
Encoding 15293 → 7b bd (2 bytes)
Encoding 494878333 → 9d 7f 3e 7d (4 bytes)
Encoding 151288809941952652 → c2 19 7c 5e ff 14 e8 8c (8 bytes)
```

These match the RFC 9000 §16 examples exactly.

## 5. Tests

```bash
make test
```

The test suite ([`tests/test_varint.c`](tests/test_varint.c)) covers:

| Test | What it verifies |
|------|-----------------|
| `test_rfc_examples` | All four RFC 9000 §16 examples decode correctly |
| `test_encode_rfc_examples` | Encoding the RFC values produces byte-identical output |
| `test_roundtrip` | Encode then decode returns the original value, for values at every boundary |
| `test_boundaries` | Values 63, 64, 16383, 16384, 1073741823, 1073741824 use the correct encoding length |
| `test_zero` | 0 encodes as a single byte `0x00` |
| `test_max` | The maximum value 4611686018427387903 (`2^62 - 1`) encodes and decodes correctly |
| `test_overflow` | Values ≥ `2^62` return an error |
| `test_truncated` | Buffers shorter than the prefix indicates return an error |
| `test_short_encode_buf` | Encoding into a too-small buffer returns an error without writing |

## 6. Exercises

### 1. Decode the RFC examples by hand (★)

Given these hex bytes, decode each variable-length integer *on paper*:

```
25
7b bd
9d 7f 3e 7d
c2 19 7c 5e ff 14 e8 8c
```

For each: identify the 2-bit prefix, determine the byte count, mask off the prefix, compute the integer in decimal. Verify your answers against `./quic_varint` output.

### 2. Fuzz the varint codec (★)

Write a simple fuzz harness: generate 10,000 random `uint64_t` values in range `[0, 2^62)`, encode each, then decode. Assert that the decoded value matches the original. Use `rand()` or read from `/dev/urandom`.

### 3. Parse a real QUIC Initial packet's header (★★)

Capture a QUIC connection with:

```bash
SSLKEYLOGFILE=/tmp/quic-keys.log curl --http3 https://quic.tech:8443 -o /dev/null
tshark -r /tmp/quic.pcap -T fields -e quic.packet_number -e quic.dcid -e quic.scid
```

From the first Initial packet, extract the raw bytes and manually decode:
1. The Header Form bit and packet type
2. The Version field (should be `0x00000001` for QUIC v1)
3. The Destination Connection ID Length and DCID
4. The Source Connection ID Length and SCID
5. Any variable-length integer fields (Token Length, Length)

### 4. Measure QUIC vs TCP+TLS handshake latency (★★)

Write a script that measures time-to-first-byte for the same URL over HTTP/3 (QUIC) and HTTP/2 (TCP+TLS 1.3):

```bash
curl -w '%{time_connect} %{time_appconnect} %{time_starttransfer}\n' \
     --http3 https://cloudflare-quic.com/
curl -w '%{time_connect} %{time_appconnect} %{time_starttransfer}\n' \
     --http2 https://cloudflare-quic.com/
```

Run 20 trials each. Compare `time_appconnect` (handshake completion). Is the 1-RTT vs 2-RTT difference visible? What other factors dominate?

### 5. Implement minimum-length encoding validation (★★)

RFC 9000 §16 does not *require* minimum-length encoding, but a strict parser might reject non-minimal encodings (e.g., encoding `37` as 2 bytes `0x40 0x25` instead of 1 byte `0x25`). Implement `quic_varint_decode_strict()` that returns an error if the encoding is not the shortest possible. Write tests with deliberately non-minimal encodings.

### 6. QUIC Version Negotiation packet parser (★★★)

A Version Negotiation packet (RFC 9000, §17.2.1) is sent when the server does not support the client's proposed version. It has a Long Header with Version = `0x00000000` followed by a list of 32-bit supported versions.

Implement a parser that:
1. Identifies the packet as Version Negotiation (version field == 0)
2. Extracts DCID and SCID
3. Reads and lists all supported versions
4. Validates that the packet length is consistent (each version is exactly 4 bytes)

Test with a hand-crafted Version Negotiation packet containing versions `0x00000001` (QUICv1) and `0x6b3343cf` (QUICv2, RFC 9369).

## References

- **RFC 9000** — *QUIC: A UDP-Based Multiplexed and Secure Transport* (Iyengar & Thomson, May 2021). The core transport specification.
- **RFC 9001** — *Using TLS to Secure QUIC* (Thomson & Turner, May 2021). How TLS 1.3 integrates with QUIC.
- **RFC 9002** — *QUIC Loss Detection and Congestion Control* (Iyengar & Swett, May 2021).
- **RFC 8999** — *Version-Independent Properties of QUIC* (Thomson, May 2021). What stays constant across QUIC versions.
- **RFC 9369** — *QUIC Version 2* (Duke, May 2023).
- **Langley et al.** — *"The QUIC Transport Protocol: Design and Internet-Scale Deployment"*, SIGCOMM 2017. Google's paper reporting early deployment results: 8% reduction in search latency, 18% reduction in YouTube rebuffer rate.
- **Trammell & Kühlewind** — *"Measuring the Ossification of the Protocol Stack"*, ACM IMC 2017. Empirical evidence for TCP ossification.
- **Marx et al.** — *"Same Standards, Different Decisions: A Study of QUIC and HTTP/3 Implementation Diversity"*, ACM SIGCOMM 2020.
