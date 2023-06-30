# SAT16 Relays board control software

## Usage

### Funtion 0x38 (Multiple relays control)

Set relay 1 to ON and all others to OFF

```c
./sat16ctrl -comport:/dev/ttyUSB0 -address:0x20 -write:380100
```

Set all relays to ON

```c
./sat16ctrl -comport:/dev/ttyUSB0 -address:0x20 -write:38FFFF
```

### Funtion 0x34 (One relay control)

Switch on relay 1

```c
./sat16ctrl -comport:/dev/ttyUSB0 -address:0x20 -write:3401FF
```

Switch on relay 6

```c
./sat16ctrl -comport:/dev/ttyUSB0 -address:0x20 -write:3406FF
```

Switch off relay 1

```c
./sat16ctrl -comport:/dev/ttyUSB0 -address:0x20 -write:340100
```

Switch on relay 3 during 4 seconds

```c
./sat16ctrl -comport:/dev/ttyUSB0 -address:0x20 -write:340304
```

## SAT16 protocol

### Serial

8 bits, no parity, 1 stop bit, default bitrate : 4800 Bauds

### Frame

AA (Start of Frame) XX (Address) SS (Size) MM (Message) DDDD... (Data) CC CC (CRC) 04 (End of frame)

CRC : CRC-16/XMODEM (Poly 0x1021, Init 0x0000)

### Messages

01 - Polling - The SAT16 answers with Mess 60/70 without any data.
09 - Clear Memory - The SAT16 deactivate all the relays and answers with Mess 61
34 - One relay control - The SAT16 answers with Mess 61
38 - Multiple relays control - The SAT16 answers with Mess 61
4B - Send Status - The SAT16 answers with Mess 63
56 - Action execution - The SAT16 answers with Mess 57
73 - Send firmware version - The SAT16 answers with the firmware version and the checksum.
7F - Change Baud Rate - The SAT16 answers with Mess 61

### Full frames examples

AA 20 03 38 FF FF 85 49 04

AA 20 03 38 55 55 63 5C 04

AA 20 03 34 03 55 B2 24 04

(c) 2023 Jean-François DEL NERO
