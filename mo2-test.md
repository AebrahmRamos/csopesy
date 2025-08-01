## Test 1

    The parameters for your "config.txt" should be:
    num-cpu 2
    scheduler "fcfs" 
    quantum-cycles 0 
    batch-process-freq 1
    min-ins 4000
    max-ins 4000
    delay-per-exec 0
    max-overall-mem 512
    mem-per-frame 256
    min-mem-per-proc 512
    max-mem-per-proc 512

    1. Create two processes using the "screen" command.
    2. Wait for 5 seconds.
    3. Type the "screen -Is" command.
    4. Type the "process-smi" command.

    Expected output:

    While there are two CPU cores to allow two processes to execute, there is not enough memory. Thus, only the first process created gets executed in memory


## Test 2

    The parameters for your "config.txt" should be:
    num-cpu 16
    scheduler "rr"
    quantum-cycles 5
    batch-process-freq 1
    min-ins 1000
    max-ins 2000
    delay-per-exec 1
    max-overall-mem 32768
    mem-per-frame 32768
    min-mem-per-proc 32768
    max-mem-per-proc 32768
    
    1. Execute "scheduler-test" command.
    2. Wait for 5 seconds. 
    3. Type the "process-smi" command. 
    4. Type the "screen -Is" command. 
    5. Type the "vmstat" command.

    Expected output:

    For screen-Is, it should show < 100% CPU utilization printed in the screen-ls command, as only one process is allowed to run due to memory limits.

    For vmstat, It should show the following information such as total memory, used memory, and free memory. Used memory = 32768KB Free memory = 0KB

## Test 3
    The parameters for your "config.txt" should be: 
    num-cpu 1 
    scheduler "rr" 
    quantum-cycles 10 
    batch-process-freq 1 
    min-ins 1000 
    max-ins 1000 
    delay-per-exec 0 
    max-overall-mem 256 
    mem-per-frame 256 
    min-mem-per-proc 256 
    max-mem-per-proc 256

    1. Execute the following command:
    screen -c faulty_process "DECLARE varA 10; DECLARE varB 5; ADD varA varA varB; WRITE 0x500 varA; READ varC 0x500; PRINT(\"Variable A: \" + varA); PRINT(\"Result: \" + varC)"
    2. Type the "screen -ls" command.
    3. Type the "screen -r" command to access faulty_process


    The correct variables, A and C are printed in the console.

## Test 4
    This part is a STRESS TEST. Make sure your program + computer is prepared The parameters for your "config.txt" should be: 
    num-cpu 32 
    scheduler "fcfs" 
    quantum-cycles 5 
    batch-process-freq 1 
    min-ins 100 
    max-ins 100 
    delay-per-exec 0 
    max-overall-mem 4096 
    mem-per-frame 64 
    min-mem-per-proc 512 
    max-mem-per-proc 512
    

    1. Run the "scheduler-start" command
    2. Wait for 20 seconds.
    3. At every 2-second interval, type the "process-smi" command, and then type the "vmstat" command. Repeat this process for 20 seconds.

    Expected output: No deadlock occurs.

    The vmstat should clearly indicate a full memory, and the number of paged in/outs are continously increasing. If possible, the processes are moving to the "finished processes" list as they have short instructions.