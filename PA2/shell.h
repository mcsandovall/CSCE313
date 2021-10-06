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

// function for testing pruposes
void printVector(vector<string> cmd){
    for(int i =0; i < cmd.size();++i){
        cout << cmd[i] << " ";
    }
    cout << endl;
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

//requirement 1 check for quotes single and double 
void parseQuotes(string input, vector<string> &cmd){
    bool singleQuote = (input.find("\'") != -1);
    bool doubleQuote = (input.find("\"") != -1);

    // base case
    if(!singleQuote && !doubleQuote){
        cmd.push_back(input);
        return; // end no quotes
    }

    if(singleQuote && !doubleQuote){
        string str = input.substr(0,input.find("\'"));
        cmd.push_back(str);
        input = input.substr(input.find("\'")+1);
        parseQuotes(input,cmd);
    }

    if(!singleQuote && doubleQuote){
        string str = input.substr(0,input.find("\""));
        cmd.push_back(str);
        input = input.substr(input.find("\"")+1);
        parseQuotes(input,cmd);
    }   
}

// make an object to hold the commands and the filename if there is any


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


int caseIORedirect(string command){
    // check to see what type of io redirect do you have 
    bool inputExist = (command.find(">") != -1);
    bool outputExist = (command.find("<") != -1);

    if(!inputExist && !outputExist){
        return 0; // no io redirection
    }
    if(!inputExist && outputExist){
        return 1; // output redirection
    }
    if(inputExist && !outputExist){
        return 2; // input redirection
    }
    if(inputExist && outputExist){
        return 3; // both input and output redirection
    }
    return -1; // error neither of those worked 
}

void parseQuotes(string input){
    // this is to parse single and double quotes NOTE: single quotes are literal and double quotes is the command
    //
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
