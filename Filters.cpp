#include <cstdint>
#include <iostream>
#include <cmath>
#include "Filters.h"

using namespace std;

/**
 * Metoda koja ne radi filtriranje bajtova tj. vrijednosti pixela svakog channela
 *
 * Metoda prima redak B_channela, redak G_channela i redak R_channela
 * izlazna linija "none_line" je konkatenacija bajtova svakog kanala posloženo u oblik:
 * 1_px_B_value, 2_px_B_value... 1_px_G_value, 2_px_G_value... 1_px_R_value, 2_px_R_value...  svaki kanal do širine slike "width"
 */
Bytef * Filters::none_filter(const Bytef *B_channel_line, const Bytef *G_channel_line, const Bytef *R_channel_line,int width) {
    Bytef *none_line = (Bytef *) malloc(width * 3 * sizeof(Bytef));

    // konkateniraj u izlaznu liniju BBB...GGG...RRR... svaki kanal koliko je široka slika
    for (int i = 0; i < width; i++) {
        none_line[i] = B_channel_line[i];
        none_line[i + width] = G_channel_line[i];
        none_line[i + 2*width] = R_channel_line[i];
    }
    return none_line;
}

/**
 * Metoda koja primjenjuje SUB filter nad svakim bajtom svakog od BGR channela
 *
 * Metoda prima redak slike razdvojen u B_channel_line, G_channel_line i R_channel_line
 * Svaka od tih linija je širine koliko je široka slika
 *
 * Metoda prvo na svaki kanal dodaje nulti element tj. zero padding
 * Nakon toga se za svaki channel izračuna operacija SUB
 * Te na kraju slijedi konkatenacija sva 3 filtrirana channela u jednu izlazni liniju
 */
Bytef * Filters::sub_filter(Bytef *B_channel_line, Bytef *G_channel_line, Bytef *R_channel_line, int width, int height) {
    Bytef* sub_line = (Bytef*) malloc(width*3*sizeof(Bytef));

    // prvo dodaj lijevo 0 i pripremi liniju za izvođenje filtriranja
    // u for petlji za svaki kanal izracunaj sub i napravi "liniju" kao "B_channel+G_channel+R_channel"
    Bytef *B_channel_line_to_filter = (Bytef*) malloc((width + 1)*sizeof(Bytef));
    Bytef *G_channel_line_to_filter = (Bytef*) malloc((width + 1)*sizeof(Bytef));
    Bytef *R_channel_line_to_filter = (Bytef*) malloc((width + 1)*sizeof(Bytef));
    B_channel_line_to_filter[0] = 0x00;
    G_channel_line_to_filter[0] = 0x00;
    R_channel_line_to_filter[0] = 0x00;
    for (int j = 1; j <= width; j++) {
        B_channel_line_to_filter[j] = B_channel_line[j-1];
        G_channel_line_to_filter[j] = G_channel_line[j-1];
        R_channel_line_to_filter[j] = R_channel_line[j-1];
    }

    Bytef *B_channel_line_result = (Bytef *)malloc(width * sizeof(Bytef));
    Bytef *G_channel_line_result = (Bytef *)malloc(width * sizeof(Bytef));
    Bytef *R_channel_line_result = (Bytef *)malloc(width * sizeof(Bytef));
    // izvođenje filtriranja
    for (int i = 1; i <= width; i++) {
        B_channel_line_result[i-1] = (B_channel_line_to_filter[i] - B_channel_line_to_filter[i-1]) % 256;
        G_channel_line_result[i-1] = (G_channel_line_to_filter[i] - G_channel_line_to_filter[i-1]) % 256;
        R_channel_line_result[i-1] = (R_channel_line_to_filter[i] - R_channel_line_to_filter[i-1]) % 256;
    }

    // konkateniraj u izlaznu liniju B-G-R na koju će se primjenjivati heuristika
    for (int i = 0; i < width; i++) {
        sub_line[width*i] = B_channel_line_result[i]; // samo od [i] ?
        sub_line[width*i + 1] = G_channel_line_result[i];
        sub_line[width*i + 2] = R_channel_line_result[i];
    }

    free(B_channel_line_to_filter);
    free(G_channel_line_to_filter);
    free(R_channel_line_to_filter);
    free(B_channel_line_result);
    free(G_channel_line_result);
    free(R_channel_line_result);
    return sub_line;
}

/**
 * Metoda prima cijele kanale jer mora gledat i gornji redak za UP operaciju te indeks trenutnog retka za koji se izvodi filter
 *
 * Ukoliko je prvi redak simulira se da je gornji redak null redak
 * Ukoliko je redak > 0 u originalnoj slici, tada se dohvaća prethodni/gornji redak da bi se izvela UP operacija
 *
 * Na kraju se rezultat sprema u jednu liniju tipa BBB...GGG...RRR
 */
Bytef * Filters::up_filter(Bytef *B_channel, Bytef *G_channel, Bytef *R_channel, int width, int height, int current_row) {

    Bytef *current_line = (Bytef *) malloc(width*3*sizeof(Bytef));
    for (int i = 0; i < width; i++) {
        current_line[i] = B_channel[i + current_row*width];
        current_line[i + 1*width] = G_channel[i + current_row*width];
        current_line[i + 2*width] = R_channel[i + current_row*width];
    }

    Bytef *up_row = (Bytef *) malloc(width*3*sizeof(Bytef));
    // ako je prvi redak onda dodajemo gornju null redak za filtriranje
    // tj. samo vratimo konkateniranu B-G-R liniju
    if (current_row == 0) {
        return current_line;

    }else {
        // inače dohvati redak prije za filtriranje
        for (int i = 0; i < width; i++) {
            up_row[i] = B_channel[i+current_row*width - current_row*width];
            up_row[i + 1*width] = G_channel[i+current_row*width - current_row*width];
            up_row[i + 2*width] = R_channel[i+current_row*width - current_row*width];
        }

        // trenutna linija konkatenirana B-G-R prije filtriranja je up_line
        // filtriranje nad cijelom B-G-R linijom
        for (int i = 0; i < width*3; i++) {
            current_line[i] = (current_line[i] - up_row[i]) % 256;
        }
    }

    free(up_row);
    return current_line;
}

/*
 * Metoda prima cijele BGR kanale te indeks trenutnog retka za koji se izvodi filtriranje
 *
 * Na početku se alocira prostor za izlaznu liniju koja će biti konkatenacija filtriranih BGR kanala trenutnog retka
 *
 * Ukoliko je nulti redak, simulira se da je gornji redak null redak
 * Ukoliko je redak > 0 koji se filtrira iz originalne slike, potražimo gornji redak iz channela i izvršimo AVG operaciju
 */
Bytef * Filters::avg_filter(Bytef * B_channel, Bytef * G_channel, Bytef * R_channel, int width, int height, int current_row) {

    Bytef *current_line = (Bytef *) malloc(width*3*sizeof(Bytef));
    for (int i = 0; i < width; i++) {
        current_line[i] = B_channel[i + current_row*width];
        current_line[i + 1*width] = G_channel[i + current_row*width];
        current_line[i + 2*width] = R_channel[i + current_row*width];
    }

    Bytef *return_line = (Bytef *) malloc(width*3*sizeof(Bytef));
    if (current_row == 0) {
        // up row = nul redak
        Bytef up_element = 0x00;
        for (int i = 0; i < width*3; i++) {
            // ako smo na početku channela onda je lijevi element 0 i gornji 0
            if (i % width == 0) {
                return_line[i] = current_line[i];
                continue;
            }
            return_line[i] = (current_line[i] - (current_line[i-1])/2) % 256;
        }
    }else {
        Bytef *up_row = (Bytef *) malloc(width*3*sizeof(Bytef));
        // kada je current_row > 0
        for (int i = 0; i < width; i++) {
            up_row[i] = B_channel[i+current_row*width - current_row*width];
            up_row[i + 1*width] = G_channel[i+current_row*width - current_row*width];
            up_row[i + 2*width] = R_channel[i+current_row*width - current_row*width];
        }
        // primjena filtriranja
        Bytef up_element = 0x00;
        for (int i = 0; i < width*3; i++) {
            up_element = up_row[i];
            // ako smo na početku channela onda je lijevi element 0
            if (i % width == 0) {
                return_line[i] = (current_line[i] - (up_element/2)) % 256;
                continue;
            }
            return_line[i] = (current_line[i] - (current_line[i-1]+up_element)/2) % 256;
        }
        free(up_row);
    }
    return return_line;
}

/*
 * Pomoćna metoda za Paeth koja računa minimum elemenata koji se uzimaju u obzir pri filtriranju
 * Pogledajte konkretno u metodi paeth_filter gdje se koristi...
 */
Bytef Filters::minimum(Bytef vl, Bytef vu, Bytef vul) {
    return min(min(vl,vu),vul);
}

/**
 * Metoda prima B G R cijele kanale slike jer promatra gornji redak te lijevi i gornji lijevi eleemnt prilikom filtriranja
 * Još prima parametar current_row koji govodi koji se trenutno redak filtrira
 *
 * Metoda prvo incijalizira liniju da poreda bajtove channela u redoslijed BBB...GGG...RRR
 * Ako je nulti redak slike za filtriranje simuliramo gornji redak kao null redak i računamo izlaznu liniju s primjenom PAETH operacije filtriranja
 * Ako je redak > 0, potražimo gornji redak iz channela da bi izveli PAETH operaciju filtriranja
 *
 * Vratimo filtriranu liniju oblika BBB...GGG...RRR
 */
Bytef * Filters::paeth_filter(Bytef * B_channel, Bytef * G_channel, Bytef * R_channel, int width, int height, int current_row) {

    Bytef *current_line = (Bytef *) malloc(width*3*sizeof(Bytef));
    for (int i = 0; i < width; i++) {
        current_line[i] = B_channel[i + current_row*width];
        current_line[i + 1*width] = G_channel[i + current_row*width];
        current_line[i + 2*width] = R_channel[i + current_row*width];
    }

    Bytef *return_line = (Bytef *) malloc(width*3*sizeof(Bytef));
    if (current_row == 0) {
        // up row = nul redak i up-left element = 0
        Bytef up_element = 0x00;
        Bytef up_left_element = 0x00;
        Bytef v,vl, vu, vul;
        for (int i = 0; i < width*3; i++) {
            // ako smo na početku channela onda je lijevi element 0 i gornji 0 i gornji lijevi isto 0
            // pa je rezultat operacije x-0= x
            if (i % width == 0) {
                return_line[i] = current_line[i];
                continue;
            }
            // inače
            v = current_line[i-1];  // V = U + L - UL, U,UL = 0
            vl = v - current_line[i-1]; // VL = V - L;
            vu = v - up_element; // VU = V - U
            vul = v - up_left_element; // VUL = V - UL
            Bytef vmin = minimum(vl, vu, vul);
            return_line[i] = (current_line[i] - vmin) % 256;
        }
    }else {
        Bytef *up_row = (Bytef *) malloc(width*3*sizeof(Bytef));
        // kada je current_row > 0
        for (int i = 0; i < width; i++) {
            up_row[i] = B_channel[i+current_row*width - current_row*width];
            up_row[i + 1*width] = G_channel[i+current_row*width - current_row*width];
            up_row[i + 2*width] = R_channel[i+current_row*width - current_row*width];
        }
        // primjena filtriranja
        Bytef up_element = 0x00;
        Bytef up_left_element = 0x00;
        Bytef v,vl, vu, vul;
        for (int i = 0; i < width*3; i++) {
            up_element = up_row[i];
            // ako smo na početku channela onda je lijevi element 0
            if (i % width == 0) {
                up_left_element = 0x00;
                v = up_element + 0 - up_left_element;
                vl = v - 0;
                vu = v - up_element;
                vul = v - up_left_element;
                Bytef vmin = minimum(vl, vu, vul);
                return_line[i] = (current_line[i] - vmin) % 256;
                continue;
            }
            up_left_element = up_row[i-1];
            v = up_element + current_line[i-1] - up_left_element;  // up element = up_row[i]
            vl = v - current_line[i-1];
            vu = v - up_element;
            vul = v - up_left_element;
            Bytef vmin = minimum(vl, vu, vul);
            return_line[i] = (current_line[i] - vmin) % 256;
        }
        free(up_row);
    }
    return  return_line;
}

/**
 * Metoda služi kao predprocesiranje redaka prije heuristike
 * Mapira vrijednosti bajtova koji su 128+ na negativne vrijednosti -> vrijednostBajta - 256
 */
void Filters::remap_lines(Bytef * none_line, Bytef * sub_line, Bytef * up_line, Bytef * avg_line, Bytef * paeth_line, int width) {
    for (int i = 0; i < width; i++) {
        if (none_line[i] >= 128) {
            none_line[i] = none_line[i] - 256;
        }
        if (sub_line[i] >= 128) {
            sub_line[i] = sub_line[i] - 256;
        }
        if (up_line[i] >= 128) {
            up_line[i] = up_line[i] - 256;
        }
        if (avg_line[i] >= 128) {
            avg_line[i] = avg_line[i] - 256;
        }
        if (paeth_line[i] >= 128) {
            paeth_line[i] = paeth_line[i] - 256;
        }
    }
}
