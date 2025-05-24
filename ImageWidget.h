#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/picture.h>

class ImageWidget : public Gtk::Box{
public:
    ImageWidget(std::string filename, unsigned long long filesize);

    // about
    std::string filename;
    unsigned long long filesize;

    //basic
    Gtk::Label filename_label;
    Gtk::Label file_size_label;
    Gtk::Picture picture;

    //best results
    Gtk::Label best_time;
    Gtk::Label best_compressed_size;
    Gtk::Label best_compression_ratio;
    //best parameters
    Gtk::Label best_compression_level;
    Gtk::Label best_compression_window_size;
    Gtk::Label best_compression_method;
    Gtk::Label best_compression_memory_level;
    Gtk::Label best_compression_strategy;

    //worst results
    Gtk::Label worst_time;
    Gtk::Label worst_compressed_size;
    Gtk::Label worst_compression_ratio;
    //worst parameters
    Gtk::Label worst_compression_level;
    Gtk::Label worst_compression_window_size;
    Gtk::Label worst_compression_method;
    Gtk::Label worst_compression_memory_level;
    Gtk::Label worst_compression_strategy;

    // boxes for layout
    Gtk::Box image_box;
    Gtk::Box info_box;
    Gtk::Box compression_results_box;
    Gtk::Box best_compression_box;
    Gtk::Box worst_compression_box;

};



