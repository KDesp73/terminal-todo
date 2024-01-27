#include <cstring>
#include <iostream>
#include "include/files.h"
#include "include/text.h"
#include "tui_kit.h"
#include <stdio.h>
#include <valarray>
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
	Files::writeFile(file, "");
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
		done.erase(done.begin() + selected);
	} else {
		todo.erase(todo.begin() + selected);
	}
}

string wrap_in_brackets(string text, int color){
	return Text::color("fg", color) + "[" + Text::normal + text + Text::color("fg", color) + "]" + Text::normal;
}

void print_list(int color){
	vector<string> list = (status == Status::Todo) ? todo : done;
	int max = 20;
	int selected = 0;
	bool menuActive = true;
	
    for(int i = 0; i < list.size(); i++){
        list.at(i) = TuiKit::addSpaces("- [ ] " + list.at(i), max);
    }
	
	while (menuActive) {
		Text::clearScreen();
		list = (status == Status::Todo) ? todo : done;
		cout << ((status == Status::Todo) ? wrap_in_brackets("TODO", color) + " DONE " : " TODO " + wrap_in_brackets("DONE", color)) + " " << std::endl;
		cout << endl;
		// cout << "-----------" << endl;
		
		for(int i = 0; i < list.size(); i++){
			cout << ((selected == i) ? Text::color("bg", color) : "") << ((status == Status::Todo) ? "- [ ] " : "- [x] ") << list.at(i) << Text::normal << endl;
		}

#ifdef WIN32
		int key = _getch();
#else
		int key = getchar();
#endif

		switch (key) {
		case '\n':
			if(status == Status::Done){
				list_transfer(&todo, &done, selected);
			} else {
				list_transfer(&done, &todo, selected);
			}
			if(selected == list.size()-1) selected = list.size()-2;
			break;
		case 65: // UP
			selected = (selected == 0) ? list.size()-1 : selected-1;
			break;
		case 66: // DOWN
			selected = (selected == list.size()-1) ? 0 : selected+1;
			break;
		case '\t':
			selected = 0;
			toggle_status();
			break;
		case 'a': // add
			break;
		case 'd': // remove
			remove_item(selected);
			break;
		case 'q': // quit
			save_state(todo, done, "todo.txt");
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
	save_state(todo, done, "todo.txt");
	Text::clearScreen();
	Text::enableInputBuffering();
	cout << "[WARN] Exited abnormally" << endl;
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


int main(int argc, char **argv){
	if(!Files::exists("todo.txt")){
		Files::writeFile("todo.txt", "");
	}
	
	// load todos
	vector<string> lines = Files::readFileLines("todo.txt");

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
				cout << "[ERRO] Ill-formed `todo.txt` file at line: " << i+1 << endl;
				exit(1);
			}
		}
	}


	Text::disableInputBuffering();

	signal(SIGINT, siginthandler);

	print_list(1);

	

	return 0;
}
