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

using namespace std;

class Command{
    private:
        char* name;
        char** arguments;

    public:
        void set_name(vector<string> str){
            name = (char*) str[0].c_str();
        }

        void set_arguments(vector<string> str){
            if (str.size() == 1){
                return;
            }

            arguments = new char*[str.size()+2];
            for(int i = 1; i < str.size();++i){
                arguments[i-1] = (char*) str[i].c_str();
            }
            arguments[str.size()+1] = NULL;
        }

        char* get_name(){return name;}
        char** get_arguments(){return arguments;}

};

string get_directory(){
    char buf[FILENAME_MAX];
    getcwd(buf, FILENAME_MAX);
    string current_directory(buf);
    return current_directory;
}

string get_username(){
    return getenv("USER");
}

// get the commands into tokens, then if there is a | use the pipeParse and > or < use a ex

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
