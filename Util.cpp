#include <string>
#include <vector>
#include <sstream>
#include "Util.h"

std::string get_file_name(std::string path) {
    std::vector<std::string> paths;
    std::istringstream stream(path);
    std::string word;
    size_t start = 0, end;
    while ((end = path.find('\\', start)) != std::string::npos) {
        paths.push_back(path.substr(start, end - start));
        start = end + 1;
    }
    paths.push_back(path.substr(start));
    return paths.back();
}
