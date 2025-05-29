
#include <string>
#include <gtkmm/box.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/label.h>
#include <gtkmm/picture.h>

#include "CompressionResult.h"

class ResultImageWidget : public Gtk::Box{
public:
    ResultImageWidget(std::string filename, unsigned long long filesize, std::string image_title, CompressionResult* result);

    // about
    std::string filename;
    unsigned long long filesize;
    Gtk::Label image_title_label; // BEST IMAGE or WORST IMAGE

    //basic
    Gtk::Label filename_label;
    Gtk::Label file_size_label;
    Gtk::Picture picture;

    // results
    Gtk::Label compression_speed;
    Gtk::Label compression_ratio;
    Gtk::Label compressed_size;

    // boxes for layout
    Gtk::Box image_and_info_box;
    Gtk::Box info_box;

    void apply_css() {
        auto css = R"(
            .picture {
                transition: transform ease-in 0.2s;
            }
            .picture:hover {
                transform: scale(3);
                z-index: 5;
            }
        )";
        auto provider = Gtk::CssProvider::create();
        provider->load_from_data(css);

        Gtk::StyleContext::add_provider_for_display(
            Gdk::Display::get_default(),
            provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }
};
