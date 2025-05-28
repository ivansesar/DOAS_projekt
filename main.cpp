#include <filesystem>
#include <fstream>
#include <iostream>
#include <gtkmm.h>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <zlib.h>
#include "Filters.h"
#include "Heuristics.h"
#include <map>
#include "PNGWriter.h"
#include "BMPReader.h"
#include "chrono"
#include <climits>
#include "ImageWidget.h"
#include "ResultImageWidget.h"
#include "Util.h"
using namespace std;

enum CompressionResultType {
    COMPRESSION_SPEED, COMPRESSION_RATIO
};


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
        lines_to_deflate[begin + k*3+1] = filtered_line[2*width + j];
        lines_to_deflate[begin + k*3+2] = filtered_line[1*width + j];
        lines_to_deflate[begin + k*3+3] = filtered_line[j];
    }
}

/**
 * Metoda koja obavlja kompresiju nad 1 slikom
 * Metoda definira parametre kompresije i izvodi svaku kombinaciju i ispisuje rezultat
 */
std::vector<CompressionResult*> make_compressions(int bytes_per_pixel, int width, int height, Bytef* lines_to_deflate, BMP_TYPE file_type, string filename) {
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

    std::vector<CompressionResult*> compression_results;

    // za svaki kompresijksi level odradi deflate kao tablicu compression_mem_level X compression_strategy
    int index = 0;
    for (const auto& level : compression_level) {
        for (const auto& window_size : compression_window_size) {
            for (const auto& mem_level : compression_mem_level) {
                for (const auto& strategy : compression_strategy) {

                    string parameters = "Z_DEFLATED " + level.first + " " + window_size.first + " " + mem_level.first + " " + strategy.first;
                    double compressed_ratio;
                    unsigned long compressed_size;
                    auto start = chrono::high_resolution_clock::now();
                    unsigned long full_size = height*(width*3+1);
                    z_stream strm{};
                    Bytef* compressed = (Bytef*) malloc(height*(width*3+1) * sizeof(Bytef));

                    strm.next_in = lines_to_deflate;
                    strm.avail_in = height*(width*3+1);
                    strm.next_out = compressed;
                    strm.avail_out = height*(width*3+1);

                    deflateInit2(&strm, level.second, Z_DEFLATED, window_size.second, mem_level.second, strategy.second);
                    deflate(&strm, Z_FINISH);

                    compressed_size = strm.total_out;
                    compressed_ratio = 1.0 - (static_cast<double>(strm.total_out) / full_size);
                    printf("At %d Compressed length = %lu\n",index, strm.total_out);

                    if (index == 0) {
                        // samo jednu kompresiju prevedi u png sliku za provjeru
                        cout << "Printing png" << endl;
                        PNGWriter::write_png(width, height, compressed, compressed_size, file_type, filename);
                    }

                    deflateEnd(&strm);

                    auto end = chrono::high_resolution_clock::now();
                    std::chrono::duration<double> duration = end - start;
                    long long compression_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                    double compression_speed = full_size / compression_time;
                    cout << "Compression speed = " << compression_speed  << " kB/s"<< endl;
                    CompressionResult* result = new CompressionResult(compressed_size, compression_time,compression_speed, compressed_ratio,parameters,filename);
                    compression_results.push_back(result);
                    // // izvođenje inflatea
                    // Bytef decompressed[height * (width*3)];//Bytef decompressed[height * (width*3+1)];
                    // z_stream strm2 = {nullptr};
                    // strm2.next_in = compressed;
                    // strm2.avail_in = sizeof(compressed);
                    // strm2.next_out = decompressed;
                    // strm2.avail_out = sizeof(decompressed);
                    //
                    // inflateInit(&strm2);
                    // inflate(&strm2,Z_FINISH);
                    //
                    // size_t decompressed_length = sizeof(decompressed);
                    // printf("At %d Decompressed length = %llu\n",index , decompressed_length);
                    //
                    // inflateEnd(&strm2);
                    index++;
                }
            }
        }
    }
    return compression_results;
}


class DirectoryChooser : public Gtk::Window {
public:


    DirectoryChooser() {
        set_title("Directory Chooser");
        set_default_size(1800,1200);


        init_GUI();
        create_csv();
    };
private:
    Gtk::Box scrolled_window_box;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::Box full_vertical_box;

    // first gui row
    Gtk::Grid button_grid;
    Gtk::Button button1;
    Gtk::Button button2;
    std::vector<std::string> bmp_files_text;
    std::vector<std::string> bmp_files_nature;

    // second gui row
    Gtk::Button button_run_compression;

    // third gui row -> grid with 2 columns, each column = vertical box of images
    Gtk::Grid images_grid;
    Gtk::Box images1_box;
    Gtk::Box images2_box;
    std::vector<ImageWidget*> images1_vector;
    std::vector<ImageWidget*> images2_vector;
    std::vector<CompressionResult*> results_text;
    std::vector<CompressionResult*> results_nature;
    std::vector<CompressionResult*> results_text_all_images;
    std::vector<CompressionResult*> results_nature_all_images;

    // fourth row -> best and worst image based on best average parameters considering compression speed and compression ratio
    std::map<string, std::vector<double>>  parameters_result_text_compression_speed;
    std::map<string, std::vector<double>>  parameters_result_text_compression_ratio;
    std::string best_parameter_for_best_compression_speed_text;
    std::string best_parameter_for_best_compression_ratio_text;

    std::map<string, std::vector<double>>  parameters_result_nature_compression_speed;
    std::map<string, std::vector<double>>  parameters_result_nature_compression_ratio;
    std::string best_parameter_for_best_compression_speed_nature;
    std::string best_parameter_for_best_compression_ratio_nature;

    // Boxes for images results for best avg compression parameters
    Gtk::Grid result_images_grid;
    Gtk::Box result_text_images_box;
    Gtk::Box result_nature_images_box;
    Gtk::Box text_box_speed;
    Gtk::Box text_box_ratio;
    Gtk::Box nature_box_speed;
    Gtk::Box nature_box_ratio;

    ImageWidget* best_text_img_compression_speed;
    ImageWidget* best_text_img_compression_ratio;
    ImageWidget* best_nature_img_compression_speed;
    ImageWidget* best_nature_img_compression_ratio;
    ImageWidget* worst_text_img_compression_speed;
    ImageWidget* worst_text_img_compression_ratio;
    ImageWidget* worst_nature_img_compression_speed;
    ImageWidget* worst_nature_img_compression_ratio;

    std::ofstream csv_results;

    void create_csv() {
        string filename = "./results/results_of_compressions.csv";
        csv_results = std::ofstream(filename, std::ios::out | std::ios::trunc);
        if (!csv_results.is_open()) {
            std::cerr << "Couldn't open .csv file" << std::endl;
            return;
        }

        csv_results << "File Type,Filename,Compression Method,Compression Level,Compression Window Size,Compression Memory Level,Compression Strategy,Compression Speed,Compression Ratio, Compressed Size" << std::endl;

    }

    void init_GUI() {

        // SCROLLED MAIN WINDOW
        scrolled_window_box.set_margin(10);
        scrolled_window_box.set_margin_top(0);
        scrolled_window_box.set_orientation(Gtk::Orientation::VERTICAL);
        scrolled_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
        scrolled_window.set_expand(true);
        scrolled_window_box.append(scrolled_window);
        scrolled_window_box.set_expand(true);
        set_child(scrolled_window_box);

        full_vertical_box.set_valign(Gtk::Align::START);
        full_vertical_box.set_orientation(Gtk::Orientation::VERTICAL);
        //full_vertical_box.set_margin_top(10);


        // FIRST ROW
        button1.set_label("Choose bmp text directory");
        button1.signal_clicked().connect(sigc::mem_fun(*this, &DirectoryChooser::on_button1_clicked));


        button2.set_label("Choose bmp nature directory");
        button2.signal_clicked().connect(sigc::mem_fun(*this, &DirectoryChooser::on_button2_clicked));

        button_grid.set_orientation(Gtk::Orientation::HORIZONTAL);
        button_grid.attach(button1, 0, 0, 1, 1);
        button_grid.attach(button2, 1,0,1,1);
        button_grid.set_column_homogeneous(true);
        button_grid.set_column_spacing(10);
        button_grid.set_margin_top(10);
        button1.set_expand(false);
        button2.set_expand(false);

        full_vertical_box.append(button_grid);

        // SECOND GUI ROW
        button_run_compression.set_label("Run compressions");
        button_run_compression.set_hexpand(true);
        button_run_compression.set_vexpand(false);
        button_run_compression.set_halign(Gtk::Align::CENTER);
        button_run_compression.set_margin_top(10);
        button_run_compression.signal_clicked().connect(sigc::mem_fun(*this, &DirectoryChooser::parse_and_compress_files));

        full_vertical_box.append(button_run_compression);

        images1_box.set_hexpand(true);
        images1_box.set_spacing(15);
        images2_box.set_hexpand(true);
        images2_box.set_spacing(15);
        images_grid.set_orientation(Gtk::Orientation::HORIZONTAL);
        images_grid.set_column_spacing(10);
        images_grid.set_margin_top(10);
        images_grid.set_column_homogeneous(true);
        images_grid.attach(images1_box, 0, 0, 1, 1);
        images_grid.attach(images2_box, 1, 0,1,1);
        full_vertical_box.append(images_grid);
        Gtk::Label separator("====================================================================================================================");
        full_vertical_box.append(separator);

        scrolled_window.set_child(full_vertical_box);
    }

    /**
     *
     * @param parameters su parametri kompresije za koje se trazi najbolja i najlosija slika
     * @return vektor u kojem je prvi član najbolja slika, a drugi najlosija slika za parametre kompresije
     */
    std::vector<CompressionResult *> run_compression_on_avg_parameter(const string & parameters, BMP_TYPE type, CompressionResultType result_type) {
        std::vector<CompressionResult *> best_and_worst_image(2);

        //parametar je oblika: method - compression level - window size - memory level - compression strategy
        std::vector<string> params;
        std::istringstream stream(parameters);
        std::string p;
        while (stream >> p) {
            params.push_back(p);
        }

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


        std::vector<CompressionResult*> results_for_parameters;

        // napravi deflate za svaku sliku iz kategorije
        std::vector<std::string> images_vector = type == BMP_TEXT ? bmp_files_text : bmp_files_nature;
        for (auto& s : images_vector) {
             //******* PROČITAJ BMP_data u RGB channele **********//
            BMP_data bmp_data = BMPReader::read_bmp(s);
            int width = bmp_data.width;
            int height = bmp_data.height;
            int bytes_per_pixel = bmp_data.bytes_per_pixel;
            Bytef* B_channel = bmp_data.B_channel;
            Bytef* R_channel = bmp_data.R_channel;
            Bytef* G_channel = bmp_data.G_channel;

            Bytef* lines_to_deflate = (Bytef *) malloc(height * (width * bytes_per_pixel + 1) * sizeof(Bytef));
            for (int i = 0; i < height; i++) {
                Bytef *none_line = Filters::none_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width);
                Bytef *sub_line = Filters::sub_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width, height);
                Bytef *up_line = Filters::up_filter(B_channel, G_channel, R_channel,width, height, i);
                Bytef *avg_line = Filters::avg_filter(B_channel, G_channel, R_channel,width, height, i);
                Bytef *paeth_line = Filters::paeth_filter(B_channel, G_channel, R_channel,width, height, i);

                // get best line from heuristic
                BestLine best_line = Heuristics::apply_heuristic(none_line, sub_line, up_line, avg_line, paeth_line, width);
                // make line applicable for deflate
                make_deflate_lines(lines_to_deflate, best_line, width, i);
            }

            // Izvedi deflate
            double compressed_ratio;
            unsigned long compressed_size;
            auto start = chrono::high_resolution_clock::now();
            unsigned long full_size = height*(width*3+1);
            z_stream strm{};
            Bytef* compressed = (Bytef*) malloc(height*(width*3+1) * sizeof(Bytef));

            strm.next_in = lines_to_deflate;
            strm.avail_in = height*(width*3+1);
            strm.next_out = compressed;
            strm.avail_out = height*(width*3+1);

            deflateInit2(&strm, compression_level[params[1]], compression_method[params[0]], compression_window_size[params[2]], compression_mem_level[params[3]], compression_strategy[params[4]]);
            deflate(&strm, Z_FINISH);

            compressed_size = strm.total_out;
            compressed_ratio = 1.0 - (static_cast<double>(strm.total_out) / full_size);

            deflateEnd(&strm);

            auto end = chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            long long compression_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            double compression_speed = full_size / compression_time;
            cout << "Compression speed on avg parameters = " << compression_speed  << " kB/s"<< endl;
            CompressionResult* result = new CompressionResult(compressed_size, compression_time,compression_speed, compressed_ratio,parameters,s);
            results_for_parameters.push_back(result);
            free(R_channel);
            free(G_channel);
            free(B_channel);
            free(bmp_data.pixel_data_no_padding);
            free(lines_to_deflate);
        }

        // imamo napunjen results_for_parameters
        // pretrazi najbolju i najlosiju sliku po trazenom parametru
        if (result_type == COMPRESSION_SPEED) {
            double max_speed = 0.0;
            double min_speed = numeric_limits<double>::max();
            for (CompressionResult* cr : results_for_parameters) {
                if (cr->get_compression_speed() > max_speed) {
                    max_speed = cr->get_compression_speed();
                    best_and_worst_image[0] = cr;
                }
                if (cr->get_compression_speed() < min_speed) {
                    min_speed = cr->get_compression_speed();
                    best_and_worst_image[1] = cr;
                }

            }
        }else if (result_type == COMPRESSION_RATIO) {
            double max_ratio = 0.0;
            double min_ratio = 1;
            for (CompressionResult* cr : results_for_parameters) {
                if (cr->get_compression_ratio() > max_ratio) {
                    max_ratio = cr->get_compression_ratio();
                    best_and_worst_image[0] = cr;
                }
                if (cr->get_compression_ratio() < min_ratio) {
                    min_ratio = cr->get_compression_ratio();
                    best_and_worst_image[1] = cr;
                }

            }
        }
        return best_and_worst_image;
    }

    void make_compressions_for_best_avg_parameters() {
        //parametar je oblika: method - compression level - window size - memory level - compression strategy

        std::vector<CompressionResult*> best_and_worst_text_images_speed = run_compression_on_avg_parameter(best_parameter_for_best_compression_speed_text, BMP_TEXT, COMPRESSION_SPEED);
        std::vector<CompressionResult*> best_and_worst_text_images_ratio = run_compression_on_avg_parameter(best_parameter_for_best_compression_ratio_text,BMP_TEXT, COMPRESSION_RATIO);
        std::vector<CompressionResult*> best_and_worst_nature_images_speed = run_compression_on_avg_parameter(best_parameter_for_best_compression_speed_nature,BMP_NATURE, COMPRESSION_SPEED);
        std::vector<CompressionResult*> best_and_worst_nature_images_ratio = run_compression_on_avg_parameter(best_parameter_for_best_compression_ratio_nature, BMP_NATURE,COMPRESSION_RATIO);
        cout << "<<<<<<<<<<<< IM HERE >>>>>>>>>>>>>>>>>>>" << endl;
        cout << "best text image speed = " << best_and_worst_text_images_speed[0]->get_filename() << endl;
        cout << "worst text image speed = " << best_and_worst_text_images_speed[1]->get_filename() << endl;
        cout << "best text image ratio = " << best_and_worst_text_images_ratio[0]->get_filename() << endl;
        cout << "worst text image ratio = " << best_and_worst_text_images_ratio[1]->get_filename() << endl;
        cout << "best nature image speed = " << best_and_worst_nature_images_speed[0]->get_filename() << endl;
        cout << "worst nature image speed = " << best_and_worst_nature_images_speed[1]->get_filename() << endl;
        cout << "best nature image ratio = " << best_and_worst_nature_images_ratio[0]->get_filename() << endl;
        cout << "worst nature image ratio = " << best_and_worst_nature_images_ratio[1]->get_filename() << endl;
        // stringovi putanja najboljih i najlosijih slika
        std::string best_text_img_speed = best_and_worst_text_images_speed[0]->get_filename();
        unsigned long long best_text_img_speed_size = std::filesystem::file_size(best_text_img_speed); //best_and_worst_text_images_speed[0]->get_compressed_size();
        std::string worst_text_img_speed = best_and_worst_text_images_speed[1]->get_filename();
        unsigned long long worst_text_img_speed_size = std::filesystem::file_size(worst_text_img_speed);//best_and_worst_text_images_speed[1]->get_compressed_size();
        std::string best_text_img_ratio = best_and_worst_text_images_ratio[0]->get_filename();
        unsigned long long best_text_img_ratio_size = std::filesystem::file_size(best_text_img_ratio);//best_and_worst_text_images_ratio[0]->get_compressed_size();
        std::string worst_text_img_ratio = best_and_worst_text_images_ratio[1]->get_filename();
        unsigned long long worst_text_img_ratio_size = std::filesystem::file_size(worst_text_img_ratio);//best_and_worst_text_images_ratio[1]->get_compressed_size();
        std::string best_nature_img_speed = best_and_worst_nature_images_speed[0]->get_filename();
        unsigned long long best_nature_img_speed_size = std::filesystem::file_size(best_nature_img_speed);//best_and_worst_nature_images_speed[0]->get_compressed_size();
        std::string worst_nature_img_speed = best_and_worst_nature_images_speed[1]->get_filename();
        unsigned long long worst_nature_img_speed_size = std::filesystem::file_size(worst_nature_img_speed);//best_and_worst_nature_images_speed[1]->get_compressed_size();
        std::string best_nature_img_ratio = best_and_worst_nature_images_ratio[0]->get_filename();
        unsigned long long best_nature_img_ratio_size = std::filesystem::file_size(best_nature_img_ratio);//best_and_worst_nature_images_ratio[0]->get_compressed_size();
        std::string worst_nature_img_ratio = best_and_worst_nature_images_ratio[1]->get_filename();
        unsigned long long worst_nature_img_ratio_size = std::filesystem::file_size(worst_nature_img_ratio);//best_and_worst_nature_images_ratio[1]->get_compressed_size();

        // Gtk::Grid result_images_grid;
        // Gtk::Box result_text_images_box;
        // Gtk::Box result_nature_images_box;
        // Gtk::Box text_box_speed;
        // Gtk::Box text_box_ratio;
        // Gtk::Box nature_box_speed;
        // Gtk::Box nature_box_ratio;

        // TEXT
        result_text_images_box.set_spacing(10);
        result_text_images_box.set_hexpand(true);
        result_text_images_box.set_orientation(Gtk::Orientation::VERTICAL);
        Gtk::Label label1("BEST COMPRESSION SPEED");
        result_text_images_box.append(label1);
        Gtk::Label label2(best_parameter_for_best_compression_speed_text);
        result_text_images_box.append(label2);
            text_box_speed.set_spacing(30);
            text_box_speed.set_hexpand(true);
            text_box_speed.set_halign(Gtk::Align::CENTER);
            text_box_speed.set_margin_end(10);
            ResultImageWidget* best_text_speed = new ResultImageWidget(best_text_img_speed, best_text_img_speed_size, "BEST IMAGE",  best_and_worst_text_images_speed[0]);
            ResultImageWidget* worst_text_speed = new ResultImageWidget(worst_text_img_speed, worst_text_img_speed_size, "WORST IMAGE", best_and_worst_text_images_speed[1]);
            text_box_speed.append(*best_text_speed);
            text_box_speed.append(*worst_text_speed);
        result_text_images_box.append(text_box_speed);

        Gtk::Label label3("BEST COMPRESSION RATIO");
        result_text_images_box.append(label3);
        Gtk::Label label4(best_parameter_for_best_compression_ratio_text);
        result_text_images_box.append(label4);
            text_box_ratio.set_spacing(30);
            text_box_ratio.set_halign(Gtk::Align::CENTER);
            text_box_ratio.set_margin_end(10);
            text_box_ratio.set_hexpand(true);
            ResultImageWidget* best_text_ratio = new ResultImageWidget(best_text_img_ratio, best_text_img_ratio_size, "BEST IMAGE",  best_and_worst_text_images_ratio[0]);
            ResultImageWidget* worst_text_ratio = new ResultImageWidget(worst_text_img_ratio, worst_text_img_ratio_size, "WORST IMAGE", best_and_worst_text_images_ratio[1]);
            text_box_ratio.append(*best_text_ratio);
            text_box_ratio.append(*worst_text_ratio);
        result_text_images_box.append(text_box_ratio);

        // NATURE
        result_nature_images_box.set_spacing(10);
        result_nature_images_box.set_hexpand(true);
        result_nature_images_box.set_orientation(Gtk::Orientation::VERTICAL);
        Gtk::Label label5("BEST COMPRESSION SPEED");
        result_nature_images_box.append(label5);
        Gtk::Label label6(best_parameter_for_best_compression_speed_nature);
        result_nature_images_box.append(label6);
            nature_box_speed.set_spacing(30);
            nature_box_speed.set_halign(Gtk::Align::CENTER);
            nature_box_speed.set_margin_end(10);
            nature_box_speed.set_hexpand(true);
            ResultImageWidget* best_nature_speed = new ResultImageWidget(best_nature_img_speed, best_nature_img_speed_size, "BEST IMAGE",  best_and_worst_nature_images_speed[0]);
            ResultImageWidget* worst_nature_speed = new ResultImageWidget(worst_nature_img_speed, worst_nature_img_speed_size, "WORST IMAGE", best_and_worst_nature_images_speed[1]);
            nature_box_speed.append(*best_nature_speed);
            nature_box_speed.append(*worst_nature_speed);
        result_nature_images_box.append(nature_box_speed);

        Gtk::Label label7("BEST COMPRESSION RATIO");
        result_nature_images_box.append(label7);
        Gtk::Label label8(best_parameter_for_best_compression_ratio_nature);
        result_nature_images_box.append(label8);
            nature_box_ratio.set_spacing(30);
            nature_box_ratio.set_halign(Gtk::Align::CENTER);
            nature_box_ratio.set_margin_end(10);
            nature_box_ratio.set_hexpand(true);
            ResultImageWidget* best_nature_ratio = new ResultImageWidget(best_nature_img_ratio, best_nature_img_ratio_size, "BEST IMAGE",  best_and_worst_nature_images_ratio[0]);
            ResultImageWidget* worst_nature_ratio = new ResultImageWidget(worst_nature_img_ratio, worst_nature_img_ratio_size, "WORST IMAGE", best_and_worst_nature_images_ratio[1]);
            nature_box_ratio.append(*best_nature_ratio);
            nature_box_ratio.append(*worst_nature_ratio);
        result_nature_images_box.append(nature_box_ratio);

        result_images_grid.set_margin_top(30);
        result_images_grid.set_margin_end(30);
        result_images_grid.attach(result_text_images_box, 0, 0, 1,1);
        result_images_grid.attach(result_nature_images_box, 1, 0,1,1);
        full_vertical_box.append(result_images_grid);
    }

    void calculate_and_insert_stats(std::vector<CompressionResult *> & results, BMP_TYPE image_type) {
        unsigned long best_compressed_size = 0;
        unsigned long worst_compressed_size = ULONG_MAX;

        CompressionResult* best_compression_result = nullptr;
        CompressionResult* worst_compression_result = nullptr;

        // ZA INDIVIUDALNE SLIKE
        for (auto result : results) {
            if (result->get_compressed_size() < worst_compressed_size) {
                worst_compressed_size = result->get_compressed_size();
                best_compression_result = result;
            }
            if (result->get_compressed_size() > best_compressed_size) {
                best_compressed_size = result->get_compressed_size();
                worst_compression_result = result;
            }
        }

        if (best_compression_result == nullptr || worst_compression_result == nullptr) {
            cout << "Best/worst compression result is nullptr" << endl;
        }

        // POSTAVLJANJE NAJBOLJE I NAJLOŠIJE KOMOPRESIJE ZA SVAKU SLIKU
        std::vector<ImageWidget *> vector = image_type == BMP_TEXT ? images1_vector : images2_vector;
        std::string filename_best = best_compression_result->get_filename();
        std::string filename_worst = worst_compression_result->get_filename();

        for (ImageWidget* image_widget : vector) {
            if (image_widget->full_filename == filename_best) {
                string best_comp_speed = "Compression speed: " + std::to_string(static_cast<int>(best_compression_result->get_compression_speed())) + " kB/s";
                image_widget->best_compression_speed.set_text(best_comp_speed);

                string best_comp_size = "Size: " + std::to_string(best_compression_result->get_compressed_size()/1024) + " kB";
                image_widget->best_compressed_size.set_text(best_comp_size);

                string best_comp_ratio = "Compression ratio: " + std::to_string(best_compression_result->get_compression_ratio());
                image_widget->best_compression_ratio.set_text(best_comp_ratio);

                // insert compression parameters in gui
                std::istringstream stream(best_compression_result->get_parameters());
                std::vector<std::string> parameters;
                string parameter;
                while (stream >> parameter) {
                    parameters.push_back(parameter);
                }
                cout << parameters.size() << endl;

                string best_comp_method = "Method: " + parameters[0];
                image_widget->best_compression_method.set_text(best_comp_method);
                string best_comp_level = "Compression level: " + parameters[1];
                image_widget->best_compression_level.set_text(best_comp_level);
                string best_comp_window_size = "Window size: " + parameters[2];
                image_widget->best_compression_window_size.set_text(best_comp_window_size);
                string best_comp_memory_level = "Memory level: " + parameters[3];
                image_widget->best_compression_memory_level.set_text(best_comp_memory_level);
                string best_comp_strategy = "Strategy: " + parameters[4];
                image_widget->best_compression_strategy.set_text(best_comp_strategy);

            }
            if (image_widget->full_filename == filename_worst) {
                string worst_comp_speed = "Comprssion speed: " + std::to_string(static_cast<int>(worst_compression_result->get_compression_speed())) + " kB/s";
                image_widget->worst_compression_speed.set_text(worst_comp_speed);

                string worst_comp_size = "Size: " + std::to_string(worst_compression_result->get_compressed_size()/1024) + " kB";
                image_widget->worst_compressed_size.set_text(worst_comp_size);

                string worst_comp_ratio = "Compression ratio: " + std::to_string(worst_compression_result->get_compression_ratio());
                image_widget->worst_compression_ratio.set_text(worst_comp_ratio);

                // insert compression parameters in gui
                std::istringstream stream(worst_compression_result->get_parameters());
                std::vector<std::string> parameters;
                string parameter;
                while (stream >> parameter) {
                    parameters.push_back(parameter);
                }
                cout << parameters.size() << endl;

                string worst_comp_method = "Method: " + parameters[0];
                image_widget->worst_compression_method.set_text(worst_comp_method);
                string worst_comp_level = "Compression level: " + parameters[1];
                image_widget->worst_compression_level.set_text(worst_comp_level);
                string worst_comp_window_size = "Window size: " + parameters[2];
                image_widget->worst_compression_window_size.set_text(worst_comp_window_size);
                string worst_comp_memory_level = "Memory level: " + parameters[3];
                image_widget->worst_compression_memory_level.set_text(worst_comp_memory_level);
                string worst_comp_strategy = "Strategy: " + parameters[4];
                image_widget->worst_compression_strategy.set_text(worst_comp_strategy);

            }
        }
    }

    void append_csv(const vector<CompressionResult *> & results, BMP_TYPE bmp) {
       for (CompressionResult *r : results) {
            //string parameters = "Z_DEFLATED " + level.first + " " + window_size.first + " " + mem_level.first + " " + strategy.first;
            string parameters = r->get_parameters();
            std::istringstream stream(parameters);
            std::vector<std::string> words;
            std::string word;
            while (stream >> word) {
                words.push_back(word);
            }

           string file_type = bmp == BMP_TEXT ? "Text" : "Nature";
           //csv_results << "File Type,Filename,Compression Method,Compression Level,Compression Window Size,Compression Memory Level,Compression Strategy,Compression Speed,Compression Ratio, Compressed Size" << std::endl;
           csv_results << file_type << "," << get_file_name(r->get_filename()) << "," << words[0] << "," << words[1] << "," << words[2] << "," << words[3] << "," << words[4] << "," << r->get_compression_speed() << "," << r->get_compression_ratio() << "," << r->get_compressed_size() << endl;
        }
    }

    void calculate_best_avg_parameters() {
        // ZA REZULTANTNE SLIKE - najbolji avg parametri
        for (auto result : results_text_all_images) {
            std::vector<double> vector_compression_speed;
            try {
                vector_compression_speed = parameters_result_text_compression_speed.at(result->get_parameters());
            } catch (std::out_of_range e) {
                vector_compression_speed = {};
            }
            vector_compression_speed.push_back(result->get_compression_speed());
            parameters_result_text_compression_speed.insert_or_assign(result->get_parameters(), vector_compression_speed);

            std::vector<double> vector_compression_ratio;
            try {
                vector_compression_ratio = parameters_result_text_compression_ratio[result->get_parameters()];
            }catch (std::out_of_range e) {
                vector_compression_ratio = {};
            }
            vector_compression_ratio.push_back(result->get_compression_ratio());
            parameters_result_text_compression_ratio.insert_or_assign(result->get_parameters(), vector_compression_ratio);
        }
        for (auto result : results_nature_all_images) {
            std::vector<double> vector_compression_speed;
            try {
                vector_compression_speed = parameters_result_nature_compression_speed.at(result->get_parameters());
            } catch (std::out_of_range e) {
                vector_compression_speed = {};
            }
            vector_compression_speed.push_back(result->get_compression_speed());
            parameters_result_nature_compression_speed.insert_or_assign(result->get_parameters(), vector_compression_speed);

            std::vector<double> vector_compression_ratio;
            try {
                vector_compression_ratio = parameters_result_nature_compression_ratio[result->get_parameters()];
            }catch (std::out_of_range e) {
                vector_compression_ratio = {};
            }
            vector_compression_ratio.push_back(result->get_compression_ratio());
            parameters_result_nature_compression_ratio.insert_or_assign(result->get_parameters(), vector_compression_ratio);


        }

        // IMAMO ZA SVAKI PARAMETAR BRZINU KOMPRESIJE I OMJER KOMPRESIJE OD SVAKE SLIKE -> racunaj prosjecno najbolji i najgori parametar
        // najbolji parametar za tekst za compression ratio
        double suma_max = std::numeric_limits<double>::max();
        double suma_min = 0.0;
        string best_parameter_text_compression_ratio;
        for (auto& pair : parameters_result_text_compression_ratio) {
            double suma_vektora = 0.0;
            for (double d : pair.second) {
                suma_vektora += d;
            }
            double avg_suma = suma_vektora / pair.second.size();
            if (avg_suma > suma_min) {
                suma_min = avg_suma;
                best_parameter_text_compression_ratio = pair.first;
            }
        }
        best_parameter_for_best_compression_ratio_text = best_parameter_text_compression_ratio;

        // najbolji parametar za tekst za compression speed
        suma_max = std::numeric_limits<double>::max();
        suma_min = 0.0;
        string best_parameter_text_compression_speed;
        for (auto& pair : parameters_result_text_compression_speed) {
            double suma_vektora = 0.0;
            for (double d : pair.second) {
                suma_vektora += d;
            }
            double avg_suma = suma_vektora / pair.second.size();
            if (avg_suma > suma_min) {
                suma_min = avg_suma;
                best_parameter_text_compression_speed = pair.first;
            }
        }
        best_parameter_for_best_compression_speed_text = best_parameter_text_compression_speed;

        // najbolji parametar za nature compression ratio
        suma_max = std::numeric_limits<double>::max();
         suma_min = 0.0;
        string best_parameter_nature_compression_ratio;
        for (auto& pair : parameters_result_nature_compression_ratio) {
            double suma_vektora = 0.0;
            for (double d : pair.second) {
                suma_vektora += d;
            }
            double avg_suma = suma_vektora / pair.second.size();
            if (avg_suma > suma_min) {
                suma_min = avg_suma;
                best_parameter_nature_compression_ratio = pair.first;
            }
        }
        best_parameter_for_best_compression_ratio_nature = best_parameter_nature_compression_ratio;
        // najbolji parametar za nature compression speed
        suma_max = std::numeric_limits<double>::max();
        suma_min = 0.0;
        string best_parameter_nature_compression_speed;
        for (auto& pair : parameters_result_nature_compression_speed) {
            double suma_vektora = 0.0;
            for (double d : pair.second) {
                suma_vektora += d;
            }
            double avg_suma = suma_vektora / pair.second.size();
            if (avg_suma > suma_min) {
                suma_min = avg_suma;
                best_parameter_nature_compression_speed = pair.first;
            }
        }
        best_parameter_for_best_compression_speed_nature = best_parameter_nature_compression_speed;

        cout << "Najbolji parametar u prosjeku za tekst za compression ratio je " << best_parameter_for_best_compression_ratio_text << endl;
        cout << "Najbolji parametar u prosjeku za tekst za compression speed je " << best_parameter_for_best_compression_speed_text << endl;
        cout << "Najbolji parametar u prosjeku za nature za compression ratio je " << best_parameter_for_best_compression_ratio_nature << endl;
        cout << "Najbolji parametar u prosjeku za nature za compression speed je " << best_parameter_for_best_compression_speed_nature << endl;

    }

    void parse_and_compress_files() {
        for (auto &f: bmp_files_text) {

            //******* PROČITAJ BMP_data u RGB channele **********//
            BMP_data bmp_data = BMPReader::read_bmp(f);
            int width = bmp_data.width;
            int height = bmp_data.height;
            int bytes_per_pixel = bmp_data.bytes_per_pixel;
            Bytef* B_channel = bmp_data.B_channel;
            Bytef* R_channel = bmp_data.R_channel;
            Bytef* G_channel = bmp_data.G_channel;

            Bytef* lines_to_deflate = (Bytef *) malloc(height * (width * bytes_per_pixel + 1) * sizeof(Bytef));
            for (int i = 0; i < height; i++) {
                Bytef *none_line = Filters::none_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width);
                cout << "im here none " << i << endl;
                Bytef *sub_line = Filters::sub_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width, height);
                cout << "im here sub " << i << endl;
                Bytef *up_line = Filters::up_filter(B_channel, G_channel, R_channel,width, height, i);
                cout << "im here up " << i << endl;
                Bytef *avg_line = Filters::avg_filter(B_channel, G_channel, R_channel,width, height, i);
                cout << "im here avg " << i << endl;
                Bytef *paeth_line = Filters::paeth_filter(B_channel, G_channel, R_channel,width, height, i);
                cout << "im here paeth " << i << endl;

                // get best line from heuristic
                BestLine best_line = Heuristics::apply_heuristic(none_line, sub_line, up_line, avg_line, paeth_line, width);
                cout << "im here bestline " << i << endl;
                // make line applicable for deflate
                cout << "Best line filter type = " << best_line.type << endl;
                make_deflate_lines(lines_to_deflate, best_line, width, i);
                cout << "im here make_deflate_lines " << i << endl;
                // test best line
                // cout << "printing best line at index " << i << endl;
                // for (int j = 0; j < width*3+1; j++) {
                //     cout << static_cast<int>(best_line.line[j]) << " ";
                // }
                // cout << endl;

                //free(best_line.line);
                 // free(none_line);
                 // free(sub_line);
                 // free(up_line);
                 // free(avg_line);
                 // free(paeth_line);
            }


            // TESTNA LINIJA
            //provjera linija koje ulaze u deflate
            // cout << "FINAL LINE FOR DEFLATE => FILTER | RGB | RGB ... | FILTER | RGB | RGB ..." << endl;
            // for (int i = 0; i < height * (width*3+1); i++) {
            //     cout << setw(3) << static_cast<int>(lines_to_deflate[i]) << " ";
            // }
            // cout << endl;
            results_text = make_compressions(bytes_per_pixel, width, height, lines_to_deflate, BMP_TYPE::BMP_TEXT,f);
            for (CompressionResult * r : results_text) {
                results_text_all_images.push_back(r);
            }
            calculate_and_insert_stats(results_text, BMP_TYPE::BMP_TEXT);
            append_csv(results_text, BMP_TYPE::BMP_TEXT);

            free(R_channel);
            free(G_channel);
            free(B_channel);
            free(bmp_data.pixel_data_no_padding);
            free(lines_to_deflate);
        }
        for (auto& f : bmp_files_nature) {
            //******* PROČITAJ BMP_data u RGB channele **********//
            BMP_data bmp_data = BMPReader::read_bmp(f);
            int width = bmp_data.width;
            int height = bmp_data.height;
            int bytes_per_pixel = bmp_data.bytes_per_pixel;
            Bytef* B_channel = bmp_data.B_channel;
            Bytef* R_channel = bmp_data.R_channel;
            Bytef* G_channel = bmp_data.G_channel;

            // cout << "Pixel data no padding " << endl;
            // for (int i = 0; i < width*height*bytes_per_pixel; i++) {
            //     cout << static_cast<int>(bmp_data.pixel_data_no_padding[i]) << " ";
            // }
            // cout << endl;

            // Imamo B G R channele i ide filtriranje
            // za svaki redak izvrši filtriranje i primjeni heuristiku i spremi u oblik za deflate
            //
            // izlazni retci koji su ulaz za deflate
            // oblika za svaki redak [filter_type] [RGB 1.px] [RGB 2.px] ... [RGB zadnji px]

            Bytef* lines_to_deflate = (Bytef *) malloc(height * (width * bytes_per_pixel + 1) * sizeof(Bytef));
            for (int i = 0; i < height; i++) {
                Bytef *none_line = Filters::none_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width);
                cout << "im here none " << i << endl;
                Bytef *sub_line = Filters::sub_filter(&B_channel[i*width], &G_channel[i*width], &R_channel[i*width],width, height);
                cout << "im here sub " << i << endl;
                Bytef *up_line = Filters::up_filter(B_channel, G_channel, R_channel,width, height, i);
                cout << "im here up " << i << endl;
                Bytef *avg_line = Filters::avg_filter(B_channel, G_channel, R_channel,width, height, i);
                cout << "im here avg " << i << endl;
                Bytef *paeth_line = Filters::paeth_filter(B_channel, G_channel, R_channel,width, height, i);
                cout << "im here paeth " << i << endl;

                // get best line from heuristic
                BestLine best_line = Heuristics::apply_heuristic(none_line, sub_line, up_line, avg_line, paeth_line, width);
                cout << "im here bestline " << i << endl;
                // make line applicable for deflate
                cout << "Best line filter type = " << best_line.type << endl;
                make_deflate_lines(lines_to_deflate, best_line, width, i);
                cout << "im here make_deflate_lines " << i << endl;
                // test best line
                // cout << "printing best line at index " << i << endl;
                // for (int j = 0; j < width*3+1; j++) {
                //     cout << static_cast<int>(best_line.line[j]) << " ";
                // }
                // cout << endl;

                //free(best_line.line);
                // free(none_line);
                // free(sub_line);
                // free(up_line);
                // free(avg_line);
                // free(paeth_line);
            }


            // TESTNA LINIJA
            //provjera linija koje ulaze u deflate
            // cout << "FINAL LINE FOR DEFLATE => FILTER | RGB | RGB ... | FILTER | RGB | RGB ..." << endl;
            // for (int i = 0; i < height * (width*3+1); i++) {
            //     cout << setw(3) << static_cast<int>(lines_to_deflate[i]) << " ";
            // }
            // cout << endl;
            results_nature = make_compressions(bytes_per_pixel, width, height, lines_to_deflate, BMP_TYPE::BMP_NATURE,f);
            for (CompressionResult * r : results_nature) {
                results_nature_all_images.push_back(r);
            }
            calculate_and_insert_stats(results_nature, BMP_TYPE::BMP_NATURE);
            append_csv(results_nature, BMP_TYPE::BMP_NATURE);
            free(R_channel);
            free(G_channel);
            free(B_channel);
            free(bmp_data.pixel_data_no_padding);
            free(lines_to_deflate);
        }
        csv_results.close();

        // izracunaj best avg parametre za compression speed/ratio
        calculate_best_avg_parameters();
        // IZVRTI SVE SLIKE SVAKE KATEGORIJE NA NAJBOLJEM PROSJECNOM PROSJEKU ZA COMPRESSION SPEED/RATIO
        make_compressions_for_best_avg_parameters();
    }
protected:
    void on_button1_clicked() {
        auto folder_dialog = Gtk::FileDialog::create();
        folder_dialog->set_title("Choose directory");
        folder_dialog->set_accept_label("Select");
        folder_dialog->select_folder(*this, sigc::bind(sigc::mem_fun(*this, &DirectoryChooser::on_folder_selected), folder_dialog, BMP_TYPE::BMP_TEXT));
        std::filesystem::create_directory("./text");

    }
    void on_button2_clicked() {
        auto folder_dialog = Gtk::FileDialog::create();
        folder_dialog->set_title("Choose directory");
        folder_dialog->set_accept_label("Select");
        folder_dialog->select_folder(*this, sigc::bind(sigc::mem_fun(*this, &DirectoryChooser::on_folder_selected), folder_dialog, BMP_TYPE::BMP_NATURE));
        std::filesystem::create_directory("./nature");
    }

    void load_images_in_gui(BMP_TYPE file_type) {
        if (file_type == BMP_TEXT) {
            images1_box.set_orientation(Gtk::Orientation::VERTICAL);
            for (auto& file : bmp_files_text) {
                cout << "Im here"<< endl;
                uintmax_t size = std::filesystem::file_size(file);
                ImageWidget* img_widget = new ImageWidget(file, size);
                images1_vector.push_back(img_widget);
                }
            for (ImageWidget* img_widget : images1_vector) {
                images1_box.append(*img_widget);
            }
            images_grid.attach(images1_box,0,0,1,1);
        }else if (file_type == BMP_NATURE) {
            images2_box.set_orientation(Gtk::Orientation::VERTICAL);
            for (auto& file : bmp_files_nature) {
                ImageWidget* img_widget = new ImageWidget(file, std::filesystem::file_size(file));
                images2_vector.push_back(img_widget);
            }
            for (ImageWidget* img_widget : images2_vector) {
                images2_box.append(*img_widget);
            }
            images_grid.attach(images2_box,1,0,1,1);
        }
        full_vertical_box.append(images_grid);
    }

    void on_folder_selected(Glib::RefPtr<Gio::AsyncResult> result, Glib::RefPtr<Gtk::FileDialog> folder_dialog, BMP_TYPE file_type) {
        try {
            auto folder = folder_dialog->select_folder_finish(result);
            for (const auto & file : std::filesystem::directory_iterator(folder->get_path())) {
                if (file.path().extension() == ".bmp") {
                    if (file_type == BMP_TYPE::BMP_TEXT) {
                        bmp_files_text.push_back(file.path().string());
                    }else if (file_type == BMP_TYPE::BMP_NATURE) {
                        bmp_files_nature.push_back(file.path().string());
                    }
                }
            }
            if (file_type == BMP_TYPE::BMP_TEXT) {
                load_images_in_gui(file_type);
            }else if (file_type == BMP_TYPE::BMP_NATURE) {
                load_images_in_gui(file_type);

            }
            std::cout << folder->get_path() << std::endl;

        }catch (Gtk::DialogError& de) {
            std::cout << de.what() << std::endl;
        }catch (Glib::Error& e) {
            std::cout << e.what() << std::endl;
        }
    }
};

int main(int argc, char** argv) {
    auto app = Gtk::Application::create("org.gtkmm.example.DirectoryChooser");
    return app->make_window_and_run<DirectoryChooser>(argc, argv);
}