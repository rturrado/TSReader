#ifndef __TS_EXCEPTION_H__
#define __TS_EXCEPTION_H__

#include "Packet.h"

#include <exception>
#include <filesystem>
#include <string>
#include <sstream>

namespace TS
{
    // TS file

    class TSFilePathNotFound : public std::exception
    {
    public:
        explicit TSFilePathNotFound(const std::filesystem::path& fp) { _message += fp.string(); }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "couldn't find TS file path: " };
    };

    class CouldNotOpenTSFile : public std::exception
    {
    public:
        explicit CouldNotOpenTSFile(const std::filesystem::path& fp) { _message += fp.string(); }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "couldn't open TS file: " };
    };



    // Command line parser

    struct CommandLineParserException : public std::exception
    {
        CommandLineParserException(const char* message) : std::exception{ message } {}
    };
    struct InvalidNumberOfArguments : public CommandLineParserException
    {
        InvalidNumberOfArguments() : CommandLineParserException{ "invalid number of arguments" } {}
    };
    struct UnrecognizedOption : public CommandLineParserException
    {
        UnrecognizedOption(const char* message) : CommandLineParserException{ message } {}
    };



    // Packet parser

    class InvalidSyncByte : public std::exception
    {
    public:
        InvalidSyncByte(const Packet& packet, size_t index)
        {
            _message += std::to_string(packet.header.sync_byte) + " in packet " + std::to_string(index);
        }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "invalid sync byte" };
    };

    class InvalidStuffingBytes : public std::exception
    {
    public:
        InvalidStuffingBytes(const Packet& packet, size_t index) { _message += " in packet " + std::to_string(index); }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "invalid stuffing bytes" };
    };



    // Packet buffer

    class PacketBufferOverrun : public std::exception
    {
    public:
        PacketBufferOverrun(uint8_t bytes_requested_to_read, uint8_t bytes_left_to_read)
        {
            std::ostringstream oss;
            oss << "trying to read " << static_cast<int16_t>(bytes_requested_to_read) << " bytes from buffer, "
                << "but there are only " << static_cast<int16_t>(bytes_left_to_read) << " bytes left to read";
            _message = oss.str();
        }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{};
    };
}

#endif
