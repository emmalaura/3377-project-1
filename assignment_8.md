# Assignment 8

**Simple shell design** :

- **Input Loop**: continuous loop that prompts and waits for input of a command from the user
- **Parser**: breaks down user input into a _command_ and its _arguments_
- **executing executables:** create a child process using `fork()` and then we call `execvp` in the child process while our parent process waits for the child process to finish

**Native commands design :**

- **exit**
  - if we detect the exit command then we should just call the `exit()` system call with `EXIT_SUCCESS`
- **cd**
  - If we detect the cd command then we should just call the `chdir()` system call with the argument from the parser
  - If it errors or fails we should print the appropriate error to the command line
- **history**
  - After we successfully read an input we store the input into an internal list
  - we keep a running total of all of the commands that are input and if that total is above 100 (the maximum amount of commands we store at one time) we then delete the oldest command (the head) and then append the most recent command
  - for clearing the history we can just go through our linked list freeing each nodes memory
  - for getting a given index from history we can just iterate through our linked list until we reached the desired index and return to standard output our command

**Pipe design :**

- Pipe characters should be detected by the parser and we should identity the commands and arguments on either side of the pipe character
- we then create two pipes using the `pipe` system call
- forking twice we then create two child processes
- the first child process executes the first command and then reads it into the pipe,
- we then redirect the pipes read to the standard input of the second child and the pipes write end to standard output of the second child
