#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctime>
#include <stack>

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

class Command{
    public:
        string name; // main cmd for the program
        int redirect[2]; // -1 for no redirects[0]: FILEIN [1]: FILEOUT
        vector<string> args; // arguments for the program
        bool backgroundProcess;// check if it is a backgorund process
        string filename; // name of the file

        Command();

        // make a simple command
        Command(vector<string> args){
            this->name = args[0];
            this->args = args;
            redirect[0] = 0; redirect[1] = 0;
            backgroundProcess = false;
            filename = "";
        } // from a simple command there can be more additions

        const char* get_name(){
            return name.c_str();
        }

        char** get_args(){
            const char** arguments = new const char*[args.size() + 1];
            for (int i = 0; i < args.size() ; ++i){
                arguments[i] = args[i].c_str();
            }
            arguments[args.size()+1] = NULL; // make the end null
            return (char**) arguments;
        }
};

class pipeline_commands{
    public:
        int size; // the total number of commands
        vector<Command> cmds; // the commands

        pipeline_commands(){
            this->size = cmds.size();
        }

        pipeline_commands(string inputline){
            vector<Command> commands;
            vector<string> cmd;
            string token;
            int pos = 0;
            bool removeLast = false;
            for (int i = 0; i <= inputline.size(); ++i){
                if(inputline[i] == ' ' || i == inputline.size()){
                    token = inputline.substr(pos,i-pos);
                    if(removeLast){
                        token = token.substr(0,token.size()-1);
                        removeLast = false;
                    }
                    cmd.push_back(token);
                    pos = i+1; // get the pos after the command
                }

                if(inputline[i] == '\''){
                    pos = i+1;
                    i += (inputline.substr(++i).find("\'"));
                    removeLast = true;
                }

                if(inputline[i] == '\"'){
                    pos = i+1;
                    i += (inputline.substr(++i).find("\""));
                    removeLast = true;
                }

                // parse by pipes 
                if(inputline[i] == '|' || i == inputline.size()){
                    Command new_command(cmd);
                    commands.push_back(new_command);
                    cmd.clear();
                    i += 2;
                    pos = i;
                }
                
            }
            this->cmds = commands;
            this->size = commands.size();
        }
};

void printCommand(Command cmd){
    cout << "Program Name: " << cmd.name << endl;
    for (int i = 0; i < cmd.args.size();++i){
        cout << "args[" << i << "]: " << cmd.args[i] << endl; 
    }
}

void print_pipeCmds(pipeline_commands comds){
    for (int i = 0; i < comds.size ; ++i){
        printCommand(comds.cmds[i]);
    }
}

int exec_chdr(Command cmd){
    // get the cmd after cd
    if(cmd.filename == ".."){
        return chdir(cmd.filename.c_str());
    }
    // get the current working directory and add the filename
    string filepath = get_directory() + "/" + cmd.filename;
    if(chdir(filepath.c_str()) != 0){
        cout << "cd: no such file or directory: " + cmd.filename << endl;
        exit(EXIT_FAILURE);
    }
    return 0; // the state in which the file exits
}

int exec_redir(Command command){
    int fd = -1;
    string filepath = get_directory() + command.filename;
    if(command.redirect[STDIN_FILENO] != 1){ // standard input <
        fd = open(filepath.c_str(), O_RDONLY);
        dup2(fd, STDIN_FILENO);
    }
    if(command.redirect[STDOUT_FILENO] != -1){ // standard output
        fd = open(filepath.c_str(), O_WRONLY | O_CREAT | 0700);
        dup2(fd, STDOUT_FILENO); 
    }
    return execlp(command.get_name(), (char*) command.get_args());
}

// check to see if there is any quotes, pipe, or io operation
vector<string> split(string input, const char* symbol){
    vector<string> cmd;
    int index = input.find(symbol);
    if(index == -1){
        cmd.push_back(input);
        return cmd;
    }
    
    while(index != -1){
        cmd.push_back(input.substr(0,index));
        input = input.substr(++index);
        if (input[0] == ' '){
            input = input.substr(1,input.size());
        }
        index = input.find(symbol);
    }
    cmd.push_back(input);
    return cmd;
}

pipeline_commands parseString(string inputline){
    vector<Command> commands;
    vector<string> cmd;
    string token;
    int pos = 0;
    bool removeLast = false;
    for (int i = 0; i <= inputline.size(); ++i){
        if(inputline[i] == ' ' || i == inputline.size()){
            token = inputline.substr(pos,i-pos);
            if(removeLast){
                token = token.substr(0,token.size()-1);
                removeLast = false;
            }
            cmd.push_back(token);
            pos = i+1; // get the pos after the command
        }

        if(inputline[i] == '\''){
            pos = i+1;
            i += (inputline.substr(++i).find("\'"));
            removeLast = true;
        }

        if(inputline[i] == '\"'){
            pos = i+1;
            i += (inputline.substr(++i).find("\""));
            removeLast = true;
        }

        // parse by pipes 
        if(inputline[i] == '|' || i == inputline.size()){
            Command new_command(cmd);
            commands.push_back(new_command);
            cmd.clear();
            i += 2;
            pos = i;
        }
        
    }
    pipeline_commands n_cmds;
    n_cmds.cmds = commands;
    n_cmds.size = commands.size();
    return n_cmds;
}

#endif 