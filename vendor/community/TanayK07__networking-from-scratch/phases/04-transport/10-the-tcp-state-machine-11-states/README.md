# 10. The TCP state machine (11 states)

> By the end of this lesson you will trace every TCP state transition from CLOSED through ESTABLISHED to TIME_WAIT, explain why each state exists, implement the finite state machine as a lookup table in C, and use it to simulate connection scenarios including simultaneous open and simultaneous close.

## 1. Problem

Every TCP connection is a state machine. When `curl` opens a connection, the kernel walks through CLOSED → SYN_SENT → ESTABLISHED. When the server calls `close()`, it enters FIN_WAIT_1, waits for acknowledgement, and eventually reaches TIME_WAIT — where it stays for two minutes, holding onto resources. If you've ever seen `ss -t` show thousands of connections stuck in TIME_WAIT, or a server refusing new connections because it's stuck in CLOSE_WAIT, or a connection hanging because one side is in FIN_WAIT_2 while the other side has crashed — you're looking at bugs that can only be diagnosed by understanding this state machine.

The original TCP specification (RFC 793, September 1981) defined these 11 states in a single diagram that has been reproduced in every networking textbook since. RFC 9293 (August 2022) consolidated 30+ years of errata and updates into the current canonical spec, but the state machine itself is unchanged.

> "A TCP connection progresses through a series of states during its lifetime. The states are: LISTEN, SYN-SENT, SYN-RECEIVED, ESTABLISHED, FIN-WAIT-1, FIN-WAIT-2, CLOSE-WAIT, CLOSING, LAST-ACK, TIME-WAIT, and CLOSED."
> — RFC 9293, Section 3.3.2

This lesson implements the complete state machine — not as a networking stack, but as a pure function: given a current state and an event, it returns the next state and the action to take. This is the core abstraction that every TCP implementation builds on.

## 2. Theory

### The 11 states

TCP defines exactly 11 states. They divide into three groups:

**Connection establishment** (getting to ESTABLISHED):

| State | Meaning |
|-------|---------|
| CLOSED | No connection exists. The TCB (Transmission Control Block) has been deleted. |
| LISTEN | Server is waiting for incoming SYN. Created by a passive OPEN (the `listen()` syscall). |
| SYN_SENT | Client has sent SYN, waiting for SYN-ACK. Created by an active OPEN (`connect()`). |
| SYN_RCVD | Server received SYN, sent SYN-ACK, waiting for ACK to complete the handshake. |
| ESTABLISHED | Both sides have completed the three-way handshake. Data can flow in both directions. |

**Connection teardown** (closing down):

| State | Meaning |
|-------|---------|
| FIN_WAIT_1 | Application called `close()`. FIN has been sent. Waiting for ACK of the FIN. |
| FIN_WAIT_2 | Our FIN has been acknowledged. Waiting for the remote side to send its FIN. |
| CLOSE_WAIT | Received FIN from the remote side. Application has not yet called `close()`. |
| CLOSING | Both sides sent FIN simultaneously. Each is waiting for the other's ACK. |
| LAST_ACK | Received FIN (we were in CLOSE_WAIT), sent our FIN. Waiting for final ACK. |
| TIME_WAIT | Both FINs have been acknowledged. Waiting 2×MSL before deleting the TCB. |

### The state diagram (RFC 9293, Figure 5)

This is the TCP state diagram from the RFC, the same diagram Cerf and Kahn's original design leads to:

```
                              +---------+ ---------\      active OPEN
                              |  CLOSED |            \    -----------
                              +---------+<---------\   \   snd SYN
                                |     ^              \   \
                   passive OPEN |     |   CLOSE        \   \
                   ----------- |     | ----------       \   \
                    create TCB |     | delete TCB         \   \
                                V     |                     \   V
              +---------+            +---------+
              |  LISTEN |            | SYN     |
              +---------+            | SENT    |
    rcv SYN  /  |     \              +---------+
   --------/    |      \    rcv SYN,ACK
  snd SYN,ACK   |       \  ----------
     /          |        \ snd ACK
    V           |         V
+---------+     |   +---------+
| SYN     |     |   |  ESTAB  |
| RCVD    |<----+   +---------+
+---------+ rcv ACK   |     |
  |       of SYN  CLOSE|    | rcv FIN
  |                ------    | -------
  |               snd FIN    | snd ACK
  V                   V      V
+---------+     +---------+ +---------+
| FIN     |     | FIN     | | CLOSE   |
| WAIT-1  |     | WAIT-1  | | WAIT    |
+---------+     +---------+ +---------+
```

(The full diagram continues through FIN_WAIT_2, CLOSING, LAST_ACK, and TIME_WAIT — see the implementation for all transitions.)

### Why 11 states, not fewer?

You might think: "Do we really need CLOSING? Or FIN_WAIT_2?" Each state exists because TCP must handle every possible ordering of events:

- **Simultaneous open**: Both sides send SYN at the same time. Both go SYN_SENT → SYN_RCVD → ESTABLISHED. Without SYN_RCVD as a distinct state, this case is unhandled.
- **Simultaneous close**: Both sides call `close()` at the same time. Both go ESTABLISHED → FIN_WAIT_1 → CLOSING → TIME_WAIT → CLOSED. Without the CLOSING state, this sequence has nowhere to go.
- **Half-close**: One side sends FIN but the other continues sending data. The closer is in FIN_WAIT_2, the other in CLOSE_WAIT. Without separate states, the protocol cannot distinguish "I'm done sending but still receiving" from "the connection is fully closed."

> "In a simultaneous close attempt, the receipt of a FIN causes the FIN-WAIT-1 state to be entered, and the receipt of the ACK causes the TIME-WAIT state to be entered."
> — RFC 9293, Section 3.6.1

### TIME_WAIT and the 2MSL problem

TIME_WAIT is the state that confuses newcomers and frustrates operators. After both FINs are acknowledged, the connection sits in TIME_WAIT for 2×MSL (Maximum Segment Lifetime — typically 60 seconds, so 120 seconds total). Why?

1. **Delayed duplicates**: Old packets from a previous connection might still be in the network. If a new connection reuses the same 4-tuple (src IP, src port, dst IP, dst port) immediately, those old packets could be mistaken for new data. TIME_WAIT ensures they've expired.

2. **Lost final ACK**: If the last ACK is lost, the peer will retransmit its FIN. The TIME_WAIT state allows us to re-ACK it instead of sending a RST.

> Stevens, *TCP/IP Illustrated Vol. 1* (§18.6): "The end that performs the active close is the end that stays in the TIME_WAIT state, because that is the end that might have to retransmit the final ACK."

On a busy server, TIME_WAIT sockets can accumulate. Linux provides `net.ipv4.tcp_tw_reuse` to allow reuse of TIME_WAIT sockets for new outgoing connections (safe because TCP timestamps disambiguate old packets).

### Events and actions

The state machine is driven by 9 event types:

| Event | Source |
|-------|--------|
| OPEN_PASSIVE | Application calls `listen()` |
| OPEN_ACTIVE | Application calls `connect()` |
| CLOSE | Application calls `close()` |
| RECV_SYN | Received SYN segment |
| RECV_SYNACK | Received SYN+ACK segment |
| RECV_ACK | Received ACK segment |
| RECV_FIN | Received FIN segment |
| RECV_FINACK | Received FIN+ACK segment |
| TIMEOUT | 2MSL timer expired |

Each transition produces an action: SEND_SYN, SEND_SYNACK, SEND_ACK, SEND_FIN, DELETE_TCB, or NONE.

## 3. Math / Spec

### The complete transition table

The state machine is fully defined by this table. Each cell contains (next_state, action). Empty cells are invalid transitions.

| Current State | OPEN_PASSIVE | OPEN_ACTIVE | CLOSE | RECV_SYN | RECV_SYNACK | RECV_ACK | RECV_FIN | RECV_FINACK | TIMEOUT |
|---|---|---|---|---|---|---|---|---|---|
| CLOSED | LISTEN, — | SYN_SENT, snd SYN | — | — | — | — | — | — | — |
| LISTEN | — | — | CLOSED, del TCB | SYN_RCVD, snd SYN+ACK | — | — | — | — | — |
| SYN_SENT | — | — | CLOSED, del TCB | SYN_RCVD, snd SYN+ACK | ESTAB, snd ACK | — | — | — | — |
| SYN_RCVD | — | — | FIN_WAIT_1, snd FIN | — | — | ESTAB, — | — | — | — |
| ESTABLISHED | — | — | FIN_WAIT_1, snd FIN | — | — | — | CLOSE_WAIT, snd ACK | — | — |
| FIN_WAIT_1 | — | — | — | — | — | FIN_WAIT_2, — | CLOSING, snd ACK | TIME_WAIT, snd ACK | — |
| FIN_WAIT_2 | — | — | — | — | — | — | TIME_WAIT, snd ACK | — | — |
| CLOSE_WAIT | — | — | LAST_ACK, snd FIN | — | — | — | — | — | — |
| CLOSING | — | — | — | — | — | TIME_WAIT, — | — | — | — |
| LAST_ACK | — | — | — | — | — | CLOSED, del TCB | — | — | — |
| TIME_WAIT | — | — | — | — | — | — | — | — | CLOSED, del TCB |

This table is exactly 11 × 9 = 99 cells. In the implementation, it is a 2D array of `{next_state, action}` structs.

### Counting transitions

Of the 99 cells, only 21 are valid transitions — the rest are invalid. This is typical of protocol state machines: most state/event combinations are errors.

### MSL and TIME_WAIT duration

RFC 9293 defines the Maximum Segment Lifetime (MSL) as an implementation-defined value. Most systems use 30 or 60 seconds, giving a TIME_WAIT duration of 60–120 seconds:

- Linux: `TCP_TIMEWAIT_LEN` = 60 seconds (not 2×MSL, but a fixed constant)
- BSD/macOS: 2 × 30 seconds = 60 seconds
- Windows: 2 × 120 seconds = 240 seconds (historically)

## 4. Code

The implementation is in three files:

- [`tcp_fsm.h`](tcp_fsm.h) — enums for the 11 states, 9 events, and 7 actions, plus the FSM struct and API declarations.
- [`tcp_fsm.c`](tcp_fsm.c) — the 11×9 transition table as a 2D array, and the `nfs_tcp_fsm_handle()` function that performs lookups.
- [`main.c`](main.c) — an interactive CLI simulator where you type events and see state transitions.

### The transition table

The core of the implementation is a `struct transition table[11][9]` — a 2D array indexed by `[state][event]`. Each entry holds `{next_state, action}`, or `{-1, NONE}` for invalid transitions:

```c
struct transition {
    int next_state;
    int action;
};

static const struct transition table[11][9] = {
    [NFS_TCP_CLOSED] = {
        [NFS_TCP_EV_OPEN_PASSIVE] = {NFS_TCP_LISTEN, NFS_TCP_ACT_NONE},
        [NFS_TCP_EV_OPEN_ACTIVE]  = {NFS_TCP_SYN_SENT, NFS_TCP_ACT_SEND_SYN},
        /* all others: INVALID */
    },
    /* ... 10 more states ... */
};
```

This is the table-driven approach: no `switch` statements, no nested `if-else` chains. The entire protocol behavior is captured in a data structure. This style is common in production TCP implementations — it makes the state machine auditable against the RFC.

### The handler

`nfs_tcp_fsm_handle()` is a single table lookup:

```c
int nfs_tcp_fsm_handle(struct nfs_tcp_fsm *fsm, int event, int *action) {
    const struct transition *t = &table[fsm->state][event];
    if (t->next_state < 0) {
        *action = NFS_TCP_ACT_NONE;
        return -1;  /* invalid transition */
    }
    fsm->state = t->next_state;
    *action = t->action;
    return fsm->state;
}
```

No side effects, no I/O, no timers — just a pure function from (state, event) → (state, action). This separation is deliberate: the state machine logic is testable independently of the networking stack.

### The interactive simulator

The CLI (`main.c`) wraps the FSM in a read-eval-print loop. Type event names, see transitions:

```
$ ./tcp_fsm
TCP State Machine Simulator
State: CLOSED
Event: OPEN_ACTIVE
-> State: SYN_SENT (action: SEND_SYN)
Event: RECV_SYNACK
-> State: ESTABLISHED (action: SEND_ACK)
Event: CLOSE
-> State: FIN_WAIT_1 (action: SEND_FIN)
```

Or pipe events non-interactively:

```bash
echo -e "OPEN_ACTIVE\nRECV_SYNACK\nCLOSE\nRECV_ACK\nRECV_FIN\nTIMEOUT" | ./tcp_fsm
```

### Building and running

```bash
make            # builds ./tcp_fsm
./tcp_fsm       # interactive simulator
make test       # runs 96 assertions across 15 test cases
```

## 5. Tests

```bash
make test
```

The test suite ([`tests/test_tcp_fsm.c`](tests/test_tcp_fsm.c)) exercises 15 scenarios with 96 individual assertions:

| Test | Scenario |
|------|----------|
| `test_init` | FSM initializes to CLOSED |
| `test_client_3whs` | CLOSED → SYN_SENT → ESTABLISHED (active open) |
| `test_server_3whs` | CLOSED → LISTEN → SYN_RCVD → ESTABLISHED (passive open) |
| `test_client_close` | ESTABLISHED → FIN_WAIT_1 → FIN_WAIT_2 → TIME_WAIT → CLOSED |
| `test_server_close` | ESTABLISHED → CLOSE_WAIT → LAST_ACK → CLOSED |
| `test_simultaneous_close` | ESTABLISHED → FIN_WAIT_1 → CLOSING → TIME_WAIT → CLOSED |
| `test_finwait1_to_timewait` | FIN_WAIT_1 → TIME_WAIT via FIN+ACK |
| `test_invalid_transitions` | Invalid events return -1, state unchanged |
| `test_state_names` | All 11 state names correct, out-of-range returns "UNKNOWN" |
| `test_event_names` | All 9 event names correct |
| `test_action_names` | All 7 action names correct |
| `test_close_from_listen` | LISTEN → CLOSED (delete TCB) |
| `test_close_from_syn_sent` | SYN_SENT → CLOSED (delete TCB) |
| `test_close_from_syn_rcvd` | SYN_RCVD → FIN_WAIT_1 (send FIN) |
| `test_simultaneous_open` | SYN_SENT → SYN_RCVD → ESTABLISHED |

## 6. Exercises

1. **★** Trace the full lifecycle of an HTTP request through the state machine. Start from CLOSED, walk through the three-way handshake, then the graceful close. Write down every (state, event) → (next_state, action) transition. How many state transitions occur for a single HTTP/1.0 request-response?

2. **★★** Use `ss -t state time-wait | wc -l` on a busy Linux server to count TIME_WAIT sockets. Then calculate: if your server handles 10,000 short-lived connections per second and TIME_WAIT lasts 60 seconds, how many TIME_WAIT sockets exist at steady state? What happens when you exceed the local port range (32768–60999)?

3. **★★** Feed the simultaneous open sequence into the simulator: `OPEN_ACTIVE, RECV_SYN, RECV_ACK`. Then feed the simultaneous close sequence: reach ESTABLISHED via client handshake, then `CLOSE, RECV_FIN, RECV_ACK, TIMEOUT`. Verify the simulator output matches the transition table.

4. **★★** Add a `RECV_RST` event to the state machine. RFC 9293 §3.5.2 specifies: in SYN_SENT, a valid RST returns to CLOSED. In ESTABLISHED, a RST aborts the connection (no FIN exchange needed). Implement at least these two transitions and add tests for them.

5. **★★★** Run `ss -t -o state all` and identify every connection in a non-ESTABLISHED state on your system. For each, explain which side initiated the action and what event would move it to the next state. Bonus: find a connection stuck in CLOSE_WAIT — what does that imply about the application?

6. **★★★** Extend the FSM to track sequence numbers. Add `snd_nxt` and `rcv_nxt` to the FSM struct. On SEND_SYN, consume one sequence number. On SEND_FIN, consume one sequence number. On RECV_ACK, verify the acknowledged number matches. This is the foundation of TCP's reliability.

## References

- **RFC 9293** — *Transmission Control Protocol (TCP)* (Eddy, August 2022). The current canonical TCP specification, consolidating RFC 793 and 30+ years of updates. Section 3.3.2 defines the state machine; Figure 5 is the state diagram.
- **RFC 793** — *Transmission Control Protocol* (Postel, September 1981). The original TCP spec. Still worth reading for the design philosophy. Obsoleted by RFC 9293.
- **Stevens, W. R.** — *TCP/IP Illustrated, Volume 1*, 2nd edition, Chapters 13–14. The definitive walkthrough of TCP connection establishment and termination, with packet traces and state machine diagrams.
- **Comer, D.** — *Internetworking with TCP/IP, Volume 1*, Chapter 12. Clear treatment of the state machine with emphasis on the why behind each state.
- **Wright, G. R. & Stevens, W. R.** — *TCP/IP Illustrated, Volume 2: The Implementation*, Chapter 24. The BSD implementation of the TCP state machine, showing how the transition table maps to real kernel code.
