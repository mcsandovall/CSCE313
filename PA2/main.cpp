#include <iostream> 
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shell.h"
using namespace std;




int main(){

    string inputline;
    char** args;
    
    while(true){
        cout << get_username() <<"'s shell" << get_directory() << " $ ";
        getline(cin, inputline);
        args = pareseString(inputline);
        runCommand(args);
    }
    
    // while(true){
    //     cout << "My Shell " << get_directory() << " $ ";
    //     getline (cin, inputline); // get the line from standard input 
    //     if(inputline == string("exit")){
    //         cout << "Bye!! End of Shell" << endl;
    //         break;
    //     }
    //     int pid = fork();
    //     if(pid == 0){ // child process
    //         // preparing the input command for execution
    //         char* args[] = {(char*) inputline.c_str(),NULL};
    //         execvp (args[0], args);
    //     }else{
    //         waitpid(pid, 0, 0);
    //     }
        
    // }
}