#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ts_reader_test <TS FILE PATH>\n";
        std::cerr << "\n";
        std::cerr << "  E.g: ts_reader_test elephants.ts\n";
        exit(EXIT_FAILURE);
    }
    if (!std::filesystem::exists(argv[1]))
    {
        std::cerr << "Error: couldn't find TS file path: '" << argv[1] << "'\n";
        exit(EXIT_FAILURE);
    }

    // Write bitset to ofstream
    {
        auto start = std::chrono::high_resolution_clock::now();

        boost::dynamic_bitset<> bv(10'000'000);  // bits
        bv[0] = 1;   bv[1] = 1;  bv[4] = 1;  bv[7] = 1;  bv[9] = 1;
        
        std::vector<boost::dynamic_bitset<>::block_type> v(bv.num_blocks());  // dwords
        to_block_range(bv, v.begin());  // write bitset to vector

        std::filesystem::path ts_file_path{ "my_bitset.dat" };
        std::ofstream ofs(ts_file_path, std::ios::out | std::ios::binary);
        ofs.write(reinterpret_cast<char*>(&v[0]), v.size() * sizeof(boost::dynamic_bitset<>::block_type));  // write vector to ifstream

        size_t file_size = static_cast<size_t>(std::filesystem::file_size(ts_file_path));

        auto end = std::chrono::high_resolution_clock::now();
        using fms = std::chrono::duration<long double, std::chrono::milliseconds::period>;
        auto interval = fms(end - start);

        std::cout << "Writing " << file_size << " bytes"
            << " in " << bv.num_blocks() << " blocks"
            << " of " << sizeof(boost::dynamic_bitset<>::block_type) << " bytes: "
            << std::fixed << fms(interval).count() << " ms\n";
    }

    // Read ifstream to bitset (dword chunks)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::filesystem::path ts_file_path{ argv[1] };
        std::ifstream ifs{ ts_file_path, std::fstream::binary };
        size_t file_size = static_cast<size_t>(std::filesystem::file_size(ts_file_path));
        size_t num_blocks = file_size / sizeof(boost::dynamic_bitset<>::block_type);

        std::vector<boost::dynamic_bitset<>::block_type> v(num_blocks);  // dwords
        ifs.read(reinterpret_cast<char*>(&v[0]), num_blocks * sizeof(boost::dynamic_bitset<>::block_type));  // read ifstream to vector

        boost::dynamic_bitset<> bv(num_blocks * sizeof(boost::dynamic_bitset<>::block_type) * 8);  // bits
        from_block_range(v.cbegin(), v.cend(), bv);  // read vector to bitset

        auto end = std::chrono::high_resolution_clock::now();
        using fms = std::chrono::duration<long double, std::chrono::milliseconds::period>;
        auto interval = fms(end - start);

        std::cout << "Reading " << file_size << " bytes"
            << " in " << num_blocks << " blocks"
            << " of " << sizeof(boost::dynamic_bitset<>::block_type) << " bytes: "
            << std::fixed << fms(interval).count() << " ms\n";
    }

    // Read ifstream to bitset (byte chunks)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::filesystem::path ts_file_path{ argv[1] };
        std::ifstream ifs{ ts_file_path, std::fstream::binary };
        size_t file_size = static_cast<size_t>(std::filesystem::file_size(ts_file_path));
        size_t num_blocks = file_size;

        std::vector<uint8_t> v_byte(num_blocks);  // bytes
        ifs.read(reinterpret_cast<char*>(&v_byte[0]), num_blocks * sizeof(boost::dynamic_bitset<uint8_t>::block_type));  // read ifstream to vector

        boost::dynamic_bitset<uint8_t> bv_byte(num_blocks * 8);  // bits
        from_block_range(v_byte.cbegin(), v_byte.cend(), bv_byte);  // read vector to bitset

        auto end = std::chrono::high_resolution_clock::now();
        using fms = std::chrono::duration<long double, std::chrono::milliseconds::period>;
        auto interval = fms(end - start);

        std::cout << "Reading " << file_size << " bytes"
            << " in " << num_blocks << " blocks"
            << " of " << sizeof(boost::dynamic_bitset<uint8_t>::block_type) << " bytes: "
            << std::fixed << fms(interval).count() << " ms\n";
    }

    // Read ifstream to bitset (byte chunks)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::filesystem::path ts_file_path{ argv[1] };
        std::ifstream ifs{ ts_file_path, std::fstream::binary };
        size_t file_size = static_cast<size_t>(std::filesystem::file_size(ts_file_path));
        size_t file_chunk_size = 1024 * 1024; // 1 MB
        size_t num_blocks = file_chunk_size;

        size_t num_chunks{ 0 };
        while (!ifs.eof())
        {
            std::vector<uint8_t> v(num_blocks);  // bytes
            ifs.read(reinterpret_cast<char*>(&v[0]), num_blocks * sizeof(boost::dynamic_bitset<uint8_t>::block_type));  // read ifstream to vector
            size_t bytes_read{ static_cast<size_t>(ifs.gcount()) };
            boost::dynamic_bitset<uint8_t> bv(bytes_read * 8);  // bits
            from_block_range(v.cbegin(), v.cbegin() + bytes_read, bv);  // read vector to bitset
            num_chunks++;
        }

        auto end = std::chrono::high_resolution_clock::now();
        using fms = std::chrono::duration<long double, std::chrono::milliseconds::period>;
        auto interval = fms(end - start);

        std::cout << "Reading " << file_size << " bytes"
            << " in " << num_chunks << " chunks"
            << " of " << file_chunk_size << " bytes, each divided"
            << " in " << num_blocks << " blocks"
            << " of " << sizeof(boost::dynamic_bitset<uint8_t>::block_type) << " bytes: "
            << std::fixed << fms(interval).count() << " ms\n";
    }
}
