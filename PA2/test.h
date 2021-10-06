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
    private:
        void structure(vector<string> args){ // this will be called every time there is an insertion
            // check for io redirection and background process
            this->name = args[0];
            for(int i = args.size()-1; i > 0;--i){
                if(args[i] == ">"){ // output
                    this->filename[STDOUT_FILENO] = args[i+1];
                    args.pop_back();
                    this->redirect[STDOUT_FILENO] = -1; // to trigger the flag
                    args.pop_back();
                }

                if(args[i] == "<"){ // input
                    this->filename[STDIN_FILENO] = args[i+1];
                    args.pop_back();
                    this->redirect[STDIN_FILENO] = -1;
                    args.pop_back();
                }

                if(args[i] == "&"){ // backgorund process
                    this->backgroundProcess = true;
                    args.pop_back();
                }
            }
            this->args = args;
        }
    public:
        string name; // main cmd for the program
        int redirect[2]; // -1 for no redirects[0]: FILEIN [1]: FILEOUT
        vector<string> args; // arguments for the program
        bool backgroundProcess;// check if it is a backgorund process
        string filename[2]; // name of the file

        Command();

        // make a simple command
        Command(vector<string> args){
            structure(args);
        }

        const char* get_name(){
            return name.c_str();
        }

        char** get_args(){
            const char** arguments = new const char*[args.size() + 2];
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
                if(inputline[i] == '\''){
                    pos = i+1;
                    i += (inputline.substr(pos).find("\'")+1);
                    removeLast = true;
                }

                if(inputline[i] == '\"'){
                    pos = i+1;
                    i += (inputline.substr(pos).find("\"")+1);
                    removeLast = true;
                }

                if(inputline[i] == ' ' || i >= inputline.size()){
                    token = inputline.substr(pos,i-pos);
                    if(removeLast){
                        token = token.substr(0,token.size()-1);
                        removeLast = false;
                    }
                    cmd.push_back(token);
                    pos = i+1; // get the pos after the command
                }

                // parse by pipes 
                if(inputline[i] == '|' || i >= inputline.size()){
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
        cout << "args[" << i << "]: " << cmd.args[i] << " "; 
    }
    cout << "File name: " << cmd.filename[STDIN_FILENO] << cmd.filename[STDOUT_FILENO] << endl;
    cout << "Redirect: " << "< " << cmd.redirect[STDIN_FILENO] << " > " << cmd.redirect[STDOUT_FILENO] << endl;
}

void print_pipeCmds(pipeline_commands comds){
    for (int i = 0; i < comds.size ; ++i){
        printCommand(comds.cmds[i]);
    }
}

int exe_simple(Command cmd){
    return execvp(cmd.get_name(),cmd.get_args());
}

int exec_chdr(Command cmd){
    // for all cd commands I will use the STDOUT fileaname
    if(cmd.filename[STDOUT_FILENO] == ".."){
        return chdir(cmd.filename[STDOUT_FILENO].c_str());
    }
    // get the current working directory and add the filename
    string filepath = get_directory() + "/" + cmd.filename[STDOUT_FILENO];
    if(chdir(filepath.c_str()) != 0){
        cout << "cd: no such file or directory: " + cmd.filename[STDOUT_FILENO] << endl;
        exit(EXIT_FAILURE);
    }
    return 0; // the state in which the file exits
}

int exec_redir(Command command){
    int fd[2];
    string filepath = get_directory();
    // check for both IO redirections
    if(command.redirect[STDIN_FILENO] == -1 && command.redirect[STDOUT_FILENO] == -1){
        // assume testcase
        
    }
    if(command.redirect[STDIN_FILENO] == -1){ // standard input <
        fd[STDIN_FILENO] = open(filepath.c_str(), O_RDONLY);
        dup2(fd[STDIN_FILENO], STDIN_FILENO);
    }
    if(command.redirect[STDOUT_FILENO] == -1){ // standard output >
        fd[STDOUT_FILENO] = open(filepath.c_str(), O_WRONLY | O_CREAT | 0700);
        dup2(fd[STDOUT_FILENO], STDOUT_FILENO); 
    }
    return execlp(command.get_name(), (char*) command.get_args());
}
#endif 