#include <cstdint>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <zlib.h>
#include "Filters.h"
#include "Heuristics.h"
#include "CompressionResult.h"
#include <map>

using namespace std;


void read_bmp_data(Byte* bmp_data, Bytef* B_channel,Bytef* G_channel,Bytef* R_channel,int width, int height) {
    // izvadi iz bmp_izlaz BGR bajtove/vrijednsoti_pixela u for petlji
    int bajtova_po_retku = width * 3;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < bajtova_po_retku; j++) {
            switch (j%3) {
                case 0:
                    B_channel[i*width + j/3] = bmp_data[width*3*i + j];
                    break;
                case 1:
                    G_channel[i*width + j/3] = bmp_data[width*3*i + j];
                    break;
                case 2:
                    R_channel[i*width + j/3] = bmp_data[width*3*i + j];
                    break;
                default:
                    break;
            }
        }
    }
}

/**
 *
 * @param lines_to_deflate je data ulaz u deflate fju, u njena upisujemo sve filtrirane linije
 * @param best_line je filtrirana linija
 * @param current_row govori na kojem smo retku tj. gdje trebamo upisivati u lines_to_deflate
 */
void make_deflate_lines(Bytef* lines_to_deflate, BestLine best_line, int width, int current_row) {
    // create input for deflate from bestLine ===> filterType + RGB ordered 1px 2px .... n.th px
    // best_line ide kao B filtered channel pa G filtered channel pa R filtered channel ==> treba prebaciti u ^^ oblik

    const Bytef *filtered_line = best_line.line;
    lines_to_deflate[current_row * (width*3+1)] = best_line.type;
    int begin = current_row * (width*3+1);
    for (int j = 1, k = 0; k < width; j++, k++) {
        lines_to_deflate[begin + k*width+1] = filtered_line[2*width + k];
        lines_to_deflate[begin + k*width+2] = filtered_line[1*width + k];
        lines_to_deflate[begin + k*width+3] = filtered_line[k];
    }
}

/**
 * Metoda koja obavlja kompresiju nad 1 slikom
 * Metoda definira parametre kompresije i izvodi svaku kombinaciju i ispisuje rezultat
 */
void make_compressions(int width, int height, Byte * lines_to_deflate) {
    std::map<string, int> compression_level = {
        {"Z_BEST_COMPRESSION", Z_BEST_COMPRESSION},
        {"Z_BEST_SPEED", Z_BEST_SPEED},
        {"Z_DEFAULT_COMPRESSION", Z_DEFAULT_COMPRESSION}
    };
    std::map<string, int> compression_method = {
        {"Z_DEFLATED", Z_DEFLATED}
    };
    std::map<string, int> compression_window_size = {
        {"9", 9},{"12", 12},{"15", 15}
    };
    std::map<string, int> compression_mem_level = {
        {"1", 1},{"6", 6 },{"9", 9}
    };
    std::map<string, int> compression_strategy = {
        {"Z_DEFAULT_STRATEGY", Z_DEFAULT_STRATEGY},
        {"Z_FILTERED", Z_FILTERED},
        {"Z_HUFFMAN_ONLY", Z_HUFFMAN_ONLY},
        {"Z_RLE", Z_RLE},
        {"Z_FIXED", Z_FIXED}};

    // REZULTANTNA MAPA
    std::map<string, CompressionResult> results;

    // za svaki kompresijksi level odradi deflate kao tablicu compression_mem_level X compression_strategy
    for (const auto& level : compression_level) {
        for (const auto& window_size : compression_window_size) {
            for (const auto& mem_level : compression_mem_level) {
                for (const auto& strategy : compression_strategy) {

                    // kreiranje strukture za deflate funkciju
                    Bytef compressed[height * (width*3+1)]; // ne znam koliki treba biti buffer za komprimiranje
                    z_stream strm = {nullptr};
                    strm.next_in = lines_to_deflate;
                    strm.avail_in = height*(width*3+1);
                    strm.next_out = compressed;
                    strm.avail_out = sizeof(compressed);

                    // izvođenje deflatea
                    deflateInit2(&strm, level.second, Z_DEFLATED, window_size.second , mem_level.second, strategy.second);
                    deflate(&strm, Z_FINISH);

                    size_t compressed_length = strm.total_out;
                    //printf("Compressed length = %llu\n", compressed_length);
                    // for (unsigned long long i = 0; i < compressed_length; i++) {
                    //     printf("%d ", compressed[i]);
                    // }
                    //printf("\n");

                    deflateEnd(&strm);

                    // izvođenje inflatea
                    Bytef decompressed[height * (width*3+1)];
                    z_stream strm2 = {nullptr};
                    strm2.next_in = compressed;
                    strm2.avail_in = sizeof(compressed);
                    strm2.next_out = decompressed;
                    strm2.avail_out = sizeof(decompressed);

                    inflateInit(&strm2);
                    inflate(&strm2,Z_FINISH);

                    size_t decompressed_length = sizeof(decompressed);
                    // printf("Decompressed length = %llu\n", decompressed_length);
                    // for (Bytef b : decompressed) {
                    //     printf("%d ", b);
                    // }
                    // printf("\n");

                    inflateEnd(&strm2);
                }
            }
        }
    }
}

/**
 * Glavna metoda
 * Na početku iz podataka(polja Bytef) iz BMP-a kreira RGB channele za primjenu filtriranja
 *
 * Za svaki redak slike poziva svaki filtar te pretprocesira scanline prije heuristike i samom
 * heuristikom odredi najbolju liniju
 *
 * Nakon heuristike linija je oblika BBB...GGG...RRR, a za deflate treba biti oblika filter_type,rgb,rgb,rgb...
 * koju spremi u varijablu "deflate_input_line"
 */
int main() {
    // image width and height in px
    const int width = 3;
    const int height = 3;

    // slika s 6 plavih pixela
    //Bytef bmp_data[] = {0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,0xff, 0x00, 0x00};

    // slika s pixel vrijenostima 1 2 3 4 5 6 7 8 9
    Bytef bmp_data[] = {0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08,0x09, 0x09, 0x09};

    // PARSIRAJ BMP SLIKU U bmp_data slijedno u polje ide blue green red, blue green, red...


    // kanali nad kojima će se obavljati operacije filtriranja
    Bytef *B_channel = (Bytef *) malloc(width * height * sizeof(Bytef));
    Bytef *G_channel = (Bytef *) malloc(width * height * sizeof(Bytef));
    Bytef *R_channel = (Bytef *) malloc(width * height * sizeof(Bytef));
    // pročitaj bmp_data u kanale
    read_bmp_data(bmp_data, B_channel, G_channel, R_channel, width, height);

    // Imamo B G R channele i ide filtriranje
    // za svaki redak izvrši filtriranje i primjeni heuristiku i spremi u oblik za deflate
    //
    // izlazni retci koji su ulaz za deflate
    // oblika za svaki redak [filter_type] [RGB 1.px] [RGB 2.px] ... [RGB zadnji px]
    Bytef* lines_to_deflate = (Bytef *) malloc(height * (width * 3 + 1) * sizeof(Bytef));
    for (int i = 0; i < height; i++) {
        Bytef *none_line = Filters::none_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width);
        Bytef *sub_line = Filters::sub_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width, height);
        Bytef *up_line = Filters::up_filter(B_channel, G_channel, R_channel,width, height, i);
        Bytef *avg_line = Filters::avg_filter(B_channel, G_channel, R_channel,width, height, i);
        Bytef *paeth_line = Filters::paeth_filter(B_channel, G_channel, R_channel,width, height, i);

        // remapiraj vrijednosti px da bi se primjenila heuristika
        Filters::remap_lines(none_line, sub_line, up_line, avg_line, paeth_line, width);

        // get best line from heuristic
        BestLine best_line = Heuristics::apply_heuristic(none_line, sub_line, up_line, avg_line, paeth_line, width);

        // make line applicable for deflate
        make_deflate_lines(lines_to_deflate, best_line, width, i);

        free(none_line);
        free(sub_line);
        free(up_line);
        free(avg_line);
        free(paeth_line);
    }

    // TESTNA LINIJA
    //provjera linija koje ulaze u deflate
    // cout << "FINAL LINE FOR DEFLATE => FILTER | RGB | RGB ... | FILTER | RGB | RGB ..." << endl;
    // for (int i = 0; i < height * (width*3+1); i++) {
    //     cout << setw(3) << static_cast<int>(lines_to_deflate[i]) << " ";
    // }
    // cout << endl;

    make_compressions(width, height, lines_to_deflate);

    free(lines_to_deflate);
    free(B_channel);
    free(G_channel);
    free(R_channel);
    return 0;
}