# MPEG Transport Stream reader

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

## Requirements

- CMake 3.20.4.
- Boost libraries 1.75.0.
  - Boost program_options static multithreaded library (libboost_program_options-vcXXX-mt-[gd-][x32|x64]-1_75.lib)

## Compilation

### Visual Studio (Windows)

- Open `ts_reader.sln` with Visual Studio.
- Right click on the `ts_reader` project, and go to `Properties`.
  - Update the Boost root path in `Configuration Properties > C/C++ > General > Additional Include Directories` to point to your Boost root path.
  - Update the Boost library folder in `Configuration Properties > Linker > General > Additional Library Directories`.
  - Save these changes and exit the project properties.
- Right click on the `ts_reader` solution, and `Build Solution`.

### CMake (Windows/Linux)

- Open `CMakeLists.txt` and edit `BOOST_ROOT` and `BOOST_LIBRARY_DIR` variables to point to your Boost root and Boost library folders, respectively.
- From a shell, go to the project folder, and execute:

```
~/projects/ts_reader> cmake -S . -B build
~/projects/ts_reader> cmake --build build
```

## Usage

`ts_reader <TS FILE PATH> [-e|--extract <STREAM TYPE LIST>] [-s|--stats]`, where:<br/>
- `<TS FILE PATH>` is the path to the TS file. It can be absolute or relative to the `ts_reader.exe` location, and
- `<STREAM TYPE LIST>` is a comma separated list of stream types.<br/>
    It supports hexadecimal, decimal or octal notation (e.g. 0xf, 15 or 017 for audio).

As an example, you can try with the provided sample:

```
~/projects/ts_reader/build> ./ts_reader ../samples/elephants.ts -e 0xf,0x1b
~/projects/ts_reader/build> ./ts_reader ../samples/elephants.ts --stats
```
