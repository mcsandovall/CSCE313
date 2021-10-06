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
        pipeline_commands commands(input);
        
        // check if the commands were structured correctly
        print_pipeCmds(commands);

    }
}