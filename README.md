# COP4610: Operating Systems: Project 1
"The purpose of this project is to familiarize you with the mechanics of process control through the implementation of a shell user interface. This includes the relationship between child and parent processes, the steps needed to create a new process, including search of the path, and an introduction to user-input parsing and verification. Furthermore,you will come to understand how input/output redirection, pipes, and background processes are implemented."

https://github.com/trd16/OSProject1


Contributions
-------------
Scott: Built-Ins, Background Processing

Thomas: Parsing, Shortcut Resolution, Path Resolution

Taylor: Execution, I/O Redirection, Piping

Git Log
------------


Tar-Archive Contents
--------------------
shellp1.c

makefile

README.md

How We Compile Our Executables Using Makefile
---------------------------------------------
1. Type make.
2. Run executable shell.

Known Bugs
-----------
-After using the input operator, occasionally the compiler gets stuck but typing another command can fix the problem so that our shell may keep running.

-After using one pipe command, the shell breaks and you have to exit and restart.

-The background processing for I/O Redirection does not inform you when it has finished.

Unfinished Portions of the Project
------------------------------------
-Jobs code is written but it is not outputting to the screen as it should be.

Special Considerations
------------------------
There are no special considerations for our project.
