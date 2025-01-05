#include <iostream>

#include "clicker.h"
#include "recorder.h"

void get_mode() {
    std::cout << "Made with love by github.com/ponktacology" << std::endl;
    std::cout << std::endl << std::endl;
    std::cout << "Wybierz tryb (c/n):" << std::endl;
    char mode;
    std::cin >> mode;

    if (mode == 'c') {
        run_clicker();
    } else if (mode == 'n') {
        run_recorder();
    } else {
        get_mode();
    }
}

int main() {
    get_mode();
    return 0;
}
