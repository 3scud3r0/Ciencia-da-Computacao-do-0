# 02. UDP header (RFC 768)

> By the end of this lesson you will parse and build UDP datagrams from raw bytes, compute the UDP checksum with an IPv4 pseudo-header, and understand why this 8-byte header is the simplest useful transport protocol.

## 1. Problem

DNS, DHCP, NTP, QUIC, game servers, video streaming — all of these run on UDP. When you type `dig example.com`, your resolver builds a UDP datagram, stuffs a DNS query into the payload, and sends it to port 53. The response comes back the same way: an 8-byte UDP header followed by the DNS answer.

TCP gives you reliable, ordered delivery. UDP gives you *nothing* — no connections, no retransmission, no flow control, no ordering. In exchange, it gives you the lowest possible overhead: 8 bytes of header, then your data is on the wire. If your application can tolerate loss (real-time audio/video) or has its own reliability (DNS retries, QUIC's built-in recovery), UDP is the right substrate.

This lesson parses the UDP header from raw bytes, builds UDP datagrams, and computes the checksum that protects them. It is the first hands-on protocol implementation in this phase.

## 2. Theory

### The header

UDP's header is defined in RFC 768 (J. Postel, August 1980) — one of the shortest RFCs ever written, at three pages. The entire protocol is this:

```
 0      7 8     15 16    23 24    31
+--------+--------+--------+--------+
|   Source Port    |  Dest Port      |
+--------+--------+--------+--------+
|     Length       |   Checksum      |
+--------+--------+--------+--------+
|          Payload ...               |
+------------------------------------+
```

Four fields, 8 bytes, no options, no extensions, no negotiation. That's it.

- **Source Port** (16 bits): the sender's port. Can be zero if no reply is expected.
- **Destination Port** (16 bits): the receiver's port. This is how the kernel demultiplexes datagrams to the right socket.
- **Length** (16 bits): total size of header + payload, in bytes. Minimum is 8 (empty payload). Maximum is 65535, though in practice IP fragmentation limits effective size.
- **Checksum** (16 bits): protects the header and payload against bit errors. In IPv4 it is optional — a value of `0x0000` means "not computed." In IPv6, it is mandatory (RFC 2460).

### Why the checksum covers a pseudo-header

The UDP checksum is not computed over the UDP datagram alone. It includes a 12-byte *pseudo-header* that contains the source IP, destination IP, protocol number (17), and UDP length. This catches the case where a datagram is delivered to the wrong host or with corrupted IP addresses — errors that a checksum over UDP bytes alone would miss.

> **RFC 768:** "If the computed checksum is zero, it is transmitted as all ones (the equivalent in one's complement arithmetic). An all zero transmitted checksum value means that the transmitter generated no checksum."

This means `0x0000` on the wire is special: it means the sender skipped the checksum. A computed checksum that happens to equal zero is sent as `0xFFFF` instead.

### UDP vs. TCP vs. raw IP

| Property | Raw IP | UDP | TCP |
|----------|--------|-----|-----|
| Multiplexing (ports) | ✗ | ✓ | ✓ |
| Checksum | ✗ (optional) | ✓ (optional IPv4) | ✓ (mandatory) |
| Reliable delivery | ✗ | ✗ | ✓ |
| Ordered delivery | ✗ | ✗ | ✓ |
| Connection state | ✗ | ✗ | ✓ |
| Header size | 20 bytes | 8 bytes | 20-60 bytes |

## 3. Math / Spec

### RFC 768 — the full spec

The entirety of the protocol format (RFC 768, August 1980):

```
 0      7 8     15 16    23 24    31
+--------+--------+--------+--------+
|     Source      |   Destination   |
|      Port       |      Port       |
+--------+--------+--------+--------+
|                 |                 |
|     Length       |    Checksum    |
+--------+--------+--------+--------+
|
|          data octets ...
+---------------- ...
```

All multi-byte fields are in network byte order (big-endian).

### IPv4 pseudo-header (RFC 768)

```
+--------+--------+--------+--------+
|           Source Address           |   4 bytes
+--------+--------+--------+--------+
|        Destination Address        |   4 bytes
+--------+--------+--------+--------+
|  zero  | Proto=17 |  UDP Length   |   4 bytes
+--------+--------+--------+--------+
```

The checksum is the 16-bit one's complement of the one's complement sum of the pseudo-header, the UDP header, and the payload. This is the standard Internet checksum (RFC 1071) — the same algorithm used for IPv4 headers and TCP segments.

### Length constraints

- Minimum UDP datagram: 8 bytes (header only, empty payload)
- Maximum UDP datagram: 65535 bytes (limited by the 16-bit Length field)
- Maximum payload over IPv4 without fragmentation: typically ~1472 bytes (1500 MTU − 20 IP header − 8 UDP header)

## 4. Code

The implementation is in three files:

- [`udp.h`](udp.h) — the packed header struct and function declarations.
- [`udp.c`](udp.c) — parsing, building, checksum (using `common/c/checksum.c`), and validation.
- [`main.c`](main.c) — a CLI tool that parses hex datagrams or builds new ones.

### The header struct

The struct mirrors the wire layout exactly. `_Static_assert` guarantees the compiler did not insert padding:

```c
struct nfs_udp_hdr {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

_Static_assert(sizeof(struct nfs_udp_hdr) == 8,
               "nfs_udp_hdr must be exactly 8 bytes");
```

### Parsing: network to host order

`nfs_udp_parse` reads the first 8 bytes and converts all fields from network (big-endian) to host order. This means callers work with plain integers — no `ntohs()` scattered through the code:

```c
out->src_port = (uint16_t)((buf[0] << 8) | buf[1]);
out->dst_port = (uint16_t)((buf[2] << 8) | buf[3]);
out->length   = (uint16_t)((buf[4] << 8) | buf[5]);
out->checksum = (uint16_t)((buf[6] << 8) | buf[7]);
```

### Checksum: pseudo-header + datagram

The checksum reuses the shared `internet_checksum_partial` / `internet_checksum_fold` functions. It accumulates the pseudo-header and UDP datagram separately, then folds:

```c
uint32_t sum = 0;
sum = internet_checksum_partial(pseudo, 12, sum);
sum = internet_checksum_partial(udp_buf, udp_len, sum);
uint16_t cs = internet_checksum_fold(sum);
if (cs == 0x0000) return 0xFFFF;  /* RFC 768 special case */
```

### Building and running

```bash
make                    # builds ./udp_parse
./udp_parse "003500350013a1b2 48656c6c6f"       # parse hex
./udp_parse --build 12345 53 "hello"             # build datagram
make test               # runs the test suite
```

## 5. Tests

```bash
make test
```

The test suite ([`tests/`](tests/)) covers:

| Test | What it verifies |
|------|-----------------|
| `test_struct_size` | `sizeof(nfs_udp_hdr) == 8` — struct matches wire |
| `test_parse_known` | Hand-crafted hex bytes parse to expected field values |
| `test_build_roundtrip` | Build then parse returns original values |
| `test_minimum_length` | Length < 8 is rejected |
| `test_truncated` | Buffer shorter than 8 bytes is rejected |
| `test_checksum_ipv4` | Pseudo-header checksum matches manual calculation |
| `test_checksum_zero_special` | Computed zero is transmitted as `0xFFFF` |
| `test_validate_good` | Valid datagram passes `nfs_udp_validate()` |
| `test_validate_bad` | Corrupted datagram fails validation |
| `test_validate_no_checksum` | Checksum `0x0000` (not computed) is accepted |

## 6. Exercises

See [`exercises.md`](exercises.md).

## References

- **RFC 768** — *User Datagram Protocol* (J. Postel, August 1980). The entire spec is three pages.
- **RFC 1071** — *Computing the Internet Checksum* (Braden, Borman, Partridge, 1988). The one's complement algorithm shared by IP, TCP, and UDP.
- **RFC 2460, §8.1** — *IPv6 Specification* (Deering & Hinden, 1998). Makes the UDP checksum mandatory over IPv6.
- **Stevens, W. R.** — *TCP/IP Illustrated, Volume 1*, Chapter 11. The definitive walkthrough of UDP's design and behavior.
