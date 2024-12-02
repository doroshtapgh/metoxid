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
void printEditingValueAndCursor(std::string value, int total_subtracts, int& charstoleft, int col);
void printFieldName(std::string fieldname, int& charstoleft);
void printEditingFields(const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& total_subtracts, int& size, std::string& editing_data, std::string& temp, int& charstoleft, size_t& i, int row, int col);
void printRegularly(size_t i, int row, int col, const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& charstoleft);

int main(int argc, char* argv[]) {
	signal(SIGINT, sigintHandler); // Register the signal handler

    initscr();
	
	keypad(stdscr, TRUE);
	curs_set(0);

	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_BLACK, COLOR_WHITE);
	}
	
	if (argc == 1) { //nothing specified
		browseDirectory(std::filesystem::current_path());
	} else if (argc == 2) { //one argument specified
		std::filesystem::path path = std::filesystem::absolute(argv[1]);

		if (std::filesystem::exists(path)) { //check if the path exists and then do stuff with it
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
	auto contents = listDirectory(dir); //Get the contents of the directory
	size_t num_of_elems = contents.size(); //Number of elements in the directory
	size_t selected_index = 0; //Index of the selected file
	size_t offset = 0; //offset from top of the screen
	int row, col;

	while (true) {
		getmaxyx(stdscr, row, col); 

		for (size_t i = 0; i < row; ++i) {
			if (i + offset < num_of_elems) {
				if (i + offset == selected_index) { //makes the one you are on look cooler
					attron(COLOR_PAIR(2));
					printw("%s\n", contents[i + offset].filename().c_str());
					attroff(COLOR_PAIR(2));
				} else {
					printw("%s\n", contents[i + offset].filename().c_str());
				}
			}
		}

		refresh(); //refreshes the screen
		
		char ch = getch(); //waits for user input and store it

		if (ch == (char)KEY_UP) {
			if (selected_index > 0) {
				selected_index--;

				if (selected_index < offset) { //scrolls up if you are at the top
					offset--;
				}
			}
		} else if (ch == (char)KEY_DOWN) {
			if (selected_index + 1 < num_of_elems) {
				selected_index++;

				if (selected_index > offset + row - 1) { //scrolls down if you are at bottom
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
	Metadata metadata(path); // Load the metadata of the file
	auto dict = metadata.GetDict(); // Get the metadata dictionary
	size_t num_of_elems = dict.size(); //Gets number of metadata categories
	size_t selected_index = 0; //Index currently selected in terminal
	size_t offset = 0; //Top offset, for printing purposes
	int row, col; //Row and column of terminal
	bool editing = false; //Is the user editing a field
	std::string editing_name = ""; //Name of the field being edited
	std::string temp = ""; //Temporary string, ignore
	int editing_field = 0; //Index of the field being edited
	int charstoleft = 0; //Number of characters to the left of the terminal (for printing purposes)
	int total_subtracts = 0; //Number of characters user is from right of the string (for editing)
	int size = 0; //Size of the string being edited (in chars)
	std::string editing_data = ""; //Data being edited
	int non_catagory_offest = 0; //Spaces taken up in terminal by non-category things (aka fields)
	int curr_index = 0; //Index of the current field being printed
	std::vector<int> drop_indices; //List of the indices of all catagories
	
	for (size_t i = 0; i < num_of_elems; ++i) {
		drop_indices.push_back(i);
	}
	
	while (true) {
		size_t category_index = 0;
		getmaxyx(stdscr, row, col);
		
			for (size_t i = 0; i < row; ++i) {
				int top_down_increament = 0;
			
				if (i == 0){ 
					for (size_t j = 0; j < drop_indices.size(); ++j) {
						if (drop_indices[j] == offset) {
							break;
						} 
						if (drop_indices[j] > offset) {
							top_down_increament = drop_indices[j-1]+1;
							for (auto& field : dict[j-1].fields) {
								
								if (top_down_increament == offset){
									i += 1;
									charstoleft = 0;
									if (i > row){
										break;
									}					
									if (i + offset == selected_index+1) {
										attron(COLOR_PAIR(2));
										editing_name = field.first;
										editing_field = j-1;
									}

									printFieldName(field.first.c_str(), charstoleft);
									attroff(COLOR_PAIR(2));
									
									if (editing_name == field.first && editing == true) {
										printEditingFields(field, total_subtracts, field_size, editing_data, temp, charstoleft, i, row, col);
									} 
									else {
										printRegularly(i, row, col, field, charstoleft);
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

					category_index = offset - non_catagory_offest;

					if (i + offset == selected_index) {
						attron(COLOR_PAIR(2));
						if (dict[category_index].expanded) {
							printw("v %s\n", dict[category_index].name.c_str());
						} else {
							printw("> %s\n", dict[category_index].name.c_str());
						}
						attroff(COLOR_PAIR(2));

						if (dict[category_index].expanded) {
								
							for (auto& field : dict[category_index].fields) {
								i += 1;
								if (i < row){
									
									charstoleft = 0;
									printFieldName(field.first.c_str(), charstoleft);
									printRegularly(i, row, col, field, charstoleft);
								}
							}
						}
					} else {
						if (dict[category_index].expanded) {
							printw("v %s\n", dict[category_index].name.c_str());
							
							for (auto& field : dict[category_index].fields) {
								i += 1;
								charstoleft = 0;

								if (i < row){
									
									if (i + offset == selected_index) {
										attron(COLOR_PAIR(2));
										editing_name = field.first;
										editing_field = category_index;
									}

									printFieldName(field.first.c_str(), charstoleft);
									attroff(COLOR_PAIR(2));
									
									if (editing_name == field.first && editing == true) {
										printEditingFields(field, total_subtracts, field_size, editing_data, temp, charstoleft, i, row, col);
									} 
									else {
										printRegularly(i, row, col, field, charstoleft);
									}
								}
							}
							
						} else {
							printw("> %s\n", dict[category_index].name.c_str());
						}
					}
					category_index ++;
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
				if (total_subtracts < field_size){
					total_subtracts++;
				}
			}
			else if (ch == char(KEY_RIGHT)) {
				if (total_subtracts > 0){
					total_subtracts--;
				}
			}
			else if (ch == char(KEY_UP)){
				if (total_subtracts + col > field_size){
					total_subtracts = field_size;
				}
				else{
					total_subtracts += col;
				}
			}
			else if (ch == char(KEY_DOWN)){
				if (total_subtracts - col < 0){
					total_subtracts = 0;
				}
				else{
					total_subtracts -= col;
				}
			}
			else if (ch == '~') {
				break;
			}
			else{
				
				if (ch == char(KEY_BACKSPACE)){
					
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
					else if constexpr (std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
						const_cast<Exiv2::Value&>(value.get()).read(editing_data);
					}
				}, dict[editing_field].fields[editing_name]);
			}
			

		}

		clear();
	}

	clear();
	metadata.Save(); // Save the edited metadata
	browseDirectory(path.parent_path());

}

void printRegularly(size_t i, int row, int col, const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& charstoleft){
	
	if (i < row){
		attron(COLOR_PAIR(1));

		std::visit([&](auto&& value){
			using T = std::decay_t<decltype(value)>;
			if constexpr(std::is_same_v<T, std::string>){
				printFields(value, charstoleft, row, col);
			}
			else if  constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
				printFields(value.get().toString().c_str(), charstoleft, row, col);
			}
		}, field.second);

		if (charstoleft < col){
			printw("\n");
		}

		attroff(COLOR_PAIR(1));

	}
}

void printFields(std::string value, int& charstoleft, int row, int col){
	printw(" ");
	charstoleft++;
	for(int i = 0; i < value.length(); i++){
		char c = value[i];
		charstoleft++;
		if (charstoleft <= col){
			printw("%c", c);
		}else{
			break;
		}
	}
}

void printEditingFields(const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& total_subtracts, int& size, std::string& editing_data, std::string& temp, int& charstoleft, size_t& i, int row, int col){
	attron(COLOR_PAIR(1));
	
	std::visit([&](auto&& value) {
		using T = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<T, std::string>) {
			
			printEditingValueAndCursor(value, total_subtracts, charstoleft, col);

			editing_data = value;
			size = value.length();
			
		}
		else if constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
			std::string temp = value.get().toString().c_str();
			printEditingValueAndCursor(temp, total_subtracts, charstoleft, col);

			editing_data = temp;
			size = temp.length();
		}

	}, field.second);

	attroff(COLOR_PAIR(1));
	if ((charstoleft - (charstoleft/col)*col) < col){
		printw("\n");
	}

	i += charstoleft/col; 
}

void printEditingValueAndCursor(std::string value, int total_subtracts, int& charstoleft, int col){
	printw(" ");
	charstoleft++;								
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
		charstoleft++;
		
	}
	if (total_subtracts == 0 && (charstoleft - (charstoleft/col)*col) < col){
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(2));
		printw(" ");
		attroff(COLOR_PAIR(2));
		attron(COLOR_PAIR(1));
		charstoleft++;
	}
}

void printFieldName(std::string fieldname, int& charstoleft){
	printw("  %s:", fieldname.c_str());
	charstoleft += 3 + fieldname.length();
}
