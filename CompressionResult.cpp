#include <string>
#include "CompressionResult.h"
/**
 * Klasa koja enkapsulira trajanje kompresije i omjer kompresije
 */

CompressionResult::CompressionResult(unsigned long compressed_size, long long duration, double compression_speed, double compression_ratio, std::string parameters, std::string filename)
    : compressed_size(compressed_size) ,duration(duration), compression_speed(compression_speed), compression_ratio(compression_ratio), parameters(parameters), filename(filename) {};

long long CompressionResult::get_duration() const {
    return duration;
}
double CompressionResult::get_compression_speed() const {
    return compression_speed;
}

double CompressionResult::get_compression_ratio() const  {
    return compression_ratio;
}
unsigned long CompressionResult::get_compressed_size() const {
    return compressed_size;
}
std::string CompressionResult::get_parameters() {
    return parameters;
}
std::string CompressionResult::get_filename() {
    return filename;
}
