#include <vector>
#include <string>
typedef unsigned char Byte;
typedef Byte Bytef;

enum BMP_TYPE {
    BMP_TEXT, BMP_NATURE
};

class PNGWriter {
public:
    static void write_png(int width, int height, Bytef* data, long data_length, BMP_TYPE file_type, std::string filename);
};
