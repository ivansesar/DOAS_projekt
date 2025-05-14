typedef unsigned char Byte;
typedef Byte Bytef;
/**
* Govori koji tip filtra se koristi za svaki redak slike
 */
enum FILTER_TYPE {
    None,Sub,Up,Average,Paeth
};

/**
 * Rezultantna struktura koju daje primjena heuristike, sastoji se od tipa filtirranja retka i samog retka
 */
typedef struct {
    FILTER_TYPE type;
    const Bytef *line;
} BestLine;

class Heuristics {
public:
    static FILTER_TYPE getFilterType(int min_key);

    static BestLine apply_heuristic(const Bytef * none_line, const Bytef * sub_line, const Bytef * up_line, const Bytef * avg_line, const Bytef * paeth_line, int width);

};