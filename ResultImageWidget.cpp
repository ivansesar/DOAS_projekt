#include "Util.h"
#include "ResultImageWidget.h"


ResultImageWidget::ResultImageWidget(std::string fname, unsigned long long filesize, std::string image_title, CompressionResult* result)
    : filename(get_file_name(fname)), filesize(filesize)
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_spacing(10);

    // title
    image_title_label.set_halign(Gtk::Align::CENTER);
    image_title_label.set_label(image_title);
    append(image_title_label);

    // picture
    image_and_info_box.set_spacing(10);
        picture.set_file(Gio::File::create_for_path(fname));
        picture.set_size_request(75, 75);
        picture.set_hexpand(false);
        picture.get_style_context()->add_class("picture");
        apply_css();
    image_and_info_box.append(picture);

    // info box
    info_box.set_orientation(Gtk::Orientation::VERTICAL);
    filename_label.set_halign(Gtk::Align::START);
        filename_label.set_text("Filename: " + filename);
    file_size_label.set_halign(Gtk::Align::START);
        file_size_label.set_text("File size: " + std::to_string(filesize/1024) + " kB");
    compression_speed.set_halign(Gtk::Align::START);
        compression_speed.set_text("Compression speed: " + std::to_string(static_cast<int>(result->get_compression_speed())) + " kB/s");
    compression_ratio.set_halign(Gtk::Align::START);
        compression_ratio.set_text("Compression ratio: " + std::to_string(result->get_compression_ratio()));
    compressed_size.set_halign(Gtk::Align::START);
        compressed_size.set_text("Compressed size: " + std::to_string(result->get_compressed_size()/1024) + " kB");
    info_box.append(filename_label);
    info_box.append(file_size_label);
    info_box.append(compression_speed);
    info_box.append(compression_ratio);
    info_box.append(compressed_size);

    image_and_info_box.append(info_box);

    append(image_and_info_box);
}

