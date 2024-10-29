#include <metoxid.hpp>
#include <stdarg.h>
#if defined(METOXID_LINUX) || defined(METOXID_MACOS)
#include <ncurses.h>
#else
#include <ncursesw/ncurses.h>
#endif

void fatalError(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (has_colors()) {
        attron(COLOR_PAIR(1));
        printw("fatal error: ");
        attroff(COLOR_PAIR(1));
    } else {
        printw("fatal error: ");
    }

    printw(fmt, args);

    va_end(args);

    printw("\npress any key to exit.");

    refresh();
    getch();
    endwin();
    exit(1);
}

void sigintHandler(int dummy) {
    curs_set(1);
    endwin();
    exit(1);
}

std::vector<std::filesystem::path> listDirectory(const std::filesystem::path& dir) {
    std::vector<std::filesystem::path> contents;

	if (dir.has_parent_path()) {
		contents.push_back(dir / "..");
	}

	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		contents.push_back(entry.path());
	}

	return contents;
}
