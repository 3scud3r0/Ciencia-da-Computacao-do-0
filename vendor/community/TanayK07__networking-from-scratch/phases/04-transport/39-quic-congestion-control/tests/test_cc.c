/*
 * test_cc.c -- Tests for QUIC congestion control (RFC 9002)
 *
 * Each test pins specific numeric values derived from the RFC constants.
 * No tolerance ranges — values must match exactly.
 */
#include "quic_cc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Init ---------------------------------------------------------- */

static void test_init_values(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* kInitialWindow = min(10*1200, max(14720, 2*1200))
     *               = min(12000, 14720) = 12000 */
    assert(cc.cwnd == 12000);
    assert(cc.ssthresh == UINT64_MAX);
    assert(cc.bytes_in_flight == 0);
    assert(cc.state == QUIC_CC_SLOW_START);
    assert(cc.rtt.has_sample == 0);
    assert(cc.sent_count == 0);
}

/* ---- Packet sending ------------------------------------------------ */

static void test_packet_sent_tracking(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    assert(quic_cc_on_packet_sent(&cc, 0, 1200, 1000, 1) == 0);
    assert(cc.bytes_in_flight == 1200);
    assert(cc.sent_count == 1);
    assert(cc.sent[0].pkt_num == 0);
    assert(cc.sent[0].sent_time_us == 1000);
    assert(cc.sent[0].in_flight == 1);

    assert(quic_cc_on_packet_sent(&cc, 1, 1200, 2000, 1) == 0);
    assert(cc.bytes_in_flight == 2400);
    assert(cc.sent_count == 2);
}

/* ---- Slow start exponential growth --------------------------------- */

static void test_slow_start_growth(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Send and ack 5 packets of 1200 bytes each.
     * Slow start: cwnd += acked_bytes each time.
     * Start: cwnd = 12000
     * After ack 0: cwnd = 12000 + 1200 = 13200
     * After ack 1: cwnd = 13200 + 1200 = 14400
     * After ack 2: cwnd = 14400 + 1200 = 15600
     * After ack 3: cwnd = 15600 + 1200 = 16800
     * After ack 4: cwnd = 16800 + 1200 = 18000 */
    for (uint64_t i = 0; i < 5; i++) {
        quic_cc_on_packet_sent(&cc, i, 1200, i * 50000, 1);
    }

    for (uint64_t i = 0; i < 5; i++) {
        quic_cc_on_ack(&cc, i, (i + 1) * 50000);
    }

    assert(cc.cwnd == 18000);
    assert(cc.state == QUIC_CC_SLOW_START);
}

/* ---- RTT estimation ------------------------------------------------ */

static void test_rtt_first_sample(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    quic_cc_on_packet_sent(&cc, 0, 1200, 0, 1);
    quic_cc_on_ack(&cc, 0, 50000); /* RTT = 50ms */

    assert(cc.rtt.has_sample == 1);
    assert(cc.rtt.smoothed_rtt_us == 50000);
    assert(cc.rtt.rttvar_us == 25000); /* sample / 2 */
    assert(cc.rtt.min_rtt_us == 50000);
    assert(cc.rtt.latest_rtt_us == 50000);
}

static void test_rtt_subsequent_sample(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* First sample: 50ms */
    quic_cc_on_packet_sent(&cc, 0, 1200, 0, 1);
    quic_cc_on_ack(&cc, 0, 50000);

    /* Second sample: 60ms
     * abs_diff = |50000 - 60000| = 10000
     * RTTVAR = (3*25000 + 10000) / 4 = 85000/4 = 21250
     * SRTT   = (7*50000 + 60000) / 8 = 410000/8 = 51250 */
    quic_cc_on_packet_sent(&cc, 1, 1200, 50000, 1);
    quic_cc_on_ack(&cc, 1, 110000);

    assert(cc.rtt.smoothed_rtt_us == 51250);
    assert(cc.rtt.rttvar_us == 21250);
    assert(cc.rtt.latest_rtt_us == 60000);
    assert(cc.rtt.min_rtt_us == 50000); /* min unchanged */
}

/* ---- Loss detection: packet threshold ------------------------------ */

static void test_loss_packet_threshold(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Send packets 0-5 */
    for (uint64_t i = 0; i < 6; i++) {
        quic_cc_on_packet_sent(&cc, i, 1200, i * 1000, 1);
    }

    /* Ack packets 1, 2, 3 (skip 0).
     * After acking 3, largest_acked = 3.
     * Packet 0: 0 + 3 <= 3 → declared lost. */
    quic_cc_on_ack(&cc, 1, 100000);
    quic_cc_on_ack(&cc, 2, 100000);
    quic_cc_on_ack(&cc, 3, 100000);

    int lost = quic_loss_detect(&cc, 100000);
    assert(lost == 1);
    assert(cc.sent[0].lost == 1);
    assert(cc.sent[4].lost == 0); /* not enough gap */
}

/* ---- Loss detection: time threshold -------------------------------- */

static void test_loss_time_threshold(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Establish an RTT of 100ms */
    quic_cc_on_packet_sent(&cc, 0, 1200, 0, 1);
    quic_cc_on_ack(&cc, 0, 100000);

    /* Send packets 1 and 2 */
    quic_cc_on_packet_sent(&cc, 1, 1200, 100000, 1);
    quic_cc_on_packet_sent(&cc, 2, 1200, 200000, 1);

    /* Ack packet 2 only.  largest_acked = 2. */
    quic_cc_on_ack(&cc, 2, 300000);

    /* Time threshold = max(SRTT, latest_rtt) * 9/8
     * SRTT=100000, latest=100000 → threshold = 100000*9/8 = 112500 us
     * Packet 1 sent at 100000.  Now = 300000.
     * 300000 >= 100000 + 112500 (=212500) → lost. */
    int lost = quic_loss_detect(&cc, 300000);
    assert(lost == 1);
    assert(cc.sent[1].lost == 1);
}

/* ---- Congestion window on loss ------------------------------------- */

static void test_cwnd_on_loss(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Grow cwnd in slow start to 18000 (5 acks of 1200 bytes) */
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_packet_sent(&cc, i, 1200, i * 50000, 1);
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_ack(&cc, i, (i + 1) * 50000);
    assert(cc.cwnd == 18000);

    /* Send packet 5 and declare it lost */
    quic_cc_on_packet_sent(&cc, 5, 1200, 300000, 1);
    quic_cc_on_loss(&cc, 5);

    /* ssthresh = 18000 / 2 = 9000
     * cwnd = max(9000, 2400) = 9000 */
    assert(cc.ssthresh == 9000);
    assert(cc.cwnd == 9000);
    assert(cc.state == QUIC_CC_RECOVERY);
}

/* ---- Multiple losses in same recovery epoch ------------------------ */

static void test_no_double_reduction(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Grow cwnd to 18000 */
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_packet_sent(&cc, i, 1200, i * 50000, 1);
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_ack(&cc, i, (i + 1) * 50000);

    /* Send packets 5 and 6 */
    quic_cc_on_packet_sent(&cc, 5, 1200, 300000, 1);
    quic_cc_on_packet_sent(&cc, 6, 1200, 310000, 1);

    /* Lose both — second loss should NOT reduce cwnd again */
    quic_cc_on_loss(&cc, 5);
    uint64_t cwnd_after_first = cc.cwnd; /* 9000 */

    quic_cc_on_loss(&cc, 6);
    assert(cc.cwnd == cwnd_after_first); /* still 9000 */
    assert(cc.state == QUIC_CC_RECOVERY);
}

/* ---- Congestion avoidance linear growth ----------------------------- */

static void test_congestion_avoidance_growth(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Force into congestion avoidance by setting ssthresh low */
    cc.ssthresh = 12000;
    cc.state = QUIC_CC_CONGESTION_AVOIDANCE;
    cc.cwnd = 12000;

    /* Send and ack one 1200-byte packet.
     * cwnd += max_datagram_size * acked_bytes / cwnd
     *       = 1200 * 1200 / 12000 = 120 */
    quic_cc_on_packet_sent(&cc, 0, 1200, 0, 1);
    quic_cc_on_ack(&cc, 0, 50000);

    assert(cc.cwnd == 12120);
}

/* ---- Persistent congestion resets to minimum ----------------------- */

static void test_persistent_congestion(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Grow window */
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_packet_sent(&cc, i, 1200, i * 50000, 1);
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_ack(&cc, i, (i + 1) * 50000);
    assert(cc.cwnd > QUIC_MINIMUM_WINDOW);

    quic_cc_on_persistent_congestion(&cc);

    assert(cc.cwnd == QUIC_MINIMUM_WINDOW); /* 2400 */
    assert(cc.state == QUIC_CC_SLOW_START);
    assert(cc.bytes_in_flight == 0);
}

/* ---- PTO computation ----------------------------------------------- */

static void test_pto_no_sample(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Before any RTT sample: initial_rtt(333ms) + max_ack_delay(25ms) */
    uint64_t pto = quic_pto_compute(&cc);
    assert(pto == 333000 + 25000); /* 358000 us */
}

static void test_pto_with_rtt(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Establish RTT = 50ms */
    quic_cc_on_packet_sent(&cc, 0, 1200, 0, 1);
    quic_cc_on_ack(&cc, 0, 50000);
    /* SRTT = 50000, RTTVAR = 25000 */

    /* PTO = 50000 + max(4*25000, 1000) + 25000
     *     = 50000 + 100000 + 25000 = 175000 us */
    uint64_t pto = quic_pto_compute(&cc);
    assert(pto == 175000);
}

/* ---- Recovery exit ------------------------------------------------- */

static void test_recovery_exit(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Send and ack packets 0-4 to grow cwnd */
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_packet_sent(&cc, i, 1200, i * 50000, 1);
    for (uint64_t i = 0; i < 5; i++)
        quic_cc_on_ack(&cc, i, (i + 1) * 50000);

    /* Enter recovery via loss of packet 5 */
    quic_cc_on_packet_sent(&cc, 5, 1200, 300000, 1);
    quic_cc_on_loss(&cc, 5);
    assert(cc.state == QUIC_CC_RECOVERY);

    /* Send and ack a NEW packet (pkt 6, after recovery_start_pn).
     * recovery_start_pn = largest_acked_pn = 4.
     * pkt 6 > 4 → exits recovery. */
    quic_cc_on_packet_sent(&cc, 6, 1200, 350000, 1);
    quic_cc_on_ack(&cc, 6, 400000);
    assert(cc.state == QUIC_CC_CONGESTION_AVOIDANCE);
}

/* ---- Minimum window floor ------------------------------------------ */

static void test_minimum_window_floor(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    /* Set cwnd to something small (just above minimum) */
    cc.cwnd = QUIC_MINIMUM_WINDOW + 100; /* 2500 */

    /* Send and lose a packet */
    quic_cc_on_packet_sent(&cc, 0, 1200, 0, 1);
    quic_cc_on_ack(&cc, 0, 50000);
    quic_cc_on_packet_sent(&cc, 1, 1200, 50000, 1);
    quic_cc_on_loss(&cc, 1);

    /* ssthresh = 2500 / 2 = 1250, which is < kMinimumWindow(2400).
     * cwnd = max(1250, 2400) = 2400 */
    assert(cc.cwnd == QUIC_MINIMUM_WINDOW);
}

/* ---- State and space name helpers ---------------------------------- */

static void test_state_names(void) {
    assert(strcmp(quic_cc_state_name(QUIC_CC_SLOW_START), "SlowStart") == 0);
    assert(strcmp(quic_cc_state_name(QUIC_CC_CONGESTION_AVOIDANCE), "CongAvoid") == 0);
    assert(strcmp(quic_cc_state_name(QUIC_CC_RECOVERY), "Recovery") == 0);
}

static void test_pn_space_names(void) {
    assert(strcmp(quic_pn_space_name(QUIC_PN_INITIAL), "Initial") == 0);
    assert(strcmp(quic_pn_space_name(QUIC_PN_HANDSHAKE), "Handshake") == 0);
    assert(strcmp(quic_pn_space_name(QUIC_PN_APPLICATION), "AppData") == 0);
}

/* ---- Bytes-in-flight bookkeeping ----------------------------------- */

static void test_bytes_in_flight_on_loss(void) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    quic_cc_on_packet_sent(&cc, 0, 1200, 0, 1);
    quic_cc_on_packet_sent(&cc, 1, 1200, 1000, 1);
    assert(cc.bytes_in_flight == 2400);

    /* Ack packet 1, lose packet 0 */
    quic_cc_on_ack(&cc, 1, 50000);
    assert(cc.bytes_in_flight == 1200);

    quic_cc_on_loss(&cc, 0);
    assert(cc.bytes_in_flight == 0);
}

/* ---- Constant verification ----------------------------------------- */

static void test_constants(void) {
    /* Verify RFC 9002 constants are correctly defined */
    assert(QUIC_MAX_DATAGRAM_SIZE == 1200);
    assert(QUIC_INITIAL_WINDOW == 12000);
    assert(QUIC_MINIMUM_WINDOW == 2400);
    assert(QUIC_PACKET_THRESHOLD == 3);
    assert(QUIC_TIME_THRESH_NUM == 9);
    assert(QUIC_TIME_THRESH_DEN == 8);
    assert(QUIC_PERSISTENT_CONGESTION_THRESHOLD == 3);
    assert(QUIC_LOSS_REDUCTION_DENOM == 2);
}

/* ---- Main ---------------------------------------------------------- */

int main(void) {
    test_init_values();
    test_packet_sent_tracking();
    test_slow_start_growth();
    test_rtt_first_sample();
    test_rtt_subsequent_sample();
    test_loss_packet_threshold();
    test_loss_time_threshold();
    test_cwnd_on_loss();
    test_no_double_reduction();
    test_congestion_avoidance_growth();
    test_persistent_congestion();
    test_pto_no_sample();
    test_pto_with_rtt();
    test_recovery_exit();
    test_minimum_window_floor();
    test_state_names();
    test_pn_space_names();
    test_bytes_in_flight_on_loss();
    test_constants();

    printf("All 19 QUIC congestion control tests passed.\n");
    return 0;
}
