#include "PNGWriter.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include "zlib.h"
typedef unsigned char Byte;
typedef Byte Bytef;

// PNG Header
const uint8_t PNG_HEADER[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

// Chunk Types
const char IHDR[] = "IHDR";
const char IDAT[] = "IDAT";
const char IEND[] = "IEND";

void write_uint32_be(std::ofstream& out_stream, uint32_t value) {
    uint8_t bytes[4] = {
        static_cast<uint8_t>((value >> 24) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    out_stream.write(reinterpret_cast<char*>(bytes), 4);
}

// Compute CRC for a chunk
uint32_t compute_crc(const char* type, const uint8_t* data, unsigned long length) {
    uint32_t crc = crc32(0, reinterpret_cast<const Bytef*>(type), 4);
    return crc32(crc, data, length);
}

void write_chunk(std::ofstream& file, const char* chunk_type, const uint8_t* data, unsigned long length) {
    write_uint32_be(file, length);
    file.write(chunk_type, 4);
    if (length > 0 && data != nullptr)
        file.write(reinterpret_cast<const char*>(data), length);
    uint32_t crc = compute_crc(chunk_type, data, length);
    write_uint32_be(file, crc);
}

void PNGWriter::write_png(int width, int height, Bytef* data, long data_length,  BMP_TYPE file_type, std::string filename) {

    std::filesystem::path p(filename.c_str());
    filename = p.filename().string();
    filename = filename.substr(0, filename.length() - 4);
    //std::cout << "Filename is " << filename << std::endl;
    std::string output_directory = file_type == BMP_TEXT ? std::string("./text/") + filename + std::string(".png") : std::string("./nature/") + filename + std::string(".png");

    std::ofstream png_file(output_directory, std::ios::binary);
    if (!png_file) {
        std::cerr << "Could not create PNG file." << std::endl;
        return;
    }

    // Write PNG header
    png_file.write(reinterpret_cast<const char*>(PNG_HEADER), 8);

    // Create the IHDR chunk
    std::vector<uint8_t> ihdr_data(13);
    ihdr_data[0] = (width >> 24) & 0xFF;
    ihdr_data[1] = (width >> 16) & 0xFF;
    ihdr_data[2] = (width >> 8) & 0xFF;
    ihdr_data[3] = width & 0xFF;
    ihdr_data[4] = (height >> 24) & 0xFF;
    ihdr_data[5] = (height >> 16) & 0xFF;
    ihdr_data[6] = (height >> 8) & 0xFF;
    ihdr_data[7] = height & 0xFF;
    ihdr_data[8] = 8;  // bits per px
    ihdr_data[9] = 2; // For RGB
    ihdr_data[10] = 0; // Compression method (0 for deflate)
    ihdr_data[11] = 0; // Filter method (0)
    ihdr_data[12] = 0; // Interlace method (0)

    // Write IHDR chunk
    write_chunk(png_file, IHDR, ihdr_data.data(), ihdr_data.size());


    write_chunk(png_file, IDAT, data, data_length);  // Write the compressed data chunk


    write_chunk(png_file, IEND, nullptr, 0);
    png_file.close();
}

