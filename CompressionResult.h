
class CompressionResult {
    unsigned long compressed_size;
    long long duration;
    double compression_ratio;
    std::string parameters;
    std::string filename;
public:
    CompressionResult(unsigned long compressed_size, long long duration, double compression_ratio, std::string parameters, std::string filename);
    unsigned long get_compressed_size() const;
    long long get_duration() const;
    double get_compression_ratio() const;
    std::string get_parameters();
    std::string get_filename();

};
