#ifndef __TS_EXCEPTION_H__
#define __TS_EXCEPTION_H__

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
        InvalidSyncByte(uint8_t sync_byte) { _message += sync_byte; }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "invalid sync byte: " };
    };



    // Packet buffer

    class PacketBufferOverrun : public std::exception
    {
    public:
        PacketBufferOverrun(uint8_t bytes_requested_to_read, uint8_t bytes_left_to_read)
        {
            std::ostringstream oss;
            oss << "trying to read " << bytes_requested_to_read << " bytes from buffer, "
                << "but there are only " << bytes_left_to_read << " bytes left to read";
            _message = oss.str();
        }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{};
    };
}

#endif
