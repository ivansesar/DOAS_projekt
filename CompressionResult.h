
class CompressionResult {
public:
    CompressionResult(double duration, double compression_ratio);
    double get_duration() const;
    double get_compression_ratio() const;
};
