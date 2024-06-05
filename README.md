# RUSH
## Rapid Unix Shell Clone

This is a unix shell clone. It supports redirection and multiple commands. To stop the program from executing, simply type exit. To change the current path, simply use the path command.

The path command takes 0 or more arguments, with each argument separated by whitespace from the others. A typical usage would be like this: rush> path /bin /usr/bin, which would add /bin and /usr/bin to the search path of the shell. If the user sets path to be empty, then the shell should not be able to run any programs (except built-in commands). The path command always overwrites the old path with the newly specified path.
