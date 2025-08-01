#!/bin/bash

echo "=== Manual Phase 2 Test ==="
echo "Testing memory operations manually..."

cat > temp_manual_test.exp << 'EOF'
#!/usr/bin/expect -f
set timeout 15

spawn ./csopesy

expect "Enter a command:"
send "initialize\r"

expect "Enter a command:"
send "screen -s vm_test 512\r"

sleep 1

expect "Enter a command:"
send "process-smi\r"

expect "Enter a command:"
send "vmstat\r"

expect "Enter a command:"
send "exit\r"

expect eof
EOF

chmod +x temp_manual_test.exp
echo "Running manual Phase 2 test..."
./temp_manual_test.exp

rm -f temp_manual_test.exp
echo "=== Manual Phase 2 Test Complete ==="