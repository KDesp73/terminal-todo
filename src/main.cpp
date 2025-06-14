#include <cstdlib>
#include <cstring>
#include <iostream>
#include "include/files.h"
#include "include/text.h"
#include "tui_kit.h"
#include <stdio.h>
#include <vector>
#include "../tui-kit/include/menu_utils.h"
#include <signal.h>
#include <stdio.h>

#ifdef _WIN32
    #include <conio.h>  
    #include <Windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif


using namespace TuiKit;
using namespace std;

static string todo_file;
vector<string> todo;
vector<string> done;

enum class Status {
	Todo,
	Done
};

Status status = Status::Todo;

void list_transfer(vector<string> *target, vector<string> *src, int index){
	target->push_back(src->at(index));
	src->erase(src->begin() + index);
}

void save_state(vector<string> todo, vector<string> done, string file){
	Files::writeFile(file, ""); // Empty the file
	for(string task : todo){
		Files::appendFile(file, "TODO: " + task + "\n");
	}

	for(string task : done){
		Files::appendFile(file, "DONE: " + task + "\n");
	}
}

void toggle_status(){
	if(status == Status::Todo) status = Status::Done;
	else status = Status::Todo;
}
void remove_item(int selected){
	if(status == Status::Done) {
		if(done.empty()) return;
		done.erase(done.begin() + selected);
	} else {
		if(todo.empty()) return;
		todo.erase(todo.begin() + selected);
	}
}

string wrap_in_brackets(string text, int color){
	return Text::color("fg", color) + "[" + Text::normal + text + Text::color("fg", color) + "]" + Text::normal;
}

void print_keys(){
	cout << endl;
	cout << wrap_in_brackets("  h  ", 4) + " toggle help" << endl;
	cout << wrap_in_brackets("  ↑  ", 4) + " go up" << endl;
	cout << wrap_in_brackets("  ↓  ", 4) + " go down" << endl;
	cout << wrap_in_brackets("  q  ", 4) + " exit the program" << endl;
	cout << wrap_in_brackets("  d  ", 4) + " delete task" << endl;
	cout << wrap_in_brackets("  a  ", 4) + " add a new task" << endl;
	cout << wrap_in_brackets("  i  ", 4) + " create issue for selected todo" << endl;
	cout << wrap_in_brackets("  I  ", 4) + " create issues for all todos" << endl;
	cout << wrap_in_brackets(" tab ", 4) + " switch between TODO and DONE" << endl;
	cout << wrap_in_brackets("enter", 4) + " switch task status" << endl;
}
string add_todo(){
	Text::clearScreen();
	Text::enableInputBuffering();
	string input;
	cout << endl;
	cout << "  TODO: ";
	getline(cin, input);

	if(input.length() != 0){
		todo.push_back(input);
	}

	Text::disableInputBuffering();
	return input;
}

bool isUserLoggedIn() {
    FILE *pipe = popen("gh auth status 2>&1", "r");
    if (!pipe) {
        std::cerr << "Error: Failed to execute command.\n";
        return false;
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    return result.find("Logged in to github.com") != std::string::npos;
}

void create_issue(int selected, bool with_body = true){
	if (system("gh --version") != 0) {
		return;
	}

	if(!isUserLoggedIn()) {
		return;
	}

	if(todo.empty()){
		return;
	}

	string todo_item = todo[selected];
	
	string body = "";
	if(with_body){
		Text::clearScreen();
		Text::enableInputBuffering();
		cout << endl;
		cout << "  Enter body for \"" + todo_item + "\" (Press enter to leave empty): ";
		getline(cin, body);
		Text::disableInputBuffering();
	}

	std::string command = "gh issue create --title \"" + todo_item + "\" --body \"" + body + "\"";

    system(command.c_str());
}

void issue_all(){
	if(!isUserLoggedIn()) return;
	for(int i = 0; i < todo.size(); i++){
		create_issue(i, false);
	}
}

void print_list(int color){
	vector<string> *list;
	int max = 25;
	int selected = 0;
	bool menuActive = true;
	bool help = false;
	
	while (menuActive) {
		Text::clearScreen();
		list = (status == Status::Todo) ? &todo : &done;

		// for(int i = 0; i < list->size(); i++){
		// 	list->at(i) = TuiKit::addSpaces(list->at(i), max);
		// }
		
		cout << endl;
		cout << " " << ((status == Status::Todo) ? wrap_in_brackets("TODO", color) + " DONE " : " TODO " + wrap_in_brackets("DONE", color)) + " " << std::endl;
		cout << endl;
		// cout << "-----------" << endl;
		
		for(int i = 0; i < list->size(); i++){
			cout << " " << ((selected == i) ? Text::color("bg", color) : "") << ((status == Status::Todo) ? " - [ ] " : " - [x] ") << list->at(i) << " " << Text::normal << endl;
		}

		if(help){
			print_keys();
		}

		// Handle keys
		
#ifdef WIN32
		int key = _getch();
#else
		int key = getchar();
#endif

		switch (key) {
		case '\n':
			if(status == Status::Done){
				if(done.empty()) break;
				list_transfer(&todo, &done, selected);
			} else {
				if(todo.empty()) break;
				list_transfer(&done, &todo, selected);
			}
			if(selected == list->size()) selected = list->size()-1;
			break;
		case 65: // UP
			selected = (selected == 0) ? list->size()-1 : selected-1;
			break;
		case 66: // DOWN
			selected = (selected == list->size()-1) ? 0 : selected+1;
			break;
		case '\t':
			selected = 0;
			toggle_status();
			break;
		case 'h': // toggle help
			help = !help;
			break;
		case 'a': // add
			add_todo();
			break;
		case 'd': // remove
			remove_item(selected);
			if(selected == list->size()) selected = list->size()-1;
			break;
		case 'i': // create issue
			if(status == Status::Done) {
				continue;
			}
			create_issue(selected);
			break;
		case 'I': // Make all todos issues
			issue_all();
			break;
		case 'q': // quit
			save_state(todo, done, todo_file);
			menuActive = false;
			Text::clearScreen();
			Text::enableInputBuffering();
			exit(0);
			break;
		default:
			break;
		}
	}

}

void siginthandler(int param){
	save_state(todo, done, todo_file);
	Text::clearScreen();
	Text::enableInputBuffering();
	// cout << "[WARN] Exited abnormally" << endl;
	exit(0);
}

bool startsWith(const std::string& fullString, const std::string& prefix) {
    return fullString.substr(0, prefix.length()) == prefix;
}

std::string removeSubstring(const std::string& mainString, const std::string& substring) {
    std::string result = mainString;
    size_t pos = result.find(substring);

    if (pos != std::string::npos) {
        result.erase(pos, substring.length());
    }

    return result;
}

void load_state(){
	vector<string> lines = Files::readFileLines(todo_file);

	string todo_prefix = "TODO: ";
	string done_prefix = "DONE: ";
	if(lines.size() > 0){
		for(int i = 0; i < lines.size(); i++){
			string line = lines.at(i);
			if(startsWith(line, todo_prefix)){
				todo.push_back(removeSubstring(line, todo_prefix));
			} else if(startsWith(line, done_prefix)){
				done.push_back(removeSubstring(line, done_prefix));
			} else {
				cerr << "[ERRO] Ill-formed `" << todo_file << "` file at line: " << i+1 << endl;
				exit(1);
			}
		}
	}
}

int main(int argc, char **argv){
	if(!Files::exists("todo.txt")){
		if(!Files::exists("TODO")){
			Files::writeFile("TODO", "");
		}
		todo_file = "TODO";
	} else {
		todo_file = "todo.txt";
	}
	
	// load todos
	load_state();

	Text::disableInputBuffering();

	signal(SIGINT, siginthandler);

	print_list(1);

	

	return 0;
}