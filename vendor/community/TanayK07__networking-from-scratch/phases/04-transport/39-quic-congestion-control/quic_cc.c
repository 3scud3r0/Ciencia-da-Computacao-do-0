/*
 * quic_cc.c -- QUIC NewReno congestion control (RFC 9002)
 *
 * Implements the congestion controller state machine, RTT estimation,
 * loss detection (packet threshold + time threshold), and PTO calculation.
 */
#include "quic_cc.h"
#include <string.h>

/* ---------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------- */

static uint64_t max_u64(uint64_t a, uint64_t b) {
    return a > b ? a : b;
}

/* Find a sent packet by number.  Returns NULL if not found. */
static struct quic_sent_pkt *find_sent(struct quic_cc *cc, uint64_t pkt_num) {
    for (size_t i = 0; i < cc->sent_count; i++) {
        if (cc->sent[i].pkt_num == pkt_num)
            return &cc->sent[i];
    }
    return NULL;
}

/* ---------------------------------------------------------------
 * Initialization
 * --------------------------------------------------------------- */

void quic_cc_init(struct quic_cc *cc) {
    memset(cc, 0, sizeof(*cc));
    cc->cwnd = QUIC_INITIAL_WINDOW;
    cc->ssthresh = UINT64_MAX; /* no threshold until first loss */
    cc->state = QUIC_CC_SLOW_START;
    cc->rtt.smoothed_rtt_us = 0;
    cc->rtt.rttvar_us = 0;
    cc->rtt.min_rtt_us = UINT64_MAX;
    cc->rtt.latest_rtt_us = 0;
    cc->rtt.has_sample = 0;
    cc->max_ack_delay_us = 25000; /* 25 ms default */
}

/* ---------------------------------------------------------------
 * RTT update (RFC 9002, Section 5.3)
 * --------------------------------------------------------------- */

static void rtt_update(struct quic_rtt *rtt, uint64_t rtt_sample_us) {
    rtt->latest_rtt_us = rtt_sample_us;

    if (rtt_sample_us < rtt->min_rtt_us)
        rtt->min_rtt_us = rtt_sample_us;

    if (!rtt->has_sample) {
        /* First sample: SRTT = sample, RTTVAR = sample / 2 */
        rtt->smoothed_rtt_us = rtt_sample_us;
        rtt->rttvar_us = rtt_sample_us / 2;
        rtt->has_sample = 1;
        return;
    }

    /* Subsequent samples (RFC 6298 / RFC 9002, §5.3):
     *   RTTVAR = 3/4 * RTTVAR + 1/4 * |SRTT - sample|
     *   SRTT   = 7/8 * SRTT   + 1/8 * sample           */
    uint64_t abs_diff = (rtt->smoothed_rtt_us > rtt_sample_us)
                            ? rtt->smoothed_rtt_us - rtt_sample_us
                            : rtt_sample_us - rtt->smoothed_rtt_us;
    rtt->rttvar_us = (3 * rtt->rttvar_us + abs_diff) / 4;
    rtt->smoothed_rtt_us = (7 * rtt->smoothed_rtt_us + rtt_sample_us) / 8;
}

/* ---------------------------------------------------------------
 * Packet sent
 * --------------------------------------------------------------- */

int quic_cc_on_packet_sent(struct quic_cc *cc, uint64_t pkt_num, uint32_t sent_bytes,
                           uint64_t sent_time_us, int ack_eliciting) {
    if (cc->sent_count >= QUIC_MAX_SENT_PACKETS)
        return -1;

    struct quic_sent_pkt *pkt = &cc->sent[cc->sent_count++];
    pkt->pkt_num = pkt_num;
    pkt->sent_time_us = sent_time_us;
    pkt->sent_bytes = sent_bytes;
    pkt->ack_eliciting = ack_eliciting;
    pkt->in_flight = (sent_bytes > 0) ? 1 : 0;
    pkt->lost = 0;
    pkt->acked = 0;

    if (pkt->in_flight)
        cc->bytes_in_flight += sent_bytes;

    return 0;
}

/* ---------------------------------------------------------------
 * ACK processing (RFC 9002, Section 7)
 * --------------------------------------------------------------- */

int quic_cc_on_ack(struct quic_cc *cc, uint64_t pkt_num, uint64_t ack_time_us) {
    struct quic_sent_pkt *pkt = find_sent(cc, pkt_num);
    if (!pkt || pkt->acked || pkt->lost)
        return -1;

    pkt->acked = 1;

    /* Update largest acked */
    if (!cc->has_acked || pkt_num > cc->largest_acked_pn) {
        cc->largest_acked_pn = pkt_num;
        cc->has_acked = 1;
    }

    /* RTT sample: only from ack-eliciting packets */
    if (pkt->ack_eliciting && ack_time_us >= pkt->sent_time_us) {
        uint64_t rtt_sample = ack_time_us - pkt->sent_time_us;
        rtt_update(&cc->rtt, rtt_sample);
    }

    /* Remove from bytes_in_flight */
    if (pkt->in_flight) {
        if (cc->bytes_in_flight >= pkt->sent_bytes)
            cc->bytes_in_flight -= pkt->sent_bytes;
        else
            cc->bytes_in_flight = 0;
        pkt->in_flight = 0;
    }

    uint32_t acked_bytes = pkt->sent_bytes;

    /* Do not grow cwnd if in recovery and this packet was sent before
     * recovery started (RFC 9002, §7.3.2). */
    if (cc->state == QUIC_CC_RECOVERY) {
        if (pkt_num > cc->recovery_start_pn) {
            /* Exiting recovery */
            cc->state = QUIC_CC_CONGESTION_AVOIDANCE;
        } else {
            return 0; /* still in recovery, no cwnd increase */
        }
    }

    /* Slow start (RFC 9002, §7.3.1):
     * cwnd += acked_bytes (exponential growth) */
    if (cc->state == QUIC_CC_SLOW_START) {
        cc->cwnd += acked_bytes;
        if (cc->cwnd >= cc->ssthresh) {
            cc->state = QUIC_CC_CONGESTION_AVOIDANCE;
        }
        return 0;
    }

    /* Congestion avoidance (RFC 9002, §7.3.3):
     * cwnd += max_datagram_size * acked_bytes / cwnd  (linear growth) */
    if (cc->state == QUIC_CC_CONGESTION_AVOIDANCE) {
        uint64_t increment = (uint64_t)QUIC_MAX_DATAGRAM_SIZE * acked_bytes / cc->cwnd;
        if (increment == 0)
            increment = 1; /* ensure at least 1 byte progress */
        cc->cwnd += increment;
        return 0;
    }

    return 0;
}

/* ---------------------------------------------------------------
 * Loss handling (RFC 9002, Section 7.3.2)
 * --------------------------------------------------------------- */

int quic_cc_on_loss(struct quic_cc *cc, uint64_t pkt_num) {
    struct quic_sent_pkt *pkt = find_sent(cc, pkt_num);
    if (!pkt || pkt->lost || pkt->acked)
        return -1;

    pkt->lost = 1;

    /* Remove from bytes_in_flight */
    if (pkt->in_flight) {
        if (cc->bytes_in_flight >= pkt->sent_bytes)
            cc->bytes_in_flight -= pkt->sent_bytes;
        else
            cc->bytes_in_flight = 0;
        pkt->in_flight = 0;
    }

    /* If already in recovery and this packet was sent before recovery
     * started, don't reduce cwnd again. */
    if (cc->state == QUIC_CC_RECOVERY && pkt_num <= cc->recovery_start_pn) {
        return 0;
    }

    /* Enter recovery:
     *   ssthresh = cwnd * kLossReductionFactor = cwnd / 2
     *   cwnd = max(ssthresh, kMinimumWindow)
     *   recovery_start_pn = largest sent packet number */
    cc->ssthresh = cc->cwnd / QUIC_LOSS_REDUCTION_DENOM;
    cc->cwnd = max_u64(cc->ssthresh, QUIC_MINIMUM_WINDOW);
    cc->state = QUIC_CC_RECOVERY;

    /* Track recovery epoch by the largest packet sent so far.
     * Any packet with pkt_num <= recovery_start_pn was in-flight
     * when recovery began and should not trigger another reduction. */
    uint64_t max_sent = 0;
    for (size_t i = 0; i < cc->sent_count; i++) {
        if (cc->sent[i].pkt_num > max_sent)
            max_sent = cc->sent[i].pkt_num;
    }
    cc->recovery_start_pn = max_sent;

    return 0;
}

/* ---------------------------------------------------------------
 * Persistent congestion (RFC 9002, Section 7.6)
 * --------------------------------------------------------------- */

void quic_cc_on_persistent_congestion(struct quic_cc *cc) {
    cc->cwnd = QUIC_MINIMUM_WINDOW;
    cc->ssthresh = cc->cwnd;
    cc->state = QUIC_CC_SLOW_START;
    cc->bytes_in_flight = 0;
}

/* ---------------------------------------------------------------
 * Loss detection (RFC 9002, Section 6.1)
 * --------------------------------------------------------------- */

int quic_loss_detect(struct quic_cc *cc, uint64_t now_us) {
    if (!cc->has_acked)
        return 0;

    int newly_lost = 0;

    /* Time threshold (RFC 9002, §6.1.2):
     * max(smoothed_rtt, latest_rtt) * 9/8 */
    uint64_t base_rtt = max_u64(cc->rtt.smoothed_rtt_us, cc->rtt.latest_rtt_us);
    uint64_t time_threshold = base_rtt * QUIC_TIME_THRESH_NUM / QUIC_TIME_THRESH_DEN;
    if (time_threshold < QUIC_TIMER_GRANULARITY_US)
        time_threshold = QUIC_TIMER_GRANULARITY_US;

    for (size_t i = 0; i < cc->sent_count; i++) {
        struct quic_sent_pkt *pkt = &cc->sent[i];
        if (pkt->acked || pkt->lost)
            continue;

        /* Packet threshold: lost if kPacketThreshold packets with
         * higher numbers have been acked (RFC 9002, §6.1.1). */
        if (pkt->pkt_num + QUIC_PACKET_THRESHOLD <= cc->largest_acked_pn) {
            pkt->lost = 1;
            if (pkt->in_flight) {
                if (cc->bytes_in_flight >= pkt->sent_bytes)
                    cc->bytes_in_flight -= pkt->sent_bytes;
                else
                    cc->bytes_in_flight = 0;
                pkt->in_flight = 0;
            }
            newly_lost++;
            continue;
        }

        /* Time threshold: lost if sent more than time_threshold ago
         * relative to the time the largest acked was sent. */
        if (now_us >= pkt->sent_time_us + time_threshold && pkt->pkt_num < cc->largest_acked_pn) {
            pkt->lost = 1;
            if (pkt->in_flight) {
                if (cc->bytes_in_flight >= pkt->sent_bytes)
                    cc->bytes_in_flight -= pkt->sent_bytes;
                else
                    cc->bytes_in_flight = 0;
                pkt->in_flight = 0;
            }
            newly_lost++;
        }
    }

    return newly_lost;
}

/* ---------------------------------------------------------------
 * PTO calculation (RFC 9002, Section 6.2.1)
 * --------------------------------------------------------------- */

uint64_t quic_pto_compute(const struct quic_cc *cc) {
    if (!cc->rtt.has_sample) {
        /* Before any RTT sample, use initial RTT = 333ms (RFC 9002, §6.2.2) */
        return 333000 + cc->max_ack_delay_us;
    }

    /* PTO = smoothed_rtt + max(4 * rttvar, kGranularity) + max_ack_delay */
    uint64_t var_component = max_u64(4 * cc->rtt.rttvar_us, QUIC_TIMER_GRANULARITY_US);
    return cc->rtt.smoothed_rtt_us + var_component + cc->max_ack_delay_us;
}

/* ---------------------------------------------------------------
 * Name helpers
 * --------------------------------------------------------------- */

const char *quic_cc_state_name(enum quic_cc_state state) {
    switch (state) {
    case QUIC_CC_SLOW_START:
        return "SlowStart";
    case QUIC_CC_CONGESTION_AVOIDANCE:
        return "CongAvoid";
    case QUIC_CC_RECOVERY:
        return "Recovery";
    }
    return "Unknown";
}

const char *quic_pn_space_name(enum quic_pn_space space) {
    switch (space) {
    case QUIC_PN_INITIAL:
        return "Initial";
    case QUIC_PN_HANDSHAKE:
        return "Handshake";
    case QUIC_PN_APPLICATION:
        return "AppData";
    default:
        return "Unknown";
    }
}
