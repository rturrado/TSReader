#ifndef __TS_EXCEPTION_HPP__
#define __TS_EXCEPTION_HPP__

#include "Packet.hpp"

#include <exception>
#include <filesystem>
#include <string>
#include <sstream>
#include <stdexcept>

namespace TS
{
    // TS file

    class TSFilePathNotFound : public std::exception
    {
    public:
        explicit TSFilePathNotFound(const std::filesystem::path& fp) { _message += fp.string(); }
        virtual const char* what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message{ "couldn't find TS file path: " };
    };

    class CouldNotOpenTSFile : public std::exception
    {
    public:
        explicit CouldNotOpenTSFile(const std::filesystem::path& fp) { _message += fp.string(); }
        virtual const char* what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message{ "couldn't open TS file: " };
    };



    // Command line parser

    struct CommandLineParserException : public std::runtime_error
    {
        explicit CommandLineParserException(const char* message) : std::runtime_error{ message } {}
    };
    struct InvalidNumberOfArguments : public CommandLineParserException
    {
        InvalidNumberOfArguments() : CommandLineParserException{ "invalid number of arguments" } {}
    };
    struct InvalidStreamType : public CommandLineParserException
    {
        explicit InvalidStreamType(const char* message) : CommandLineParserException{ std::string{_message + message}.c_str() } {}
        virtual const char* what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message{ "invalid stream type: " };
    };
    struct UnrecognizedOption : public CommandLineParserException
    {
        explicit UnrecognizedOption(const char* message) : CommandLineParserException{ message } {}
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
        virtual const char* what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message{};
    };



    // Packet parser

    struct PacketParserException : public std::runtime_error
    {
        explicit PacketParserException(const char* message) : std::runtime_error{ message } {}
    };
    struct InvalidPrivateBit : public PacketParserException
    {
        InvalidPrivateBit() : PacketParserException{ "invalid private bit" } {};
    };
    struct InvalidReservedBits : public PacketParserException
    {
        InvalidReservedBits() : PacketParserException{ "invalid reserved bits" } {};
    };
    struct InvalidSectionLength : public PacketParserException
    {
        InvalidSectionLength() : PacketParserException{ "invalid section length" } {};
    };
    struct InvalidSectionSyntaxIndicator : public PacketParserException
    {
        InvalidSectionSyntaxIndicator() : PacketParserException{ "invalid section syntax indicator" } {};
    };
    struct InvalidStuffingBytes : public PacketParserException
    {
        InvalidStuffingBytes() : PacketParserException{ "invalid stuffing bytes" } {};
    };
    struct InvalidSyncByte : public PacketParserException
    {
        InvalidSyncByte() : PacketParserException{ "invalid sync byte" } {};
    };
    struct InvalidUnusedBits : public PacketParserException
    {
        InvalidUnusedBits() : PacketParserException{ "invalid unused bits" } {};
    };
    struct TransportError : public PacketParserException
    {
        TransportError() : PacketParserException{ "transport error" } {};
    };
    struct UnknownPES : public PacketParserException
    {
        UnknownPES() : PacketParserException{ "unknown PES" } {};
    };



    // Packet processor

    struct InvalidCRC32 : public std::runtime_error
    {
        InvalidCRC32() : std::runtime_error{ "invalid CRC32" } {};
    };



    // PSI tables

    struct Duplicated_PES_PID : public std::runtime_error
    {
        Duplicated_PES_PID() : std::runtime_error{ "duplicated PES PID" } {};
    };
    struct Duplicated_PMT_PID : public std::runtime_error
    {
        Duplicated_PMT_PID() : std::runtime_error{ "duplicated PMT PID" } {};
    };
    struct Unknown_PMT_Program_Number : public std::runtime_error
    {
        Unknown_PMT_Program_Number() : std::runtime_error{ "unknown PMT program number" } {};
    };



    // Other

    class Unimplemented : public std::exception
    {
    public:
        explicit Unimplemented(const char* message) { _message += message; }
        virtual const char* what() const noexcept override { return _message.c_str(); }
    private:
        std::string _message{ "unimplemented: " };
    };
}

#endif
