# TCP Protocol: Beginner to Automotive Software Architect

## 1. The Interview Answer First

**TCP (Transmission Control Protocol) is a connection-oriented transport-layer protocol that provides a reliable, ordered, full-duplex byte stream between application endpoints. It uses sequence numbers, acknowledgments, retransmissions, flow control, and congestion control. It does not preserve application message boundaries, encrypt data, or guarantee delivery within a deadline.**

Remember **CONNECT -> NUMBER -> ACK -> RETRY -> REGULATE -> CLOSE**.

| Memory word | What TCP does |
|---|---|
| CONNECT | Establishes connection state with a handshake |
| NUMBER | Numbers bytes so it can order data and detect missing ranges |
| ACK | Reports the next byte expected |
| RETRY | Retransmits data when loss is inferred |
| REGULATE | Protects the receiver and the network with separate controls |
| CLOSE | Supports orderly shutdown of each sending direction |

Think of two applications exchanging pages of a document over an unreliable delivery network. Pages can arrive late, out of order, or twice. TCP reconstructs the ordered content, but the application must still understand and act on it.

**Analogy limit:** TCP numbers bytes, not pages or application messages.

### Reading Route

- **First pass:** Sections 2-7 for the basic mental model.
- **Deep understanding:** Sections 8-17 for reliability, performance, and lifecycle.
- **Automotive work:** Sections 18-23 for architecture, implementation, and diagnosis.
- **Interview revision:** Sections 24-26 for questions, exercises, and a one-page recap.

Start with the [full forms and abbreviations](#full-forms-and-abbreviations) below. Return to these tables whenever a short form is unfamiliar; each entry includes what it means in this guide, not just its expansion.

This guide covers core TCP and practical architectural decisions. Advanced extensions vary by operating system and configuration; the references identify the underlying specifications.

### Full Forms and Abbreviations

#### Networking and Security

| Short form | Full form or correct name | Plain-language meaning |
|---|---|---|
| TCP | Transmission Control Protocol | Delivers an ordered, reliable stream of bytes between endpoints. |
| IP | Internet Protocol | Provides addressing and packet delivery across networks. |
| TCP/IP | Transmission Control Protocol / Internet Protocol | TCP running over IP; also a common name for the broader Internet protocol suite. |
| UDP | User Datagram Protocol | Sends separate datagrams without built-in reliable, ordered delivery. |
| UDP/IP | User Datagram Protocol / Internet Protocol | UDP running over IP. |
| IPv4 | Internet Protocol version 4 | IP version using 32-bit addresses. |
| IPv6 | Internet Protocol version 6 | IP version using 128-bit addresses. |
| OSI | Open Systems Interconnection | Seven-layer reference model used to discuss network responsibilities. |
| ARP | Address Resolution Protocol | Resolves an IPv4 neighbor's link-layer address on a local network. |
| DNS | Domain Name System | Resolves names and publishes other naming information; commonly maps hostnames to IP addresses. |
| ICMP | Internet Control Message Protocol | Carries IP-layer control and error information; IPv6 uses ICMPv6. |
| ICMPv6 | Internet Control Message Protocol for IPv6 | IPv6 control protocol, also used by Neighbor Discovery. |
| HTTP | Hypertext Transfer Protocol | Application protocol commonly used for web requests and responses. |
| HTTPS | Hypertext Transfer Protocol Secure | HTTP protected by TLS; in this guide's TCP examples, HTTP over TLS over TCP. |
| TLS | Transport Layer Security | Protects communication using encryption, integrity checks, and configured peer authentication. |
| MQTT | Modern standard name: MQTT; historically MQ Telemetry Transport | Publish/subscribe messaging used in telemetry. Often expanded informally as Message Queuing Telemetry Transport, but the current standard uses MQTT as its name. |
| NAT | Network Address Translation | Rewrites network addresses and often transport ports between networks. |
| NIC | Network Interface Controller (also commonly Network Interface Card) | Hardware that connects a computer or ECU to a network. |
| QoS | Quality of Service | Mechanisms for managing traffic priority, bandwidth, and delay. |
| VLAN | Virtual Local Area Network | Logical separation of a switched network into distinct link-layer networks. |
| TSN | Time-Sensitive Networking | Ethernet standards for time synchronization and controlled traffic delivery. |
| RFC | Request for Comments | Numbered technical publication series containing Internet specifications and other documents; not every RFC is a standard. |

#### TCP Flags and Packet Notation

Flags are shortened labels. Their expanded words help memory, but the protocol meaning is what matters.

| Short form | Expanded wording | Plain-language meaning |
|---|---|---|
| SYN | Synchronize | Synchronizes sequence numbers when opening a connection. |
| ACK / Ack | Acknowledgment | ACK is a flag; Ack in diagrams labels the acknowledgment number, meaning the next expected sequence number. |
| FIN | Finish | The sender has finished sending stream data in this direction. |
| RST | Reset | Aborts or rejects a connection in the applicable TCP state. |
| PSH | Push | A push indication; not an application-message boundary. |
| URG | Urgent | Indicates that the urgent-pointer field is meaningful. |
| ECE | Explicit Congestion Notification Echo (ECN-Echo) | Used in ECN negotiation and to echo congestion indications in classic ECN. |
| CWR | Congestion Window Reduced | In classic ECN, signals that the sender has responded to congestion feedback. |
| SYN+ACK | Synchronize + Acknowledgment | A segment with both flags set, commonly the second handshake message. |
| FIN/ACK | Finish / Acknowledgment | Refers to close-related FIN and ACK signaling; when combined on one segment, both flags are set. |
| Seq | Sequence number | The segment's position in TCP sequence space. |
| Len | Length | In the examples, the number of TCP payload bytes in a segment. |

State names such as LISTEN, ESTABLISHED, CLOSED, and TIME-WAIT are descriptive English labels, not acronyms. SYN-SENT means "synchronize sent," FIN-WAIT means "waiting during finish/close," and LAST-ACK means "waiting for the last acknowledgment." Section 15 explains their precise state-machine meanings.

#### TCP Timing, Windows, and Loss Recovery

| Short form | Full form | Plain-language meaning |
|---|---|---|
| ISN | Initial Sequence Number | Starting sequence number chosen independently for each direction. |
| RTT | Round-Trip Time | Time for traffic to reach the peer and a response to return. |
| RTO | Retransmission Timeout | Time used to decide when unacknowledged data needs timeout-based retransmission. |
| SACK | Selective Acknowledgment | Reports received byte ranges beyond a gap. |
| RACK | Recent Acknowledgment | Time-based TCP loss-detection mechanism. |
| TLP | Tail Loss Probe | Probing mechanism that helps recover loss near the end of a transfer. |
| RACK-TLP | Recent Acknowledgment / Tail Loss Probe | Combined loss-detection and recovery mechanisms described in RFC 8985. |
| ECN | Explicit Congestion Notification | Signals congestion through marking and endpoint feedback rather than only packet loss. |
| `rwnd` | Receive window | Receiver-advertised capacity for incoming bytes. |
| `cwnd` | Congestion window | Sender-maintained limit based on network congestion control. |
| `ssthresh` | Slow-start threshold | Boundary used in classic algorithms between slow start and congestion avoidance. |
| MSS | Maximum Segment Size | Maximum TCP data payload size advertised by an endpoint; excludes TCP/IP headers. |
| MTU | Maximum Transmission Unit | Maximum IP packet size a link can carry, in this guide's context. |
| BDP | Bandwidth-Delay Product | Amount of data needed in flight to fill a path at a given bandwidth and RTT. |
| MSL | Maximum Segment Lifetime | Assumed maximum time a segment can remain in the network; used in TIME-WAIT reasoning. |
| BBR | Bottleneck Bandwidth and Round-trip propagation time | Congestion-control algorithm family that models the path's bandwidth and propagation time. |

**Memory pairs:** RTT measures a round trip; RTO controls a retransmission timer. MSS limits TCP payload; MTU limits the IP packet. `rwnd` protects the receiver; `cwnd` protects the network.

#### Automotive and AUTOSAR

| Short form | Full form | Plain-language meaning |
|---|---|---|
| ECU | Electronic Control Unit | An embedded computer performing vehicle functions. |
| CAN | Controller Area Network | Vehicle communication network widely used between ECUs. |
| CAN FD | Controller Area Network with Flexible Data-Rate | CAN extension supporting larger payloads and an optionally faster data phase. |
| LIN | Local Interconnect Network | Lower-cost vehicle network often used for simpler local devices. |
| OEM | Original Equipment Manufacturer | In this automotive context, usually the vehicle manufacturer. |
| OTA | Over-the-Air | Remote delivery of software or data, commonly through wireless connectivity. |
| DoIP | Diagnostics over Internet Protocol | Carries vehicle diagnostic communication over IP networks. |
| UDS | Unified Diagnostic Services | Defines diagnostic requests and responses used to interact with ECUs. |
| SOME/IP | Scalable service-Oriented MiddlewarE over IP | Automotive middleware protocol for service-oriented communication. IP means Internet Protocol. |
| AUTOSAR | AUTomotive Open System ARchitecture | Standardized automotive software architectures and interfaces. |
| BSW | Basic Software | AUTOSAR Classic infrastructure beneath application software. |
| SoAd | Socket Adaptor | AUTOSAR Classic module adapting socket communication to upper-layer interfaces. |
| TcpIp | TCP/IP module: Transmission Control Protocol / Internet Protocol | AUTOSAR Classic module name for TCP/IP stack functionality. |
| EthIf | Ethernet Interface | AUTOSAR Classic module abstracting Ethernet-controller interfaces. |
| PDU | Protocol Data Unit | A unit of data handled by a protocol layer. |
| PduR | Protocol Data Unit Router | AUTOSAR Classic module routing PDUs between configured modules. |
| DCM | Diagnostic Communication Manager | AUTOSAR Classic module handling diagnostic communication and service processing. |
| ISO | International Organization for Standardization | Organization publishing standards such as the DoIP and UDS families. ISO is its official short name, not the initials of its English name. |

**Automotive memory:** ECU is the computer; CAN/LIN/Ethernet are network technologies; IP and TCP handle network/transport communication; DoIP carries diagnostics; UDS defines diagnostic services.

#### Programming, Operating Systems, and Socket Symbols

Socket constants and error names are API identifiers, not necessarily formal acronyms. The table gives their readable meaning without inventing protocol full forms.

| Short form or symbol | Full form or readable meaning | Plain-language meaning |
|---|---|---|
| OS | Operating System | Software managing hardware, scheduling, memory, and communication resources. |
| API | Application Programming Interface | Functions and contracts software uses to access another component. |
| POSIX | Portable Operating System Interface | Standards family defining operating-system interfaces, including many used in Unix-like systems. |
| CPU | Central Processing Unit | Processor executing software instructions. |
| RAM | Random Access Memory | Working memory used for buffers, connection state, and application data. |
| RAII | Resource Acquisition Is Initialization | C++ ownership technique tying resource lifetime to object lifetime. |
| ID / IDs | Identifier / identifiers | Values used to distinguish requests or other objects. |
| EOF | End Of File | General end-of-input term; on a TCP stream, orderly end-of-stream from the peer. |
| `recv()` | Receive | Socket function that reads available stream bytes. |
| `SHUT_WR` | Shut down writing | Stops the socket's sending direction while permitting receives. |
| `EAGAIN` | Error: try again | Operation cannot complete now; nonblocking code normally waits for readiness. |
| `EWOULDBLOCK` | Error: operation would block | Nonblocking operation would need to wait; may have the same value as EAGAIN. |
| `EINTR` | Error: interrupted | A signal interrupted the operation; handle according to the API contract. |
| `SO_ERROR` | Socket option: error status | Used to retrieve and clear a socket's pending error, including when checking connection completion. |
| `SIGPIPE` | Signal: broken pipe | Signal that can occur when writing to a pipe/socket whose reading peer is no longer available. |
| `EPIPE` | Error: broken pipe | Error indicating that the write cannot proceed on that pipe/socket. |
| `MSG_NOSIGNAL` | Message flag: no signal | On Linux, suppresses SIGPIPE for that send; the error can still be returned. |
| `TCP_NODELAY` | TCP option: no Nagle delay | Disables Nagle's algorithm, not all sources of delay. |
| `TCP_USER_TIMEOUT` | TCP option: user timeout | On supporting systems, bounds specified unacknowledged-data or zero-window waiting conditions. |

#### Units and Tool Shorthand

| Notation | Expanded meaning | How to read it |
|---|---|---|
| bit / byte | Binary digit / group of 8 bits in this networking context | Do not confuse bits with bytes when calculating throughput. |
| ms | Millisecond | One thousandth of a second. |
| Mbit/s | Megabits per second | 1 Mbit/s = 1,000,000 bits per second. |
| KiB | Kibibyte | 1 KiB = 1,024 bytes; 64 KiB = 65,536 bytes. |
| bytes/s | Bytes per second | Multiply by 8 to convert to bits per second. |
| p95 / p99 | 95th percentile / 99th percentile | Latency values at or below which approximately 95% / 99% of measured observations fall. |
| `ss` | Socket statistics | Linux command for inspecting sockets and their states. |
| `ip` | Internet Protocol networking utility | Linux command for addresses, routes, and neighbors. |
| `addr` / `neigh` | Address / neighbor | Abbreviated command arguments in the Linux examples. |
| `tcpdump` | TCP dump, a packet-capture tool name | Captures many protocols, not only TCP. |
| `pcap` | Packet capture | Common capture-file format and filename extension. |
| `eth0` | Conventional Ethernet interface name, index 0 | Example interface name, not an acronym or a required name on every machine. |
| `eq` | Equals | Equality operator used in a Wireshark display filter. |

In the `ss` examples: `-l` means listening sockets, `-t` selects TCP, `-n` keeps addresses/ports numeric, `-i` shows internal TCP information, and `-a` includes listening and non-listening sockets. Combined options such as `-ltn` simply combine these switches.

In the `tcpdump` example: `-i` selects the interface, `-nn` disables address and port-name resolution, `-s 0` requests the tool's full-packet snapshot setting, `-c 500` stops after 500 packets, and `-w` writes a capture file. These switches are tool-specific; `-i` does not mean the same thing in every command.

#### Names That Are Not Formal Acronyms

- **Wi-Fi:** a brand name, not a formal expansion of "Wireless Fidelity."
- **QUIC:** the current standardized protocol name, not a formal acronym. Its historical Google predecessor used "Quick UDP Internet Connections"; do not present that as the current IETF protocol's official full form. **IETF** means **Internet Engineering Task Force**.
- **CUBIC:** named after its cubic congestion-window growth function; no acronym expansion is needed.
- **Reno:** a TCP congestion-control variant name, not an acronym.
- **Ethernet, Linux, Wireshark, and Nagle:** technology, software, or personal names, not abbreviations requiring full forms.
- **C++:** a programming-language name using C's increment-operator notation, not an acronym.
- **SOCKS5:** version 5 of the SOCKS proxy protocol mentioned in the related reading; SOCKS is a name derived from "sockets," not a separate set of initials to expand.

## 2. Where TCP Fits

### Start Here: Layers Are Responsibilities, Not Separate Wires

Your understanding is correct: **TCP is above the network layer. To reach another computer, a TCP message must travel down through IP, the link layer, and the physical layer.** At the receiving computer, it travels up those layers in reverse order.

"Above" means a higher level of responsibility. It does not mean TCP has its own physical connection to the other computer.

For now, use this simple Ethernet example:

| Layer | Simple question it answers | What it does |
|---|---|---|
| Application | What do I want to say? | Creates a request, such as a diagnostic request. |
| TCP: transport | Which connection, and which bytes? | Adds ports, sequence numbers, and connection-control information. |
| IP: network | Which destination computer/network? | Adds IP addresses; routing selects where to send the packet next. |
| Ethernet: link | Which next device on this local link? | Wraps the packet in an Ethernet frame with link-layer addresses. |
| Physical | How do the bits cross the connection? | Hardware encodes the information as electrical signals on the cable. |

**Important:** the physical link must already be usable before a TCP handshake can cross it. TCP creates a logical connection, not a new cable or an electrical link.

### Sending Down, Receiving Up

```text
CLIENT                                         SERVER
Application                                    Application
	|                                              ^
	v                                              |
   TCP                                            TCP
	|                                              ^
	v                                              |
   IP                                             IP
	|                                              ^
	v                                              |
Ethernet                                       Ethernet
	|                                              ^
	v                                              |
Physical -------- electrical signals --------> Physical
```

This drawing assumes a direct Ethernet connection. Switches and routers may exist in a real path, but there is still no shortcut from client TCP directly to server TCP.

### Wrapping a Message: Encapsulation

**Encapsulation** simply means "put information inside another layer's container." Each layer adds information needed for its own job.

```text
Application:  [request bytes]
TCP:          [TCP header | request bytes]
IP:           [IP header | TCP header | request bytes]
Ethernet:     [Ethernet header | IP header | TCP header | request bytes | trailer]
Physical:     encoded signals carrying the frame across the link
```

**Decapsulation** means "unpack the containers at the receiver." The receiving layers check and interpret their information, then pass the inner content upward. Hardware and software share this work; the exact split depends on the platform.

Headers are part of the transmitted information. They do not disappear when the frame becomes electrical signals; the receiver reconstructs that information from the signals.

### But What if the Application Has Not Sent Data Yet?

**TCP can create its own control segments.** A normal opening SYN does not need application data. It contains a TCP header, usually with options, but normally no application payload.

```text
Ethernet carries [IP header | TCP header with SYN set]
```

That is enough for the server's TCP stack to recognize a connection request. The SYN still goes through IP, Ethernet, and physical transmission, just like a segment carrying application data. Section 6 follows this exact journey.

### Detailed Layer Names: Read After the Simple Example

```text
Application       DoIP, SOME/IP, HTTP, MQTT, custom application protocol
Security          TLS, when used above TCP
Transport         TCP or UDP
Network           IPv4 or IPv6
Link              Ethernet, Wi-Fi, cellular link technology, etc.
Physical          Electrical, optical, or radio transmission
```

TCP is OSI Layer 4. IP handles addressing and packet forwarding across networks. Ethernet handles local link delivery. TCP operates between endpoints over IP.

An automotive Ethernet link can carry TCP/IP, UDP/IP, and other traffic. **Ethernet is not TCP**, and **TCP is not a replacement name for CAN**.

### Essential Vocabulary

| Term | Meaning |
|---|---|
| ECU | Electronic Control Unit: an embedded computer in a vehicle |
| IP address | Network-layer address associated with an interface; routing uses it to reach a destination |
| Port | 16-bit transport-layer number used to identify an endpoint/service |
| Socket | Operating-system communication object exposed to an application |
| TCP segment | TCP header plus any TCP payload |
| IP packet | IP header plus payload, which may contain a TCP segment |
| Ethernet frame | Link-layer framing containing, for example, an IP packet |
| RTT | Round-trip time: time for traffic to reach the peer and a response to return |
| Bandwidth | Available data transfer capacity per unit time |
| Goodput | Rate of useful application data, excluding overhead and retransmissions |

```text
Ethernet frame
	+-- IP packet
				+-- TCP segment
							+-- Application bytes
```

ARP resolves IPv4 neighbors on a local link; IPv6 uses Neighbor Discovery. DNS can resolve a hostname before connection establishment. Neither is part of TCP itself.

## 3. What TCP Guarantees, and What It Does Not

### What It Provides

- **Connection orientation:** both endpoints maintain connection state.
- **Ordered delivery:** the application receives bytes in sending order within each direction.
- **Reliability:** TCP detects missing data and attempts retransmission; it does not silently skip a missing byte range in the delivered stream.
- **Duplicate suppression:** retransmitted copies do not appear as duplicated bytes in the stream.
- **Full duplex:** both endpoints can send simultaneously, with independent sequence spaces.
- **Flow control:** avoids overrunning the peer's advertised receive capacity.
- **Congestion control:** adjusts transmission to network conditions.
- **Error detection:** a mandatory TCP checksum detects many accidental corruptions.

### What It Does Not Provide

- Message boundaries: one `send()` does not correspond to one `recv()`.
- A maximum delivery time, fixed latency, or guaranteed bandwidth.
- Infinite persistence: failures, timeouts, or resets can end a connection.
- Encryption, peer authentication, or cryptographic integrity.
- Proof that the remote application parsed, persisted, or executed a request.
- Exactly-once business operations across retries or reconnects.
- Multicast or broadcast delivery.

**Critical distinction:** a TCP ACK means the peer TCP endpoint accepted bytes, not that the business operation succeeded. With a TCP-terminating proxy, that peer may be the proxy, not the final application server.

## 4. How a Connection Is Identified

A TCP connection is normally identified by a **four-tuple**:

```text
(source IP, source port, destination IP, destination port)

Example:
(192.0.2.10, 53000, 192.0.2.20, 13400)
 diagnostic tester          vehicle DoIP endpoint
```

These are documentation-only example addresses, not a vehicle configuration.

When discussing different transport protocols together, add the protocol to obtain a **five-tuple**. TCP port 13400 and UDP port 13400 are separate transport endpoints.

The client commonly uses an automatically selected ephemeral port. The server listens on a known port. Many accepted connections can share the server's port because their full tuples differ.

```text
Tester A:53000 ----> Vehicle:13400
Tester B:54000 ----> Vehicle:13400
```

A listening socket waits for connections. An accepted socket represents an individual connection. A port number alone is not a unique connection identifier.

## 5. The TCP Header

The base header is **20 bytes**. Options can extend it to **60 bytes**. These sizes exclude the IP header and application payload.

| Field | Size | Purpose |
|---|---|---|
| Source port | 16 bits | Sending endpoint's port |
| Destination port | 16 bits | Receiving endpoint's port |
| Sequence number | 32 bits | Position of the first data byte in this segment, subject to SYN handling |
| Acknowledgment number | 32 bits | Next sequence number expected; meaningful when ACK is set |
| Data offset | 4 bits | TCP header length in 32-bit words |
| Reserved/control bits | Part of control area | Flags and extension-dependent signaling |
| Window | 16 bits | Advertised receive window, possibly scaled after negotiation |
| Checksum | 16 bits | Error detection over pseudo-header, TCP header, and payload |
| Urgent pointer | 16 bits | Marks urgent-data position when URG is set; rarely used today |
| Options and padding | 0-40 bytes | MSS, window scale, SACK negotiation/blocks, timestamps, etc. |

### Flags to Remember

| Flag | Meaning | Common misconception |
|---|---|---|
| SYN | Synchronize sequence numbers | It does not carry an application's login or service activation |
| ACK | Acknowledgment field is valid | An ACK-bearing segment can also carry data |
| FIN | Sender has no more stream data to send | It closes only one sending direction |
| RST | Reset/abort a connection, or reject certain unexpected segments | It is not an orderly FIN exchange |
| PSH | Indicates a push request in TCP's delivery model | It is not a message delimiter or a real-time delivery guarantee |
| URG | Urgent pointer is meaningful | It is not a general-purpose priority mechanism |
| ECE / CWR | ECN-related congestion signaling | Congestion feedback need not always depend on packet loss |

Newer extensions can assign additional control-bit semantics. Understand this common set first; packet decoders may display extension-specific flags.

### Checksum: Detection, Not Security

The checksum includes a pseudo-header derived from IP information, including source/destination addresses, transport protocol identification, and TCP length. This helps detect corruption and misdelivery.

It is not cryptographic: an attacker can modify data and recompute it. TLS supplies cryptographic protection when correctly configured.

## 6. Connection Establishment: The Three-Way Handshake

### First Understand the Conversation

Imagine a diagnostic tester connecting to a vehicle's diagnostic endpoint:

```text
Tester TCP                              Vehicle TCP
	 |                                       |
	 | SYN: "Let's start a connection."      |
	 |-------------------------------------->|
	 | SYN+ACK: "Received; let's start."      |
	 |<--------------------------------------|
	 | ACK: "I received your reply."         |
	 |-------------------------------------->|
```

**SYN = Synchronize. ACK = Acknowledgment.** These sentences are memory aids, not text actually sent on the wire. TCP sets bits in its header to identify these messages.

Each horizontal arrow above hides a complete trip **down the sender's layers, across the physical network, and up the receiver's layers**.

### Follow the First SYN All the Way to the Wire

Assume the tester and vehicle endpoint are on the same Ethernet network, their link is working, their IP settings are correct, and the server is listening on TCP port 13400. No TLS or application request is involved in this opening TCP exchange.

1. **The client application asks to connect.** For example, it calls `connect()` for the vehicle's IP address and port 13400. It does not normally construct the SYN itself.
2. **The client's TCP stack builds a SYN segment.** It supplies a source port, destination port 13400, an initial sequence number, and the SYN flag. This is a control segment, even though no diagnostic request has been sent yet.
3. **IP wraps that segment in a packet.** It supplies source and destination IP addresses. Routing selects the outgoing interface and next hop.
4. **Ethernet wraps the packet in a frame.** It supplies source and destination MAC addresses. **MAC means Media Access Control**; a MAC address identifies an interface on the link. If the IPv4 next-hop address is not already known at the link layer, **ARP (Address Resolution Protocol)** can resolve it first.
5. **The driver and network hardware transmit it.** The Ethernet controller and physical-layer hardware turn the frame into signals on the cable. **PHY** is common shorthand for a physical-layer device/transceiver, not a separate transport protocol.
6. **The vehicle's hardware receives the signals.** It reconstructs the frame; the receive path checks the Ethernet information and passes the IP packet upward when appropriate.
7. **The vehicle's IP layer identifies TCP.** The IPv4 Protocol field, or the corresponding IPv6 Next Header chain, identifies TCP using protocol number **6**. This number is not a TCP port.
8. **The vehicle's TCP stack processes the SYN.** It checks the destination port and listening state, then normally creates handshake state and sends SYN+ACK. The application does not need to read the SYN or manually reply to it.

**Same-network detail:** the destination MAC address in this example is the vehicle endpoint's. If the destination is on another network, the first frame normally targets the next-hop router's MAC address instead; the destination IP still identifies the remote endpoint in this simple non-translated example.

### The Reply Uses the Same Layers in Reverse Direction

```text
1. SYN
	Client TCP -> IP -> Ethernet -> Physical
						-> network ->
	Server Physical -> Ethernet -> IP -> TCP

2. SYN+ACK
	Server TCP -> IP -> Ethernet -> Physical
						-> network ->
	Client Physical -> Ethernet -> IP -> TCP

3. ACK
	Client TCP -> IP -> Ethernet -> Physical
						-> network ->
	Server Physical -> Ethernet -> IP -> TCP
```

The client TCP stack generates the final ACK after accepting the SYN+ACK. The server completes establishment when it receives that ACK. The server application can then obtain the established connection through `accept()` or the platform's corresponding interface.

**Key answer:** the handshake happens *between TCP implementations*, but its messages are *carried by all the lower layers*. The physical layer carries encoded bits; it does not decide what SYN or ACK means.

### What Happens After the Handshake?

The application can now exchange bytes through the established TCP connection. Those bytes follow the same downward and upward paths. If TLS is required, its exchange comes next in this normal example; DoIP routing activation and diagnostic requests are further application-level steps.

An ordinary Ethernet switch forwards frames without completing the TCP handshake itself. An ordinary IP router forwards packets and uses new link-layer framing for the next link. A TCP-terminating proxy is different: it can be the endpoint of one TCP connection and open another.

**Say this in an interview:** "The application asks TCP to connect. TCP generates a SYN, IP puts it in a packet, Ethernet puts the packet in a frame, and hardware transmits the frame as signals. The server reverses that process and its TCP stack replies with SYN+ACK through the same layers. The client then sends ACK."

### Second Pass: Add the Sequence Numbers

Once the journey above is clear, add one detail: each side chooses a starting number so it can track bytes in its own sending direction. You can leave options and timing details until your next reading.

Each endpoint chooses an initial sequence number, or **ISN**. Each direction has its own sequence space.

```text
Client                                               Server
CLOSED                                               LISTEN
	 |                                                    |
	 | SYN, Seq=1000                                      |
	 |--------------------------------------------------->|
SYN-SENT                                                |
	 |                       SYN+ACK, Seq=5000, Ack=1001     |
	 |<---------------------------------------------------|
	 |                                                SYN-RECEIVED
	 | ACK, Seq=1001, Ack=5001                             |
	 |--------------------------------------------------->|
ESTABLISHED                                         ESTABLISHED
```

1. Client sends SYN and announces its ISN.
2. Server acknowledges the client's SYN and announces its own ISN.
3. Client acknowledges the server's SYN.

**SYN consumes one sequence number.** That is why the acknowledgment is ISN + 1.

The client can enter ESTABLISHED after receiving a valid SYN+ACK; the server does so after receiving the final ACK. The drawing shows the completed exchange, not simultaneous state transitions.

### Why Three Messages?

Both endpoints must synchronize their independent sequence spaces and obtain acknowledgment of their SYN. The final ACK confirms receipt of the server's SYN and helps prevent stale connection attempts from being treated as completed connections.

The handshake does **not** authenticate the peer or prove that the connection will remain reachable.

### What Is Negotiated?

- MSS: each endpoint advertises the TCP payload size it is prepared to receive.
- Window scaling: allows larger advertised receive windows.
- SACK permitted: indicates support for selective acknowledgment.
- Timestamps: supports RTT measurement and protection against old duplicates.
- ECN capability, when supported and enabled.

Some capabilities are directional advertisements, not one symmetric value shared by both directions.

### Setup Cost and Failure

A normal TCP open takes approximately **one RTT before the initiating application can normally send data**. TLS and application handshakes add their own cost. TCP Fast Open is an extension that can change startup behavior, not a baseline assumption.

Lost handshake segments are handled using retransmission behavior. Retransmission limits and connection timeouts depend on the stack and configuration.

## 7. TCP Is a Byte Stream, Not a Message Queue

Suppose the sender calls:

```text
send("HELLO")
send("WORLD")
```

The receiving application might observe:

```text
recv() -> "HELLOWORLD"
```

or:

```text
recv() -> "HE"
recv() -> "LLOW"
recv() -> "ORLD"
```

All are valid. TCP preserves byte order, not the sender's call boundaries.

### Applications Must Define Framing

| Strategy | Example | Main concern |
|---|---|---|
| Fixed length | Every record is 32 bytes | Limited flexibility |
| Delimiter | Newline-terminated text | Escaping, encoding, and length limits |
| Length prefix | Header contains payload length | Validate length before allocation |
| Self-describing encoding | A format with well-defined complete-value boundaries | Correct incremental parsing is still required |

For a length-prefixed binary protocol:

```text
Accumulate enough bytes for the fixed header.
Decode and validate the declared payload length.
Wait until the complete payload is available.
Process one message and retain any remaining bytes.
Repeat, because one read can contain several messages.
```

Use the protocol's specified byte order, often network byte order (big-endian). Bound message size, buffered bytes, and how long an incomplete message may occupy resources.

**Interview memory:** TCP is a pipe of bytes; the application draws the message boundaries.

## 8. Sequence Numbers and Acknowledgments

TCP counts **bytes, not packets**. Sequence arithmetic wraps modulo 2^32; implementations must handle wraparound correctly.

Example after the handshake:

```text
Client sends Seq=1001, Len=500
Bytes covered: 1001 through 1500
Server replies Ack=1501
```

`Ack=1501` means: "I have received the contiguous stream before 1501; send me byte 1501 next."

TCP acknowledgments are **cumulative**.

```text
Sender                                     Receiver
Seq=1001, Len=500 ------------------------> Ack=1501
Seq=1501, Len=500 -------- lost
Seq=2001, Len=500 ------------------------> Ack=1501
```

Even if later data is buffered, the cumulative ACK cannot advance past the gap. Once bytes 1501-2000 arrive, it can advance to 2501 if the later bytes were retained.

- Data bytes consume sequence numbers.
- SYN and FIN each consume one sequence number.
- A pure ACK consumes no sequence number.
- Opposite-direction data uses the other endpoint's independent sequence space.
- ACKs may be piggybacked on segments carrying data.

Wireshark commonly displays **relative sequence numbers**, so a SYN may appear as sequence 0 rather than its actual wire value.

## 9. Reliability: Loss, Retransmission, and Reordering

### Retransmission Timeout

The sender tracks outstanding data. If acknowledgment progress does not arrive within the retransmission timeout (**RTO**), it retransmits according to the stack's recovery rules.

RTO adapts to smoothed RTT and RTT variation; it is not a fixed small delay. RFC 6298 specifies the baseline algorithm, including a recommended initial RTO of one second, with implementation-specific behavior possible.

Repeated timeout-based retransmissions use exponential backoff. Unless timestamps disambiguate measurements, RTT samples from retransmitted data are avoided because the acknowledgment's origin is uncertain (Karn's algorithm).

**RTO is not the same as an application's response deadline.**

### Fast Retransmit and Modern Recovery

In classic TCP, three duplicate ACKs can trigger fast retransmit before the RTO expires. Subsequent recovery behavior depends on the congestion-control/recovery algorithm.

Modern stacks can also use mechanisms such as **RACK-TLP**, which use timing and probes to detect or recover from loss. Do not assume every retransmission requires three duplicate ACKs.

Duplicate ACKs can result from reordering as well as loss; they are evidence, not absolute proof.

### Selective Acknowledgment: SACK

When negotiated, the receiver can report blocks received beyond the cumulative ACK:

```text
Ack=1501, SACK block=[2001, 2501)
```

This tells the sender that bytes 2001-2500 were received while the earlier gap remains. SACK helps target retransmissions, especially when several ranges are missing. It does not change the ordered stream delivered to the application.

### Lost ACKs and Duplicate Data

A later cumulative ACK may cover data whose earlier ACK was lost. Otherwise, the sender can retransmit. The receiver recognizes duplicate bytes and does not deliver them twice.

This duplicate suppression applies to one TCP stream, **not to application requests retried over a new connection**.

### Head-of-Line Blocking

If earlier bytes are missing, later bytes cannot be delivered past that gap, even when they belong to a different application message multiplexed on the same stream.

**Architectural consequence:** one lost segment can delay multiple unrelated messages sharing a TCP connection.

## 10. Flow Control: Protect the Receiver

The receiver advertises a **receive window (`rwnd`)**, indicating the sequence range it is currently prepared to accept beyond the acknowledgment point.

Its available buffering and how quickly its application drains data affect the advertised window.

```text
Fast sender -> receiving TCP buffer -> slow application
										 |
										 +-- available receive window shrinks
```

If `rwnd` becomes zero, the sender pauses normal new-data transmission. TCP uses **zero-window probing/persist behavior** to discover when the window opens again, including when a window update is lost.

A zero window often points to a slow, blocked, or overloaded receiving application. It does not necessarily indicate a congested link.

### Window Scaling

The unscaled window field can represent at most 65,535 bytes. Window scaling, negotiated in SYN segments, allows larger windows using a shift count of up to 14.

```text
Effective advertised window = window field * 2^scale
```

Scaling is directional. The window fields in SYN/SYN+ACK themselves are not scaled.

## 11. Congestion Control: Protect the Network

The sender maintains a **congestion window (`cwnd`)** to limit data in flight based on its assessment of network capacity.

| Question | Flow control | Congestion control |
|---|---|---|
| What is protected? | Receiver capacity | Network capacity |
| Main limit | `rwnd` | `cwnd` |
| Who determines it? | Receiver advertises it | Sender computes it |
| Common signals | Receive buffer availability | ACK progress, loss, delay/model information, ECN |

Simplified, excluding other limits:

```text
Maximum outstanding data <= min(rwnd, cwnd)
Allowance for new data ~= max(0, min(rwnd, cwnd) - bytes_in_flight)
```

Actual transmission also depends on pacing, available application data, local buffers, and recovery state.

### Classic Reno-Style Concepts

1. **Slow start:** `cwnd` grows roughly exponentially per RTT under ideal acknowledgment conditions. The name is historical; growth is rapid.
2. **Congestion avoidance:** growth becomes more gradual after a threshold (`ssthresh`).
3. **Fast retransmit / fast recovery:** duplicate-ACK-based loss recovery adjusts the sending rate without necessarily returning to the initial window.
4. **Timeout recovery:** generally imposes a stronger rate reduction and backoff.

These are a teaching model, not a universal description of every algorithm. CUBIC uses a different growth function; BBR uses a bandwidth/RTT model. Defaults vary by operating system and version.

### ECN

Explicit Congestion Notification lets supporting network devices mark congestion instead of necessarily dropping a packet. Endpoints negotiate support and exchange feedback; the sender responds by adapting its sending behavior.

**Interview memory:** `rwnd` asks "Can the receiver cope?"; `cwnd` asks "Can the network cope?"

## 12. MTU, MSS, and Fragmentation

| Term | Meaning |
|---|---|
| MTU | Maximum IP packet size supported on a link, in this context |
| Path MTU | Smallest applicable MTU along the path |
| MSS | Maximum TCP payload size advertised by an endpoint |

For an Ethernet IP MTU of 1500 bytes and no IP/TCP options:

```text
IPv4: 1500 - 20-byte IP header - 20-byte TCP header = 1460-byte payload
IPv6: 1500 - 40-byte IP header - 20-byte TCP header = 1440-byte payload
```

Options, IPv6 extension headers, and tunnels reduce the payload that fits in the path MTU. The sender must account for these overheads; the SYN MSS value alone does not describe every later segment's size.

TCP segmentation and IP fragmentation are different:

- **Segmentation:** TCP divides stream data into segments before handing it to IP.
- **Fragmentation:** IP divides an IP packet where allowed, with reassembly at the destination.
- IPv4 routers may fragment when permitted; IPv6 routers do not fragment forwarded packets.

Path MTU Discovery uses network feedback to avoid oversized packets. Packetization-layer methods can use probing. Blocked required ICMP feedback can cause an **MTU black hole**: handshakes or small requests succeed, while larger transfers stall.

NIC/stack offloads can make host captures show packets larger than the wire MTU. Do not infer wire packet size from an offloaded host capture alone.

## 13. Performance and Latency

### Bandwidth-Delay Product

To keep a path busy, sufficient data must be in flight to cover its bandwidth-delay product (**BDP**).

```text
BDP in bytes = bandwidth in bits/second * RTT in seconds / 8

100 Mbit/s and 20 ms RTT:
100,000,000 * 0.020 / 8 = 250,000 bytes
```

An idealized window-limited throughput bound is:

```text
Throughput <= min(rwnd, cwnd) / RTT
```

With only 64 KiB outstanding and a 20 ms RTT:

```text
65,536 / 0.020 = 3,276,800 bytes/s ~= 26.2 Mbit/s
```

These are simplified bounds, not predictions. Real goodput also depends on loss, protocol overhead, pacing, CPU, storage, and the application.

### Nagle's Algorithm and Delayed ACKs

- **Nagle:** coalesces small writes under conditions such as existing unacknowledged data, reducing tiny segments.
- **Delayed ACK:** allows a receiver to briefly defer some ACKs, reducing overhead and enabling piggybacking.
- Their interaction can add latency for applications that issue small dependent writes.
- `TCP_NODELAY` disables Nagle on that socket. It does not disable the peer's delayed ACK behavior or remove all buffering.

Prefer assembling a complete small application message where practical. Measure before changing socket options; lower latency and packet efficiency can trade off.

### Backpressure and Bufferbloat

Large buffers can improve bulk throughput, but oversized queues add latency. An application that ignores backpressure can consume excessive memory even when TCP itself limits transmission.

Measure **tail latency** (for example, p95/p99), not only averages. Retransmissions and queueing can dominate worst-case behavior.

## 14. Graceful Close, Half-Close, and Reset

Each sending direction closes independently. A typical exchange is:

```text
Endpoint A                                         Endpoint B
	 | FIN, Seq=3001                                    |
	 |------------------------------------------------>|
	 |                                  ACK, Ack=3002  |
	 |<------------------------------------------------|
	 |           B may still send remaining data        |
	 |                                  FIN, Seq=8001  |
	 |<------------------------------------------------|
	 | ACK, Ack=8002                                    |
	 |------------------------------------------------>|
 TIME-WAIT                                          CLOSED
```

FIN consumes a sequence number. ACK and FIN can be combined, so termination does not always require four separate packets. Simultaneous close is also possible.

### Half-Close

`shutdown(socket, SHUT_WR)` stops the local sending direction while allowing the application to keep receiving. The peer receives end-of-stream after all preceding data has been delivered.

In POSIX stream-socket code, `recv()` returning **0 for a nonzero requested buffer length** indicates orderly end-of-stream. It is not an empty application message. The local application may still be able to send if its sending direction remains open.

End-of-stream is not proof of a complete application response: validate application framing and expected length.

### Reset

RST aborts a connection instead of completing an orderly byte-stream close. Examples include a connection attempt to a closed port, an abortive application close, or traffic arriving for connection state that no longer exists.

The application typically sees an error; exact timing and error codes depend on the API and state. Buffered data can be lost. RST does not by itself explain *why* the peer or an intermediate device rejected the traffic.

## 15. TCP States You Should Recognize

| State | Meaning | Diagnostic interpretation |
|---|---|---|
| CLOSED | No active connection state | Before open or after completion |
| LISTEN | Waiting for incoming connection requests | Server listener |
| SYN-SENT | SYN sent; awaiting establishment response | Reachability, filtering, or peer startup may be relevant |
| SYN-RECEIVED | SYN received and SYN+ACK sent; awaiting final ACK | Incomplete handshake |
| ESTABLISHED | Connection established | Does not prove application responsiveness |
| FIN-WAIT-1 | Local FIN sent, not yet acknowledged | Local sending side is closing |
| FIN-WAIT-2 | Local FIN acknowledged; awaiting peer FIN | Peer has not completed its sending-side close |
| CLOSE-WAIT | Peer FIN received; local side has not closed | Persistent buildup often means application cleanup is missing |
| CLOSING | FINs crossed; waiting for acknowledgment of local FIN | Simultaneous close path |
| LAST-ACK | Peer FIN received; local FIN sent; awaiting its ACK | Last stage of passive close |
| TIME-WAIT | Waiting after completing close | Protects against delayed segments and supports final ACK retransmission |

### TIME-WAIT Versus CLOSE-WAIT

**TIME-WAIT** normally occurs on the endpoint that actively closes in the usual exchange, not automatically "the client." Simultaneous close can leave both endpoints in TIME-WAIT.

The protocol specifies **2 x MSL** (Maximum Segment Lifetime). Actual durations and implementation policies vary; do not memorize a universal 60-second value.

Its purposes include:

1. Allowing the final ACK to be sent again if the peer retransmits FIN.
2. Allowing delayed old segments to expire before safe reuse of the same connection identity.

**CLOSE-WAIT** means the peer has closed its sending direction, but the local application has not finished its close. A growing, persistent population often requires fixing socket lifecycle management, not changing TIME-WAIT settings.

## 16. Timers, Keepalive, and Failure Detection

| Mechanism | What it addresses |
|---|---|
| Retransmission timer | Unacknowledged transmitted data |
| Persist / zero-window probing | Receiver advertises zero window |
| Delayed ACK timer | Deferring certain acknowledgments briefly |
| Keepalive | Optional probes for a sufficiently idle connection |
| TIME-WAIT timer | Safe connection retirement |
| Application deadline | Maximum time the business workflow is willing to wait |

### Cable Removal Is Not an Immediate TCP Message

If the link disappears, the peer may send neither FIN nor RST. An idle connection can appear established until local events, traffic failure, keepalive, or application logic detects a problem.

TCP keepalive is optional and often configured on long timescales. Its settings are OS-specific. It tests TCP-level reachability, not that a diagnostic service, database, or application worker is healthy.

Use protocol-defined liveness checks or an application heartbeat when appropriate, plus explicit deadlines. On platforms that support it, options such as `TCP_USER_TIMEOUT` can bound certain unacknowledged-data/zero-window conditions; they are not a portable substitute for all application timers.

**Do not mix these clocks:** connect timeout, TCP retransmission behavior, TLS timeout, application-response deadline, and diagnostic-protocol timers solve different problems.

## 17. TCP, UDP, TLS, and QUIC

| Property | TCP | UDP |
|---|---|---|
| Data model | Byte stream | Datagrams |
| Transport setup | Connection handshake | No UDP handshake |
| Ordered reliable delivery | Built in, within a connection | Not provided by UDP |
| Message boundaries | Not preserved | Datagram boundaries preserved |
| Flow/congestion control | Built into TCP | Applications/protocols must provide what they need |
| Multicast/broadcast | Not supported | Can support multicast and, for IPv4, broadcast |
| Typical design fit | Reliable transfer, commands, sessions | Discovery, freshness-sensitive data, custom transports |

UDP is not inherently "real-time" or always faster. An application built over UDP may need congestion control, sequence numbers, deadlines, security, and recovery. Large UDP datagrams can also suffer fragmentation-related loss.

### TLS

TLS above TCP can provide encryption, integrity, and peer authentication. Authentication guarantees depend on configuration, certificate validation, and whether mutual authentication is used. Authorization remains an application/system responsibility.

```text
Application framing -> TLS records -> TCP byte stream -> IP
```

TCP alone provides none of those cryptographic guarantees.

### QUIC

QUIC is a secure transport over UDP with reliable streams, congestion control, and integrated TLS-based security. It avoids TCP-style head-of-line blocking **between independent streams**, although each reliable stream remains ordered and shared congestion still affects performance.

QUIC is useful context for interviews, but it is not a drop-in replacement for standardized automotive TCP protocols.

## 18. Automotive Context: Where TCP Is Used

Vehicles contain networks with different purposes and constraints. Classical CAN/CAN FD, LIN, and automotive Ethernet are not interchangeable layers. TCP/IP is commonly associated with Ethernet-connected ECUs and external connectivity; domain gateways bridge services between networks.

Examples below are generic automotive architectures, **not a claim about a particular vehicle model or OEM implementation**.

| Use case | How TCP may fit | Architectural concern |
|---|---|---|
| Diagnostics over IP (DoIP) | Reliable diagnostic communication over TCP | Routing activation, diagnostic timing, access control |
| Software download / OTA | HTTPS or another reliable transfer mechanism | Resume, package authenticity, power loss, rollback |
| SOME/IP services | TCP is one transport option; UDP is another | Interface requirements, framing, discovery, transport binding |
| Telematics | MQTT over TLS/TCP, HTTPS, or other protocols | Cellular outages, reconnects, bandwidth, credentials |
| Logging and engineering tools | Reliable export sessions | Backpressure, storage limits, interference with critical traffic |

### Why Not TCP for Every Vehicle Signal?

A fresh measurement may be more useful than a retransmitted old one. TCP's ordered recovery can delay newer data behind lost bytes.

For deadline-sensitive control and data streams, choose the entire communication architecture based on timing, freshness, loss tolerance, and safety requirements. UDP, CAN, scheduled Ethernet, or other mechanisms may be appropriate; **no transport choice alone proves a safety or real-time guarantee**.

Ethernet QoS, VLAN priority, and TSN mechanisms can support bounded network behavior when engineered correctly. They do not remove TCP recovery delays or application scheduling delays.

## 19. DoIP: Connect TCP Knowledge to Diagnostics

**DoIP = Diagnostics over Internet Protocol**, standardized in the ISO 13400 family. **UDS = Unified Diagnostic Services**, defined in the ISO 14229 family.

DoIP transports diagnostic communication over IP. UDS defines diagnostic services such as reading diagnostic data, changing sessions, and programming-related operations. They are different protocol layers.

```text
Diagnostic tester
			|
			| Ethernet / IP
			v
Vehicle DoIP entity or gateway
			|
			+-- Local diagnostic server
			|
			+-- Routed communication to another ECU
					(possibly over another in-vehicle network)
```

### Typical Lifecycle

1. **Discovery / identification:** DoIP uses UDP for discovery and certain information/status exchanges. Common standardized non-secure DoIP traffic uses port 13400 for UDP and TCP in their respective roles.
2. **TCP connection:** tester connects to the vehicle's DoIP endpoint.
3. **Security setup, where required:** the applicable profile may require TLS or other security measures. Secure-port and authentication details depend on the adopted specification/profile.
4. **Routing activation:** an application-level DoIP exchange establishes the requested diagnostic routing context. TCP establishment alone is not routing activation.
5. **Diagnostic exchange:** DoIP diagnostic messages carry source/target logical addresses and diagnostic payloads such as UDS.
6. **Liveness and closure:** DoIP/application timers and alive-check mechanisms operate separately from TCP's own mechanisms.

### Three Different Acknowledgment Levels

| Event | What it means | What it does not mean |
|---|---|---|
| TCP ACK | Bytes accepted by the peer TCP endpoint | DoIP parsing or UDS execution succeeded |
| DoIP diagnostic positive acknowledgment | DoIP-level acceptance according to the protocol | The requested UDS operation completed successfully |
| UDS response | Diagnostic-service result or response status | Always success: UDS also has negative responses and response-pending behavior |

This distinction is especially important when a DoIP gateway acknowledges traffic before a downstream ECU finishes processing it.

### DoIP Framing

The generic DoIP header is **8 bytes**:

```text
Protocol version          1 byte
Inverse protocol version  1 byte
Payload type              2 bytes
Payload length            4 bytes
Payload                   variable length
```

A TCP receive callback may contain half a header, one complete DoIP message, or several messages. Your parser must accumulate bytes, validate header fields and permitted lengths, and extract complete messages.

DoIP logical addresses are not TCP ports or IP addresses. They identify diagnostic entities within the diagnostic communication model.

UDS response timers, DoIP timers, and TCP timers must be configured and analyzed separately. Use the adopted ISO editions and OEM requirements for exact normative behavior.

## 20. SOME/IP and AUTOSAR Placement

### SOME/IP

SOME/IP supports service-oriented communication using TCP or UDP depending on the interface and configured transport mapping. **SOME/IP Service Discovery uses UDP**, not TCP service handshakes.

For TCP-based SOME/IP communication:

- TCP establishes and maintains the byte stream.
- SOME/IP headers define message framing and service/method identifiers.
- Service availability and discovery are separate from TCP connection state.
- Reliable transport does not eliminate application return codes, deadlines, or version compatibility checks.

Do not assume a deployment selects TCP dynamically merely because a message is large; transport mapping is a design/configuration decision.

### AUTOSAR Classic: Simplified Communication View

```text
Application / diagnostic or service protocol modules
											 |
											SoAd
											 |
										 TcpIp
											 |
											EthIf
											 |
							Ethernet driver / hardware
```

- **TcpIp:** provides the TCP/IP stack functions.
- **SoAd (Socket Adaptor):** adapts socket communication to upper-layer AUTOSAR communication interfaces.
- **EthIf:** abstracts Ethernet-controller interfaces toward upper layers.
- DoIP, SOME/IP-related modules, PduR, DCM, and other modules participate according to the selected stack and routing configuration; the drawing omits those detailed paths.

AUTOSAR Adaptive uses a different platform/service architecture, often with OS networking underneath. Do not apply a Classic BSW module diagram directly to every Adaptive or Linux ECU.

## 21. Socket Programming: Rules for C++ Developers

On POSIX-like systems such as Linux:

```text
Server: socket -> bind -> listen -> accept -> recv/send -> shutdown/close
Client: socket -> connect -> send/recv -> shutdown/close
```

AUTOSAR stacks and other operating systems may expose different APIs or callbacks. The TCP stream semantics still matter.

### Rules That Prevent Common Bugs

1. **Handle partial sends.** A successful `send()` may accept fewer bytes than requested. Maintain an offset and send the remainder.
2. **Handle partial and combined receives.** Parse incrementally; never cast each read buffer to a complete application message.
3. **Interpret nonblocking results correctly.** `EAGAIN`/`EWOULDBLOCK` means wait for readiness, not necessarily connection failure. Handle `EINTR` according to the operation/API contract.
4. **Check nonblocking connect completion.** Writable readiness alone does not prove success; inspect `SO_ERROR` as required by the platform.
5. **Handle EOF and errors separately.** `recv() == 0` on a nonzero-length read indicates orderly EOF; a negative return requires error inspection.
6. **Do not equate send success with delivery.** It generally means bytes were accepted locally. Require an application response for business confirmation.
7. **Handle writes after peer closure.** On POSIX systems, consider `SIGPIPE` and `EPIPE`; use an appropriate platform policy such as Linux `MSG_NOSIGNAL` where applicable.
8. **Bound memory and waiting.** Limit receive lengths, pending requests, outbound queues, and operation duration.
9. **Manage ownership.** Use RAII for socket lifetime and a defined policy for cancellation, shutdown, and concurrent access.
10. **Serialize application framing.** Concurrent writers need a policy that prevents logical messages from being interleaved through partial-write sequences.

### Why This Guide Does Not Use a One-Shot Echo Example

A short example with one `send()` and one `recv()` can accidentally teach message-boundary assumptions. The correct unit of design is a connection state machine with buffers, incremental parsing, deadlines, and cleanup.

### Minimum Test Cases

- Header delivered one byte at a time.
- Payload split across many reads.
- Several messages delivered in one read.
- Invalid, oversized, or incomplete message.
- Partial send and nonblocking backpressure.
- Peer EOF during a message and after a complete message.
- Peer reset, link interruption, and reconnect.
- Duplicate application request after an uncertain previous outcome.

## 22. Architect-Level Decisions

### Define the Contract Before Choosing Options

| Question | Why it matters |
|---|---|
| Must every byte arrive, or is only the latest value useful? | Reliability versus freshness |
| What are the deadline and tail-latency requirements? | TCP recovery has variable delay |
| How many simultaneous connections exist? | Buffers, state, descriptors, and embedded RAM |
| What is the maximum message size? | Parser safety and bounded allocation |
| Can the ECU sleep, reboot, or lose connectivity? | Lifecycle and reconnection design |
| Are operations idempotent? | Safe retries after uncertain results |
| Where does TLS terminate? | Actual confidentiality and trust boundaries |
| What does success mean? | Transport receipt versus durable application completion |

### Retries and Exactly-Once Effects

```text
Client sends command.
Server executes command.
Connection fails before client receives the result.
Client cannot tell whether execution occurred.
```

Blind retry can repeat a non-idempotent operation. Depending on the application, use request IDs, deduplication, idempotent operations, status queries, or durable transaction records. Exactly-once effects require application/storage design; TCP cannot create that guarantee by itself.

For example, after a lost programming response, follow the diagnostic/programming protocol's recovery rules rather than arbitrarily resending a state-changing command.

### Reconnection

Use bounded retries and exponential backoff with jitter where appropriate. A new TCP connection does not restore TLS state, DoIP routing activation, application authentication, or outstanding request state automatically.

Network changes, NAT expiration, vehicle sleep, and cellular handovers can break a connection. Re-establish application context explicitly and avoid synchronized reconnect storms.

### Isolation and Resource Budgets

Bulk downloads and latency-sensitive commands on one TCP stream share head-of-line blocking. Separate connections can isolate stream ordering, but still compete for shared CPU, memory, and link capacity.

Estimate memory using implementation-specific accounting for per-connection state, send/receive buffers, TLS buffers, and application queues. Do not multiply configured socket buffer values blindly and assume that equals actual allocation on every platform.

Define queue limits, overload behavior, connection limits, and observability before production deployment.

### Security

Threats include spoofing, SYN floods, connection exhaustion, malicious payload lengths, unauthorized diagnostics, and eavesdropping on unencrypted traffic.

Defenses include TLS with proper identity validation, application authorization, network segmentation, filtering, rate/resource limits, robust parsers, and secure update validation. Stack mechanisms such as SYN cookies can mitigate particular SYN-flood conditions but are not a complete denial-of-service defense.

Treat external diagnostic and telematics access as a security boundary. A reliable transport is not a trusted transport.

## 23. Debugging TCP Systematically

Use captures only on authorized networks. Diagnostic payloads, credentials, and vehicle identifiers can be sensitive; limit and protect capture files.

### First Ask: At Which Layer Does Progress Stop?

```text
Link up?
	-> IP address, route, and neighbor resolution correct?
		-> TCP handshake completes?
			-> TLS completes, if required?
				-> DoIP routing activation / application session completes?
					-> Request reaches the intended service?
						-> Valid response arrives within the required deadline?
```

A successful ping does not prove a TCP port is reachable. A successful TCP handshake does not prove the diagnostic service is ready.

### Useful Linux Commands

These are reference commands, not commands executed by this guide. Availability depends on installed tools; packet capture and process details may require appropriate permissions.

```bash
ip addr
ip route
ip neigh
ss -ltn
ss -tin
ss -tan state close-wait
ss -tan state time-wait
tcpdump -i eth0 -nn -s 0 -c 500 -w tcp_capture.pcap 'tcp port 13400'
```

Replace `eth0` and the port with the actual authorized interface/service. The example capture is bounded to 500 packets and excludes UDP discovery; include UDP when investigating DoIP discovery.

### Wireshark Display Filters

```text
tcp.port == 13400
udp.port == 13400
tcp.stream eq 0
tcp.flags.syn == 1
tcp.flags.reset == 1
tcp.analysis.retransmission
tcp.analysis.duplicate_ack
tcp.analysis.zero_window
```

Select the actual stream index for the connection of interest. "Follow TCP Stream" can reconstruct application bytes; TLS payloads remain encrypted unless authorized decryption material is available.

### Symptom-to-Hypothesis Table

| Observation | Plausible causes | Next check |
|---|---|---|
| Repeated SYN, no response | Drop, wrong destination, unavailable peer, missing return path | Capture at both ends; check route, filter, listener |
| SYN receives RST | Closed port or active rejection | Listener binding and policy |
| Handshake succeeds, application silent | Peer waits for framing, TLS, activation, or another request step | Inspect application state and payload |
| Duplicate ACKs / retransmissions | Loss, reordering, or capture artifacts | Compare capture points and sequence ranges |
| Receiver advertises zero window | Application not draining data or resource pressure | Receive path, scheduling, and buffer occupancy |
| Small writes show periodic stalls | Nagle/delayed ACK interaction, batching, or scheduling | Compare send-call timestamps and packet timing |
| Small data works, large transfer stalls | MTU issue or another size-dependent failure | Path MTU, ICMP feedback, tunnels, packet sizes |
| Persistent CLOSE-WAIT buildup | Local application not finishing close | Socket ownership and cleanup paths |
| Many TIME-WAIT connections | High connection churn, often expected | Connection reuse and ephemeral-port pressure |
| RST after peer reboot | Old connection state no longer exists | Reconnect and application recovery behavior |

Packet-analyzer labels are heuristics, not ground truth. Capture loss, asymmetric visibility, and offloads can produce misleading diagnoses.

Outbound checksum errors in a host capture may be caused by checksum offload: the NIC fills in the checksum after the capture point. Confirm on the wire or at the receiver before declaring corruption.

### Metrics Worth Collecting

- Connection success/failure rate and setup latency.
- RTT, retransmissions, congestion information, and zero-window events where available.
- Application response latency, deadline violations, and incomplete messages.
- Active connection count, queue depth, memory pressure, and reconnect rate.
- DoIP routing-activation failures and UDS response outcomes separately from TCP errors.

## 24. Interview Questions With Short Answers

| Question | Answer to remember |
|---|---|
| What is TCP? | A reliable, ordered, full-duplex byte-stream transport over IP. |
| Why three-way handshake? | To synchronize and acknowledge both independent initial sequence numbers. |
| Does TCP number packets? | No, bytes; SYN and FIN also consume one sequence number each. |
| What does ACK=100 mean? | All preceding contiguous sequence-space positions are acknowledged; 100 is next expected. |
| Does ACK prove request execution? | No, only transport-level receipt at the peer endpoint. |
| Can one send require several reads? | Yes; TCP does not preserve message boundaries. |
| Can one read contain several messages? | Yes; application framing must separate them. |
| Flow control versus congestion control? | Receiver protection via `rwnd`; network protection via sender `cwnd`. |
| How does TCP recover loss? | Retransmissions driven by timeout or loss-recovery mechanisms; SACK can identify received ranges. |
| What happens if an ACK is lost? | A later cumulative ACK can cover it; otherwise data may be retransmitted. |
| What is head-of-line blocking? | A missing byte range prevents delivery of later stream bytes. |
| What is MSS versus MTU? | TCP payload limit versus IP packet size limit on a link/path. |
| Why window scaling? | To advertise receive windows larger than the 16-bit field alone permits. |
| What does TCP_NODELAY do? | Disables Nagle, not all buffering or delayed ACKs. |
| FIN versus RST? | Orderly end of one sending direction versus abrupt reset. |
| Why TIME-WAIT? | Handle retransmitted FIN and protect against delayed old segments. |
| Which endpoint enters TIME-WAIT? | Normally the active closer; not necessarily the client. |
| Why many CLOSE-WAIT sockets? | The peer closed, but the local application has not completed its close. |
| Does TCP detect unplugging immediately? | Not necessarily; traffic, timers, or local events must reveal it. |
| Is keepalive application health? | No; use appropriate application-level checks and deadlines. |
| Is TCP secure? | Not cryptographically; normally add TLS and application authorization. |
| Is TCP suitable for hard real-time? | TCP alone cannot guarantee bounded delivery latency. |
| Where is TCP used in automotive? | DoIP diagnostic sessions, some SOME/IP services, downloads, and telematics. |
| Does DoIP use only TCP? | No; UDP supports discovery and certain other exchanges. |
| Does TCP replace UDS or DoIP? | No; these define application/diagnostic behavior above transport. |
| Can TCP ensure exactly-once commands after reconnect? | No; use application-level identity, deduplication, and recovery semantics. |

### A 60-Second Automotive Answer

> TCP gives applications a reliable, ordered byte stream over IP. It establishes state using a three-way handshake, numbers bytes, acknowledges received ranges, and retransmits missing data. Flow control protects the receiver; congestion control protects the network. Applications still need message framing, deadlines, security, and recovery logic. In automotive systems, TCP is used for DoIP diagnostic communication and some service or backend connections. DoIP discovery uses UDP, and DoIP routing activation and UDS responses are separate from TCP establishment and ACKs. TCP is useful when complete ordered transfer matters, but retransmission and head-of-line blocking mean it does not itself guarantee real-time deadlines.

## 25. Practice Scenarios

Try answering before reading each explanation.

### Scenario A: Sequence Arithmetic

Client ISN is 100. After the handshake, it sends 200 bytes. What ACK is expected?

**Answer:** SYN consumes sequence 100. Data occupies 101-300. Expected ACK is **301**.

### Scenario B: Receive Boundaries

The sender transmits two 8-byte messages. The receiver reads 5 bytes, then 11 bytes. Is TCP broken?

**Answer:** No. Buffer and parse the stream according to the application's framing.

### Scenario C: Vehicle Programming Response Lost

A tester sends a state-changing command and the connection drops before its response arrives. Should it immediately repeat the command?

**Answer:** Not blindly. Execution status is uncertain. Follow protocol-specific recovery, status checks, or permitted deduplication/idempotency rules.

### Scenario D: TCP Works, Diagnostics Fail

The handshake completes, but UDS requests receive no valid response. Where do you investigate?

**Answer:** TLS requirements, DoIP framing/routing activation, logical addressing, gateway routing, ECU diagnostic state, authorization, and diagnostic timers. TCP establishment alone is insufficient.

### Scenario E: Fast Link, Slow Download

A 100 Mbit/s connection with 20 ms RTT has an effective in-flight window of 64 KiB. Why might throughput remain around 26 Mbit/s or less?

**Answer:** The window is below the 250,000-byte BDP. Check `rwnd`, `cwnd`, scaling, loss, and application/storage behavior before assuming the Ethernet link is faulty.

### Scenario F: Receiver Pauses

A receiver advertises zero window while its worker thread is blocked. Should you increase network bandwidth?

**Answer:** Not as the first fix. Investigate why the application is not draining the receive path and how backpressure is managed.

## 26. One-Page Revision Sheet

```text
TCP = reliable + ordered + full duplex + BYTE STREAM
Layer = transport, above IP
Connection = source IP/port + destination IP/port
Base header = 20 bytes; maximum header = 60 bytes

Open = SYN -> SYN+ACK -> ACK
Sequence = counts bytes, not packets
ACK = next expected sequence number
SYN and FIN = each consume 1 sequence number
Pure ACK = consumes 0 sequence numbers

Reliability = ACK + retransmission + ordering + duplicate suppression
SACK = report received ranges beyond a gap
Loss recovery = timeout and other detection mechanisms
Head-of-line blocking = missing earlier bytes delay later delivery

rwnd = receiver capacity
cwnd = network congestion limit
Outstanding data roughly bounded by min(rwnd, cwnd)
BDP = bandwidth * RTT (use consistent units)
MSS = TCP payload; MTU = IP packet size limit

Close = FIN/ACK in each direction, sometimes combined
FIN = orderly end of sending direction
RST = abort/reset
TIME-WAIT = delayed-segment protection and final-ACK handling
CLOSE-WAIT = peer FIN received; local application still must close

TCP does NOT provide:
	Message boundaries, encryption, application success,
	exactly-once operations across reconnects, or a delivery deadline.

Automotive:
	DoIP discovery -> UDP
	DoIP diagnostic communication -> TCP
	TCP connected != DoIP routing activated != UDS success
	SOME/IP -> configured TCP or UDP transport
	SOME/IP discovery -> UDP

Implementation:
	Partial sends + incremental parsing + bounded queues
	Explicit deadlines + secure identity + safe retry semantics
```

## 27. References and Further Study

Use these to verify details and distinguish baseline TCP from optional extensions:

- [RFC 9293: Transmission Control Protocol](https://www.rfc-editor.org/rfc/rfc9293.html): modern core TCP specification; supersedes RFC 793.
- [RFC 5681: TCP Congestion Control](https://www.rfc-editor.org/rfc/rfc5681.html): classic slow start, congestion avoidance, and recovery behavior.
- [RFC 6298: Computing TCP's Retransmission Timer](https://www.rfc-editor.org/rfc/rfc6298.html): RTT/RTO calculation and backoff.
- [RFC 2018: TCP Selective Acknowledgment Options](https://www.rfc-editor.org/rfc/rfc2018.html): SACK negotiation and blocks.
- [RFC 7323: TCP Extensions for High Performance](https://www.rfc-editor.org/rfc/rfc7323.html): window scaling and timestamps.
- [RFC 3168: Explicit Congestion Notification](https://www.rfc-editor.org/rfc/rfc3168.html): baseline ECN behavior.
- [RFC 8985: RACK-TLP Loss Detection](https://www.rfc-editor.org/rfc/rfc8985.html): time-based loss detection and tail-loss probing.
- [RFC 8446: TLS 1.3](https://www.rfc-editor.org/rfc/rfc8446.html): secure transport above TCP.
- [RFC 9000: QUIC](https://www.rfc-editor.org/rfc/rfc9000.html): UDP-based secure multiplexed transport.
- [Linux tcp(7)](https://man7.org/linux/man-pages/man7/tcp.7.html): Linux-specific behavior and socket options.
- **ISO 13400 family:** authoritative DoIP requirements; use the edition/profile adopted by your project.
- **ISO 14229 family:** authoritative UDS requirements, including applicable transport/session timing rules.
- **AUTOSAR specifications:** consult the applicable release of TcpIp, Socket Adaptor, DoIP, SOME/IP, and SOME/IP Service Discovery documents.

Related local reading: [SOCKS5_GUIDE.md](SOCKS5_GUIDE.md).
