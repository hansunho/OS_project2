# OS_project2
Implementation of a shell

Due by 11:59 pm of September 28, 2016

## Objectives 
There are three objectives to this assignment:
* To familiarize yourself with the Linux programming environment.
* To learn how processes are created, destroyed, and managed.
* To gain exposure to the necessary functionality in shells.

Assignment PDF can be found [here](https://uiowa.instructure.com/courses/8059/files/folder/Assignments?).
## Overview
In this assignment, you will implement a command line interpreter or, as it is more commonly known, a shell. The
shell should operate in this basic way: when you type in a command (in response to its prompt), the shell creates
a child process that executes the command you entered and then prompts for more user input when it has finished.
The shells you implement will be similar to, but simpler than, the one you run every day in Unix. You can find out
which shell you are running by typing "echo $SHELL" at a prompt. You may then wish to look at the man pages
for csh or the shell you are running (more likely tcsh or bash , or for those few wacky ones in the crowd, zsh or ksh
or even fish ) to learn more about all of the functionality that can be present. For this project, you do not need to
implement too much functionality.
