#include <cstdint>
#include <vector>
#include <cmath>
#include <iostream>
#include "Heuristics.h"

#include <map>

#include "Filters.h"

using namespace std;

FILTER_TYPE Heuristics::getFilterType(int min_key) {
    switch (min_key) {
        case 0:
            return FILTER_TYPE::None;
        case 1:
            return FILTER_TYPE::Sub;
        case 2:
            return FILTER_TYPE::Up;
        case 3:
            return FILTER_TYPE::Average;
        case 4:
            return FILTER_TYPE::Paeth;
        default:
            return FILTER_TYPE::None;
    }
}


/**
 * Metoda prima redak u svim oblicima filtriranja
 * Provjerava heuristiku minimalne sume i vraća koji tip filtra je najbolji te samu liniju
 */
BestLine Heuristics::apply_heuristic( Bytef * none_line,  Bytef * sub_line,  Bytef * up_line,  Bytef * avg_line,  Bytef * paeth_line, int width) {
    if (none_line == nullptr || sub_line == nullptr || up_line == nullptr || avg_line == nullptr || paeth_line == nullptr) {
        cerr << "Illegal input parameters - there is null line reference in method apply_heuristic" << endl;
    }

    // remapiraj vrijednosti px da bi se primjenila heuristika
    char* none_line_copy = static_cast<char *>(malloc(width * 3 * sizeof(char)));
    Filters::remap_line(none_line, none_line_copy, width);

    char* sub_line_copy = static_cast<char *>(malloc(width * 3 * sizeof(char)));
    Filters::remap_line(sub_line, sub_line_copy, width);

    char* up_line_copy = static_cast<char *>(malloc(width * 3 * sizeof(char)));
    Filters::remap_line(up_line, up_line_copy, width);

    char* avg_line_copy = static_cast<char *>(malloc(width * 3 * sizeof(char)));
    Filters::remap_line(avg_line, avg_line_copy, width);

    char* paeth_line_copy = static_cast<char *>(malloc(width * 3 * sizeof(char)));
    Filters::remap_line(paeth_line, paeth_line_copy, width);

    unsigned long long sum_none_line = 0;
    unsigned long long sum_sub_line = 0;
    unsigned long long sum_up_line = 0;
    unsigned long long sum_avg_line = 0;
    unsigned long long sum_paeth_line = 0;

    std::map<int, unsigned long long> mapa_suma = {
        {0, sum_none_line},
        {1, sum_sub_line},
        {2, sum_up_line},
        {3, sum_avg_line},
        {4, sum_paeth_line}
    };

    for (int i = 0; i < width*3; i++) {
        sum_none_line += abs(none_line_copy[i]);
        sum_sub_line += abs(sub_line_copy[i]);
        sum_up_line += abs(up_line_copy[i]);
        sum_avg_line += abs(avg_line_copy[i]);
        sum_paeth_line += abs(paeth_line_copy[i]);
    }

    // odaberi najmanju sumu
    BestLine best_line = {};
    int min_key = -1;
    unsigned long long min_value = LONG_LONG_MAX;
    for (const auto& kv : mapa_suma) {
        if (kv.second < min_value) {
            min_value = kv.second;
            min_key = kv.first;
        }
    }
    best_line.type = getFilterType(min_key);
    switch (best_line.type) {
        case FILTER_TYPE::None:
            best_line.line = none_line;
            free(sub_line);
            free(up_line);
            free(avg_line);
            free(paeth_line);
        break;
        case FILTER_TYPE::Sub:
            best_line.line = sub_line;
            free(none_line);
            free(up_line);
            free(avg_line);
            free(paeth_line);
        break;
        case FILTER_TYPE::Up:
            best_line.line = up_line;
            free(none_line);
            free(sub_line);
            free(avg_line);
            free(paeth_line);
        break;
        case FILTER_TYPE::Average:
            best_line.line = avg_line;
            free(none_line);
            free(sub_line);
            free(up_line);
            free(paeth_line);
        break;
        case FILTER_TYPE::Paeth:
            best_line.line = paeth_line;
            free(none_line);
            free(sub_line);
            free(up_line);
            free(avg_line);
        break;
        default:
            best_line.type = FILTER_TYPE::None;
            free(sub_line);
            free(up_line);
            free(avg_line);
            free(paeth_line);
        break;
    }

    free(none_line_copy);
    free(sub_line_copy);
    free(up_line_copy);
    free(avg_line_copy);
    free(paeth_line_copy);
    return best_line;
}