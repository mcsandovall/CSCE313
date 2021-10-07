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
    cout << endl;
    cout << "File name: [<]:" << cmd.filename[STDIN_FILENO] << " [>]:" << cmd.filename[STDOUT_FILENO] << endl;
    cout << "Redirect: " << "<: " << cmd.redirect[STDIN_FILENO] << " >: " << cmd.redirect[STDOUT_FILENO] << endl;
}

void print_pipeCmds(pipeline_commands comds){
    for (int i = 0; i < comds.size ; ++i){
        printCommand(comds.cmds[i]);
    }
}

int exec_chdr(Command cmd){
    if(cmd.filename[STDOUT_FILENO].empty()){
        cout << "cd: no such file or directory: " + cmd.filename[STDOUT_FILENO] << endl;
        exit(EXIT_FAILURE);
    }
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

void exec_redir(Command command){
    int fd[2];
    int pid = fork();
    int process;
    if(!pid){
        string filepath = get_directory();
        if(command.redirect[STDIN_FILENO] != -1){ // standard input <
            filepath += command.filename[STDIN_FILENO];
            fd[STDIN_FILENO] = open(filepath.c_str(), O_RDONLY);
            dup2(fd[STDIN_FILENO], STDIN_FILENO);
            process = STDIN_FILENO;
        }
        if(command.redirect[STDOUT_FILENO] != -1){ // standard output >
            filepath = command.filename[STDOUT_FILENO];
            fd[STDOUT_FILENO] = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(fd[STDOUT_FILENO], STDOUT_FILENO); 
            process = STDOUT_FILENO;
        }
        execvp(command.get_name(),command.get_args());
    }else{
        waitpid(pid,0,0);
        close(fd[process]);
    }
}


int exe_command(Command command){
    if(command.name == "cd"){
        return exec_chdr(command);
    }

    if(command.redirect[STDOUT_FILENO] != -1 || command.redirect[STDIN_FILENO] != -1){
        exec_redir(command);
    }

    return execvp(command.get_name(),command.get_args());
}

int exec_pipes(pipeline_commands commands){
    int state;
    for(int i = 0; i < commands.size ; ++i){
        int fd[2];
        pipe(fd);
        int pid = fork();
        if(!pid){
            dup2(fd[STDOUT_FILENO], STDOUT_FILENO);
            // state = exe_command(commands.cmds[i]);
            exe_command(commands.cmds[i]);
        }else{
            waitpid(pid, 0, 0);
            dup2(fd[STDIN_FILENO],STDIN_FILENO);
            close(fd[STDOUT_FILENO]);
        }
    }
    return state;
}

void execute(Command command){
    if(command.name == "cd"){
        exec_chdr(command);
    }

    if(command.redirect[STDOUT_FILENO] != -1 || command.redirect[STDIN_FILENO] != -1){
        exec_redir(command);
    }

    execvp(command.get_name(),command.get_args());    
}
#endif 