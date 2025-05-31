#include "BMPReader.h"

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <giomm/file.h>

using namespace std;

void flip_channel(Byte * channel, int width, int height) {
    int row_size = width;
    for (int y = 0; y < height / 2; ++y) {
        int top_row = y * row_size;
        int bottom_row = (height - 1 - y) * row_size;
        for (int x = 0; x < row_size; ++x) {
            std::swap(channel[top_row + x], channel[bottom_row + x]);
        }
    }
}

    void read_bmp_data(Byte* bmp_data, Bytef* B_channel,Bytef* G_channel,Bytef* R_channel,int width, int height) {
        //izvadi iz bmp_izlaz BGR bajtove/vrijednsoti_pixela u for petlji

        int bajtova_po_retku = width*3;
        for (int i = 0; i < height; i++) {
            int index = height-1-i;
            for (int j = 0; j < bajtova_po_retku; j++) {
                switch (j%3) {
                    case 0:
                        B_channel[i*width + j/3] = bmp_data[width*3*index + j];
                        break;
                    case 1:
                        G_channel[i*width + j/3] = bmp_data[width*3*index + j];
                        break;
                    case 2:
                        R_channel[i*width + j/3] = bmp_data[width*3*index + j];
                        break;
                    default:
                        break;
                }
            }
        }
        // flip rows; ALREADY FLIPPED WHILE READING
        // flip_channel(B_channel, width, height);
        // flip_channel(G_channel, width, height);
        // flip_channel(R_channel, width, height);

    }

    struct BMP_data BMPReader::read_bmp(std::string &filename) {
        std::ifstream file(filename, std::ios::binary);
        //FILE* file = fopen(filename.c_str(), "rb");
        BMPFileHeader fileHeader{};
        BMPInfoHeader infoHeader{};

        // Read headers
        // fread(&fileHeader, sizeof(fileHeader), 1, file);
        // fread(&infoHeader, sizeof(infoHeader), 1, file);
        file.read(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader));
        file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));

        if (fileHeader.bfType != 0x4D42) {
            std::cerr << "Its not valid BMP file.\n";
        }
        int width = infoHeader.biWidth;
        int height = std::abs(infoHeader.biHeight);
        int bits_per_pixel = infoHeader.biBitCount;
        int bytes_per_pixel = bits_per_pixel / 8;
        //cout << "Bytes per pixel: " << bytes_per_pixel << endl;

        int row_stride = ((width * bytes_per_pixel + 3) / 4) * 4;
        int row_data_size = width * bytes_per_pixel;
        //cout << "Row Stride " << row_stride << endl;
        int total_bytes = row_stride*height;
        //cout << "Total bytes of image = " << total_bytes << endl;
        //cout << "bf off bits " << fileHeader.bfOffBits << endl;

        uint8_t* pixel_data_no_padding = (uint8_t*) malloc(width * height * bytes_per_pixel * sizeof(uint8_t));
        uint8_t* row_buffer = (uint8_t*) malloc(row_stride*sizeof(uint8_t));
        //fseek(file, fileHeader.bfOffBits, SEEK_SET); // move to pixel data
        file.seekg(fileHeader.bfOffBits, std::ios::beg);
        for (int y = 0; y < height; y++) {
            //fread(row_buffer, row_stride,1 ,file);
            file.read(reinterpret_cast<char*>(row_buffer), row_stride);
            static_cast<uint8_t *>(row_buffer);
            std::memcpy(pixel_data_no_padding + y*row_data_size, row_buffer, row_data_size); // swaped row_data_size and row_stride

        }

        // int index = 0;
        // for (int i = 0; i < width*height*(bytes_per_pixel); i++) {
        //     index++;
        //     //uint8_t pixel_value = pixel_data_no_padding[i];
        // }
        //cout << "Number of px " << index << endl;


        //******************* PARSED BMP IS IN pixel_data **********************//
        // kanali nad kojima će se obavljati operacije filtriranja
        //cout << "width x height " << width << " " << height << endl;

        Bytef *B_channel = (Bytef *) malloc(width*height * sizeof(Bytef));
        Bytef *G_channel = (Bytef *) malloc(width*height * sizeof(Bytef));
        Bytef *R_channel = (Bytef *) malloc(width*height * sizeof(Bytef));

        // pročitaj bmp_data u kanale
        read_bmp_data(pixel_data_no_padding, &B_channel[0], &G_channel[0], &R_channel[0], width, height);

        struct BMP_data bmp_data = {
            .pixel_data_no_padding = pixel_data_no_padding,
            .B_channel = B_channel,
            .G_channel = G_channel,
            .R_channel = R_channel,
            .width = width,
            .height = height,
            .bytes_per_pixel = bytes_per_pixel,
        };

        free(row_buffer);
        //fclose(file);
        file.close();
        return bmp_data;
    }

