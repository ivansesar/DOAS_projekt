#include <cstdint>
#include <string>

#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BMPInfoHeader {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)


typedef unsigned char Byte;
typedef Byte Bytef;

struct BMP_data {
    uint8_t* pixel_data_no_padding;
    Bytef * B_channel;
    Bytef * G_channel;
    Bytef * R_channel;
    int width;
    int height;
    int bytes_per_pixel;
};

class BMPReader {
public:
    static struct BMP_data read_bmp(std::string& filename);
};
