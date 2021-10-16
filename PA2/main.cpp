#include <iostream> 
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shell.h"

using namespace std;

int main(){
    vector<int>bgps; // for background processes
    printPrompt(); // print the prompt
    string input;
    while(true){
        for(int i = 0; i < bgps.size(); ++i){
            if(waitpid(bgps[i],0,WNOHANG) == bgps[i]){
                bgps.erase(bgps.begin() + i);
                i--;
            }
        }
        cout << get_username() << "'s shell" << get_directory() << "$ ";
        getline(cin, input);
        if(input == "exit"){
            cout << "Bye " << get_username() <<"!! End of Shell" << endl;
            break;
        }
        
        pipeline_commands commands(input); // this takes care of the parsing in the constructor
         
        int prevFD[2];
        int nextFD[2];
        bool first = true;
        for(int i = 0; i < commands.size; ++i){
            // check for change directory
            if(commands.cmds[i].name == "cd"){
                exec_chdr(commands.cmds[i]);
                break;
            }

            if(i < commands.size-1){
                pipe(nextFD);
            }
            int pid = fork();
            if(!pid){
                if(!first){
                    dup2(prevFD[0],0);
                    close(prevFD[1]);
                }

                if(i < commands.size-1){
                    close(nextFD[0]);
                    dup2(nextFD[1],1);
                }
                execute(commands.cmds[i]);
            }else{
                if(!first){
                    close(prevFD[0]);
                    close(prevFD[1]);
                }
                if(i < commands.size-1){
                    prevFD[0] = nextFD[0];
                    prevFD[1] = nextFD[1];
                }else{
                    if(!commands.cmds[i].backgroundProcess){
                        waitpid(pid, 0, 0);
                    }else{
                        bgps.push_back(pid);
                    }
                }
            }
            first = false;
        }
    }
}