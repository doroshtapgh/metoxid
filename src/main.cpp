#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    using namespace ftxui;

    Element document =
        hbox({
            text("left")   | border,
            text("middle") | border | flex,
            text("right")  | border,
        });

    auto screen = Screen::Create(
        Dimension::Full(),       // width
        Dimension::Fit(document) // height
    );

    Render(screen, document);
    screen.Print();

    return EXIT_SUCCESS;
}
