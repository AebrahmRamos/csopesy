
### Compile
```bash
g++ -std=c++20 -pthread main.cpp ConsoleManager.cpp Screen.cpp Process.cpp ProcessManager.cpp Scheduler.cpp -o csopesy
```

### Run
```bash
./csopesy
```

### nvidia-smi Commands
- `screen -ls` - List screen sessions
- `screen -s <name>` - create screen session
- `screen -r <name>` - resume screen session

### FCFS Scheduling
- `scheduler-test` - View FCFS process status (execute every 1-2s as per guideliens)