
# CSOPESY - Process Scheduler and CLI Emulator

## Compilation
Add niyo nalang other cpp files if ever may madagdag
```bash
g++ -std=c++20 -pthread -o csopesy main.cpp ConsoleManager.cpp ProcessManager.cpp   ProcessGenerator.cpp Process.cpp Scheduler.cpp Screen.cpp ReportGenerator.cpp   InstructionGenerator.cpp MemoryManager.cpp VirtualMemoryManager.cpp
```

## Running

```bash
./csopesy
```

## Quick Start

1. Initialize the system: `initialize`
2. Start process generation: `scheduler-start`
3. View processes: `screen -ls`
4. Create/resume screen: `screen -s <name>` or `screen -r <name>`
5. Generate report: `report-util`
6. Help: `help`
7. Exit: `exit`

## Key Commands

- `initialize` - Initialize the system
- `scheduler-start` - Start automatic process generation
- `scheduler-stop` - Stop scheduler
- `screen -ls` - List screen sessions and CPU utilization
- `screen -s <name>` - Create new screen session
- `screen -r <name>` - Resume existing screen session
- `report-util` - Generate report to csopesy-log.txt
- `nvidia-smi` - Show GPU summary and processes
- `help` - Show all commands
- `exit` - Exit application