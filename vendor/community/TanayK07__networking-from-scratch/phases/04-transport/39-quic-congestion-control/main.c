/*
 * main.c -- Simulate QUIC NewReno congestion control
 *
 * Sends ~120 packets, injects loss events at packets 40, 75, and 100,
 * and prints the cwnd evolution at each step.  Demonstrates slow start,
 * congestion avoidance, loss response, and recovery.
 *
 * Usage:
 *   ./quic_cc              # run the full simulation
 *   ./quic_cc --pto        # also print PTO after each RTT sample
 */
#include "quic_cc.h"
#include <stdio.h>
#include <string.h>

/* Simulated network parameters */
#define SIM_RTT_US       50000 /* 50 ms base RTT */
#define SIM_PKT_SIZE     1200  /* bytes per packet */
#define SIM_TOTAL_PKTS   120   /* total packets to send */
#define SIM_ACK_DELAY_US 5000  /* 5 ms ACK processing delay */

/* Packet numbers where we inject a loss event */
static const uint64_t loss_points[] = {40, 75, 100};
#define NUM_LOSS_POINTS (sizeof(loss_points) / sizeof(loss_points[0]))

static int is_loss_point(uint64_t pkt_num) {
    for (size_t i = 0; i < NUM_LOSS_POINTS; i++) {
        if (loss_points[i] == pkt_num)
            return 1;
    }
    return 0;
}

static void print_header(void) {
    printf("%-6s  %-10s  %-10s  %-10s  %-12s  %s\n", "PKT#", "CWND", "SSTHRESH", "IN_FLIGHT",
           "STATE", "EVENT");
    printf("------  ----------  ----------  ----------  ------------  "
           "-----\n");
}

static void print_row(uint64_t pkt_num, const struct quic_cc *cc, const char *event) {
    char ssthresh_str[24];
    if (cc->ssthresh == UINT64_MAX)
        snprintf(ssthresh_str, sizeof(ssthresh_str), "inf");
    else
        snprintf(ssthresh_str, sizeof(ssthresh_str), "%lu", (unsigned long)cc->ssthresh);

    printf("%-6lu  %-10lu  %-10s  %-10lu  %-12s  %s\n", (unsigned long)pkt_num,
           (unsigned long)cc->cwnd, ssthresh_str, (unsigned long)cc->bytes_in_flight,
           quic_cc_state_name(cc->state), event);
}

static void run_simulation(int show_pto) {
    struct quic_cc cc;
    quic_cc_init(&cc);

    printf("QUIC NewReno Congestion Control Simulation\n");
    printf("==========================================\n");
    printf("Initial cwnd:   %u bytes (%u packets)\n", QUIC_INITIAL_WINDOW,
           QUIC_INITIAL_WINDOW / SIM_PKT_SIZE);
    printf("Min window:     %u bytes\n", QUIC_MINIMUM_WINDOW);
    printf("Packet size:    %u bytes\n", SIM_PKT_SIZE);
    printf("Base RTT:       %u us (%u ms)\n", SIM_RTT_US, SIM_RTT_US / 1000);
    printf("Loss points:    pkt 40, 75, 100\n\n");

    print_header();

    uint64_t now_us = 0;

    for (uint64_t pkt = 0; pkt < SIM_TOTAL_PKTS; pkt++) {
        /* Send packet */
        uint64_t send_time = now_us;
        quic_cc_on_packet_sent(&cc, pkt, SIM_PKT_SIZE, send_time, 1);

        /* Simulate one RTT passing */
        now_us += SIM_RTT_US + SIM_ACK_DELAY_US;

        /* Check if this packet is a loss point */
        if (is_loss_point(pkt)) {
            /* Run loss detection first */
            int detected = quic_loss_detect(&cc, now_us);
            (void)detected;

            /* Declare this packet lost */
            quic_cc_on_loss(&cc, pkt);
            print_row(pkt, &cc, "LOSS");

            if (show_pto) {
                uint64_t pto = quic_pto_compute(&cc);
                printf("  PTO = %lu us (%.1f ms)\n", (unsigned long)pto, (double)pto / 1000.0);
            }
            continue;
        }

        /* ACK arrives */
        quic_cc_on_ack(&cc, pkt, now_us);

        /* Run loss detection */
        quic_loss_detect(&cc, now_us);

        const char *event = "";
        if (cc.state == QUIC_CC_SLOW_START)
            event = "slow_start";
        else if (cc.state == QUIC_CC_CONGESTION_AVOIDANCE)
            event = "cong_avoid";
        else if (cc.state == QUIC_CC_RECOVERY)
            event = "recovery";

        print_row(pkt, &cc, event);

        if (show_pto && cc.rtt.has_sample) {
            uint64_t pto = quic_pto_compute(&cc);
            printf("  PTO = %lu us (%.1f ms)\n", (unsigned long)pto, (double)pto / 1000.0);
        }
    }

    printf("\n--- Simulation complete ---\n");
    printf("Final cwnd:     %lu bytes\n", (unsigned long)cc.cwnd);
    printf("Final ssthresh: ");
    if (cc.ssthresh == UINT64_MAX)
        printf("inf\n");
    else
        printf("%lu bytes\n", (unsigned long)cc.ssthresh);
    printf("Final state:    %s\n", quic_cc_state_name(cc.state));
    printf("RTT (smoothed): %lu us (%.1f ms)\n", (unsigned long)cc.rtt.smoothed_rtt_us,
           (double)cc.rtt.smoothed_rtt_us / 1000.0);
    printf("RTT (min):      %lu us (%.1f ms)\n", (unsigned long)cc.rtt.min_rtt_us,
           (double)cc.rtt.min_rtt_us / 1000.0);

    /* Show persistent congestion example */
    printf("\n--- Persistent Congestion Demo ---\n");
    printf("Before: cwnd=%lu state=%s\n", (unsigned long)cc.cwnd, quic_cc_state_name(cc.state));
    quic_cc_on_persistent_congestion(&cc);
    printf("After:  cwnd=%lu state=%s (reset to kMinimumWindow)\n", (unsigned long)cc.cwnd,
           quic_cc_state_name(cc.state));

    /* Show packet number spaces */
    printf("\n--- QUIC Packet Number Spaces ---\n");
    for (int s = 0; s < QUIC_PN_SPACE_COUNT; s++) {
        printf("  Space %d: %s\n", s, quic_pn_space_name((enum quic_pn_space)s));
    }
    printf("Each space has independent packet numbers.\n");
    printf("Unique numbers eliminate retransmission ambiguity:\n");
    printf("  TCP:  seq 100 sent, lost, retransmitted as seq 100 — "
           "ACK is ambiguous\n");
    printf("  QUIC: pkt 5 sent, lost, retransmitted as pkt 12 — "
           "ACK for 12 is unambiguous\n");
}

int main(int argc, char *argv[]) {
    int show_pto = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--pto") == 0)
            show_pto = 1;
    }

    run_simulation(show_pto);
    return 0;
}
