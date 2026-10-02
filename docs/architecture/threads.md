# Threads

Components and data flows are described in [components.md](components.md). This doc covers which thread runs each component, how threads communicate, where they run, and the order they start and stop in.

All threads share one address space and communicate through shared-memory queues.
All threads, queues and journal buffers live on the same NUMA node so no hot path crosses the interconnect.
Hot threads run on isolated cores (`isolcpus`, `nohz_full`, IRQs steered away). SMT siblings of hot cores are left idle/disabled so they don't compete for execution resources.

**All memory is allocated and pre-faulted at startup:** Rings, order pools and journal buffers are allocated, touched and locked (`mlock`). No allocation or page fault happens on a hot path.

## Threads

| Thread | Count | Owns | Reads | May block? | Placement |
|---|---|---|---|---|---|
| Session | 1 per session (up to `MAX_SESSIONS`) | Its session's state: TCP socket, SoupBinTCP state, outbound buffer | Its socket; Event Queue (filtered by session ID) | No. Non-blocking socket I/O, busy poll | Hot, isolated core each |
| Acceptor | 1 | Session slot allocation. If all `MAX_SESSIONS` slots are taken, it closes the new connection immediately | Listening socket | Yes, on `accept` | Cold |
| Admin/Control | 1 | – | Operator input | Yes, on operator input | Cold |
| Sequencer | 1 | Input seq # counter; sequencer timestamp; input journal (append); producer side of the Order Queue | Gateway → Sequencer MPSC queue; Admin queue | No | Hot, isolated core |
| Matching Engine | 1 | Everything listed for the engine in components.md; producer side of the Event Queue | Order Queue; Input Journal (startup replay only) | No | Hot, isolated core |
| Feed Publisher | 1 | MoldUDP64 packetization state | Event Queue (public events) | No. Non-blocking `sendto`, busy poll | Hot, isolated core |
| Output Journal | 1 | Output journal file | Event Queue (all events) | No | Isolated core. Not latency-critical, but must keep up with the engine to avoid being overrun |
| Retransmission Server | 1 | Request handling state | Request socket; Output journal file | Yes, on socket and file reads | Cold |

**Isolated cores needed:** 4 + `MAX_SESSIONS` (sequencer, engine, publisher, output journal, one per session), plus at least one housekeeping core for the OS, IRQs and cold threads.

### Thread Separation Idea

**Session threads vs Sequencer:** sessions do socket I/O, framing and per-session buffering. Keeping that off the sequencer means one slow or misbehaving client never delays sequencing for others.

**Sequencer vs Matching Engine:** the sequencer absorbs the input journal append. The engine only sees inputs that are already journaled, and its core does nothing but match.

**Matching Engine vs Event Queue readers:** publishing, journaling and session delivery happen on other cores. The engine's only output cost is writing to the ring.

**Feed Publisher vs Output Journal:** a stall in file I/O must never delay market data.

**Acceptor, Admin and Retransmission Server are cold:** none is on the order path, so they don't get isolated cores.

## Queues

| Queue | Producers → Consumers | Type | Capacity | When full / overrun |
|---|---|---|---|---|
| Gateway → Sequencer | N session threads → Sequencer | Bounded MPSC | Power of two, to be sized | Session thread stops reading its socket. TCP flow control pushes back to the client. Order entry can be backpressured; market data cannot |
| Admin → Sequencer | Admin → Sequencer | SPSC | Small | Admin thread waits |
| Order Queue | Sequencer → Engine | SPSC | Power of two, to be sized | Sequencer waits. Backpressure propagates to the MPSC queue, then to clients |
| Event Queue | Engine → Sessions, Feed Publisher, Output Journal | Seqlock ring | Power of two, to be sized | Writer never waits. Readers detect overrun by sequence number (see Slow consumers) |

**Sequencer input order:** the sequencer drains the admin queue before the gateway queue on each iteration, so admin commands such as a halt take effect at the next sequence number. The resulting order is whatever the sequencer dequeues, and that order is what gets journaled.

## Slow consumers

During live trading, no reader gates the engine. The writer never waits, so a slow reader can never stall matching. Each reader handles being overrun differently:

| Reader | Why it could fall behind | On overrun |
|---|---|---|
| Session thread | Its client isn't reading from TCP, so the outbound buffer fills | Disconnect the session. The disconnect is sent to the sequencer as a session event. The client logs back in and requests replay |
| Feed Publisher | Should not happen: it only packetizes and sends | Fatal. Raise an alarm and stop. The ring is sized so this can't happen at peak rate |
| Output Journal | Disk or page-cache writeback stall | Regenerate the missing range by replaying the input journal |

**Early warning:** every reader exports its lag (engine write position minus its read position). An alarm fires well before the lag reaches ring capacity.

## Ordering and time

**Total order:** the sequencer's dequeue order. It assigns the seq #, which also defines time priority.

**Timestamps:** the sequencer stamps each input when it dequeues it. Timestamps are journaled and reported (e.g. in ITCH messages), but never used for ordering.

**Clock sources:** reported timestamps come from `CLOCK_REALTIME` via the vDSO, taken only by the sequencer. Internal latency measurement uses the TSC and never enters the journal.

## Durability

**Level:** page cache. The sequencer's append is complete once the record is in the page cache.

**Survives:** a process crash. **Does not survive:** a kernel panic or power loss. Records still in the page cache are lost, and clients may already have seen responses to them.

**Mechanism:** an append-only, pre-allocated and pre-faulted file, with a per-record checksum. Append cost: to be measured.

## Lifecycle

### Startup

1. Load config. Allocate, pre-fault and lock all memory. Pin every thread to its core.
2. Start the Output Journal and Retransmission Server threads.
3. The engine replays the input journal (recovery). Replayed events go to the Event Queue, and only the Output Journal consumes them, to rebuild its file. During replay only, the engine waits for the Output Journal before overwriting a slot. Replay runs at full speed with no live traffic, so without this the Output Journal would be overrun, and its overrun policy (replay again) would loop.
4. Start the Sequencer. It appends to the end of the input journal.
5. Start the Feed Publisher. It begins at the first event after replay, so replayed events are not re-sent. Clients recover gaps through the Retransmission Server.
6. Start the Admin thread, then the Acceptor. Session threads are pre-spawned and pinned, and pick up connections as the Acceptor assigns them.
7. The exchange accepts orders after an admin `open` command.

### Shutdown

1. Admin `close` is sequenced and processed like any other input.
2. Stop the Acceptor. Session threads stop reading new input and send logout session events.
3. The Sequencer drains its queues, then stops.
4. The engine drains the Order Queue, then stops.
5. Readers drain the Event Queue up to the engine's last write, then stop.
6. The Output Journal and input journal files are flushed and closed.

## Latency Measurements (TBC)

| Stage | p50 | p99 | p99.9 | max |
|---|---|---|---|---|
| Socket read → MPSC enqueue (session thread) | – | – | – | – |
| MPSC dequeue → journal append → Order Queue (sequencer) | – | – | – | – |
| Order Queue → Event Queue (engine) | – | – | – | – |
| Event Queue → `sendto` (publisher) | – | – | – | – |
| Event Queue → TCP send (session thread) | – | – | – | – |
