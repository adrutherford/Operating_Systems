// Anthony Rutherford
// CWID: 12192817

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#define MAX_LINE 80

int main(void) {

    int id = getpid();
    char pid[50]; 
    char end[2] = ">";
    sprintf(pid, "%d", id); 
    char shell[] = "Rutherford_";
    strcat(shell,pid);
    strcat(shell,end);
    //^^^^^^^^^^^^^^^^^ putting together string for prompt
    int should_run = 1; 
    
    while (should_run) {
        char *args[MAX_LINE/2 + 1] = {NULL};
        int waiting = 1;
        int arg_count = 0;
        char input[MAX_LINE];
        printf("%s",shell);
        fgets(input, sizeof(input), stdin);
        char *word = strtok(input, " \t\n");

        while (word != NULL) {
            args[arg_count] = malloc(10);
            //check for & to set waiting and exit to end program
            if (strcmp(word, "&") == 0){
                waiting = 0;
                

            }
            else if (strcmp(word, "exit") == 0) {
                should_run = 0;
                
            }
            else{
            strncpy(args[arg_count], word, 10 - 1);
            args[arg_count][10 - 1] = '\0';
            // Move to the next word
            word = strtok(NULL, " \t\n");
            arg_count++;
            }

            
            
        }
        //\/\/\/\/\/\/\/\/\/ fork into child process and check to see if waiting
        id = fork();
        if (id == 0){
            execvp(args[0], args);
        }
        else{
            if (waiting){
                wait(NULL);
            }
        }
        
    }

    return 0;
}

