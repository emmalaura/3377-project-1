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
int front = 0; 
int rear = MAX_HISTORY - 1;

void addtoHistory(const char *cmd){
  if(history_count >= MAX_HISTORY){
    strcpy(history[rear], cmd);
    rear = (rear + 1) % MAX_HISTORY;
    front = (front + 1) % MAX_HISTORY;
    history_count++;
  }else{
    strcpy(history[history_count], cmd);
    history_count++;
  }
}
void printHistory(){
  int i = front;
  printf("History count: %d\n", history_count);
  if(history_count < MAX_HISTORY){
    for(i = 0; i < history_count; i++){
      printf("%d %s\n", i, history[i]);
    }
    return;
  }
  else if(history_count >= MAX_HISTORY){
    i = front;
    int counter = 0;
    do{
      printf("%d %s\n", counter, history[i]);
      i = (i + 1) % MAX_HISTORY;
      counter++;
    }while(i != front);
  }
  return;
}

void clearHistory(){
  for(int i = 0; i < MAX_HISTORY; i++){
    memset(history[i], 0, MAX_CMD_ARGUMENTS);
  }
  history_count = 0;
}

char* printHistoryOffset(int offset){
  int i = front;
  if(history_count >= MAX_HISTORY){
    return history[i + offset % MAX_HISTORY];
  }else{
    return history[offset % history_count];
  }
}

void executeCommand(char *input_token, char *saveptr1, char *delimiter, char *token){
    pid_t pid = fork();
    // if execution fails
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        char *args[MAX_CMD_ARGUMENTS];
        int i = 0;
        args[i++] = input_token;
        while ((token = strtok_r(NULL, delimiter, &saveptr1)) != NULL) {
            args[i++] = token;
            }
        args[i] = NULL;
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        wait(NULL); // Wait for the child process to finish
    }
}

int main(int argc, char *argv[]) {
    // Define the required variables
    char *line = NULL;
    char *token;
    char *delimiter = " ";
    size_t len = 0;
    ssize_t read;

  while (1) {
  // display prompt when waiting for input
  printf("%s", PROMPT);
  if ((read = getline(&line, &len, stdin)) == -1) {
      printf("Error reading prompt\n");
      break;
  }

  // Trim newline character
  if (line[read - 1] == '\n') {
      line[read - 1] = '\0';
  }

  
  // Tokenize input, to read line from stdin one at a time
  char *saveptr1;
  char *input_token = strtok_r(line, delimiter, &saveptr1);

  // checks for exit, if user enters exit, the shell will exit
  if (strcmp(input_token, "exit") == 0) {
      printf("Exiting\n");
      break;
  }
  addtoHistory(line);
  // check for cd
  if (strcmp(input_token, "cd") == 0) {
      // get the next token (directory to change to)
      char *subtoken = strtok_r(NULL, delimiter, &saveptr1);
      // if the token is not null, change to the directory provided by user
      if (subtoken != NULL) {
          if (chdir(subtoken) != 0) {
              perror("cd");
          }
          // if chdir fails, print error message
      } else {
          fprintf(stderr, "cd failed: missing directory\n");
      }
      continue;
  } else if(strcmp(input_token, "history") == 0){
      char *subtoken = strtok_r(NULL, delimiter, &saveptr1);
      if(subtoken == NULL){
        printHistory();
      }
      else if(strcmp(subtoken, "-c") == 0){
        clearHistory();
      }else{
        char* offsetCmd = printHistoryOffset(atoi(subtoken));
        executeCommand(offsetCmd, saveptr1, delimiter, token);
      }
    } else {
    // Shell should execute the commands
    executeCommand(input_token, saveptr1, delimiter, token);
    }
  }

    free(line);
    exit(EXIT_SUCCESS);
}
