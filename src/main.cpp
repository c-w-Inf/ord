#include <iostream>

#include "ord.h"

int main (int argc, char** argv) {
    size_t bound = 100;
    if (argc == 2) {
        try {
            bound = std::stoull (argv[1], nullptr, 10);
        } catch (...) {
        }
    }

    ord::ordinal o;
    do {
        std::cout << o.std () << " [" << o.complexity () << "] \\\\" << std::endl;
    } while (o.to_next (bound));

    return 0;
}
