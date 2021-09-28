#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctime>

using namespace std;

string get_directory(){
    char buf[FILENAME_MAX];
    getcwd(buf, FILENAME_MAX);
    string current_directory(buf);
    return current_directory;
}

string get_username(){
    return getenv("USER");
}

// get the date and time for the prompt 
string get_dateNtime(){
    time_t now  = time(0);
    return ctime(&now);
}

// user prompt
void printPrompt(){
    cout << "######################################" << endl;
    cout << "######################################" << endl;
    cout << "########## Current Version ###########" << endl;
    cout << "########## of mariOS shell ###########" << endl;
    cout << "######################################" << endl;
    cout << "######################################" << endl;
    cout << endl;
    cout << "Welcome " << get_username() << " Time and Date: " << get_dateNtime() << endl;
}

// make an object to hold the commands and the filename if there is any
class Command{
    private:
        string name;
        vector<string> args;

        bool isBackrgoundProcess = false;
        string redirectFile;
};

int parsePipe(string input, vector<string> &commands){
    // get all the pipes 
    int pos = input.find("|");
    if(pos == -1){ // there is no pipes
        commands.push_back(input);
        return 0; // status no pipe
    }else{ // there is a pipe
        string line;
        while(pos != -1){ // process while there is a stilla pipe
            line = input.substr(0,pos);
            commands.push_back(line); // add the line to commands
            input = input.substr(pos+1); // get everything after the pipe
            pos = input.find("|");
        }
        commands.push_back(input); // add the last commands to the vector of commands
        return 1; // status 1 for there is pipes
    }
}

int IORedirect(string command){
    // get the first kind of IO redirect that happens EX: ls -l > file.txt < args:  this will execute everything before <
    // status 0 there is no IO redirect, 1 output, 2 input
    int input = command.find("<");
    int output = command.find(">");
    if(input == -1 && output == -1){
        
    }
}

// parse the input string and return the char** for the execution
char** pareseString(string input){
    if(input == "exit" || input.size() == 0){
        cout << "Bye!! End of Shell" << endl;
        exit(EXIT_SUCCESS);
    }
    stringstream ss(input);
    vector<string> str;
    string line;
    while(ss >> line){
        str.push_back(line);
    }
    const char** args = new const char* [str.size()+2];
    for(int i = 0; i < str.size();++i){
        args[i] =  str[i].c_str();
    }
    args[str.size()+1] = NULL;
    return (char**) args;
}

// run the Parsed Commands 
void runCommand(char** ParsedCommands){
    int pid = fork();

    if(pid == 0){
        execvp(ParsedCommands[0],ParsedCommands);
        exit(0);
    }else{
        waitpid(pid, 0, 0);
        return;
    }
}



#endif // SHELL_H
