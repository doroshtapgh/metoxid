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
#include <variant>
#include <functional>
#include <fstream>
#include <iomanip>

void browseDirectory(const std::filesystem::path& dir); //Function to browse the director that the User is in
void editFile(const std::filesystem::path& path); //Function to start editing the file's meta data
void printEditingValueAndCursor(std::string value, int total_subtracts, int& charstoleft, int col); //Function to print the value (metadata) of the field that is being edited
void printFieldName(std::string fieldname, int& charstoleft); //Function to print the name of the field
void printEditingFields(const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& total_subtracts, int& size, std::string& editing_data, std::string& temp, int& charstoleft, size_t& i, int row, int col); //Function to print the fields that are being edited
void printRegularly(size_t i, int row, int col, const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& charstoleft); //Function to print the fields that are not being edited
void printFields(std::string value, int& charstoleft, int row, int col); //Function to print the fields that are not being edited
bool check_header(const std::filesystem::path& path); //Function to check if the file can be edited by Exiv2

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
	auto contents = listDirectory(dir); //Get the contents of the directory
	size_t num_of_elems = contents.size(); //Number of elements in the directory
	size_t selected_index = 0; //Index of the selected file
	size_t offset = 0; //offset from top of the screen
	int row, col;

	while (true) {
		getmaxyx(stdscr, row, col);

		for (size_t i = 0; i < row; ++i) {
			if (i + offset < num_of_elems) {
				if (i + offset == selected_index) {  //makes the one you are on look cooler
					attron(COLOR_PAIR(2));
					printw("%s\n", contents[i + offset].filename().c_str());
					attroff(COLOR_PAIR(2));
				} else {
					printw("%s\n", contents[i + offset].filename().c_str());
				}
			}
		}

		refresh();
		
		char ch = getch(); //waits for user input and store it

		if (ch == (char)KEY_UP) {
			if (selected_index > 0) {
				selected_index--;

				if (selected_index < offset) { //scrolls up if you are to top
					offset--;
				}
			}
		} else if (ch == (char)KEY_DOWN) {
			if (selected_index + 1 < num_of_elems) {
				selected_index++;

				if (selected_index > offset + row - 1) { //scrolls down if you are to bottom
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
		else if (ch == '~'){
			curs_set(1);
			endwin();
			exit(0);
		}
		
		clear();
	}
}


void editFile(const std::filesystem::path& path) {
	Metadata metadata(path);
	auto dict = metadata.GetDict(); //an array that holds the categories
	size_t num_of_elems = dict.size(); // size of the array
	size_t selected_index = 0; //index of the dictionary that is being hovered on by the cursor
	size_t offset = 0; //determines how many character rows down the screen has moved
	int row, col; //row = number of characters that fit in a vertical line on the curent screen size | col = number of characters that fit horizontally
	bool editing = false; // false if no field is being edited, allows cursor to move up and down, true if a field is being edited, only allows left and right cursor movement
	std::string editing_name = ""; //name of the field that is being edited
	std::string temp = ""; 
	int editing_field = 0; //index of the field that is being edited
	int charstoleft = 0; //like offset, but horizontally
	int total_subtracts = 0; //how many characters from the end the cursor is at when editing a field
	int field_size = 0; //length of the field being edited
	std::string editing_data = ""; //holds the value of the field that is being edited once it is turned into a string
	int non_category_offset = 0; //like offset, but disregards categories
	int category_index = 0; //index of the category the cursor is within
	std::vector<int> drop_indices; //array that holds the indices of the category dropdown positions
	
	for (size_t i = 0; i < num_of_elems + 1; ++i) { //+1 adds a dummy category at the end, used so that the last category can be expanded
		drop_indices.push_back(i);
	}

	bool should_edit = check_header(path); //checks if the file can be edited
	
	while (true) {
		size_t printed_categories = 0; //counter of categories that have been printed
		getmaxyx(stdscr, row, col); //gets the row and col for the current screen
		
			for (size_t i = 0; i < row; ++i) { //ISSUES HERE STEM FROM ONLHY ONE, ALSO CAN'T PRINT/SHOW BOTTOM
				int top_down_increament = 0; //ensures that fields are being printed even if their category is off screen
			
				if (i == 0){  //only for the first row
					for (size_t j = 0; j < drop_indices.size(); ++j) {
						if (drop_indices[j] == offset) { //if user has not scrolled at all
							break;
						} 
						if (drop_indices[j] > offset) { //checks if a field is opened after the user scrolls past the category name
							top_down_increament = drop_indices[j-1]+1;
							for (auto& field : dict[j-1].fields) { //loops through every field in the selected category
								
								if (top_down_increament == offset){
									i += 1;
									charstoleft = 0;
									if (i > row){ //if the would be off screen, break
										break;
									}					
									if (i + offset == selected_index+1) { 
										//prints out and highlights the selected field
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

				if (i + offset < num_of_elems && i < row) { //Prints out everything else

					category_index = printed_categories + offset - non_category_offset;

					if (i + offset == selected_index) {
						//prints out and highlights the selected category
						attron(COLOR_PAIR(2));
						if (dict[category_index].expanded) {
							printw("v %s\n", dict[category_index].name.c_str());
						} else {
							printw("> %s\n", dict[category_index].name.c_str());
						}
						attroff(COLOR_PAIR(2));

						if (dict[category_index].expanded) {
							for (auto& field : dict[category_index].fields) { 
								//loops through the fields of an expanded category and prints the fields
								i += 1;
								if (i < row){
									
									charstoleft = 0;
									printFieldName(field.first.c_str(), charstoleft);
									printRegularly(i, row, col, field, charstoleft);
								}
							}
						}
					} else {
						//for not selected categories
						if (dict[category_index].expanded) { //prints category name
							printw("v %s\n", dict[category_index].name.c_str());
							
							for (auto& field : dict[category_index].fields) {
								//loops through the fields of the category
								i += 1;
								charstoleft = 0;

								if (i < row){
									
									if (i + offset == selected_index) {
										//if the field is selected, print and highlight it
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
					printed_categories ++;
				}
			}
		

		refresh();

		char ch = getch();
		if (!editing){
			
			if (ch == (char)KEY_UP) {
				
				if (selected_index > 0) {
					//if the cursor is still within the bounds of the screen
					selected_index--;
					
					if (selected_index < offset) {
						//if the cursor is outside the bounds of the screen and requires the screen to move up
						offset--;

						bool found = false;
						for (size_t j = 0; j < drop_indices.size(); ++j) {
							//checks if the row that was moved to was a category. If yes, reduces non category offset.
							if (drop_indices[j] == offset) {
								found = true;
								break;
							} 
						}
						if (!found) {
							non_category_offset -= 1;
						}
					}
				}
			} else if (ch == (char)KEY_DOWN) {
				if (selected_index + 1 < num_of_elems) {
					//if the cursor is still within the bounds of the screen
					selected_index++;

					if (selected_index > offset + row - 1) {
						//if the cursor is outside the bounds of the screen and requires the screen to move up
						offset++;

						for (size_t j = 0; j < drop_indices.size(); ++j) {
							//checks if the row that was moved to was a category. If yes, reduces non category offset.
							if (drop_indices[j] == offset-1) {
								break;
							} 
							if (drop_indices[j] > offset) {
								non_category_offset += 1;
								break;
							}
						}
					}
				}
			} else if (ch == 10) {
				//if the key pressed was enter
				total_subtracts = 0;
				if(should_edit){editing = true;}
			
				for (int i = 0; i < drop_indices.size(); ++i) {
					if (drop_indices[i] == selected_index) {
						//if the selected item is a category
						if (dict[i].expanded) {
							//collapses if it is expanded
							dict[i].expanded = false;
							int sizeof_fields = dict[i].fields.size();
							num_of_elems = num_of_elems - sizeof_fields;
							for (int j = 1; j < drop_indices.size() - i; ++j) {
								drop_indices[j + i] = drop_indices[j + i] - sizeof_fields;
							}
							editing = false;
							break;
						} else {
							//expands the category if it's collapsed
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
				break; //exits and saves
			}
			
		}
		else{ //if mode is currently editing
			refresh();

			if (ch == 10) { //if the key is enter
				total_subtracts = 0;
				editing = false;
				curs_set(0);
			} 
			else if (ch == char(KEY_LEFT)) { //if the key is key_left
				if (total_subtracts < field_size){
					//changes the position of the cursor
					total_subtracts++;
				}
			}
			else if (ch == char(KEY_RIGHT)) {
				if (total_subtracts > 0){
					//changes the position of the cursor
					total_subtracts--;
				}
			}
			else if (ch == char(KEY_UP)){
				if (total_subtracts + col > field_size){
					//if the field is multiple lines, moves one line up. If not, moves to the start of the field 
					total_subtracts = field_size;
				}
				else{
					total_subtracts += col;
				}
			}
			else if (ch == char(KEY_DOWN)){
				if (total_subtracts - col < 0){
					//if the field is multiple lines, moves one line down. If not, moves to the end of the field 
					total_subtracts = 0;
				}
				else{
					total_subtracts -= col;
				}
			}
			else if (ch == '~') {
				break; //exits and saves
			}
			else{
				
				if (ch == char(KEY_BACKSPACE)){
					//deletes one character 
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
					break; //exits and saves
				}
				else{
					if (isalnum(ch) || ispunct(ch) || isspace(ch)){
						//if the character is a number, punctuation or space, type it where the cursor is
						editing_data.insert(editing_data.end() - total_subtracts, ch);
					}
				}
				
				//saves the edits into editing data
				std::visit([&](auto&& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, std::string>) {
						value = editing_data;
						if (editing_name == "Comment") {
							metadata.SetComment(editing_data);
						} else if (editing_name == "XMP Packet") {
							metadata.SetXmpPacket(editing_data);
						}
					} else if constexpr (std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>) {
						const_cast<Exiv2::Value&>(value.get()).read(editing_data);
					}
				}, dict[editing_field].fields[editing_name]);
			}
			
		}

		clear();
	}

	clear();
	if (should_edit){
		metadata.Save(); // Save the edited metadata
	}
	browseDirectory(path.parent_path()); //goes back to image select
}

bool check_header(const std::filesystem::path& path){
	
	std::vector<std::vector<char>> headers = {
		{0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C, 0x20}, //JXL
		{0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63}, //HEIC
		{0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70}, //HIEF
		{0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70} //Diferent HIEF file format
	};

	std::fstream s(path, std::ios::binary | std::ios::in | std::ios::out);
	
	if (!s.is_open()){
		fatalError("Failed to open file");
	}

	char file_header[8] = {0};
	s.read(file_header, 8);
 
	for (int i = 0; i < headers.size(); i++){
		for (int j = 0; headers[i].size(); j++){
			if (file_header[j] != headers[i][j]){
				break;
			}
			if (j == headers[i].size() - 1){
				return false;
			}
		}
	}

	return true;
}

void printRegularly(size_t i, int row, int col, const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& charstoleft){
	attron(COLOR_PAIR(1));

	std::visit([&](auto&& value){ //allows us to access field.second, use "&" to access the address of it and actually change values easilly
		using T = std::decay_t<decltype(value)>; //gets the datatype of value
		if constexpr(std::is_same_v<T, std::string>){ //if string
			printFields(value, charstoleft, row, col);
		}
		else if constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){ //if refrence wrapper
			printFields(value.get().toString().c_str(), charstoleft, row, col); //sets the value to a string before printing
		}
	}, field.second);

	if (charstoleft < col){ //if needed print a new line
		printw("\n");
	}
	attroff(COLOR_PAIR(1));
}

void printFields(std::string value, int& charstoleft, int row, int col){
	printw(" ");
	charstoleft++;

	for(int i = 0; i < value.length(); i++){ //prints the value by character (in case it is too long)
		char c = value[i];
		charstoleft++;
		if (charstoleft <= col){ //only print if there is space horizontally
			printw("%c", c);
		}else{
			break;
		}
	}
}

void printEditingFields(const std::pair<const std::string, std::variant<std::string, std::reference_wrapper<const Exiv2::Value>>>& field, int& total_subtracts, int& size, std::string& editing_data, std::string& temp, int& charstoleft, size_t& i, int row, int col){
	attron(COLOR_PAIR(1));
	
	std::visit([&](auto&& value) { //allows us to access field.second, use "&" to access the address of it and actually change values easilly
		using T = std::decay_t<decltype(value)>; //gets the datatype of value
		if constexpr(std::is_same_v<T, std::string>){
			printEditingValueAndCursor(value, total_subtracts, charstoleft, col);

			editing_data = value;
			size = value.length();
		}
		else if constexpr(std::is_same_v<T, std::reference_wrapper<const Exiv2::Value>>){
			std::string temp = value.get().toString().c_str(); //sets the value to a string before passing it
			printEditingValueAndCursor(temp, total_subtracts, charstoleft, col);

			editing_data = temp;
			size = temp.length();
		}
	}, field.second);

	attroff(COLOR_PAIR(1));
	if ((charstoleft - (charstoleft/col)*col) < col){ //if needed print a new line
		printw("\n");
	}

	i += charstoleft/col; //how many rows has been taken up by the field (in case it goes beyond 1 row)
}

void printEditingValueAndCursor(std::string value, int total_subtracts, int& charstoleft, int col){ 
	printw(" ");
	charstoleft++;								

	for(int i = 0; i < value.length(); i++){ //prints it out character by character to show the cursor
		char c = value[i];
		
		if (i == value.length() - total_subtracts){ //prints out cursor
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(2));
			printw("%c", c);
			attroff(COLOR_PAIR(2));
			attron(COLOR_PAIR(1));
		}
		else{
			printw("%c", c); //just the character
		}
		charstoleft++;
	}
	if (total_subtracts == 0 && (charstoleft - (charstoleft/col)*col) < col){ //if you are at end print out cursor at end
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(2));
		printw(" ");
		attroff(COLOR_PAIR(2));
		attron(COLOR_PAIR(1));
		charstoleft++;
	}
}

void printFieldName(std::string fieldname, int& charstoleft){ //prints the name of the field
	printw("  %s:", fieldname.c_str());
	charstoleft += 3 + fieldname.length();
}

