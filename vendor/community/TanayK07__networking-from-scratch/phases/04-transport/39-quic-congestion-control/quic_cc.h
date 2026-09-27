/*
 * quic_cc.h -- QUIC congestion control (RFC 9002, Section 7)
 *
 * Implements NewReno congestion control with QUIC-specific loss detection.
 * QUIC uses monotonically increasing packet numbers (no retransmission
 * ambiguity), three separate packet number spaces, and its own PTO timer.
 */
#ifndef NFS_QUIC_CC_H
#define NFS_QUIC_CC_H

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------
 * Constants from RFC 9002, Section 6 and Section 7
 * --------------------------------------------------------------- */

/* Maximum datagram size — typical for QUIC over IPv4+UDP (bytes). */
#define QUIC_MAX_DATAGRAM_SIZE 1200

/* kInitialWindow = min(10 * max_datagram_size,
 *                      max(14720, 2 * max_datagram_size))
 * With max_datagram_size = 1200:
 *   min(12000, max(14720, 2400)) = min(12000, 14720) = 12000 */
#define QUIC_INITIAL_WINDOW 12000

/* kMinimumWindow = 2 * max_datagram_size = 2400 */
#define QUIC_MINIMUM_WINDOW 2400

/* kLossReductionFactor = 0.5 (applied as integer division by 2) */
#define QUIC_LOSS_REDUCTION_DENOM 2

/* kPacketThreshold: number of acked packets after which an
 * earlier unacked packet is declared lost (RFC 9002, §6.1.1). */
#define QUIC_PACKET_THRESHOLD 3

/* Time threshold multiplier: 9/8 of max(smoothed_rtt, latest_rtt)
 * (RFC 9002, §6.1.2). Stored as numerator/denominator pair. */
#define QUIC_TIME_THRESH_NUM 9
#define QUIC_TIME_THRESH_DEN 8

/* kPersistentCongestionThreshold (RFC 9002, §7.6):
 * (kPacketThreshold - 1) * PTO = 3 * PTO in the formula, but the
 * threshold multiplier itself is 3. */
#define QUIC_PERSISTENT_CONGESTION_THRESHOLD 3

/* Granularity for timer calculations (microseconds). */
#define QUIC_TIMER_GRANULARITY_US 1000 /* 1 ms */

/* ---------------------------------------------------------------
 * Congestion controller states (RFC 9002, §7.3)
 * --------------------------------------------------------------- */
enum quic_cc_state { QUIC_CC_SLOW_START, QUIC_CC_CONGESTION_AVOIDANCE, QUIC_CC_RECOVERY };

/* ---------------------------------------------------------------
 * Packet number spaces (RFC 9002, §4)
 * --------------------------------------------------------------- */
enum quic_pn_space {
    QUIC_PN_INITIAL = 0,
    QUIC_PN_HANDSHAKE = 1,
    QUIC_PN_APPLICATION = 2,
    QUIC_PN_SPACE_COUNT = 3
};

/* ---------------------------------------------------------------
 * Sent packet metadata — tracks what we need for loss detection
 * --------------------------------------------------------------- */

#define QUIC_MAX_SENT_PACKETS 512

struct quic_sent_pkt {
    uint64_t pkt_num;      /* unique, monotonically increasing */
    uint64_t sent_time_us; /* microsecond timestamp when sent */
    uint32_t sent_bytes;   /* bytes in this packet */
    int ack_eliciting;     /* 1 if this packet requires an ACK */
    int in_flight;         /* 1 if counted toward bytes_in_flight */
    int lost;              /* 1 if declared lost */
    int acked;             /* 1 if acknowledged */
};

/* ---------------------------------------------------------------
 * RTT estimator (RFC 9002, §5)
 * --------------------------------------------------------------- */
struct quic_rtt {
    uint64_t smoothed_rtt_us; /* SRTT in microseconds */
    uint64_t rttvar_us;       /* RTT variance */
    uint64_t min_rtt_us;      /* minimum observed RTT */
    uint64_t latest_rtt_us;   /* most recent RTT sample */
    int has_sample;           /* 0 until first RTT sample arrives */
};

/* ---------------------------------------------------------------
 * Congestion controller state
 * --------------------------------------------------------------- */
struct quic_cc {
    /* Window and threshold */
    uint64_t cwnd;            /* congestion window (bytes) */
    uint64_t ssthresh;        /* slow start threshold (bytes) */
    uint64_t bytes_in_flight; /* unacknowledged bytes */

    /* State machine */
    enum quic_cc_state state;

    /* Recovery tracking */
    uint64_t recovery_start_pn; /* largest pkt_num when recovery began */

    /* RTT state */
    struct quic_rtt rtt;

    /* Sent packet tracking */
    struct quic_sent_pkt sent[QUIC_MAX_SENT_PACKETS];
    size_t sent_count;
    uint64_t largest_acked_pn; /* largest acknowledged packet number */
    int has_acked;             /* 0 until first ACK arrives */

    /* Max ACK delay advertised by peer (microseconds) */
    uint64_t max_ack_delay_us;
};

/* ---------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------- */

/* Initialize congestion controller with default values. */
void quic_cc_init(struct quic_cc *cc);

/* Record a packet being sent.
 * Returns 0 on success, -1 if the sent buffer is full. */
int quic_cc_on_packet_sent(struct quic_cc *cc, uint64_t pkt_num, uint32_t sent_bytes,
                           uint64_t sent_time_us, int ack_eliciting);

/* Process an acknowledgment for a specific packet number.
 * ack_time_us is the time the ACK was received.
 * Returns 0 on success, -1 if pkt_num not found. */
int quic_cc_on_ack(struct quic_cc *cc, uint64_t pkt_num, uint64_t ack_time_us);

/* Declare a packet lost (called after loss detection).
 * Returns 0 on success, -1 if pkt_num not found. */
int quic_cc_on_loss(struct quic_cc *cc, uint64_t pkt_num);

/* Handle persistent congestion: reset to minimum window. */
void quic_cc_on_persistent_congestion(struct quic_cc *cc);

/* Run loss detection over all sent packets.
 * Uses packet threshold and time threshold to detect losses.
 * now_us is the current time in microseconds.
 * Returns the number of packets newly declared lost. */
int quic_loss_detect(struct quic_cc *cc, uint64_t now_us);

/* Compute the Probe Timeout (PTO) in microseconds.
 * PTO = smoothed_rtt + max(4 * rttvar, kGranularity) + max_ack_delay */
uint64_t quic_pto_compute(const struct quic_cc *cc);

/* Return the current state as a string ("SlowStart", "CongAvoid", "Recovery"). */
const char *quic_cc_state_name(enum quic_cc_state state);

/* Return the packet number space name ("Initial", "Handshake", "AppData"). */
const char *quic_pn_space_name(enum quic_pn_space space);

#endif /* NFS_QUIC_CC_H */
