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

char history[MAX_HISTORY][MAX_CMD_ARGUMENTS];
int history_count = 0;
int front = -1; rear = -1;
int isFull = 0;

int isFull(){
  if ((front == rear + 1) || (front == 0 && rear == SIZE - 1)) return 1;
  return 0;
}

int isEmpty(){
    if(front == -1){
        return 1;
    }
    return 0;
}

void enQueue(char *cmd){
  if(isFull()){
    front = (front + 1) % MAX_HISTORY;
  }
  rear = (rear + 1) % MAX_HISTORY;
  addtoHistory(cmd);
}

void clearHistory(){
    history_count = 0;
}

void addtoHistory(const char *cmd){
    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count], cmd);
        history_count++;
    }else {
      strcpy(history[rear], cmd);
      history_count++;
    }
}

void printHistory(int offset){
  if(offset < 0 || offset >= history_count){
    fprintf(stderr, "history: invalid offset\n");
    return;
  }
  for(int i = 0; i < history_count; i++){
    printf("%d: %s\n", i, history[i]);
  }
}


int main(int argc, char *argv[]) {
    // Define the required variables
    char line[MAX_CMD_ARGUMENTS];
    char *token;
    char *delimiter = " ";
    size_t len = 0;
    ssize_t read;

    while (1) {
        printf("%s", PROMPT);
        if ((read = getline(&line, &len, stdin)) == -1) {
            printf("Error reading prompt\n");
            break;
        }

        // Tokenize input, to read line from stdin one at a time
        token = strtok(line, delimiter);

        // checks for exit, if user enters exit, the shell will exit
        if (strcmp(token, "exit") == 0) {
            printf("Exiting");
            break;
        }

        // check for cd
        if (strcmp(token, "cd") == 0) 
        {
            // get the next token (directory to change to)
            token = strtok(NULL, delimiter);
            // if the token is not null, change to the directory provided by user
            if (token != NULL) {
                if (chdir(token) != 0) {
                    perror("cd");
                }
                // if chdir fails, print error message
            } else {
                fprintf(stderr, "cd failed: missing directory\n");
            }
            continue;
        }

        // check for history -c, if -c clear history
        if (strcmp(token, "history") == 0 && strcmp(strtok(NULL, delimiter), "-c") == 0) {
            clearHistory();
            continue;
        } 
        // checks for history [offset], executes the command in history at the given offset. to check for offset we probably check if the second token is a number(is there an easier way to do this)
        // prints error message if offset is not valid
        if(strcmp(token, "history") == 0 && strcmp(strtok(NULL, delimitor), "1") == 0){
          int offset = atoi(strtok(NULL, delimiter));
          printHistory(offset);
        }
        
        // check for history, if history print history
        else if (strcmp(token, "history") == 0) {
            for (int i = 0; i < history_count; i++) {
                printf("%d: %s\n", i, history[i]);
            }
            continue;
        }

        addtoHistory(line);


        // Shell should execute the commands
        pid_t pid = fork();
        // if execution fails
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execvp(token, &token);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }

    }
    free(line);
    exit(EXIT_SUCCESS);
}
