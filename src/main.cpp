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
#include <optional>

void browseDirectory(const std::filesystem::path& dir);
void editFile(const std::filesystem::path& path);
void printEditingValueAndCursor(std::string value, int total_subtracts);

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
	Metadata metadata(path);
	auto dict = metadata.GetDict();
	size_t num_of_elems = dict.size();
	size_t selected_index = 0;
	size_t offset = 0;
	int row, col;
	bool editing = false;
	std::string editing_name = "";
	std::string temp = "";
	int editing_field = 0;
	bool exiv_value = false;
	char prev_char = 'a';

	std::vector<int> drop_indices;
	
	//for(size_t i = 0; i < dict.size(); i++){
	//	printw("%s\n", dict[i].name.c_str());
		
	//}
	
	
	for (size_t i = 0; i < num_of_elems; ++i) {
		drop_indices.push_back(i);
	}

	int total_subtracts = 0;
	int size = 0;
	std::string editing_data = "";

	
	int non_catagory_offest = 0;
	int curr_index = 0;

	while (true) {
		size_t actual_values = 0;
		getmaxyx(stdscr, row, col);
		
			for (size_t i = 0; i < row; ++i) {
				int top_down_increament = 0;
			
				if (i == 0){ //EDITING DOES NOT WORK FOR THESE VAKUES
				
					for (size_t j = 0; j < drop_indices.size(); ++j) {
						if (drop_indices[j] == offset) {
							break;
						} 
						if (drop_indices[j] > offset) {
							top_down_increament = drop_indices[j-1]+1;
							for (auto& field : dict[j-1].fields) {
								
								if (top_down_increament == offset){
									i += 1;
									if (i > row){
										break;
									}					
									if (i + offset == selected_index+1) {
										attron(COLOR_PAIR(2));
										editing_name = field.first;
										editing_field = j-1;
									}
									printw("  %s:", field.first.c_str());
									attroff(COLOR_PAIR(2));
									
									if (editing_name == field.first && editing == true) {
										attron(COLOR_PAIR(1));

										std::visit([&](auto&& value) {
											using T = std::decay_t<decltype(value)>;
											if constexpr (std::is_same_v<T, std::string>) {
												
												printEditingValueAndCursor(value,total_subtracts);

													
												
												editing_data = value;
												size = value.size();
												
											}
											else if constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){ //must copy paste
												temp = value.get().toString().c_str();
												printEditingValueAndCursor(temp,total_subtracts);

													
												
												editing_data = temp;
												size = temp.length();
											}
									
											
										}, field.second);
										attroff(COLOR_PAIR(1));
										printw("\n");
									} 
									else {
										attron(COLOR_PAIR(1));
										std::visit([&](auto&& value){
											using T = std::decay_t<decltype(value)>;
											if constexpr(std::is_same_v<T, std::string>){
												printw(" %s\n", value.c_str());

											}
											else if  constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
												printw(" %s\n", value.get().toString().c_str());
												
											}
										}, field.second);
										attroff(COLOR_PAIR(1));
									}
								}else{
									top_down_increament++;
								}
							}
							break;
						}
					}
				}

				if (i + offset < num_of_elems && i < row) {

					curr_index = actual_values + offset - non_catagory_offest;

					if (i + offset == selected_index) {
						attron(COLOR_PAIR(2));
						if (dict[curr_index].expanded) {
							printw("v %s\n", dict[curr_index].name.c_str());
						} else {
							printw("> %s\n", dict[curr_index].name.c_str());
						}
						attroff(COLOR_PAIR(2));

						if (dict[curr_index].expanded) {
								
							for (auto& field : dict[curr_index].fields) {
								i += 1;
								if (i < row){
									
									printw("  %s:", field.first.c_str());
									attron(COLOR_PAIR(1));
									
									std::visit([&](auto&& value){
										using T = std::decay_t<decltype(value)>;
										if constexpr(std::is_same_v<T, std::string>){
											printw(" %s\n", value.c_str());

										}
										else if  constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
											printw(" %s\n", value.get().toString().c_str());
											
										}
									}, field.second);
									
									

									attroff(COLOR_PAIR(1));
								}
							}
						}
					} else {
						if (dict[curr_index].expanded) {
							printw("v %s\n", dict[curr_index].name.c_str());
							
							for (auto& field : dict[curr_index].fields) {
								i += 1;

								if (i < row){
									
									if (i + offset == selected_index) {
										attron(COLOR_PAIR(2));
										editing_name = field.first;
										editing_field = curr_index;
									}
									printw("  %s:", field.first.c_str());
									attroff(COLOR_PAIR(2));
									
									if (editing_name == field.first && editing == true) {
										attron(COLOR_PAIR(1));

										std::visit([&](auto&& value) {
											using T = std::decay_t<decltype(value)>;
											if constexpr (std::is_same_v<T, std::string>) {
												
												printEditingValueAndCursor(value,total_subtracts);
												
												editing_data = value;
												size = value.length();
												exiv_value = false;
												
												
											}
											else if constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){ //must copy paste
												temp = value.get().toString().c_str();
												printEditingValueAndCursor(temp,total_subtracts);

													
												
												editing_data = temp;
												size = temp.size();
												exiv_value = true;
											}
										}, field.second);
										attroff(COLOR_PAIR(1));
										printw("\n");
									} 
									else {
										attron(COLOR_PAIR(1));
										std::visit([&](auto&& value){
											using T = std::decay_t<decltype(value)>;
											if constexpr(std::is_same_v<T, std::string>){
												printw(" %s\n", value.c_str());

											}
											else if  constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
												printw(" %s\n", value.get().toString().c_str());
												
											}
										}, field.second);
										attroff(COLOR_PAIR(1));
									}
								}
							}
							
						} else {
							printw("> %s\n", dict[curr_index].name.c_str());
						}
					}
					actual_values ++;
				}
			}
		

		refresh();

		char ch = getch();
		if (!editing){
			
			if (ch == (char)KEY_UP) {
				
				if (selected_index > 0) {
					selected_index--;
					
					if (selected_index < offset) {
						offset--;

						bool found = false;
						for (size_t j = 0; j < drop_indices.size(); ++j) {
							if (drop_indices[j] == offset) {
								found = true;
								break;
							} 
						}
						if (!found) {
							non_catagory_offest -= 1;
						}
					}
				}
			} else if (ch == (char)KEY_DOWN) {
				if (selected_index + 1 < num_of_elems) {
					selected_index++;

					if (selected_index > offset + row - 1) {
						offset++;

						for (size_t j = 0; j < drop_indices.size(); ++j) {
							if (drop_indices[j] == offset-1) {
								break;
							} 
							if (drop_indices[j] > offset) {
								non_catagory_offest += 1;
								break;
							}
						}
					}
				}
			} else if (ch == 10) {
				total_subtracts = 0;
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
		else{
			refresh();

			if (ch == 10) {
				total_subtracts = 0;
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
			else{
				
				if (ch == 127){
					if (editing_data.length() != 0 && total_subtracts != editing_data.length()){
						if (total_subtracts == 0){
							editing_data.erase(editing_data.end() - 1);
						}
						else{
							editing_data.erase(editing_data.end() - total_subtracts);
						}
					}
				}
				else if(ch == '~'){
					break;
				}
				else{
					if (isalnum(ch) || ispunct(ch) || isspace(ch)){
						editing_data.insert(editing_data.end() - total_subtracts, ch);
					}
				}
				
				std::visit([&](auto&& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, std::string>) {
						value = editing_data;
						
					}
					else if constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
						
						
							
					}					
				}, dict[editing_field].fields[editing_name]);
			}
			

		}

		clear();
	}

	clear();
	printw("%d\n", total_subtracts);
	printw("%ld\n", editing_data.length());
	printw("%d\n", num_of_elems);
	printw("%s\n", editing_name.c_str());
	printw("%d\n", editing_field);
	printw("Press any key to exit.");
	printw("sadge :(");
	
	
	refresh();
	char hi = getch();
	
}

void printEditingValueAndCursor(std::string value, int total_subtracts) {
	printw(" ");
												
	for(int i = 0; i < value.length(); i++){
		char c = value[i];

		if (i == value.length() - total_subtracts){
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(2));
			printw("%c", c);
			
			attroff(COLOR_PAIR(2));
			attron(COLOR_PAIR(1));
		}
		else{
			printw("%c", c);
		}
		
	}
	if (total_subtracts == 0){
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(2));
		printw(" ");
		attroff(COLOR_PAIR(2));
		attron(COLOR_PAIR(1));
		
	}
}

