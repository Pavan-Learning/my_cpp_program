# Operator Call - Quick Understanding Guide

## Purpose

Operator Call allows a vehicle user to establish a voice communication channel with a remote operator when:

- Call Button is pressed
- SOS Button is pressed

The system performs two major activities:

1. Call Establishment (Control Flow)
2. Voice Communication (Audio Flow)

---

# High-Level Architecture

```text
User
 ↓
Call Button / SOS Button
 ↓
Vehicle Event
 ↓
LUM Infrastructure
 ↓
Operator Call (QNX VM)
 ↓
Domain Separation
 ↓
Data Manager
 ↓
ComUnit / Modem
 ↓
Mobile Network
 ↓
Backend
 ↓
Operator Console
```

---

# 1. Control Flow (Call Setup)

The control flow is responsible for establishing the call.

```text
Call Button / SOS Button
        ↓
Event Generated
        ↓
LUM Client (QNX)
        ↓
Operator Call Service
        ↓
Domain Separation
        ↓
Data Manager
        ↓
ComUnit (Modem)
        ↓
Mobile Network
        ↓
Backend
        ↓
Operator Available
        ↓
Voice Session Established
```

### Note

Current architecture clearly shows:

```text
Operator Call
 ↕
LUM Client QNX
 ↕
MPCI LUM
```

However, it is NOT confirmed whether VDCM is the actual publisher of the SOS/Call event.

This requires verification from:

- OperatorCall source code
- LUM subscriptions
- Project documentation

---

# 2. Voice Communication Flow

Once the call is established, audio communication starts.

```text
Vehicle Microphone
        ↓
PCM Audio
        ↓
Opus Encoder
        ↓
Network Packets
        ↓
Backend
        ↓
Operator

Operator Voice
        ↓
Opus Encoder
        ↓
Network
        ↓
Vehicle
        ↓
Opus Decoder
        ↓
Vehicle Speaker
```

---

# Audio Concepts

## PCM (Pulse Code Modulation)

PCM is raw digital audio.

Example:

```text
1200
1210
1225
1190
1185
...
```

These values represent the captured sound wave.

---

## Sample Rate

Number of audio samples captured per second.

Example:

```text
8000 Hz
16000 Hz
48000 Hz
```

Operator Call systems typically use:

```text
48000 Hz (48 kHz)
```

Meaning:

```text
48000 samples/sec
```

---

## Audio Frame

Opus processes audio in chunks called frames.

Common frame size:

```text
20 ms
```

Calculation:

```text
48000 × 20 ms

48000 × 0.02

= 960 samples
```

Therefore:

```text
20 ms Frame
=
960 PCM Samples
```

---

# PCM Frame Creation

Microphone continuously produces PCM samples.

```text
PCM Stream:

1
2
3
...
960
```

After collecting 960 samples:

```text
Frame #1
```

Next 960 samples:

```text
Frame #2
```

And so on.

Visualization:

```text
PCM Stream

|----960 Samples----|
      Frame 1

|----960 Samples----|
      Frame 2

|----960 Samples----|
      Frame 3
```

---

# Opus Codec

## Why Opus?

Raw PCM audio is large.

Opus compresses audio before transmission.

```text
PCM Audio
    ↓
Opus Encoder
    ↓
Compressed Audio Packet
```

Benefits:

- Low latency
- Good voice quality
- Lower bandwidth usage
- Optimized for voice communication

---

# Opus Encoding Example

Input:

```text
960 PCM Samples
```

Example size:

```text
1920 Bytes
```

Encoding:

```text
PCM Frame
    ↓
Opus Encoder
```

Output:

```text
100-200 Bytes
```

Opus packet is sent over network.

---

# Opus Decoding Example

Receiving side:

```text
Opus Packet
      ↓
Opus Decoder
      ↓
PCM Samples
```

The reconstructed PCM samples are sent to speaker output.

---

# WAV vs Opus

Important:

```text
Opus ≠ Encryption
```

Opus is an audio compression codec.

Relationship:

```text
PCM
 ↓
Opus Encoder
 ↓
.opus
```

Reverse:

```text
.opus
 ↓
Opus Decoder
 ↓
PCM
 ↓
.wav
```

WAV is typically used for:

- Debugging
- Playback
- Audio analysis
- Recording review

Real-time communication uses Opus packets, not WAV files.

---

# End-to-End Flow

```text
SOS / Call Button
        ↓
Event Triggered
        ↓
LUM Infrastructure
        ↓
Operator Call Application
        ↓
Backend Connection Established
        ↓
Microphone Captures Audio
        ↓
PCM Samples Generated
        ↓
960 Samples Collected
        ↓
20 ms Audio Frame Created
        ↓
Opus Encoding
        ↓
Network Transmission
        ↓
Backend
        ↓
Operator

Reverse Path

Operator Voice
        ↓
Opus Encoding
        ↓
Network
        ↓
Vehicle
        ↓
Opus Decoding
        ↓
PCM Audio
        ↓
Vehicle Speaker
```

---

# Key Takeaways

- SOS/Call button initiates call setup.
- Operator Call application manages the call session.
- LUM infrastructure is involved in communication between services.
- PCM is raw audio data.
- 48 kHz audio generates 48,000 samples per second.
- 20 ms frame at 48 kHz = 960 samples.
- Opus compresses PCM audio for transmission.
- Opus Decoder reconstructs PCM audio.
- WAV is mainly used for storage/debugging, not live communication.
- Control Flow establishes the call.
- Voice Flow carries the conversation.