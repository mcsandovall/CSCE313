#include <iostream> 
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "test.h"

using namespace std;




int main(){
    printPrompt();
    string input;
    while(true){
        cout << get_username() << "'s shell" << get_directory() << "$ ";
        getline(cin, input);
        if(input == "exit" || input == " "){
            cout << "Bye!! End of Shell" << endl;
            exit(EXIT_SUCCESS);
        }
        pipeline_commands cmds = parseString(input);
        print_pipeCmds(cmds);
        cout << cmds.size << endl;
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