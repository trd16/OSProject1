# COP4610: Operating Systems: Project 1
"The purpose of this project is to familiarize you with the mechanics of process control through the implementation of a shell user interface. This includes the relationship between child and parent processes, the steps needed to create a new process, including search of the path, and an introduction to user-input parsing and verification. Furthermore,you will come to understand how input/output redirection, pipes, and background processes are implemented."

https://github.com/trd16/OSProject1


Contributions
-------------
Scott: Built-Ins, Background Processing

Thomas: Parsing, Shortcut Resolution, Path Resolution

Taylor: Execution, I/O Redirection, Piping

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

-Jobs code is written but it is not outputting to the screen as it should be.

-More than one pipe runs as a single pipe.

Unfinished Portions of the Project
------------------------------------
There are no unfinished portions of the project, code was written for every part.

Special Considerations
------------------------
There are no special considerations for our project.

Git Log
------------

8b57126 - trd16, Tue Jan 28 22:37:34 2020 -0500 : Update README.md

c581195 - trd16, Tue Jan 28 22:35:33 2020 -0500 : Update README.md

e306744 - EarlySD, Tue Jan 28 22:34:42 2020 -0500 : added background func

af80c9b - trd16, Tue Jan 28 22:20:07 2020 -0500 : Update README.md

b5e3371 - trd16, Tue Jan 28 22:19:39 2020 -0500 : Update README.md

37379f6 - trd16, Tue Jan 28 22:15:56 2020 -0500 : Update shellP1.c

c4c8ae6 - trd16, Tue Jan 28 20:48:54 2020 -0500 : Update shellP1.c

7d36642 - tstar98, Tue Jan 28 20:46:36 2020 -0500 : Added variables to stru

908034c - trd16, Tue Jan 28 20:42:41 2020 -0500 : Update shellP1.c

d5bdfdf - trd16, Tue Jan 28 20:40:49 2020 -0500 : Update shellP1.c

ea51762 - tstar98, Tue Jan 28 20:33:18 2020 -0500 : Updated parser

029971a - trd16, Tue Jan 28 20:22:26 2020 -0500 : Update README.md

79ea645 - EarlySD, Tue Jan 28 19:39:16 2020 -0500 : added background procs,

ad2a3bb - tstar98, Tue Jan 28 19:24:21 2020 -0500 : Update shellP1.c

4530aa7 - trd16, Tue Jan 28 19:21:51 2020 -0500 : Update README.md

8a5fa1a - trd16, Tue Jan 28 12:24:46 2020 -0500 : Update README.md

1cab306 - trd16, Tue Jan 28 12:23:10 2020 -0500 : Update README.md

3c9bf11 - trd16, Tue Jan 28 12:16:47 2020 -0500 : Update shellP1.c

661dd19 - trd16, Tue Jan 28 12:00:35 2020 -0500 : Update shellP1.c

b3a4457 - trd16, Tue Jan 28 11:11:49 2020 -0500 : Update README.md

83ccf6e - trd16, Tue Jan 28 11:08:23 2020 -0500 : Update README.md

f3d88b6 - trd16, Tue Jan 28 11:07:38 2020 -0500 : Update README.md

bd98a12 - trd16, Tue Jan 28 11:06:09 2020 -0500 : Update README.md

b3c5578 - trd16, Tue Jan 28 11:03:29 2020 -0500 : Update README.md

ba67894 - trd16, Tue Jan 28 11:02:53 2020 -0500 : Update README.md

907ba2a - trd16, Mon Jan 27 12:13:08 2020 -0500 : Update README.md

de0698e - trd16, Mon Jan 27 12:12:31 2020 -0500 : Update README.md

037b37c - EarlySD, Sun Jan 26 22:52:13 2020 -0500 : new filename

b256828 - tstar98, Sun Jan 26 15:49:35 2020 -0500 : Added input and output 

35d8b3f - tstar98, Sun Jan 26 15:14:01 2020 -0500 : Updated parser, still w

60d8a70 - EarlySD, Sun Jan 26 14:56:44 2020 -0500 : Update parser_help.c

56158e7 - trd16, Sun Jan 26 14:53:29 2020 -0500 : Fixed execut

5bc90b6 - trd16, Sun Jan 26 14:39:00 2020 -0500 : Update README.md

57a77be - trd16, Sun Jan 26 14:34:08 2020 -0500 : Update README.md

4ee0369 - tstar98, Sun Jan 26 13:26:52 2020 -0500 : Fixed resolvePath funct

755e521 - EarlySD, Sun Jan 26 10:55:25 2020 -0500 : Update parser_help.c

5cff822 - EarlySD, Sun Jan 26 10:54:16 2020 -0500 : added built ins

5e8cac9 - trd16, Sat Jan 25 23:39:40 2020 -0500 : Updates to e

24ca8d9 - trd16, Sat Jan 25 22:35:45 2020 -0500 : Added pipe f

5bb7a7a - trd16, Sat Jan 25 22:22:12 2020 -0500 : Added indent

ea46d4c - trd16, Sat Jan 25 22:18:50 2020 -0500 : Added error 

5c695ae - tstar98, Tue Jan 21 14:06:33 2020 -0500 : Update parser_help.c

4e64d2d - tstar98, Tue Jan 21 14:04:28 2020 -0500 : Update parser_help.c

8f9c05f - EarlySD, Wed Jan 15 13:47:57 2020 -0500 : Update makefile

de97049 - EarlySD, Wed Jan 15 13:35:42 2020 -0500 : Create makefile

b84b43e - tstar98, Wed Jan 15 13:04:04 2020 -0500 : Update README.md

c00231f - EarlySD, Wed Jan 15 13:00:22 2020 -0500 : Update README.md

546d769 - tstar98, Wed Jan 15 12:53:50 2020 -0500 : Update README.md

667b093 - EarlySD, Wed Jan 15 12:51:55 2020 -0500 : Add files via upload

c579964 - trd16, Wed Jan 15 12:40:14 2020 -0500 : Initial commit




