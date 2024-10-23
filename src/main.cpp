#include <iostream>
#include <fstream>
#include <ncurses.h>
#include <exiv2/exiv2.hpp>
#include <gdal_priv.h>  // Include GDAL header

int main() {
    std::cout << "RUNNDING" << "\n";
    // Initialize GDAL
    GDALAllRegister();

    // Initialize the screen
    initscr();
    
    // Get the screen size
    int row, col;
    getmaxyx(stdscr, row, col);
    
    // Print "Hello, World!" at the center of the screen
    mvprintw(row / 2, (col - 12) / 2, "Hello, World!");
    
    // Refresh the screen to show the text
    refresh();
    
    // Wait for user input
    getch();
    
    // End the ncurses mode
    endwin();
    
    return 0;
}


//g++ myprog.cpp -o myprog 
//g++ -o main src/main.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -lncurses -lgdal