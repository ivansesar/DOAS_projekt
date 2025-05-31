#include "ImageWidget.h"
#include <iostream>
#include "Util.h"

ImageWidget::ImageWidget(std::string fname, unsigned long long filesize) :
 full_filename(fname) ,filename(get_file_name(fname)), filesize(filesize)
{
    // container box is horizontal
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_spacing(10);
    // apply css class
    this->get_style_context()->add_class("container");

    picture.set_file(Gio::File::create_for_path(fname));
    picture.set_size_request(150, 150);
    picture.set_hexpand(false);
    picture.get_style_context()->add_class("picture");
    picture.set_margin_top(30);
    picture.set_valign(Gtk::Align::CENTER);
    apply_css();
    image_box.set_orientation(Gtk::Orientation::VERTICAL);
    image_box.append(filename_label);
    image_box.append(file_size_label);
    image_box.append(picture);
    filename_label.set_text("Filename: " + filename);
    file_size_label.set_text("File size: " + std::to_string(filesize/1024) + " kB");
    info_box.set_orientation(Gtk::Orientation::VERTICAL);


    // compression_results_box.set_orientation(Gtk::Orientation::HORIZONTAL);
    compression_results_box.set_margin_top(10);
    compression_results_box.set_column_spacing(10);
    compression_results_box.set_row_spacing(10);
    // compression_results_box.set_homogeneous(true);
        Gtk::Label text_best_compression_ratio("BEST COMPRESSION RATIO");
        cr_best_compression_speed.set_halign(Gtk::Align::START);
        cr_best_compression_speed.set_text("Compression speed: ");
        cr_best_compressed_size.set_halign(Gtk::Align::START);
        cr_best_compressed_size.set_text("Compressed size: ");
        cr_best_compression_ratio.set_halign(Gtk::Align::START);
        cr_best_compression_ratio.set_text("Compression ratio: ");
        cr_best_compression_level.set_halign(Gtk::Align::START);
        cr_best_compression_level.set_text("Compression level: ");
        cr_best_compression_window_size.set_halign(Gtk::Align::START);
        cr_best_compression_window_size.set_text("Window size: ");
        cr_best_compression_method.set_halign(Gtk::Align::START);
        cr_best_compression_method.set_text("Method: ");
        cr_best_compression_memory_level.set_halign(Gtk::Align::START);
        cr_best_compression_memory_level.set_text("Memory level: ");
        cr_best_compression_strategy.set_halign(Gtk::Align::START);
        cr_best_compression_strategy.set_text("Strategy: ");

        best_compression_ratio_box.set_hexpand(true);
        best_compression_ratio_box.set_orientation(Gtk::Orientation::VERTICAL);
        best_compression_ratio_box.append(text_best_compression_ratio);
        best_compression_ratio_box.append(cr_best_compression_speed);
        best_compression_ratio_box.append(cr_best_compressed_size);
        best_compression_ratio_box.append(cr_best_compression_ratio);
        best_compression_ratio_box.append(cr_best_compression_level);
        best_compression_ratio_box.append(cr_best_compression_window_size);
        best_compression_ratio_box.append(cr_best_compression_method);
        best_compression_ratio_box.append(cr_best_compression_memory_level);
        best_compression_ratio_box.append(cr_best_compression_strategy);

        Gtk::Label text_worst_compression_ratio("WORST COMPRESSION RATIO");
        cr_worst_compression_speed.set_halign(Gtk::Align::START);
        cr_worst_compression_speed.set_text("Compression speed: ");
        cr_worst_compressed_size.set_halign(Gtk::Align::START);
        cr_worst_compressed_size.set_text("Compressed size: ");
        cr_worst_compression_ratio.set_halign(Gtk::Align::START);
        cr_worst_compression_ratio.set_text("Compression ratio: ");
        cr_worst_compression_level.set_halign(Gtk::Align::START);
        cr_worst_compression_level.set_text("Compression level: ");
        cr_worst_compression_window_size.set_halign(Gtk::Align::START);
        cr_worst_compression_window_size.set_text("Window size: ");
        cr_worst_compression_method.set_halign(Gtk::Align::START);
        cr_worst_compression_method.set_text("Method: ");
        cr_worst_compression_memory_level.set_halign(Gtk::Align::START);
        cr_worst_compression_memory_level.set_text("Memory level: ");
        cr_worst_compression_strategy.set_halign(Gtk::Align::START);
        cr_worst_compression_strategy.set_text("Strategy: ");

        worst_compression_ratio_box.set_hexpand(true);
        worst_compression_ratio_box.set_orientation(Gtk::Orientation::VERTICAL);
        worst_compression_ratio_box.append(text_worst_compression_ratio);
        worst_compression_ratio_box.append(cr_worst_compression_speed);
        worst_compression_ratio_box.append(cr_worst_compressed_size);
        worst_compression_ratio_box.append(cr_worst_compression_ratio);
        worst_compression_ratio_box.append(cr_worst_compression_level);
        worst_compression_ratio_box.append(cr_worst_compression_window_size);
        worst_compression_ratio_box.append(cr_worst_compression_method);
        worst_compression_ratio_box.append(cr_worst_compression_memory_level);
        worst_compression_ratio_box.append(cr_worst_compression_strategy);

    Gtk::Label text_best_compression_speed("BEST COMPRESSION SPEED");
        cs_best_compression_speed.set_halign(Gtk::Align::START);
        cs_best_compression_speed.set_text("Compression speed: ");
        cs_best_compressed_size.set_halign(Gtk::Align::START);
        cs_best_compressed_size.set_text("Compressed size: ");
        cs_best_compression_ratio.set_halign(Gtk::Align::START);
        cs_best_compression_ratio.set_text("Compression ratio: ");
        cs_best_compression_level.set_halign(Gtk::Align::START);
        cs_best_compression_level.set_text("Compression level: ");
        cs_best_compression_window_size.set_halign(Gtk::Align::START);
        cs_best_compression_window_size.set_text("Window size: ");
        cs_best_compression_method.set_halign(Gtk::Align::START);
        cs_best_compression_method.set_text("Method: ");
        cs_best_compression_memory_level.set_halign(Gtk::Align::START);
        cs_best_compression_memory_level.set_text("Memory level: ");
        cs_best_compression_strategy.set_halign(Gtk::Align::START);
        cs_best_compression_strategy.set_text("Strategy: ");

        best_compression_speed_box.set_hexpand(true);
        best_compression_speed_box.set_orientation(Gtk::Orientation::VERTICAL);
        best_compression_speed_box.append(text_best_compression_speed);
        best_compression_speed_box.append(cs_best_compression_speed);
        best_compression_speed_box.append(cs_best_compressed_size);
        best_compression_speed_box.append(cs_best_compression_ratio);
        best_compression_speed_box.append(cs_best_compression_level);
        best_compression_speed_box.append(cs_best_compression_window_size);
        best_compression_speed_box.append(cs_best_compression_method);
        best_compression_speed_box.append(cs_best_compression_memory_level);
        best_compression_speed_box.append(cs_best_compression_strategy);

        Gtk::Label text_worst_compression_speed("WORST COMPRESSION SPEED");
        cs_worst_compression_speed.set_halign(Gtk::Align::START);
        cs_worst_compression_speed.set_text("Compression speed: ");
        cs_worst_compressed_size.set_halign(Gtk::Align::START);
        cs_worst_compressed_size.set_text("Compressed size: ");
        cs_worst_compression_ratio.set_halign(Gtk::Align::START);
        cs_worst_compression_ratio.set_text("Compression ratio: ");
        cs_worst_compression_level.set_halign(Gtk::Align::START);
        cs_worst_compression_level.set_text("Compression level: ");
        cs_worst_compression_window_size.set_halign(Gtk::Align::START);
        cs_worst_compression_window_size.set_text("Window size: ");
        cs_worst_compression_method.set_halign(Gtk::Align::START);
        cs_worst_compression_method.set_text("Method: ");
        cs_worst_compression_memory_level.set_halign(Gtk::Align::START);
        cs_worst_compression_memory_level.set_text("Memory level: ");
        cs_worst_compression_strategy.set_halign(Gtk::Align::START);
        cs_worst_compression_strategy.set_text("Strategy: ");

        worst_compression_speed_box.set_hexpand(true);
        worst_compression_speed_box.set_orientation(Gtk::Orientation::VERTICAL);
        worst_compression_speed_box.append(text_worst_compression_speed);
        worst_compression_speed_box.append(cs_worst_compression_speed);
        worst_compression_speed_box.append(cs_worst_compressed_size);
        worst_compression_speed_box.append(cs_worst_compression_ratio);
        worst_compression_speed_box.append(cs_worst_compression_level);
        worst_compression_speed_box.append(cs_worst_compression_window_size);
        worst_compression_speed_box.append(cs_worst_compression_method);
        worst_compression_speed_box.append(cs_worst_compression_memory_level);
        worst_compression_speed_box.append(cs_worst_compression_strategy);


    compression_results_box.attach(best_compression_ratio_box, 0, 0, 1, 1);
    compression_results_box.attach(worst_compression_ratio_box, 1, 0, 1, 1);
    compression_results_box.attach(best_compression_speed_box, 0, 1, 1, 1);
    compression_results_box.attach(worst_compression_speed_box, 1, 1, 1, 1);
    info_box.append(compression_results_box);

    append(image_box);
    append(info_box);

}
