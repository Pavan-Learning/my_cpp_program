# SOCKS5 — Beginner to Technical Architect Guide

## 1. What is SOCKS5?

SOCKS5 is a **proxy protocol**.

It allows a client application to communicate with a destination through an intermediate SOCKS server.

Without SOCKS5:

    Client Application
            |
            | TCP
            v
       Destination

With SOCKS5:

    Client Application
            |
            | SOCKS5
            v
       SOCKS5 Server
            |
            | TCP/UDP
            v
       Destination

A simple mental model:

> "Please make a network connection to this destination for me, and relay the data between us."

SOCKS5 is defined by RFC 1928.

---

## 2. SOCKS5 is NOT a VPN

SOCKS5 normally works at the application/proxy level.

    Browser ------> SOCKS5
    SSH ----------> SOCKS5
    App A --------> Direct
    App B --------> Direct

A VPN generally affects system/network traffic more broadly.

    Client
      |
      +-- Application A
      +-- Application B
      +-- Browser
      +-- SSH
      |
      v
    VPN Tunnel
      |
      v
    VPN Server

Important:

    SOCKS5 != VPN

---

## 3. Basic Networking Concepts

Before understanding SOCKS5, know these concepts.

### IP address

Identifies a network host/interface.

Example:

    10.10.10.20

### Port

Identifies a service/process endpoint.

Example:

    10.10.10.20:443

So:

    IP + Port = network endpoint

### TCP

TCP provides a reliable, ordered byte stream between endpoints.

Typical TCP connection setup:

    Client                         Server
      |                              |
      | -------- SYN --------------> |
      | <------- SYN + ACK --------- |
      | -------- ACK --------------> |
      |                              |
      |       TCP connection         |
      |                              |

### UDP

UDP sends independent datagrams and does not establish a TCP-style connection.

    Client ---- packet ----> Server
    Client ---- packet ----> Server

---

# 4. What changes with SOCKS5?

Without proxy:

    Client -----------------------> Destination

With SOCKS5:

    Client -----------------------> SOCKS Server
                                      |
                                      |
                                      v
                                  Destination

For a TCP CONNECT operation there are effectively two TCP connections:

    Connection #1
    Client <=================> SOCKS Server

    Connection #2
    SOCKS Server <============> Destination

The SOCKS server bridges/relays the byte streams.

---

# 5. Typical SOCKS5 TCP Flow

The high-level sequence is:

    1. Client establishes TCP connection to SOCKS server
                         |
                         v
    2. SOCKS5 method negotiation
                         |
                         v
    3. Authentication, if required
                         |
                         v
    4. Client sends CONNECT request
                         |
                         v
    5. SOCKS server connects to destination
                         |
                         v
    6. SOCKS server returns success/failure
                         |
                         v
    7. Bidirectional application data relay
                         |
                         v
    8. Connection closes

Detailed diagram:

    Client                         SOCKS5 Server                 Destination
      |                                  |                           |
      | -------- TCP connect ----------> |                           |
      |                                  |                           |
      | ---- SOCKS5 greeting ----------> |                           |
      | <--- method selection ---------- |                           |
      |                                  |                           |
      | ---- authentication ----------> |                           |
      | <--- authentication result ---- |                           |
      |                                  |                           |
      | ---- CONNECT destination ------> |                           |
      |                                  | ---- TCP connect --------> |
      |                                  | <--- connection result -- |
      | <--- CONNECT result ------------ |                           |
      |                                  |                           |
      | <========= application data ===> | <====== application data =>|
      |                                  |                           |
      | -------- close ----------------> | -------- close ----------> |

---

# 6. SOCKS5 Greeting

The client tells the server:

> I am a SOCKS5 client and these are the authentication methods I support.

General format:

    +--------+----------+----------------+
    | VER    | NMETHODS | METHODS        |
    +--------+----------+----------------+
    | 1 byte | 1 byte   | variable       |
    +--------+----------+----------------+

Example:

    05 01 00

Meaning:

    05 = SOCKS5
    01 = one authentication method
    00 = no authentication required

Server response:

    +--------+--------+
    | VER    | METHOD |
    +--------+--------+
    | 1 byte | 1 byte |
    +--------+--------+

Example:

    05 00

Meaning:

    SOCKS5
    Use NO AUTHENTICATION

---

# 7. Authentication Methods

Common SOCKS5 methods include:

    0x00 = NO AUTHENTICATION REQUIRED
    0x01 = GSSAPI
    0x02 = USERNAME/PASSWORD

Example:

    Client supports:
        NO AUTH
        USERNAME/PASSWORD

    Server selects:
        USERNAME/PASSWORD

Authentication answers:

> "Is this client allowed to use the proxy?"

Authentication does NOT automatically mean encryption.

Important:

    Authentication != Encryption

---

# 8. CONNECT Request

After authentication, the client can request:

> Connect me to this destination.

General structure:

    +-----+-----+-----+------+----------+----------+
    | VER | CMD | RSV | ATYP | DST.ADDR | DST.PORT |
    +-----+-----+-----+------+----------+----------+

Fields:

    VER
        SOCKS version = 05

    CMD
        Command

    RSV
        Reserved = normally 00

    ATYP
        Address type

    DST.ADDR
        Destination address

    DST.PORT
        Destination port

---

# 9. SOCKS5 Commands

Three important commands:

    01 = CONNECT
    02 = BIND
    03 = UDP ASSOCIATE

## CONNECT

Most common.

Meaning:

> Connect me to this destination and relay the traffic.

    Client
       |
       | CONNECT 10.20.30.40:443
       v
    SOCKS Server
       |
       | TCP connect
       v
    10.20.30.40:443

## BIND

Used for cases where the proxy assists with an incoming connection.

Conceptually:

    Client
       |
       | BIND
       v
    SOCKS Server
       |
       | listens for incoming connection
       |
       | <--------- Remote peer
       |
       v
    Client

BIND is less common in typical modern SOCKS5 usage.

## UDP ASSOCIATE

Used to proxy UDP traffic.

    Client
       |
       | TCP control connection
       v
    SOCKS Server
       |
       | UDP
       v
    UDP Destination

The TCP connection is used for the SOCKS association/control, while proxied application data is UDP.

---

# 10. Address Types (ATYP)

SOCKS5 supports:

    01 = IPv4
    03 = DOMAIN NAME
    04 = IPv6

Example IPv4:

    192.168.1.100:443

Example domain:

    example.com:443

Example IPv6:

    2001:db8::100:443

---

# 11. Domain Name and DNS

Suppose the application wants:

    example.com:443

There are two important possibilities.

### Client-side DNS

    Application
        |
        | DNS lookup
        v
    Local DNS resolver
        |
        v
    IP address

    Application
        |
        | CONNECT IP:443
        v
    SOCKS Server

### Proxy-side DNS

    Application
        |
        | CONNECT example.com:443
        v
    SOCKS Server
        |
        | DNS lookup
        v
    DNS resolver
        |
        v
    Destination IP

Proxy-side DNS can be useful when the SOCKS server has access to private/internal DNS that the client cannot access.

---

# 12. CONNECT Response

The server responds to the CONNECT request.

General structure:

    +-----+-----+-----+------+----------+----------+
    | VER | REP | RSV | ATYP | BND.ADDR | BND.PORT |
    +-----+-----+-----+------+----------+----------+

The most important field is REP.

Important reply codes:

    00 = Succeeded
    01 = General SOCKS server failure
    02 = Connection not allowed by ruleset
    03 = Network unreachable
    04 = Host unreachable
    05 = Connection refused
    06 = TTL expired
    07 = Command not supported
    08 = Address type not supported

Example:

    REP = 05

Means:

    Destination connection was refused.

---

# 13. Two Different Failure Points

This is extremely useful for debugging.

### Failure A — Client cannot reach SOCKS

    Client
       |
       X
       |
    SOCKS Server

The SOCKS protocol negotiation may never happen.

### Failure B — SOCKS cannot reach destination

    Client
       |
       v
    SOCKS Server
       |
       X
       |
    Destination

The first TCP connection works, but the second connection fails.

Always think about these as two separate network legs.

---

# 14. Data Relay

Once CONNECT succeeds:

    Client
       |
       | bytes
       v
    SOCKS Server
       |
       | bytes
       v
    Destination

Reverse direction:

    Destination
       |
       | bytes
       v
    SOCKS Server
       |
       | bytes
       v
    Client

The SOCKS server normally does not need to understand the application protocol.

It can relay bytes.

Therefore SOCKS can be used with protocols such as:

    HTTP
    HTTPS
    SSH
    MQTT
    FTP
    Custom binary protocols
    Database protocols
    Other TCP protocols

---

# 15. SOCKS5 is Not an HTTP Proxy

HTTP proxy:

    HTTP Client
        |
        | HTTP proxy protocol
        v
    HTTP Proxy

SOCKS5:

    Application
        |
        | SOCKS5 protocol
        v
    SOCKS5 Proxy

HTTP proxies understand HTTP concepts such as:

    GET
    POST
    Headers
    Host
    HTTP methods

SOCKS5 generally does not need to understand the application protocol.

---

# 16. SOCKS5 and HTTPS/TLS

SOCKS5 does not provide application-data encryption by itself.

For example:

    Application
        |
        | TLS encrypted data
        v
    SOCKS5 Proxy
        |
        | same encrypted bytes
        v
    Destination

Responsibilities are different:

    SOCKS5
        "How do I reach the destination?"

    TLS
        "How do I protect the application data?"

Therefore:

    SOCKS5 != TLS

---

# 17. What Can a SOCKS Server See?

Without additional encryption, the proxy can see SOCKS-level information such as:

    Client connection
    Destination address
    Destination port
    Timing
    Traffic volume

If the application uses TLS:

    Application
        |
        | TLS encrypted payload
        v
    SOCKS Server
        |
        v
    Destination

The SOCKS server normally cannot read the protected application content.

However, SOCKS5 itself does not guarantee anonymity or privacy.

---

# 18. SOCKS5 over TLS

A system can also carry SOCKS5 inside a TLS connection:

    SOCKS5 messages
          |
          v
         TLS
          |
          v
         TCP
          |
          v
      SOCKS Server

This is different from basic SOCKS5.

TLS protects the SOCKS communication path; SOCKS provides proxying.

---

# 19. SOCKS5 over SSH

Another common architecture is dynamic proxying through SSH:

    Application
        |
        | SOCKS
        v
    SSH Client
        |
        | encrypted SSH connection
        v
    SSH Server
        |
        v
    Destination

Here:

    SOCKS = proxy/destination selection
    SSH  = encrypted tunnel

Again, these are different responsibilities.

---

# 20. SOCKS5 vs VPN

| Feature | SOCKS5 | VPN |
|---|---|---|
| Main purpose | Proxy traffic | Network tunnel |
| Typical scope | Application | System/network |
| TCP | Yes | Yes |
| UDP | Yes | Yes |
| Built-in encryption | No | Usually |
| Application configuration | Usually needed | Usually not |
| General routing changes | Not inherently | Usually |
| Proxy protocol | Yes | Tunnel technology |

Simple mental model:

    SOCKS5:
        Application -> Proxy -> Destination

    VPN:
        Device/Network -> Encrypted tunnel -> VPN network

---

# 21. SOCKS5 vs HTTP Proxy

| Feature | SOCKS5 | HTTP Proxy |
|---|---|---|
| HTTP-specific | No | Yes |
| TCP proxying | Yes | Yes |
| UDP proxying | Yes | Generally no |
| Understands HTTP | No | Yes |
| Domain names | Yes | Yes, depending on mode |
| Authentication | Yes | Depends on implementation |
| Built-in encryption | No | No |

---

# 22. SOCKS4 vs SOCKS5

SOCKS5 is newer and more capable.

SOCKS4:

    TCP CONNECT
    BIND

SOCKS5:

    CONNECT
    BIND
    UDP ASSOCIATE
    IPv4
    IPv6
    Domain names
    Authentication negotiation

---

# 23. SOCKS5 Server Responsibilities

A SOCKS5 server generally performs:

    1. Accept client connection
    2. Parse SOCKS5 greeting
    3. Select authentication method
    4. Authenticate client
    5. Parse request
    6. Validate destination
    7. Resolve destination if required
    8. Establish outbound connection
    9. Send response
    10. Relay traffic
    11. Handle timeout/error
    12. Close connections correctly

---

# 24. SOCKS5 Client Responsibilities

A SOCKS5 client generally performs:

    1. Connect to proxy
    2. Send greeting
    3. Negotiate authentication
    4. Authenticate
    5. Send CONNECT/BIND/UDP request
    6. Parse response
    7. Start data transfer
    8. Handle proxy errors
    9. Handle connection close

---

# 25. SOCKS5 as a State Machine

A useful architecture model:

                     +-------+
                     | START |
                     +---+---+
                         |
                         v
                  +--------------+
                  | TCP CONNECT  |
                  +------+-------+
                         |
                         v
                  +--------------+
                  | NEGOTIATION  |
                  +------+-------+
                         |
                         v
                  +--------------+
                  | AUTHENTICATE |
                  +------+-------+
                         |
                         v
                  +--------------+
                  | SEND REQUEST |
                  +------+-------+
                         |
                         v
                  +--------------+
                  | WAIT REPLY   |
                  +------+-------+
                         |
                    +----+----+
                    |         |
                 success    failure
                    |         |
                    v         v
                +-------+    ERROR
                | RELAY |
                +---+---+
                    |
                    v
                  CLOSE

This state-machine view is particularly useful when implementing SOCKS5.

---

# 26. SOCKS5 Server in C++ — Architectural View

A possible design:

    SOCKS5Server
          |
          +---- accept()
          |
          +---- ClientSession
                    |
                    +---- negotiate()
                    |
                    +---- authenticate()
                    |
                    +---- parseRequest()
                    |
                    +---- connectDestination()
                    |
                    +---- relay()

Each client session may contain:

    ClientSession
        |
        +-- client socket
        +-- destination socket
        +-- protocol state
        +-- authentication context
        +-- timeout state

Multiple clients:

    SOCKS5Server
        |
        +---- Session 1
        +---- Session 2
        +---- Session 3
        +---- Session 4

---

# 27. Production Concerns

As an architect, don't stop at the protocol fields.

Consider:

## Connectivity

    Client -> SOCKS
    SOCKS -> Destination

## Transport

    TCP?
    UDP?

## SOCKS command

    CONNECT?
    BIND?
    UDP ASSOCIATE?

## DNS

    Client-side?
    Proxy-side?

## Security

    Authentication?
    TLS?
    Credentials?

## Authorization

    Which clients?
    Which destinations?
    Which ports?

## Reliability

    Connection timeout?
    Read/write timeout?
    Idle timeout?
    Retry?
    Proxy failover?

## Capacity

    Maximum clients?
    Maximum connections?
    Maximum bandwidth?

## Observability

    Connection logs?
    Authentication failures?
    Destination failures?
    Latency?
    Bytes transferred?

---

# 28. Open Proxy Risk

A SOCKS server that accepts connections from everyone and allows arbitrary destinations can become an open proxy.

Potential architecture:

    Internet
       |
       v
    SOCKS5 Server
       |
       +----> Anywhere

This can be abused.

Production systems should consider:

    Authentication
    Authorization
    Destination restrictions
    Client restrictions
    Rate limiting
    Connection limits
    Logging
    Monitoring

---

# 29. SOCKS5 and Embedded/Automotive Architecture

A possible embedded architecture:

    +-------------------------+
    | Embedded ECU            |
    |                         |
    | Application             |
    | SOCKS5 Client           |
    +------------+------------+
                 |
                 | TCP
                 v
    +-------------------------+
    | Gateway                 |
    |                         |
    | SOCKS5 Server           |
    +------------+------------+
                 |
                 | TCP
                 v
    +-------------------------+
    | Backend                 |
    |                         |
    | 10.20.30.40:443         |
    +-------------------------+

The ECU may only need reachability to the gateway.

The gateway can have reachability to the backend.

This can be useful in restricted network environments.

---

# 30. Local SOCKS5 Example

A SOCKS server may run locally:

    127.0.0.1:1080

Then:

    Application
        |
        | SOCKS5
        v
    127.0.0.1:1080
        |
        v
    Remote Destination

Port 1080 is a commonly used SOCKS port, but it is only a convention. A SOCKS server can use another port.

---

# 31. SOCKS5 Does Not Automatically Proxy Every Application

If a SOCKS5 server exists, that does not mean all applications automatically use it.

For example:

    Browser ----> SOCKS
    SSH --------> SOCKS
    App A ------> Direct
    App B ------> Direct

Applications generally need:

    Native SOCKS support

or:

    SOCKS-aware library

or:

    Traffic redirection/interception mechanism

Transparent proxying is a more advanced architecture.

---

# 32. Transparent Proxy Architecture

The application may not know that SOCKS exists:

    Application
        |
        | normal connect()
        v
    Network interception
        |
        v
    SOCKS proxy
        |
        v
    Destination

Linux systems can implement related architectures using mechanisms such as:

    iptables
    nftables
    TPROXY
    redsocks

The exact architecture depends on requirements.

---

# 33. SOCKS5 and Connection Multiplexing

Basic SOCKS5 does not mean that many application connections automatically share one TCP connection to the proxy.

Usually:

    Application connection 1
             |
             v
       SOCKS connection 1

    Application connection 2
             |
             v
       SOCKS connection 2

    Application connection 3
             |
             v
       SOCKS connection 3

Multiplexing is a separate design concern.

---

# 34. TCP Connection Ownership

For:

    Client <----> SOCKS <----> Destination

There are two connections.

    Connection #1:

    Client <------------> SOCKS


    Connection #2:

    SOCKS <-------------> Destination

The SOCKS server therefore manages sockets on both sides.

This is important when designing a C++ implementation.

---

# 35. Connection Closing

Suppose the client closes:

    Client
       |
       X
       |
    SOCKS
       |
       |
       v
    Destination

The SOCKS server must detect the closure and handle the destination-side connection appropriately.

Likewise, if the destination closes:

    Destination
       |
       X
       |
    SOCKS
       |
       v
    Client

A production proxy must correctly handle:

    EOF
    socket errors
    shutdown
    half-close
    timeouts

TCP half-close is an advanced topic worth understanding when implementing a robust proxy.

---

# 36. Failure Debugging Checklist

When SOCKS5 fails, ask these questions in order:

    1. Is the SOCKS server running?
    2. Is the SOCKS port listening?
    3. Can the client reach the SOCKS server?
    4. Did SOCKS5 negotiation succeed?
    5. Did authentication succeed?
    6. Was CONNECT accepted?
    7. Did DNS resolution succeed?
    8. Can SOCKS reach the destination?
    9. Is the destination port open?
    10. Did the destination refuse the connection?
    11. Did a firewall drop the connection?
    12. Did a timeout occur?

Always distinguish:

    Client -> SOCKS

from:

    SOCKS -> Destination

---

# 37. Architect's Mental Model

Think of SOCKS5 as:

    "A standardized way for an application to ask another
     machine to make a network connection to a destination
     and relay bytes between the application and destination."

Core architecture:

                     SOCKS5 PROXY

    +-------------+
    | Application |
    +------+------+
           |
           | SOCKS5 negotiation/data
           v
    +-------------------+
    | SOCKS5 Server     |
    |                   |
    | Authenticate      |
    | Authorize         |
    | Connect           |
    | Relay             |
    +---------+---------+
              |
              | TCP/UDP
              v
    +-------------------+
    | Destination       |
    |                   |
    | HTTPS / SSH /     |
    | Custom protocol   |
    +-------------------+

---

# 38. Most Important Things to Remember

1. SOCKS5 is a proxy protocol.

2. SOCKS5 is not a VPN.

3. SOCKS5 does not inherently encrypt application traffic.

4. Authentication is not encryption.

5. TCP CONNECT is the most common SOCKS5 operation.

6. SOCKS5 supports TCP and UDP.

7. SOCKS5 supports IPv4, IPv6 and domain names.

8. Domain names can be resolved by the client or proxy depending on how the client uses SOCKS.

9. For TCP proxying, think in terms of two network connections:

       Client <-> SOCKS
       SOCKS  <-> Destination

10. SOCKS5 itself generally does not need to understand HTTP, SSH, TLS, etc.

11. A SOCKS server can see connection metadata even when application data is protected by TLS.

12. Production SOCKS servers need authentication/authorization and protection against becoming open proxies.

---

# 39. Recommended Learning Path for a Networking Beginner

If networking is new, learn in this order:

    1. IP address
            |
            v
    2. Subnet and routing
            |
            v
    3. Port and socket
            |
            v
    4. TCP vs UDP
            |
            v
    5. TCP three-way handshake
            |
            v
    6. Client/server socket programming
            |
            v
    7. HTTP vs HTTPS
            |
            v
    8. Proxy concept
            |
            v
    9. SOCKS5 protocol
            |
            v
    10. Wireshark packet analysis
            |
            v
    11. SOCKS5 C++ architecture
            |
            v
    12. Multi-client SOCKS5 server
            |
            v
    13. Security and production architecture

---

# 40. Final Cheat Sheet

    SOCKS5
    |
    +-- Proxy protocol
    |
    +-- Application-level proxying
    |
    +-- TCP
    |     |
    |     +-- CONNECT
    |     +-- BIND
    |
    +-- UDP
    |     |
    |     +-- UDP ASSOCIATE
    |
    +-- IPv4
    +-- IPv6
    +-- Domain names
    +-- Authentication negotiation
    |
    +-- Does NOT inherently provide encryption
    |
    +-- Does NOT automatically proxy every application

Typical TCP sequence:

    TCP connect to SOCKS
            |
            v
    SOCKS5 greeting
            |
            v
    Authentication negotiation
            |
            v
    Authentication
            |
            v
    CONNECT destination
            |
            v
    SOCKS connects to destination
            |
            v
    SOCKS returns success/failure
            |
            v
    Bidirectional byte relay
            |
            v
    Close

The most important diagram:

                 Connection #1
    Client =======================> SOCKS
                                      |
                                      |
                                      | Connection #2
                                      v
                                  Destination

SOCKS5 coordinates these two sides and relays the traffic.
