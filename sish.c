// sish.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_ARGUMENTS 1024
#define MAX_HISTORY 100
#define PROMPT "sish> "

int main(int argc, char *argv[])
{
    // Define the required variables
    char line[MAX_CMD_ARGUMENTS];
    char *token;
    char *delimiter = " \n";
    size_t len = 0;
    ssize_t read;

while (1)
{
    printf("%s", PROMPT);
    if ((read = getline(&line, &len, stdin)) == -1)
    {
        printf("Error reading prompt\n");
        break;
    }

    // Tokenize input
    token = strtok(line, delimiter);

    // checks for exit 
    if(strcmp(token, "exit")== 0)
    {
        printf("Exiting.");
        break;
    }
    
    // check for cd
    if(strcmp(token, "cd") == 0){
    token = strtok(NULL, delimiter);    
    if(token != NULL)
    {
        if(chdir(token) != 0){
            perror("cd");
        }
    }
    else
    {
        fprintf(stderr, "cd failed: missing directory");
    }
    continue;
    }
   
    // check for history

    // execute the command
    pid_t pid = fork();
    // if execution fails
    if(pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        execvp(token, argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else{
        wait(NULL);
    }

    // prints tokens
    while (token!= NULL)
    {
        printf("Token: %s\n", token);
        token = strtok(NULL, delimiter);
    }
}
    exit(EXIT_SUCCESS);
 }
