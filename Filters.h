typedef unsigned char Byte;
typedef Byte Bytef;

class Filters {
public:
    static Bytef *none_filter(const Bytef *B_channel_line, const Bytef *G_channel_line, const Bytef *R_channel_line,int width);
    static Bytef *sub_filter(Bytef *B_channel_line, Bytef *G_channel_line, Bytef *R_channel_line, int width, int height);
    static Bytef *up_filter(Bytef *B_channel, Bytef *G_channel, Bytef *R_channel, int width, int height, int current_row);
    static Bytef *avg_filter(Bytef * B_channel, Bytef * G_channel, Bytef * R_channel, int width, int height, int current_row);
    static Bytef *paeth_filter(Bytef * B_channel, Bytef * G_channel, Bytef * R_channel, int width, int height, int current_row);
    static Bytef minimum(Bytef vl, Bytef vu, Bytef vul);

    static void remap_lines(Bytef * none_line, Bytef * sub_line, Bytef * up_line, Bytef * avg_line, Bytef * paeth_line, int width);

};
