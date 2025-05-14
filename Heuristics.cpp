#include <cstdint>
#include <vector>
#include <cmath>
#include <iostream>
#include "Heuristics.h"

#include <map>

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
BestLine Heuristics::apply_heuristic(const Bytef * none_line, const Bytef * sub_line, const Bytef * up_line, const Bytef * avg_line, const Bytef * paeth_line, int width) {
    if (none_line == nullptr || sub_line == nullptr || up_line == nullptr || avg_line == nullptr || paeth_line == nullptr) {
        cerr << "Illegal input parameters - there is null line reference in method apply_heuristic" << endl;
    }
    unsigned long long sum_none_line = 0;
    unsigned long long sum_sub_line = 0;
    unsigned long long sum_up_line = 0;
    unsigned long long sum_avg_line = 0;
    unsigned long long sum_paeth_line = 0;
    for (int i = 0; i < width*3; i++) {
        sum_none_line += abs(none_line[i]);
        sum_sub_line += abs(sub_line[i]);
        sum_up_line += abs(up_line[i]);
        sum_avg_line += abs(avg_line[i]);
        sum_paeth_line += abs(paeth_line[i]);
    }

    std::map<int, unsigned long long> mapa_suma = {
        {0, sum_none_line},
        {1, sum_sub_line},
        {2, sum_up_line},
        {3, sum_avg_line},
        {4, sum_paeth_line}
    };

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
        break;
        case FILTER_TYPE::Sub:
            best_line.line = sub_line;
        break;
        case FILTER_TYPE::Up:
            best_line.line = up_line;
        break;
        case FILTER_TYPE::Average:
            best_line.line = avg_line;
        break;
        case FILTER_TYPE::Paeth:
            best_line.line = paeth_line;
        break;
        default:
            best_line.type = FILTER_TYPE::None;
        break;
    }
    return best_line;
}