# TSReader
MPEG Transport Stream reader.

I've attended recently C\+\+Now 2021, which has been held online for the last two years.
Some of the sponsors of this event used a channel in the Discord server to post different job offers and one of them, Unified Streaming, suggested to write the program below before applying for a position.
Although I'm not looking for a job at the moment, I thought it would be a good way of doing some programming using modern C\+\+.

## Description

Could you write a program that takes a [TS file](http://en.wikipedia.org/wiki/MPEG_transport_stream) and writes the audio and video as two separate files to disk (raw AAC and raw AVC1)?

The code should use:
- standard C\+\+17.
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
- `FileReader` opens the TS file and, for each TS packet:
  - reads it into a `PacketBuffer`,
  - asks `PacketParser` to parse it,
  - asks `PacketProcessor` to do some processing on the parsed packet, and, finally,
  - performs some post processing: e.g. collect stats or write some streams out to file.
- `PacketParser` parses the TS packet header and, if present, the adaptation field and payload data.<br/>
    In order to do that, it reads byte chunks from `PacketBuffer`.<br/>
    It also does some error checking: stuffing bytes, reserved and unused bits, CRC32...
- `PacketBuffer` lets byte chunks to be read in big-endian, which is needed for all the TS headers.
- `PacketProcessor` basically:
  - builds the PSI tables (PAT and PMT) for packets containing PSI information, and
  - saves the PES data for packets containing PES payloads.

## Implementation

- `Packet`, `Header`, `AdaptationField`, `AdaptationExtension` and so on are structs.
- Field masks and byte chunks are `boost::dynamic_bitset`s. This way we can easily perform binary operations.

## DONE

- [X] Read elephants.ts and print out a summary of the TS packet PIDs (PID hex code, count, stream type, and stream description).
- [X] Parse adaptation field, adaptation extension, and payload data.<br/>
    Parsing of PSI (Program Specific Information) lets us know the PIDs for audio and video packets.
- [X] Write audio and video to output files.<br/>
    This is done as a post packet processing stage, together with the gathering of stats.<br/>
    These actions are optional, and requested by the user through command line parameters.
- [X] Revisit the PacketParser implementation: code more clear and elegant.
  - [X] Encapsulate basic repetitive operations (e.g. read from buffer and create bitset)
    - [ ] Implement the special case where the type is uint8_t?
  - [X] Simplify complex functions.
  - [ ] Read a whole section (a struct) with a single variadic templated function?
- [X] Revisit the PacketBuffer implementation: performance.
  - [X] Minimize the copy of vectors of bytes.
  - [X] Reimplement read operation as a get view returning a span.
- [X] CMake: add a CMakeLists.txt to compile the program in Windows and Linux.

## TODO

- [ ] Test with other TS files than elephants.ts.
- [ ] New branch: reimplement using modules.<br/>
    This would be especially useful for Packet.h, since it is linked from many compilation units.
- [ ] New branch: reimplement the FileReader as a pipeline of stages (reading, parsing, processing, printing stats...)
- [ ] Add tests?

## Requirements

- CMake 3.20.4.
- Boost libraries 1.75.0.
  - Boost program_options static multithreaded library (libboost_program_options-vcXXX-mt-[gd-][x32|x64]-1_75.lib)

## Compilation

### Windows

#### Visual Studio 2019

- Go to the project folder, open the `ts_reader.sln` with Visual Studio 2019, and build the solution.

#### CMake

- Open `CMakeLists.txt` and edit `BOOST_ROOT` and `BOOST_LIBRARY_DIR` variables to point to your Boost root and Boost library folders, respectively.
- Open a shell as administrator, go to the project folder, and execute: `cmake -S . -B build`
- Open the `build\ts_reader.sln` with Visual Studio 2019, and build the solution.

### Linux

- Go to the project folder, and execute: `cmake -S . -B build && cd build && make`

## Usage

`ts_reader <TS FILE PATH> [-e|--extract <STREAM TYPE LIST>] [-s|--stats]`, where:<br/>
- `<TS FILE PATH>` is the path to the TS file. It can be absolute or relative to the `ts_reader.exe` location, and
- `<STREAM TYPE LIST>` is a comma separated list of stream types.<br/>
    It supports hexadecimal, decimal or octal notation (e.g. 0xf, 15 or 017 for audio).
