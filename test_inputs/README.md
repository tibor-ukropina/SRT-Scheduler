# SRT-Scheduler
This is a program that emulates process scheduling by the CPU using shortest remaining time.

Project Summary: This program reads incoming "processes" which are handled by the program using the shortest remaining time dispatching algorithm. The procecess are
stored in files under the test_inputs directory, or you can enter your own. Each line contains the following information for a single process, [incoming time][PID]
[Run time][Ready time][Blocked time]. The program will output procecess with the following format: [pid][total running time][total ready time][total blocked time].

For example an input of:
100 12 800 500
300 11 1000 500
700 13 800

will output 
0 900
13 800 0 0
12 800 100 800
11 1000 1400 800

Tools used: C, make
