/**
 * Klasa koja enkapsulira trajanje kompresije i omjer kompresije
 */
class CompressionResult {
    double duration;
    double compression_ratio;

public:
    CompressionResult(long long duration, double compression_ratio) : duration(duration), compression_ratio(compression_ratio) {};
    double get_duration() const {
        return duration;
    }
    double get_compression_ratio() const {
        return compression_ratio;
    }
};