# TSReader
MPEG Transport Stream reader.

I've attended recently C++Now 2021, which has been held online for the last two years.
Some of the sponsors of this event used a channel in the Discord server to post different job offers and one of them, Unified Streaming, suggested to write the program below before applying for a position.
Although I'm not looking for a job at the moment, I thought it would be a good way of doing some programming using modern C++.

## Description

Could you write a program that takes a [TS file](http://en.wikipedia.org/wiki/MPEG_transport_stream) and writes the audio and video as two separate files to disk (raw AAC and raw AVC1)?

The code should use:
- standard C++17.
- using only .hpp/.cpp and a Makefile.
- no platform specifics (like MFC/COM/ATL).
- the raw streams should be viewable/playable with for instance FFplay, as verification/test.
- example input: http://test.unified-streaming.com/elephants.ts
- We also would like to mention to not use ffmpeg/avconv or something other from say Github: we would like to see your work.
- We would like to suggest to do this as a Github project and write up your design/docs in the README.md Github provides.

## Design

- Command line parsing errors print the usage in standard output and exit.
- Runtime errors such as parsing errors print the error message in standard output and exit.
- `main` parses the command line, creates a `FileReader` to read the TS file, and proceeds to read it.
- `FileReader` opens the TS file and, for each TS packet, reads it into a `PacketBuffer` and ask `PacketParser` to parse it.
- `PacketParser` parses the TS packet header and, if present, the adaptation field and payload data. In order to do that, it reads byte chunks from `PacketBuffer`.
- `PacketBuffer` lets byte chunks to be read in big-endian, which is needed for the header.

## Implementation

- `Packet`, `Header`, `AdaptationField`, and `AdaptationExtension` are structs.
- Field masks and byte chunks are `boost::dynamic_bitset`. This way we can easily perform binary operations.

## DONE

- [X] Read elephants.ts and print out a summary of the TS packet PIDs (PID hex code and count).

## TODO

- [ ] Distinguish audio and video packets.
- [ ] Get audio or video information from the packet.
- [ ] Write audio and video to output files.
    It would be good to do this in a generic way, e.g., as a stage where different actions could be performed on the parsed TS packet.
    Read > Parse > (Write audio packet to audio file or video packet to video file _and/or_ Do some statistics... )

## Usage

### Windows

`ts_reader.exe <TS FILE PATH>`, where `<TS FILE PATH>` is the path to the TS file. It can be absolute or relative to the `ts_reader.exe` location.
