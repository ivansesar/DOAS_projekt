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
    Bytef *line;
} BestLine;

class Heuristics {
public:
    static FILTER_TYPE getFilterType(int min_key);

    static BestLine apply_heuristic( Bytef * none_line,  Bytef * sub_line,  Bytef * up_line,  Bytef * avg_line,  Bytef * paeth_line, int width);

};