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
void test();

int main(int argc, char* argv[]) {
	signal(SIGINT, sigintHandler);

    initscr();
	test();
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
	// MAYBE LEFT CLICK TO GO INTO???
	//ADD ACTULL VALUE COUNTER AND TRY TO FIX THE NULL, MAYBE MAKE A PROPER THING??
	//Num of elements cause bug becayse you can't see at bottom, so is replaced with null.
	Metadata metadata(path);
	auto dict = metadata.GetDict();
	size_t num_of_elems = dict.size();
	size_t selected_index = 0;
	size_t offset = 0;
	int row, col;
	bool editing = false;
	std::string editing_name = "";
	int editing_field = 0;

	std::vector<int> drop_indices;

	for (size_t i = 0; i < num_of_elems; ++i) {
		drop_indices.push_back(i);
	}

	size_t top_drop_index = 0;
	int total_subtracts = 0;
	int size = 0;
	while (true) {
		getmaxyx(stdscr, row, col);

		size_t actual_values = 0;


		if (editing){
			printw("Editing %s\n", dict[editing_field].name.c_str());
			
			for (auto& field : dict[editing_field].fields) {
				if (editing_name == field.first) {
					
					curs_set(1);
					printw("  %s: ", field.first.c_str());

					std::visit([&](auto&& value) {
						using T = std::decay_t<decltype(value)>;
						if constexpr (std::is_same_v<T, std::string>) {
							printw("%s", value.c_str());
							move(1, value.length() - total_subtracts + 3 + field.first.length());
							size = value.length();
						} else if constexpr (std::is_same_v<T, int>) {
							printw("%d", value);
							move(1, std::to_string(value).length() - total_subtracts + 3 + field.first.length());
							size = std::to_string(value).length();
						} else if constexpr (std::is_same_v<T, double>) {
							printw("%f", value);
							move(1, std::to_string(value).length() - total_subtracts + 3 + field.first.length());
							size = std::to_string(value).length();
						}
					}, field.second);

					}		
				} 

			char ch = getch();

			if (ch == 10) {
				editing = false;
				curs_set(0);
			} 
			else if (ch == char(KEY_LEFT)) {
				if (total_subtracts < size){
					total_subtracts++;
				}
			}
			else if (ch == char(KEY_RIGHT)) {
				if (total_subtracts > 0){
					total_subtracts--;
				}
			}
			
		}
		else{
			for (size_t i = 0; i < row; ++i) {

				top_drop_index = getTopVisibleDropdownIndex(dict, offset, row);

				/*if (top_drop_index > 0) {
					top_drop_index = top_drop_index - 1;
					if (dict[top_drop_index].expanded) {
					for (auto& field : dict[top_drop_index].fields) {
						i += 1;
						if (i < row){
							printw("  %s: ", field.first.c_str());
							attron(COLOR_PAIR(1));
							if (std::holds_alternative<std::string>(field.second)) {
								printw("%s\n", std::get<std::string>(field.second).c_str());
							} else if (std::holds_alternative<int>(field.second)) {
								printw("%d\n", std::get<int>(field.second));
							} else if (std::holds_alternative<double>(field.second)) {
								printw("%f\n", std::get<double>(field.second));
							}
							attroff(COLOR_PAIR(1));
						}
					}
				}
				}*/

				if (i + offset < num_of_elems) {

					if (i + offset == selected_index) {
						attron(COLOR_PAIR(2));
						if (dict[actual_values + offset].expanded) {
							printw("v %s\n", dict[actual_values + offset].name.c_str());
						} else {
							printw("> %s\n", dict[actual_values + offset].name.c_str());
						}
						attroff(COLOR_PAIR(2));

						if (dict[actual_values + offset].expanded) {

							for (auto& field : dict[actual_values + offset].fields) {
								i += 1;
								if (i < row){
									
									printw("  %s: ", field.first.c_str());
									attron(COLOR_PAIR(1));
									if (std::holds_alternative<std::string>(field.second)) {
										printw("%s\n", std::get<std::string>(field.second).c_str());
									} else if (std::holds_alternative<int>(field.second)) {
										printw("%d\n", std::get<int>(field.second));
									} else if (std::holds_alternative<double>(field.second)) {
										printw("%f\n", std::get<double>(field.second));
									}
									attroff(COLOR_PAIR(1));
								}
							}
						}
					} else {
						if (dict[actual_values + offset].expanded) {
							printw("v %s\n", dict[actual_values + offset].name.c_str());

							for (auto& field : dict[actual_values + offset].fields) {
								i += 1;

								if (i < row){
									
									if (i + offset == selected_index) {
										attron(COLOR_PAIR(2));
										editing_name = field.first;
										editing_field = actual_values;
									}
									printw("  %s: ", field.first.c_str());
									attroff(COLOR_PAIR(2));

									attron(COLOR_PAIR(1));
									if (std::holds_alternative<std::string>(field.second)) {
										printw("%s\n", std::get<std::string>(field.second).c_str());
									} else if (std::holds_alternative<int>(field.second)) {
										printw("%d\n", std::get<int>(field.second));
									} else if (std::holds_alternative<double>(field.second)) {
										printw("%f\n", std::get<double>(field.second));
									}
									attroff(COLOR_PAIR(1));
								}
							}
						} else {
							printw("> %s\n", dict[actual_values + offset].name.c_str());
						}
					}
					actual_values ++;
				}
			}
		}

		refresh();

		if (!editing){
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
			editing = true;
				for (int i = 0; i < drop_indices.size(); ++i) {
					if (drop_indices[i] == selected_index) {
						if (dict[i].expanded) {
							dict[i].expanded = false;
							int sizeof_fields = dict[i].fields.size();
							num_of_elems = num_of_elems - sizeof_fields;
							for (int j = 1; j < drop_indices.size() - i; ++j) {
								drop_indices[j + i] = drop_indices[j + i] - sizeof_fields;
							}
							editing = false;
							break;
						} else {
							dict[i].expanded = true;
							int sizeof_fields = dict[i].fields.size();
							num_of_elems = num_of_elems + sizeof_fields;
							for (int j = 1; j < drop_indices.size() - i; ++j) {
								drop_indices[j + i] = drop_indices[j + i] + sizeof_fields;
							}
							editing = false;
							break;
						}
					}
				}
		

			} else if (ch == '~') {
				break; //REMEMBER TO REMOVE THIS LINE
			}
		}

		clear();
		exit(0);

	}

	clear();
	printw("Drop indices size: %zu\n", drop_indices.size());
	for (int i = 0; i < drop_indices.size(); ++i) {
		printw("%d\n", drop_indices[i]);
	}
	printw("Press any key to exit.");
	printw("Number of elements: %zu\n", num_of_elems);
	printw("Selected index: %zu\n", selected_index);
	printw("Offset: %zu\n", offset);
	printw("Top drop index: %zu\n", top_drop_index);
	refresh();
	char hi = getch();
}
