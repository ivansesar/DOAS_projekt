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
#include "CompressionResult.h"
#include <map>
#include "PNGWriter.h"
#include "BMPReader.h"
#include "chrono"
using namespace std;

// void make_deflate_lines(std::vector<Bytef *>& lines_to_deflate, BestLine best_line, int width, int current_row) {
//     Bytef *filtered_line = best_line.line;
//     lines_to_deflate.push_back(filtered_line);
// }

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
void make_compressions(int bytes_per_pixel, int width, int height, Bytef* lines_to_deflate, BMP_TYPE file_type, string filename) {
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
    std::map<string, CompressionResult*> results;

    // za svaki kompresijksi level odradi deflate kao tablicu compression_mem_level X compression_strategy
    int index = 0;
    for (const auto& level : compression_level) {
        for (const auto& window_size : compression_window_size) {
            for (const auto& mem_level : compression_mem_level) {
                for (const auto& strategy : compression_strategy) {

                    string parameters = level.first + " " + window_size.first + " " + mem_level.first + " " + strategy.first;
                    double compressed_ratio;
                    auto start = chrono::high_resolution_clock::now();

                    z_stream strm{};
                    Bytef* compressed = (Bytef*) malloc(height*(width*3+1) * sizeof(Bytef));

                    strm.next_in = lines_to_deflate;
                    strm.avail_in = height*(width*3+1);
                    strm.next_out = compressed;
                    strm.avail_out = height*(width*3+1);

                    deflateInit2(&strm, level.second, Z_DEFLATED, window_size.second, mem_level.second, strategy.second);
                    deflate(&strm, Z_FINISH);
                    unsigned long compressed_length = strm.total_out;
                    if (index == 0) {
                        // samo jednu kompresiju prevedi u png sliku za provjeru
                        PNGWriter::write_png(width, height, compressed, compressed_length, file_type, filename);

                    }
                    compressed_ratio = 1 - (strm.total_out / height*(width*3+1));
                    printf("At %d Compressed length = %llu\n",index, strm.total_out);
                    deflateEnd(&strm);

                    auto end = chrono::high_resolution_clock::now();
                    std::chrono::duration<double> duration = end - start;
                    long long compression_time = duration.count();


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
}


class DirectoryChooser : public Gtk::Window {
public:
    DirectoryChooser() {
        set_title("Directory Chooser");
        set_default_size(800,800);


        init_GUI();
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

    void init_GUI() {

        // SCROLLED MAIN WINDOW
        scrolled_window_box.set_margin(10);
        scrolled_window_box.set_margin_top(0);
        scrolled_window_box.set_orientation(Gtk::Orientation::VERTICAL);
        scrolled_window.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
        //scrolled_window.set_expand();
        scrolled_window_box.append(scrolled_window);
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
        //button_grid.set_margin_top(10);
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

        scrolled_window_box.append(full_vertical_box);
    }

    void parse_and_compress_files() {
        for (auto& f : bmp_files_text) {

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
            make_compressions(bytes_per_pixel, width, height, lines_to_deflate, BMP_TYPE::BMP_TEXT,f);
            //calculate_and_insert_stats();
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
            make_compressions(bytes_per_pixel, width, height, lines_to_deflate, BMP_TYPE::BMP_NATURE,f);
            //calculate_and_insert_stats();
            free(R_channel);
            free(G_channel);
            free(B_channel);
            free(bmp_data.pixel_data_no_padding);
            free(lines_to_deflate);
        }
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
        images_grid.set_orientation(Gtk::Orientation::HORIZONTAL);
        images_grid.set_column_spacing(10);
        images_grid.set_margin_top(10);
        images_grid.set_column_homogeneous(true);

        if (file_type == BMP_TEXT) {
            images1_box.set_orientation(Gtk::Orientation::VERTICAL);
            for (auto& file : bmp_files_text) {
                Gtk::Grid picture_grid;
                picture_grid.set_orientation(Gtk::Orientation::HORIZONTAL);
                picture_grid.set_column_homogeneous(true);
                picture_grid.set_column_spacing(10);
                picture_grid.set_margin_top(10);

                int height = 200;
                int width = 200;
                Gtk::Picture picture;
                picture.set_file(Gio::File::create_for_path(file));
                picture.set_size_request(width, height);
                picture_grid.attach(picture, 0, 0, 1, 1);

                unsigned long long file_size = std::filesystem::file_size(file);
                Gtk::Label label_size;
                label_size.set_label("File size: " + std::to_string(file_size));
                picture_grid.attach(label_size, 1,0,1,1);

                Gtk::Label label_compression_speed;
                label_compression_speed.set_label("Compression speed: ");
                picture_grid.attach(label_compression_speed, 2,0,1,1);

                Gtk::Label label_compression_rate;
                label_compression_rate.set_label("Compression rate: ");
                picture_grid.attach(label_compression_rate, 3, 0,1,1);

                images1_box.append(picture_grid);
            }
            images_grid.attach(images1_box,0,0,1,1);
        }else if (file_type == BMP_NATURE) {
            images2_box.set_orientation(Gtk::Orientation::VERTICAL);
            for (auto& file : bmp_files_nature) {
                Gtk::Grid picture_grid;
                picture_grid.set_orientation(Gtk::Orientation::HORIZONTAL);
                picture_grid.set_column_homogeneous(true);
                picture_grid.set_column_spacing(10);
                picture_grid.set_margin_top(10);

                int height = 200;
                int width = 200;
                Gtk::Picture picture;
                picture.set_file(Gio::File::create_for_path(file));
                picture.set_size_request(width, height);
                picture_grid.attach(picture, 0, 0, 1, 1);

                unsigned long long file_size = std::filesystem::file_size(file);
                Gtk::Label label_size;
                label_size.set_label("File size: " + std::to_string(file_size));
                picture_grid.attach(label_size, 1,0,1,1);

                Gtk::Label label_compression_speed;
                label_compression_speed.set_label("Compression speed: ");
                picture_grid.attach(label_compression_speed, 2,0,1,1);

                Gtk::Label label_compression_rate;
                label_compression_rate.set_label("Compression rate: ");
                picture_grid.attach(label_compression_rate, 3, 0,1,1);

                images2_box.append(picture_grid);
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