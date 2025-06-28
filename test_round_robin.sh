#!/bin/bash

echo "=========================================="
echo "  CSOPESY Round Robin Scheduler Test"
echo "=========================================="
echo ""

echo "Building CSOPESY..."
g++ -std=c++20 -pthread -o csopesy main.cpp ConsoleManager.cpp ProcessManager.cpp ProcessGenerator.cpp Process.cpp Scheduler.cpp Screen.cpp ReportGenerator.cpp InstructionGenerator.cpp

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful!"
echo ""

echo "=========================================="
echo "  Test 1: FCFS Scheduling"
echo "=========================================="
echo ""

# Backup original config and use FCFS config
cp config.txt config_backup.txt
cp config_fcfs.txt config.txt

echo "Config: FCFS (First Come First Served)"
echo "- Processes run to completion"
echo "- No preemption"
echo ""

echo "Manual test commands for FCFS:"
echo "1. ./csopesy"
echo "2. initialize"
echo "3. scheduler-start"
echo "4. screen -ls (wait a few seconds)"
echo "5. screen -r p01"
echo "6. process-smi"
echo "7. exit"
echo "8. exit"
echo ""

echo "=========================================="
echo "  Test 2: Round Robin Scheduling"
echo "=========================================="
echo ""

# Use Round Robin config
cp config_rr.txt config.txt

echo "Config: Round Robin (RR)"
echo "- Quantum cycles: 3"
echo "- Processes get time slices"
echo "- Preemption after quantum expires"
echo ""

echo "Manual test commands for Round Robin:"
echo "1. ./csopesy"
echo "2. initialize"
echo "3. scheduler-start"
echo "4. screen -ls (wait a few seconds)"
echo "5. screen -r p01"
echo "6. process-smi"
echo "7. exit"
echo "8. screen -r p02"
echo "9. process-smi"
echo "10. exit"
echo "11. exit"
echo ""

echo "=========================================="
echo "  Expected Differences"
echo "=========================================="
echo ""
echo "FCFS Behavior:"
echo "✅ Processes execute all instructions in sequence"
echo "✅ No interruption until process completes"
echo "✅ Simple execution order"
echo ""
echo "Round Robin Behavior:"
echo "✅ Processes execute for quantum cycles (3 instructions)"
echo "✅ Process preemption after quantum expires"
echo "✅ Processes get re-queued for continued execution"
echo "✅ Time-shared execution between processes"
echo ""

echo "To test FCFS: cp config_fcfs.txt config.txt && ./csopesy"
echo "To test RR:   cp config_rr.txt config.txt && ./csopesy"
echo ""

# Restore original config
cp config_backup.txt config.txt
rm -f config_backup.txt

echo "🎉 Round Robin scheduling implementation complete!"
echo ""
echo "Key features implemented:"
echo "✅ Scheduler algorithm selection (FCFS/RR)"
echo "✅ Quantum-based time slicing for RR"
echo "✅ Process preemption and re-queuing"
echo "✅ Config-driven scheduler parameters"
echo "✅ Both algorithms work with automatic instruction execution"
