#include <ncurses.h>
#include <iostream>
#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <cassert>

using namespace std;

int main(int argc, char* const argv[]) { 
    // to do allow user to go through file system to pick
    if (argc != 2) { // User put not enough or too many arguments 
        cout << "Proper usage: " << argv[0] << " file path\n";
        return 1;
    }

    try {
        string file(argv[1]);
        Exiv2::Image::AutoPtr greg = Exiv2::ImageFactory::open(file);
        cout << "Hello world!" << endl;
    return 0;
        }
    catch (Exiv2::Error& e) {
        std::cerr << "Error: " << e << std::endl;
        return 1;
    }

    
    initscr();            // Start curses mode
    printw("Hello World !!!");    // Print Hello World
    refresh();            // Print it on to the real screen 
    getch();            // Wait for user input
    endwin();            // End curses mode
    

    return 0;
}

//g++ -std=c++11 -o main main.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -lncurses

//g++ -o main src/main.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -lncurses

//g++ -o main -std=c++14 src/main.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -lncurses

// Ensure the correct linkage to Exiv2 library
// g++ -o main -std=c++14 src/main.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -lncurses -lstdc++fs