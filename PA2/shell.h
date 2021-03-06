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
#include <curses.h>

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
            if(args[0] == "cd"){
                chdrStructure(args);
                return;
            }
            // check for io redirection and background process
            this->name = args[0];
            for(int i = args.size()-1; i > 0;--i){
                if(args[i] == ">"){ // output
                    this->filename[STDOUT_FILENO] = args[i+1];
                    args.pop_back();
                    this->redirect[STDOUT_FILENO] = STDOUT_FILENO; // to trigger the flag
                    args.pop_back();
                }

                if(args[i] == "<"){ // input
                    this->filename[STDIN_FILENO] = args[i+1];
                    args.pop_back();
                    this->redirect[STDIN_FILENO] = STDIN_FILENO;
                    args.pop_back();
                }

                if(args[i] == "&"){ // backgorund process
                    this->backgroundProcess = true;
                    args.pop_back();
                }
            }
            this->args = args;
        }

        void chdrStructure(vector<string> args){
            // put the filename in the STDOUT file
            this->name = args[0];
            this->filename[STDOUT_FILENO] = args[1];
            args.pop_back();
            this->args = args;
        }   
    public:
        string name; // main cmd for the program
        int redirect[2] = {-1,-1}; // -1 for no redirects[0]: FILEIN [1]: FILEOUT
        vector<string> args; // arguments for the program
        bool backgroundProcess;// check if it is a backgorund process
        string filename[2]; // name of the file input and output for multiple implementation

        Command();

        // make a simple command
        Command(vector<string> args){
            structure(args);
        }

        const char* get_name(){
            return name.c_str();
        }

        char** get_args(){ // convert into a char **
            const char** arguments = new const char*[args.size() + 1];
            for (int i = 0; i < args.size() ; ++i){
                arguments[i] = args[i].c_str();
            }
            arguments[args.size()] = NULL; // make the end null
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

        pipeline_commands(string inputline){ // split function but handles the quotes
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

void printCommand(Command cmd){ // for testing purposes
    cout << "Program Name: " << cmd.name << endl;
    for (int i = 0; i < cmd.args.size();++i){
        cout << "args[" << i << "]: " << cmd.args[i] << " "; 
    }
    cout << endl;
    cout << "File name: [<]:" << cmd.filename[STDIN_FILENO] << " [>]:" << cmd.filename[STDOUT_FILENO] << endl;
    cout << "Redirect: " << "<: " << cmd.redirect[STDIN_FILENO] << " >: " << cmd.redirect[STDOUT_FILENO] << endl;
}

void print_pipeCmds(pipeline_commands comds){ // for testing
    for (int i = 0; i < comds.size ; ++i){
        printCommand(comds.cmds[i]);
    }
}
vector<string> directories; // for the previous command

int exec_chdr(Command cmd){
    string filepath;
    directories.push_back(get_directory());
    if(cmd.filename[STDOUT_FILENO].empty()){
        cout << "cd: no such file or directory: " + cmd.filename[STDOUT_FILENO] << endl;
        exit(EXIT_FAILURE);
    }
    if(cmd.filename[STDOUT_FILENO] == "-"){ // for the previous directory
        directories.pop_back(); // get rid of the current one
        filepath = directories.back(); // get the last one 
        directories.pop_back(); // get rid of it 
        return chdir(filepath.c_str());
    }
    // for all cd commands I will use the STDOUT fileaname
    if(cmd.filename[STDOUT_FILENO] == ".."){
        return chdir(cmd.filename[STDOUT_FILENO].c_str());
    }
    // get the current working directory and add the filename
    if(cmd.filename[STDOUT_FILENO][0] == '/'){
        filepath = cmd.filename[STDOUT_FILENO];
    }else{
        filepath = get_directory() + "/" + cmd.filename[STDOUT_FILENO];
    }
    // filepath = get_directory() + "/" + cmd.filename[STDOUT_FILENO];
    if(chdir(filepath.c_str()) != 0){
        cout << "cd: no such file or directory: " + cmd.filename[STDOUT_FILENO] << endl;
        exit(EXIT_FAILURE);
    }
    return 0; // the state in which the file exits
}

void exec_redir(Command command){
    int fd[2];
    string filepath = get_directory() + "/";
    if(command.redirect[STDIN_FILENO] != -1){ // standard input <
        filepath += command.filename[STDIN_FILENO];
        fd[STDIN_FILENO] = open(filepath.c_str(), O_RDONLY); // open for read only
        dup2(fd[STDIN_FILENO], STDIN_FILENO);
    }
    if(command.redirect[STDOUT_FILENO] != -1){ // standard output >
        filepath = command.filename[STDOUT_FILENO];
        fd[STDOUT_FILENO] = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); // open with privileges to make files
        dup2(fd[STDOUT_FILENO], STDOUT_FILENO);
    }
    execvp(command.get_name(),command.get_args());
    close(fd[STDIN_FILENO]); // close the input redirection
}

void execute(Command command){

    if(command.redirect[STDOUT_FILENO] != -1 || command.redirect[STDIN_FILENO] != -1){
        exec_redir(command);
        return;
    }

    if(execvp(command.get_name(),command.get_args()) != 0){ // execute command
        cout << "shell: command not found: " << command.name << endl;
        return;
    }   
}

#endif 