#ifndef __TS_EXCEPTION_H__
#define __TS_EXCEPTION_H__

#include <exception>
#include <filesystem>
#include <string>
#include <sstream>

namespace TS
{
    class CouldNotOpenTSFile : public std::exception
    {
    public:
        explicit CouldNotOpenTSFile(std::filesystem::path fp) { _message += fp.string(); _message += "\n"; }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "Error: couldn't open TS file: " };
    };



    class InvalidNumberOfArguments : public std::exception
    {
    public:
        InvalidNumberOfArguments() : std::exception{ "Error: invalid number of arguments\n" } {}
    };



    class InvalidSyncByte : public std::exception
    {
    public:
        InvalidSyncByte(uint8_t sync_byte) { _message += sync_byte; _message += "\n"; }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "Error: invalid sync byte: " };
    };



    class PacketBufferOverrun : public std::exception
    {
    public:
        PacketBufferOverrun(uint8_t bytes_requested_to_read, uint8_t bytes_left_to_read)
        {
            std::ostringstream oss;
            oss << "Error:" 
                << " trying to read " << bytes_requested_to_read << " bytes from buffer,"
                << " but there are only " << bytes_left_to_read << " bytes left to read\n";
            _message = oss.str();
        }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{};
    };



    class TSFilePathNotFound : public std::exception
    {
    public:
        explicit TSFilePathNotFound(const char* fp) { _message += fp; _message += "\n"; }
        virtual const char* what() const override { return _message.c_str(); }
    private:
        std::string _message{ "Error: couldn't find TS file path: " };
    };
}

#endif
