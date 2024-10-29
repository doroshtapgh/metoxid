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
#include <exiv2/exiv2.hpp>

#else
#include <ncurses/ncurses.h>
#endif

#include <stdarg.h>
#include <signal.h>
#include <filesystem>
#include <vector>
#include <iostream>

std::vector<std::filesystem::path> list_directory(const std::filesystem::path& dir);
void browse_directory(const std::filesystem::path& dir);
void edit_file(const std::filesystem::path& path);
void fatal_error(const char* fmt, ...);
void sigint_handler(int dummy);

int main(int argc, char* argv[]) {
	signal(SIGINT, sigint_handler);

    initscr();
	keypad(stdscr, TRUE);
	curs_set(0);

	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_WHITE);
	}
	
	if (argc == 1) {
		browse_directory(std::filesystem::current_path());
	} else if (argc == 2) {
		std::filesystem::path path = std::filesystem::absolute(argv[1]);

		if (std::filesystem::exists(path)) {
			if (std::filesystem::is_regular_file(path)) {
				edit_file(path);
                fatal_error("%s Will Edit Now", argv[1]);
			} else if (std::filesystem::is_directory(path)) {
				browse_directory(path);
			} else {
				fatal_error("%s is not a file or a directory.", argv[1]);
			}
		} else {
			fatal_error("%s path doesn't exist.", argv[1]);
		}
	} else {
		fatal_error("you can pass only one command line argument at a time.");
	}

	endwin();

    return 0;
}

std::vector<std::filesystem::path> list_directory(const std::filesystem::path& dir) {
	std::vector<std::filesystem::path> contents;

	if (dir.has_parent_path()) {
		contents.push_back(dir / "..");
	}

	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		contents.push_back(entry.path());
	}

	return contents;
}

void browse_directory(const std::filesystem::path& dir) {
 
	auto contents = list_directory(dir);
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
				contents = list_directory(canonical_path);
				num_of_elems = contents.size();
			} else if (std::filesystem::is_regular_file(contents[selected_index])) {
				clear();
				edit_file(contents[selected_index]);
				curs_set(1);
				endwin();
				exit(0);
			}
		}
		
		clear();
	}
}

void edit_file(const std::filesystem::path& path) {
	

}


void fatal_error(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	
	if (has_colors()) {
		attron(COLOR_PAIR(1));
		printw("fatal error: ");
		attroff(COLOR_PAIR(1));
	}

	printw(fmt, args);
	
	va_end(args);

	printw("\npress any key to exit.");

	refresh();
	getch();
	endwin();
	exit(1);
}

void sigint_handler(int dummy) {
	curs_set(1);
	endwin();
	exit(1);
}


//g++ -o main src/main.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -lncurses