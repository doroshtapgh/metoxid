#if defined(linux) || defined(__linux) || defined(__linux__)
#define LINUX_PLATFORM
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#define MACOS_PLATFORM
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define WINDOWS_PLATFORM
#else
#error "Unsupported target platform."
#endif

#if defined(LINUX_PLATFORM) || defined(MACOS_PLATFORM)
#include <ncurses.h>
#else
#include <ncursesw/ncurses.h>
#endif
#include <metoxid.hpp>
#include <stdarg.h>
#include <signal.h>
#include <filesystem>
#include <vector>
#include <iostream>

void browseDirectory(const std::filesystem::path& dir);
void editFile(const std::filesystem::path& path);

int main(int argc, char* argv[]) {
	signal(SIGINT, sigintHandler);

    initscr();
	keypad(stdscr, TRUE);
	curs_set(0);

	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_WHITE);
	}
	
	if (argc == 1) {
		browseDirectory(std::filesystem::current_path());
	} else if (argc == 2) {
		std::filesystem::path path = std::filesystem::absolute(argv[1]);

		if (std::filesystem::exists(path)) {
			if (std::filesystem::is_regular_file(path)) {
				editFile(path);
                fatalError("%s Will Edit Now", argv[1]);
			} else if (std::filesystem::is_directory(path)) {
				browseDirectory(path);
			} else {
				fatalError("%s is not a file or a directory.", argv[1]);
			}
		} else {
			fatalError("%s path doesn't exist.", argv[1]);
		}
	} else {
		fatalError("you can pass only one command line argument at a time.");
	}

	endwin();

    return 0;
}

void browseDirectory(const std::filesystem::path& dir) {
	auto contents = listDirectory(dir);
	size_t num_of_elems = contents.size();
	size_t selected_index = 0;
	size_t offset = 0;
	int row, col;

	while (true) {
		getmaxyx(stdscr, row, col);

		for (size_t i = 0; i < row; ++i) {
			if (i + offset < num_of_elems) {
				if (i + offset == selected_index) {
					attron(COLOR_PAIR(2));
					printw("%s\n", contents[i + offset].filename().c_str());
					attroff(COLOR_PAIR(2));
				} else {
					printw("%s\n", contents[i + offset].filename().c_str());
				}
			}
		}

		refresh();

		char ch = getch();

		if (ch == (char)KEY_UP) {
			if (selected_index > 0) {
				selected_index--;

				if (selected_index < offset) {
					offset--;
				}
			}
		} else if (ch == (char)KEY_DOWN) {
			if (selected_index + 1 < num_of_elems) {
				selected_index++;

				if (selected_index > offset + row - 1) {
					offset++;
				}
			}
		} else if (ch == 10) {
			if (std::filesystem::is_directory(contents[selected_index])) {
				const auto canonical_path = std::filesystem::canonical(contents[selected_index]);
				offset = 0;
				selected_index = 0;
				contents = listDirectory(canonical_path);
				num_of_elems = contents.size();
			} else if (std::filesystem::is_regular_file(contents[selected_index])) {
				clear();
				editFile(contents[selected_index]);
				curs_set(1);
				endwin();
				exit(0);
			}
		}
		
		clear();
	}
}

void editFile(const std::filesystem::path& path) {
	// NOTE: curs_set(0) is used in main function, use curs_set(1) during actual editing of a field's value
	Metadata metadata(path);
	auto dict = metadata.GetDict();

	int row, col;

	while (true) {
		getmaxyx(stdscr, row, col);
		refresh();

		getch();
		clear();
	}
}


//g++ -o main src/main.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -lncurses