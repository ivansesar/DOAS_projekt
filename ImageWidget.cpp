#include "ImageWidget.h"
#include <iostream>
#include "Util.h"

ImageWidget::ImageWidget(std::string fname, unsigned long long filesize) :
 full_filename(fname) ,filename(get_file_name(fname)), filesize(filesize)
{
    // container box is horizontal
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_spacing(10);

    picture.set_file(Gio::File::create_for_path(fname));
    picture.set_size_request(75, 75);
    picture.set_hexpand(false);
    picture.get_style_context()->add_class("picture");
    apply_css();
    append(picture);
    filename_label.set_text("Filename: " + filename);
    file_size_label.set_text("File size: " + std::to_string(filesize/1024) + " kB");
    info_box.set_orientation(Gtk::Orientation::VERTICAL);
    info_box.append(filename_label);
    info_box.append(file_size_label);

    compression_results_box.set_orientation(Gtk::Orientation::HORIZONTAL);
    compression_results_box.set_margin_top(10);
    compression_results_box.set_spacing(10);
    compression_results_box.set_homogeneous(true);
        Gtk::Label text_best_compression("BEST COMPRESSION");
        best_compression_speed.set_halign(Gtk::Align::START);
        best_compression_speed.set_text("Time: ");
        best_compressed_size.set_halign(Gtk::Align::START);
        best_compressed_size.set_text("Size: ");
        best_compression_ratio.set_halign(Gtk::Align::START);
        best_compression_ratio.set_text("Compression ratio: ");
        best_compression_level.set_halign(Gtk::Align::START);
        best_compression_level.set_text("Compression level: ");
        best_compression_window_size.set_halign(Gtk::Align::START);
        best_compression_window_size.set_text("Window size: ");
        best_compression_method.set_halign(Gtk::Align::START);
        best_compression_method.set_text("Method: ");
        best_compression_memory_level.set_halign(Gtk::Align::START);
        best_compression_memory_level.set_text("Memory level: ");
        best_compression_strategy.set_halign(Gtk::Align::START);
        best_compression_strategy.set_text("Strategy: ");

        best_compression_box.set_hexpand(true);
        best_compression_box.set_orientation(Gtk::Orientation::VERTICAL);
        best_compression_box.append(text_best_compression);
        best_compression_box.append(best_compression_speed);
        best_compression_box.append(best_compressed_size);
        best_compression_box.append(best_compression_ratio);
        best_compression_box.append(best_compression_level);
        best_compression_box.append(best_compression_window_size);
        best_compression_box.append(best_compression_method);
        best_compression_box.append(best_compression_memory_level);
        best_compression_box.append(best_compression_strategy);

        Gtk::Label text_worst_compression("WORST COMPRESSION");
        worst_compression_speed.set_halign(Gtk::Align::START);
        worst_compression_speed.set_text("Time: ");
        worst_compressed_size.set_halign(Gtk::Align::START);
        worst_compressed_size.set_text("Size: ");
        worst_compression_ratio.set_halign(Gtk::Align::START);
        worst_compression_ratio.set_text("Compression ratio: ");
        worst_compression_level.set_halign(Gtk::Align::START);
        worst_compression_level.set_text("Compression level: ");
        worst_compression_window_size.set_halign(Gtk::Align::START);
        worst_compression_window_size.set_text("Window size: ");
        worst_compression_method.set_halign(Gtk::Align::START);
        worst_compression_method.set_text("Method: ");
        worst_compression_memory_level.set_halign(Gtk::Align::START);
        worst_compression_memory_level.set_text("Memory level: ");
        worst_compression_strategy.set_halign(Gtk::Align::START);
        worst_compression_strategy.set_text("Strategy: ");

        worst_compression_box.set_hexpand(true);
        worst_compression_box.set_orientation(Gtk::Orientation::VERTICAL);
        worst_compression_box.append(text_worst_compression);
        worst_compression_box.append(worst_compression_speed);
        worst_compression_box.append(worst_compressed_size);
        worst_compression_box.append(worst_compression_ratio);
        worst_compression_box.append(worst_compression_level);
        worst_compression_box.append(worst_compression_window_size);
        worst_compression_box.append(worst_compression_method);
        worst_compression_box.append(worst_compression_memory_level);
        worst_compression_box.append(worst_compression_strategy);


    compression_results_box.append(best_compression_box);
    compression_results_box.append(worst_compression_box);
    info_box.append(compression_results_box);

    append(info_box);

}
