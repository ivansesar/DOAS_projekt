#include <gtkmm/box.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/picture.h>

class ImageWidget : public Gtk::Box{
public:
    ImageWidget(std::string filename, unsigned long long filesize);

    // about
    std::string full_filename;
    std::string filename;
    unsigned long long filesize;

    //basic
    Gtk::Label filename_label;
    Gtk::Label file_size_label;
    Gtk::Picture picture;

    // PREFIX "cr" means "compression ratio", PREFIX "cs" means "compression speed"
    // LABELS FOR COMPRESSION RATIO
    //best results
    Gtk::Label cr_best_compression_speed;
    Gtk::Label cr_best_compressed_size;
    Gtk::Label cr_best_compression_ratio;
    //best parameters
    Gtk::Label cr_best_compression_level;
    Gtk::Label cr_best_compression_window_size;
    Gtk::Label cr_best_compression_method;
    Gtk::Label cr_best_compression_memory_level;
    Gtk::Label cr_best_compression_strategy;

    //worst results
    Gtk::Label cr_worst_compression_speed;
    Gtk::Label cr_worst_compressed_size;
    Gtk::Label cr_worst_compression_ratio;
    //worst parameters
    Gtk::Label cr_worst_compression_level;
    Gtk::Label cr_worst_compression_window_size;
    Gtk::Label cr_worst_compression_method;
    Gtk::Label cr_worst_compression_memory_level;
    Gtk::Label cr_worst_compression_strategy;

    // LABELS FOR COMPRESSION SPEED
    //best results
    Gtk::Label cs_best_compression_speed;
    Gtk::Label cs_best_compressed_size;
    Gtk::Label cs_best_compression_ratio;
    //best parameters
    Gtk::Label cs_best_compression_level;
    Gtk::Label cs_best_compression_window_size;
    Gtk::Label cs_best_compression_method;
    Gtk::Label cs_best_compression_memory_level;
    Gtk::Label cs_best_compression_strategy;

    //worst results
    Gtk::Label cs_worst_compression_speed;
    Gtk::Label cs_worst_compressed_size;
    Gtk::Label cs_worst_compression_ratio;
    //worst parameters
    Gtk::Label cs_worst_compression_level;
    Gtk::Label cs_worst_compression_window_size;
    Gtk::Label cs_worst_compression_method;
    Gtk::Label cs_worst_compression_memory_level;
    Gtk::Label cs_worst_compression_strategy;

    // boxes for layout
    Gtk::Box image_box;
    Gtk::Box info_box;
    Gtk::Grid compression_results_box;
    Gtk::Box best_compression_ratio_box;
    Gtk::Box worst_compression_ratio_box;
    Gtk::Box best_compression_speed_box;
    Gtk::Box worst_compression_speed_box;


private:
    void apply_css() {
        auto css = R"(
            .container {
                padding: 1rem;
                border: 2px solid lightgray;
                border-radius: 1rem;
            }
            .picture {
                transition: transform ease-in 0.2s;
            }
            .picture:hover {
                transform: scale(1.5);
                z-index: 2;
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



