# 39. QUIC Congestion Control

> By the end of this lesson you will implement QUIC's NewReno congestion controller from RFC 9002, simulate the cwnd sawtooth through slow start, loss, and recovery, and understand why QUIC's unique packet numbers eliminate the retransmission ambiguity that plagues TCP.

## 1. Problem

You are sending data over a QUIC connection. The first 10 packets fly through in 50 ms. You send faster. At packet 40, the network drops a packet. Your application does not notice, so it keeps blasting. By packet 60, the router queue is full, and half your packets are dropped. Throughput collapses.

TCP solved this decades ago with congestion control. But QUIC cannot reuse TCP's congestion controller directly for two reasons:

1. **Retransmission ambiguity.** TCP retransmits a lost segment with the *same* sequence number. When the ACK arrives, the sender cannot tell whether it acknowledges the original or the retransmission. Karn's algorithm works around this by disabling RTT samples on retransmitted segments, but it still loses information. QUIC assigns every packet a unique, monotonically increasing packet number. Retransmitted data goes in a new packet with a new number. Every ACK is unambiguous.

2. **Multiple packet number spaces.** QUIC uses three separate packet number spaces — Initial, Handshake, and Application Data — each with its own ACK tracking. A loss in the Initial space should not affect the Application Data congestion window. TCP has one sequence number space for the entire connection.

This lesson builds a NewReno congestion controller for QUIC following RFC 9002, Section 7. The implementation handles slow start, congestion avoidance, loss response, recovery, persistent congestion, and PTO calculation.

## 2. Theory

### The congestion window (cwnd)

The congestion window limits how many bytes a sender may have in flight (sent but not yet acknowledged). The sender may transmit new data only when `bytes_in_flight < cwnd`.

QUIC's NewReno controller moves through three states:

```
                      ┌─────────────┐
              ┌──────▶│ Slow Start  │──────┐
              │       │ cwnd += ack │      │ cwnd >= ssthresh
              │       └─────────────┘      ▼
              │                     ┌──────────────────┐
  persistent  │                     │ Cong. Avoidance  │
  congestion  │                     │ cwnd += MSS*ack/ │
              │                     │          cwnd    │
              │                     └────────┬─────────┘
              │                              │ loss detected
              │       ┌─────────────┐        │
              │       │  Recovery   │◀───────┘
              └───────│ ssthresh =  │
                      │ cwnd * 0.5  │
                      └─────────────┘
```

**Slow start** doubles the window roughly every RTT. The sender adds `acked_bytes` to `cwnd` for every ACK. This is exponential growth — aggressive, but appropriate when the sender has no idea how much bandwidth the path can handle.

**Congestion avoidance** grows the window by one maximum datagram size per RTT. The formula `cwnd += max_datagram_size * acked_bytes / cwnd` spreads the increase across all ACKs in a window, producing linear growth.

**Recovery** begins when a loss is detected. The sender halves `cwnd` (setting `ssthresh = cwnd / 2`, then `cwnd = max(ssthresh, kMinimumWindow)`) and stays in recovery until a packet *sent after recovery began* is acknowledged. Multiple losses during the same recovery epoch do not trigger additional reductions — this prevents the window from collapsing to nothing during a burst loss.

### Unique packet numbers: why this matters

TCP reuses sequence numbers on retransmission:

```
TCP:   send seq=100 → lost → retransmit seq=100 → ACK 101
       Question: did the ACK cover the original or the retransmit?
       RTT sample is ambiguous.
```

QUIC assigns fresh numbers:

```
QUIC:  send pkt#5 (stream data offset 100-200) → lost
       retransmit as pkt#12 (same stream data) → ACK for pkt#12
       RTT sample = ack_time - pkt#12_sent_time (unambiguous)
```

This eliminates Karn's algorithm entirely. Every ACK yields a valid RTT sample. Better RTT estimates mean better PTO timers and better congestion decisions.

### Persistent congestion

If all packets in a time span exceeding `kPersistentCongestionThreshold * PTO` are lost, the sender assumes a path change or severe congestion. It resets `cwnd` to `kMinimumWindow` (2 packets) and re-enters slow start. This is more aggressive than a normal loss response — it is the QUIC equivalent of a TCP timeout.

## 3. Math / Spec

### Constants (RFC 9002, Sections 6.1 and 7)

| Constant | Value | Source |
|----------|-------|--------|
| `kInitialWindow` | `min(10 * max_datagram_size, max(14720, 2 * max_datagram_size))` | §7.2 |
| `kMinimumWindow` | `2 * max_datagram_size` | §7.2 |
| `kLossReductionFactor` | `0.5` | §7.3.2 |
| `kPacketThreshold` | `3` | §6.1.1 |
| `kTimeThreshold` | `max(SRTT, latest_rtt) * 9/8` | §6.1.2 |
| `kPersistentCongestionThreshold` | `3` | §7.6 |
| `kGranularity` | `1 ms` | §6.1.2 |

With `max_datagram_size = 1200` (the QUIC minimum):
- `kInitialWindow = min(12000, max(14720, 2400)) = min(12000, 14720) = 12000`
- `kMinimumWindow = 2400`

### Loss detection (RFC 9002, Section 6.1)

A packet is declared lost if either threshold is met:

> **§6.1.1 (Packet Threshold):** "A packet is declared lost if a later packet in the same packet number space has been acknowledged, and the packet was sent kPacketThreshold packets before an acknowledged packet."

> **§6.1.2 (Time Threshold):** "A packet is declared lost if it was sent a threshold amount of time in the past. [...] The RECOMMENDED time threshold, expressed as an RTT multiplier, is 9/8."

### Congestion avoidance formula (RFC 9002, Section 7.3.3)

> **§7.3.3:** "When the congestion window is larger than the slow start threshold, the sender is in congestion avoidance. [...] A sender in congestion avoidance increments the congestion window by a single maximum datagram size per congestion window acknowledged."

In code: `cwnd += max_datagram_size * acked_bytes / cwnd`

### PTO formula (RFC 9002, Section 6.2.1)

> **§6.2.1:** "PTO = smoothed_rtt + max(4*rttvar, kGranularity) + max_ack_delay"

Before any RTT sample exists, the RFC specifies an initial RTT of 333 ms (§6.2.2).

### RTT estimation (RFC 9002, Section 5.3)

First sample:
- `smoothed_rtt = rtt_sample`
- `rttvar = rtt_sample / 2`

Subsequent samples:
- `ack_delay` — subtracted from sample when it exceeds `min_rtt` (simplified away in our implementation)
- `rttvar = 3/4 * rttvar + 1/4 * |smoothed_rtt - adjusted_rtt|`
- `smoothed_rtt = 7/8 * smoothed_rtt + 1/8 * adjusted_rtt`

## 4. Code

Three files implement the congestion controller:

- [`quic_cc.h`](quic_cc.h) — constants, structs, and public API
- [`quic_cc.c`](quic_cc.c) — state machine, RTT estimation, loss detection, PTO
- [`main.c`](main.c) — simulation that sends 120 packets with losses at packets 40, 75, and 100

### Data structures

The congestion controller state lives in `struct quic_cc`:

```c
struct quic_cc {
    uint64_t cwnd;               /* congestion window (bytes) */
    uint64_t ssthresh;           /* slow start threshold */
    uint64_t bytes_in_flight;    /* unacknowledged bytes */
    enum quic_cc_state state;    /* SLOW_START / CONGESTION_AVOIDANCE / RECOVERY */
    uint64_t recovery_start_pn;  /* largest pkt_num when recovery began */
    struct quic_rtt rtt;         /* RTT estimator state */
    struct quic_sent_pkt sent[QUIC_MAX_SENT_PACKETS];
    size_t sent_count;
    uint64_t largest_acked_pn;
    /* ... */
};
```

Each sent packet is tracked for loss detection:

```c
struct quic_sent_pkt {
    uint64_t pkt_num;       /* unique, monotonically increasing */
    uint64_t sent_time_us;  /* when it was sent */
    uint32_t sent_bytes;    /* packet size */
    int      ack_eliciting; /* requires ACK? */
    int      in_flight;     /* counted toward bytes_in_flight? */
    int      lost;          /* declared lost? */
    int      acked;         /* acknowledged? */
};
```

### Initialization

```c
void quic_cc_init(struct quic_cc *cc)
{
    memset(cc, 0, sizeof(*cc));
    cc->cwnd = QUIC_INITIAL_WINDOW;     /* 12000 bytes */
    cc->ssthresh = UINT64_MAX;          /* no threshold until first loss */
    cc->state = QUIC_CC_SLOW_START;
    cc->rtt.min_rtt_us = UINT64_MAX;
    cc->max_ack_delay_us = 25000;       /* 25 ms default */
}
```

`ssthresh` starts at `UINT64_MAX` so the sender stays in slow start until the first loss event.

### ACK processing

When an ACK arrives, the sender updates RTT, removes bytes from in-flight, and grows `cwnd` according to the current state:

```c
/* Slow start: cwnd += acked_bytes */
if (cc->state == QUIC_CC_SLOW_START) {
    cc->cwnd += acked_bytes;
    if (cc->cwnd >= cc->ssthresh)
        cc->state = QUIC_CC_CONGESTION_AVOIDANCE;
    return 0;
}

/* Congestion avoidance: linear growth */
if (cc->state == QUIC_CC_CONGESTION_AVOIDANCE) {
    uint64_t increment = (uint64_t)QUIC_MAX_DATAGRAM_SIZE *
                         acked_bytes / cc->cwnd;
    if (increment == 0)
        increment = 1;
    cc->cwnd += increment;
}
```

During recovery, `cwnd` does not grow for packets sent before the recovery epoch. When a packet sent *after* recovery started gets acknowledged, the sender exits recovery and enters congestion avoidance.

### Loss response

```c
cc->ssthresh = cc->cwnd / QUIC_LOSS_REDUCTION_DENOM;       /* halve */
cc->cwnd = max_u64(cc->ssthresh, QUIC_MINIMUM_WINDOW);     /* floor */
cc->state = QUIC_CC_RECOVERY;
cc->recovery_start_pn = max_sent;  /* largest sent pkt_num */
```

The `recovery_start_pn` is set to the largest sent packet number. Any subsequent loss with `pkt_num <= recovery_start_pn` does not trigger a second reduction. This prevents cascading window halving during burst losses.

### Loss detection

The `quic_loss_detect()` function checks both thresholds for every in-flight packet:

```c
/* Packet threshold */
if (pkt->pkt_num + QUIC_PACKET_THRESHOLD <= cc->largest_acked_pn)
    /* → lost */

/* Time threshold */
uint64_t time_threshold = max(SRTT, latest_rtt) * 9 / 8;
if (now_us >= pkt->sent_time_us + time_threshold &&
    pkt->pkt_num < cc->largest_acked_pn)
    /* → lost */
```

### Building and running

```bash
make            # builds ./quic_cc
./quic_cc       # runs the simulation (120 packets, 3 loss events)
./quic_cc --pto # also prints PTO after each RTT sample
make test       # runs the test suite
```

Sample output (abbreviated):

```
PKT#    CWND        SSTHRESH    IN_FLIGHT   STATE         EVENT
0       13200       inf         0           SlowStart     slow_start
...
39      60000       inf         0           SlowStart     slow_start
40      30000       30000       0           Recovery      LOSS
41      30048       30000       0           CongAvoid     cong_avoid
...
75      15787       15787       0           Recovery      LOSS
76      15878       15787       0           CongAvoid     cong_avoid
...
100     8918        8918        0           Recovery      LOSS
101     9079        8918        0           CongAvoid     cong_avoid
...
119     11595       8918        0           CongAvoid     cong_avoid
```

The sawtooth is visible: exponential growth in slow start, then each loss halves `cwnd`, followed by linear recovery in congestion avoidance.

## 5. Tests

```bash
make test
```

The test suite ([`tests/test_cc.c`](tests/test_cc.c)) covers 19 cases:

| Test | What it verifies |
|------|-----------------|
| `test_init_values` | cwnd=12000, ssthresh=UINT64_MAX, state=SlowStart, no RTT sample |
| `test_packet_sent_tracking` | bytes_in_flight increments by sent_bytes, sent_count tracks |
| `test_slow_start_growth` | 5 ACKs of 1200B: cwnd goes 12000 → 13200 → ... → 18000 |
| `test_rtt_first_sample` | First 50ms sample: SRTT=50000, RTTVAR=25000, min=50000 |
| `test_rtt_subsequent_sample` | Second 60ms sample: SRTT=51250, RTTVAR=21250 (exact) |
| `test_loss_packet_threshold` | Packets 1-3 acked, packet 0 skipped → packet 0 declared lost |
| `test_loss_time_threshold` | Packet sent 200ms ago with SRTT=100ms → time threshold = 112500us → lost |
| `test_cwnd_on_loss` | cwnd=18000 → loss → ssthresh=9000, cwnd=9000, state=Recovery |
| `test_no_double_reduction` | Two losses in same epoch: cwnd reduced once, not twice |
| `test_congestion_avoidance_growth` | cwnd=12000, ack 1200B → cwnd += 1200*1200/12000 = 120 |
| `test_persistent_congestion` | cwnd resets to kMinimumWindow=2400, state=SlowStart |
| `test_pto_no_sample` | No RTT: PTO = 333000 + 25000 = 358000 us |
| `test_pto_with_rtt` | SRTT=50000, RTTVAR=25000: PTO = 50000+100000+25000 = 175000 us |
| `test_recovery_exit` | ACK for pkt sent after recovery_start_pn → state=CongAvoid |
| `test_minimum_window_floor` | cwnd/2 < kMinimumWindow → cwnd clamped to 2400 |
| `test_state_names` | "SlowStart", "CongAvoid", "Recovery" string mapping |
| `test_pn_space_names` | "Initial", "Handshake", "AppData" string mapping |
| `test_bytes_in_flight_on_loss` | bytes_in_flight decremented on both ACK and loss |
| `test_constants` | All RFC 9002 constants match expected values |

Every test pins exact numeric values — no tolerances, no ranges.

## 6. Exercises

### 1. Trace the cwnd sawtooth (★)

Send 50 packets of 1200 bytes in slow start (no losses). Print `cwnd` after each ACK. Then inject a single loss event and send 50 more packets in congestion avoidance. Verify:
- cwnd after 50 slow-start ACKs = 12000 + 50 * 1200 = 72000
- ssthresh after loss = 72000 / 2 = 36000
- cwnd increment per ACK in congestion avoidance = 1200 * 1200 / cwnd (decreases as cwnd grows)

Print all 100 cwnd values and sketch the sawtooth shape.

### 2. Compare TCP vs QUIC retransmission ambiguity (★)

Write a scenario where TCP cannot distinguish an original from a retransmission:

1. Sender sends segment seq=1000, len=500 at time T=0
2. Segment is lost. Sender retransmits seq=1000 at T=200ms
3. ACK for seq=1500 arrives at T=350ms

Question: is the RTT 350ms (from original) or 150ms (from retransmit)? TCP cannot tell. Now write the equivalent QUIC scenario where pkt#5 carries the data, is lost, and pkt#12 retransmits it. Show that the ACK for pkt#12 gives an unambiguous RTT. Explain how this affects PTO accuracy.

### 3. Implement persistent congestion detection (★★)

Given PTO = 225ms, detect persistent congestion when two lost packets span more than `kPersistentCongestionThreshold * PTO = 3 * 225ms = 675ms`. Write a test:

1. Send packet A at T=0ms, packet B at T=700ms
2. Both are declared lost (no ACKs in between)
3. Time span = 700ms > 675ms → persistent congestion
4. Verify cwnd resets to kMinimumWindow (2400 bytes)

Then test the negative case: packets spanning 600ms (< 675ms) should NOT trigger persistent congestion.

### 4. Simulate the cwnd sawtooth with multiple loss events (★★)

Run a simulation with 3 loss events. After each loss, record:
- `cwnd` before and after loss
- `ssthresh` after loss
- Number of ACKs needed to recover to pre-loss cwnd

Verify that `ssthresh` halves each time:
- Loss 1: ssthresh = cwnd_1 / 2
- Loss 2: ssthresh = cwnd_2 / 2 (where cwnd_2 < cwnd_1 because recovery is incomplete)
- Loss 3: ssthresh = cwnd_3 / 2

Print a table of (loss_number, cwnd_before, cwnd_after, ssthresh, recovery_acks).

### 5. Multi-space loss independence (★★★)

Extend the congestion controller to maintain separate state per packet number space. Each space has its own `largest_acked_pn` and loss detection. A loss in the Initial space during the handshake should not affect the Application Data space.

Simulate:
1. Send 5 Initial packets (pkt 0-4)
2. Lose Initial pkt 1
3. Send 10 Application Data packets (pkt 0-9 in AppData space)
4. Verify Application Data cwnd is unaffected by the Initial loss

### 6. Implement a BBR-style controller for QUIC (★★★)

RFC 9002 specifies NewReno but notes that QUIC is congestion-controller-agnostic. Replace the NewReno controller with a simplified BBR model:
- Track `btl_bw` (bottleneck bandwidth) as max delivery rate over a sliding window
- Track `rt_prop` (minimum RTT) over a 10-second window
- Set cwnd = `btl_bw * rt_prop * 2` (2x BDP for safety)
- Implement the four BBR phases: Startup, Drain, ProbeBW, ProbeRTT

Compare throughput and latency against NewReno over the same loss pattern.

## References

- **RFC 9002** — QUIC Loss Detection and Congestion Control (the primary source for this lesson)
- **RFC 9002, §5** — RTT estimation (smoothed RTT, RTT variance, min RTT)
- **RFC 9002, §6** — Loss detection (packet threshold, time threshold, PTO)
- **RFC 9002, §7** — NewReno congestion control (slow start, avoidance, recovery)
- **RFC 9002, §7.6** — Persistent congestion
- **RFC 9000, §17.1** — Packet numbers: unique, monotonically increasing per space
- **RFC 6298** — TCP RTT estimation (QUIC's RTT formulae derive from this)
