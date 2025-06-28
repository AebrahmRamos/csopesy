===============================================================================
                                    CSOPESY
===============================================================================

Authors: 
 - Ramos, Aebrahm Clyde P.
 - Dela Cruz, Rein Luiz S.
 - Morales, Alejandro Jose E.

===============================================================================
                                PROJECT OVERVIEW
===============================================================================

This project implements a process scheduler and command-line interface emulator
that supports both First Come First Served (FCFS) and Round Robin (RR) 
scheduling algorithms. The system can generate processes with randomized 
instruction sequences and execute them using configurable scheduling policies.

MAIN ENTRY POINT: main.cpp (contains the main() function)

===============================================================================
                               SYSTEM REQUIREMENTS
===============================================================================

- C++20 compatible compiler (g++, clang++)
- POSIX threads support (pthread)

===============================================================================
                               COMPILATION
===============================================================================

Compile using

g++ -std=c++20 -pthread -o csopesy main.cpp ConsoleManager.cpp ProcessManager.cpp ProcessGenerator.cpp Process.cpp Scheduler.cpp Screen.cpp ReportGenerator.cpp InstructionGenerator.cpp

This will create an executable named 'csopesy' in the current directory.


Run csopesy file

===============================================================================
                               CONFIGURATION
===============================================================================

Before running, ensure config.txt is present with the following parameters:

num-cpu <number>            - Number of CPU cores (default: 4)
scheduler <fcfs|rr>         - Scheduling algorithm (fcfs or rr)
quantum-cycles <number>     - Time quantum for RR scheduling
batch-process-freq <number> - Process generation frequency
min-ins <number>            - Minimum instructions per process
max-ins <number>            - Maximum instructions per process
delays-per-exec <number>    - Execution delays

Example configurations:
- For FCFS: scheduler fcfs
- For Round Robin: scheduler rr, quantum-cycles 3

===============================================================================
                               RUNNING THE PROGRAM
===============================================================================

1. After compiling, start the program:
   ./csopesy

2. Initialize the system:
   Enter command: initialize

3. Start the scheduler:
   Enter command: scheduler-start

4. View running processes:
   Enter command: screen -ls

5. Access a specific process:
   Enter command: screen -r <process_name>

6. Generate reports:
   Enter command: report-util

7. Exit the program:
   Enter command: exit

===============================================================================
                               SUPPORTED COMMANDS
===============================================================================

SYSTEM COMMANDS:
- initialize              - Initialize the system with config.txt
- scheduler-start         - Start automatic process generation
- scheduler-stop          - Stop the scheduler
- screen -ls              - List all processes and their status
- screen -s <name>        - Create a new screen session
- screen -r <name>        - Resume/access an existing process
- report-util             - Generate system report
- clear                   - Clear the screen
- help                    - Show available commands
- exit                    - Exit the program

PROCESS SCREEN COMMANDS (when inside a process):
- process-smi             - Show detailed process information
- exit                    - Return to main menu

===============================================================================
                               PROCESS INSTRUCTIONS
===============================================================================

The system supports the following process instructions:

1. PRINT("<message>")                   - Print a message
2. DECLARE(<var>, <value>)              - Declare a variable with value
3. ADD(<result>, <var1>, <var2>)        - Add two variables/values
4. SUBTRACT(<result>, <var1>, <var2>)   - Subtract two variables/values
5. SLEEP(<cycles>)                      - Sleep for specified cycles
6. FOR(<instructions>, <count>)         - Execute instructions in a loop

===============================================================================
                               TESTING
===============================================================================

MANUAL TESTING:
1. Test FCFS scheduling:
   - Set scheduler fcfs in config.txt
   - Run: initialize, scheduler-start, screen -ls
   - Observe sequential process completion

2. Test Round Robin scheduling:
   - Set scheduler rr in config.txt
   - Set quantum-cycles 3
   - Run: initialize, scheduler-start, screen -ls
   - Observe concurrent process execution
