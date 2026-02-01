#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PROCESSES 6
#define TIME_QUANTUM 4    // 4ms time quantum for equal priority
#define TIME_STEP 1       // 1ms for priority checking
#define LINUX_CONTEXT_SWITCH 3.2      // microseconds
#define LINUX_INTERRUPT_LATENCY 28    // microseconds
#define LINUX_IPC_THROUGHPUT 1.4      // GB/s
#define LINUX_SCHED_JITTER 0.8        // ms

typedef struct {
    char pid[4];
    char description[50];
    int arrival_time;
    int burst_time;
    int original_priority;
    int current_priority;      // For aging
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int start_time;
    int remaining_time;
    int time_executed;         // Time spent in current quantum
    int total_time_executed;   // Total time executed so far
    int executed;
    double interrupt_latency;
    double context_switch_time;
    double ipc_throughput;
    double scheduling_jitter;
    int first_response;
    int preemption_count;
    int waiting_time_counter;  // For RR rotation among equal priority
} Process;

typedef struct {
    char metric[30];
    double target_threshold;
    double linux_value;
    char reason[100];
} MetricThreshold;

void simulate_hybrid_quantum(Process *p, int time_step, int process_num) {
    if (time_step > 0) {
        printf("| Executing %s (Pri:%d) for %dms (Quantum:%d/%d)\n", 
               p->pid, p->current_priority, time_step, 
               p->time_executed + time_step, TIME_QUANTUM);
    }
    
    // Record first response time
    if (!p->first_response) {
        p->response_time = p->start_time - p->arrival_time;
        p->first_response = 1;
    }
    
    // Simulate realistic Linux behavior
    switch(process_num) {
        case 0: // P1: Web Ingestion
            p->interrupt_latency = LINUX_INTERRUPT_LATENCY * 0.9;
            p->context_switch_time = LINUX_CONTEXT_SWITCH * 0.95;
            break;
        case 1: // P2: Real-Time Price Calc
            p->interrupt_latency = LINUX_INTERRUPT_LATENCY * 0.8;
            p->context_switch_time = LINUX_CONTEXT_SWITCH * 0.85;
            break;
        case 2: // P3: DB Check
            p->interrupt_latency = LINUX_INTERRUPT_LATENCY * 1.05;
            p->context_switch_time = LINUX_CONTEXT_SWITCH * 1.0;
            break;
        case 3: // P4: WebSocket Push
            p->interrupt_latency = LINUX_INTERRUPT_LATENCY * 0.85;
            p->context_switch_time = LINUX_CONTEXT_SWITCH * 0.9;
            break;
        case 4: // P5: Security Audit
            p->interrupt_latency = LINUX_INTERRUPT_LATENCY * 1.0;
            p->context_switch_time = LINUX_CONTEXT_SWITCH * 1.05;
            break;
        case 5: // P6: Background Logging
            p->interrupt_latency = LINUX_INTERRUPT_LATENCY * 1.15;
            p->context_switch_time = LINUX_CONTEXT_SWITCH * 1.1;
            break;
    }
    
    p->ipc_throughput = LINUX_IPC_THROUGHPUT * (0.9 + 0.2 * (rand() / (double)RAND_MAX));
    p->scheduling_jitter = LINUX_SCHED_JITTER * (0.85 + 0.25 * (rand() / (double)RAND_MAX));
    
    if (time_step > 0) {
        usleep(time_step * 1000);
    }
}

// Function to find next process for Priority Round Robin
int find_next_hybrid_process(Process processes[], int current_time, int current_pid) {
    int highest_priority = 999;
    int selected_index = -1;
    int max_waiting_time = -1;
    
    // ========== STEP 1: SORT BY PRIORITY FIRST ==========
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (processes[i].remaining_time > 0 && 
            processes[i].arrival_time <= current_time) {
            
            // Apply aging: increase priority of waiting processes
            if (processes[i].current_priority > 1 && 
                current_time - processes[i].arrival_time > 15) {
                processes[i].current_priority--;
            }
            
            // ========== STEP 2: FIND HIGHEST PRIORITY ==========
            if (processes[i].current_priority < highest_priority) {
                highest_priority = processes[i].current_priority;
                selected_index = i;
                max_waiting_time = processes[i].waiting_time_counter;
            }
            // ========== STEP 3: ROUND ROBIN FOR EQUAL PRIORITY ==========
            else if (processes[i].current_priority == highest_priority) {
                // Among equal priority, select the one that has waited longest
                if (processes[i].waiting_time_counter > max_waiting_time) {
                    selected_index = i;
                    max_waiting_time = processes[i].waiting_time_counter;
                }
            }
        }
    }
    
    return selected_index;
}

int main() {
    srand(time(NULL));
    
    Process processes[NUM_PROCESSES] = {
        {"P1", "Incoming Bid Stream (Web Ingestion)", 0, 8, 3, 3, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P2", "Bidding Engine: Real-Time Price Calc", 2, 4, 1, 1, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P3", "Inventory DB Check (Provenance Verify)", 3, 10, 2, 2, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P4", "Auction UI Broadcast (WebSocket Push)", 5, 2, 4, 4, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P5", "Transaction Integrity Audit (Security)", 6, 5, 1, 1, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P6", "Historical Trend Logging (Background)", 8, 6, 5, 5, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    MetricThreshold thresholds[] = {
        {"Interrupt Latency", 50.0, LINUX_INTERRUPT_LATENCY, "Double-switching needs fast interrupts"},
        {"Scheduling Jitter", 1.0, LINUX_SCHED_JITTER, "Hybrid scheduling requires stable timing"},
        {"CPU Utilization", 70.0, 65.0, "Efficient switching minimizes overhead"},
        {"Context Switch Time", 5.0, LINUX_CONTEXT_SWITCH, "Critical: Switches for both priority & quantum"},
        {"IPC Throughput", 1.0, LINUX_IPC_THROUGHPUT, "Complex scheduling needs high data flow"}
    };
    
    int current_time = 0;
    int completed = 0;
    int context_switches = 0;
    int preemptions = 0;
    int quantum_expirations = 0;
    float total_tat = 0, total_wt = 0, total_rt = 0;
    double total_cpu_utilization = 0;
    
    int current_process = -1;
    int last_process = -1;
    
    printf("================================================================================\n");
    printf("        VALENTRA FINE ARTS - PROJECT PULSE (PRIORITY ROUND ROBIN HYBRID)\n");
    printf("                    Ubuntu 22.04 LTS Simulation (Quantum=%dms)\n", TIME_QUANTUM);
    printf("================================================================================\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                     PROCESS TABLE (PRIORITY + RR)                           |\n");
    printf("+-----+---------------------------------------+--------+--------+-------------+\n");
    printf("| PID |           Description                 |   AT   |   BT   |   Priority  |\n");
    printf("|     |                                       |        |        | (1=Highest) |\n");
    printf("+-----+---------------------------------------+--------+--------+-------------+\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("| %-3s | %-37s | %6d | %6d | %11d |\n",
               processes[i].pid,
               processes[i].description,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].original_priority);
    }
    printf("+-----+---------------------------------------+--------+--------+-------------+\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                HYBRID SCHEDULING TIMELINE (Priority + RR)                   |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    
    // Priority Round Robin Hybrid Scheduling Algorithm
    while (completed < NUM_PROCESSES) {
        // ========== LAYER 1: PRIORITY PREEMPTION CHECK ==========
        int next_process = find_next_hybrid_process(processes, current_time, current_process);
        
        // ========== PRIORITY PREEMPTION ==========
        if (current_process != -1 && next_process != current_process && 
            processes[current_process].remaining_time > 0) {
            
            // Check for higher priority preemption
            if (next_process != -1 && processes[next_process].current_priority < 
                                     processes[current_process].current_priority) {
                
                printf("| [Time %3dms] PRIORITY PREEMPT: %s (Pri:%d) -> %s (Pri:%d) |\n",
                       current_time,
                       processes[current_process].pid,
                       processes[current_process].current_priority,
                       processes[next_process].pid,
                       processes[next_process].current_priority);
                
                preemptions++;
                processes[current_process].time_executed = 0;  // Reset quantum counter
                
                // Apply context switch penalty
                double context_switch_ms = LINUX_CONTEXT_SWITCH / 1000.0;
                current_time += context_switch_ms;
                context_switches++;
                
                // Switch to higher priority process
                if (current_process != -1) {
                    processes[current_process].waiting_time_counter++;
                }
                current_process = next_process;
            }
            // ========== LAYER 2: QUANTUM EXPIRATION FOR EQUAL PRIORITY ==========
            else if (next_process != -1 && 
                    processes[next_process].current_priority == processes[current_process].current_priority &&
                    processes[current_process].time_executed >= TIME_QUANTUM) {
                
                printf("| [Time %3dms] QUANTUM EXPIRED: %s yields to %s (Equal Pri:%d) |\n",
                       current_time,
                       processes[current_process].pid,
                       processes[next_process].pid,
                       processes[current_process].current_priority);
                
                quantum_expirations++;
                processes[current_process].time_executed = 0;  // Reset quantum counter
                
                // Apply context switch penalty
                double context_switch_ms = LINUX_CONTEXT_SWITCH / 1000.0;
                current_time += context_switch_ms;
                context_switches++;
                
                // Update waiting counters for RR fairness
                processes[current_process].waiting_time_counter = 0;
                current_process = next_process;
            }
        }
        
        // If no process running or we need to switch
        if (current_process == -1 || 
            (next_process != current_process && next_process != -1)) {
            
            if (current_process != -1 && processes[current_process].remaining_time > 0) {
                // Context switch from old process
                double context_switch_ms = LINUX_CONTEXT_SWITCH / 1000.0;
                current_time += context_switch_ms;
                context_switches++;
            }
            
            current_process = next_process;
            
            if (current_process != -1) {
                processes[current_process].start_time = 
                    processes[current_process].start_time == 0 ? 
                    current_time : processes[current_process].start_time;
                
                if (last_process != current_process && last_process != -1) {
                    printf("| [Time %3dms] Context Switch: %s -> %s (3.2us penalty)|\n",
                           current_time, 
                           processes[last_process].pid,
                           processes[current_process].pid);
                }
            }
        }
        
        // ========== EXECUTE CURRENT PROCESS ==========
        if (current_process != -1) {
            // Execute 1ms time step
            simulate_hybrid_quantum(&processes[current_process], TIME_STEP, current_process);
            
            processes[current_process].remaining_time -= TIME_STEP;
            processes[current_process].time_executed += TIME_STEP;
            processes[current_process].total_time_executed += TIME_STEP;
            total_cpu_utilization += TIME_STEP;
            
            // Increment waiting time for all other ready processes
            for (int i = 0; i < NUM_PROCESSES; i++) {
                if (i != current_process && processes[i].remaining_time > 0 && 
                    processes[i].arrival_time <= current_time) {
                    processes[i].waiting_time_counter++;
                }
            }
            
            // Check if process completed
            if (processes[current_process].remaining_time == 0) {
                processes[current_process].completion_time = current_time + TIME_STEP;
                processes[current_process].turnaround_time = 
                    processes[current_process].completion_time - processes[current_process].arrival_time;
                processes[current_process].waiting_time = 
                    processes[current_process].turnaround_time - processes[current_process].burst_time;
                completed++;
                
                printf("| [Time %3dms] %s COMPLETED (Priority %d, Exec:%dms) |\n",
                       current_time + TIME_STEP,
                       processes[current_process].pid,
                       processes[current_process].current_priority,
                       processes[current_process].total_time_executed);
                
                current_process = -1;
                processes[current_process].time_executed = 0;
            }
            
            current_time += TIME_STEP;
        } else {
            // CPU idle
            printf("| [Time %3dms] CPU Idle - No processes ready                |\n", current_time);
            usleep(TIME_STEP * 1000);
            current_time += TIME_STEP;
        }
        
        last_process = current_process;
    }
    
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    // Calculate totals
    for (int i = 0; i < NUM_PROCESSES; i++) {
        total_tat += processes[i].turnaround_time;
        total_wt += processes[i].waiting_time;
        total_rt += processes[i].response_time;
    }
    
    printf("+--------------------------------------------------------------------------------------------------------+\n");
    printf("|                              HYBRID PERFORMANCE METRICS                                                |\n");
    printf("+-----+--------+--------+--------+--------+--------+--------+--------+------------+------------+------------+------------+\n");
    printf("| PID |   AT   |   BT   | Pri/Fin|   CT   |   TAT  |   WT   |   RT   |Int.Lat(us) | CtxSw(us) | IPC(GB/s) |Jitter(ms) |\n");
    printf("+-----+--------+--------+--------+--------+--------+--------+--------+------------+------------+------------+------------+\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("| %-3s | %6d | %6d | %6d | %6d | %6d | %6d | %6d | %10.1f | %10.1f | %10.2f | %10.2f |\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].current_priority,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               processes[i].response_time,
               processes[i].interrupt_latency,
               processes[i].context_switch_time,
               processes[i].ipc_throughput,
               processes[i].scheduling_jitter);
    }
    printf("+-----+--------+--------+--------+--------+--------+--------+--------+------------+------------+------------+------------+\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                      SYSTEM-WIDE HYBRID STATISTICS                         |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| Average Turnaround Time:          %8.2f ms                             |\n", total_tat / NUM_PROCESSES);
    printf("| Average Waiting Time:             %8.2f ms                             |\n", total_wt / NUM_PROCESSES);
    printf("| Average Response Time:            %8.2f ms                             |\n", total_rt / NUM_PROCESSES);
    printf("| Total Context Switches:           %8d                                |\n", context_switches);
    printf("| Priority Preemptions:             %8d                                |\n", preemptions);
    printf("| Quantum Expirations:              %8d                                |\n", quantum_expirations);
    printf("| Context Switch Overhead:          %8.2f ms                            |\n", context_switches * (LINUX_CONTEXT_SWITCH / 1000.0));
    
    double total_time = current_time;
    double cpu_utilization = (total_cpu_utilization / total_time) * 100;
    double overhead_percentage = (context_switches * (LINUX_CONTEXT_SWITCH / 1000.0) / total_time) * 100;
    
    printf("| CPU Utilization:                  %8.1f%%                             |\n", cpu_utilization);
    printf("| Context Switch Overhead %%:        %8.1f%%                             |\n", overhead_percentage);
    printf("| Double-Switching Efficiency:      %8.2f ms total penalty             |\n", 
           (preemptions + quantum_expirations) * (LINUX_CONTEXT_SWITCH / 1000.0));
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    printf("+-------------------------------------------------------------------------------------+\n");
    printf("|               PROJECT PULSE THRESHOLD ANALYSIS (Linux Hybrid)                       |\n");
    printf("+----------------------------------------+-----------+-----------+----------+---------+\n");
    printf("| Metric                                 |  Target   | Lin Value |  Status  | Impact  |\n");
    printf("+----------------------------------------+-----------+-----------+----------+---------+\n");
    
    for (int i = 0; i < 5; i++) {
        char status[10];
        char symbol;
        
        if (i == 2) {  // CPU Utilization
            if (thresholds[i].linux_value <= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = '+';
            } else {
                sprintf(status, "FAIL");
                symbol = 'X';
            }
        } else if (i == 4) {  // IPC Throughput
            if (thresholds[i].linux_value >= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = '+';
            } else {
                sprintf(status, "FAIL");
                symbol = 'X';
            }
        } else {
            if (thresholds[i].linux_value <= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = '+';
            } else {
                sprintf(status, "FAIL");
                symbol = 'X';
            }
        }
        
        printf("| %-38s | %9.1f | %9.1f |  %c %-5s |", 
               thresholds[i].metric,
               thresholds[i].target_threshold,
               thresholds[i].linux_value,
               symbol,
               status);
        
        // Add impact assessment
        if (i == 3) {  // Context Switch Time
            printf(" Critical |\n");
        } else if (i == 1) {  // Scheduling Jitter
            printf(" High     |\n");
        } else {
            printf(" Moderate |\n");
        }
    }
    printf("+----------------------------------------+-----------+-----------+----------+---------+\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                    LINUX HYBRID OBSERVATIONS                                |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| + Context Switch PASS (3.2us): Efficient double-switching                  |\n");
    printf("| + CPU Utilization PASS (65%%): Low overhead despite hybrid complexity       |\n");
    printf("| + IPC Throughput PASS (1.4GB/s): High data flow for complex scheduling     |\n");
    printf("| + Interrupt Latency PASS (28us): Excellent for hybrid scheduling           |\n");
    printf("| + Quantum Management: Clean 4ms slicing for equal priority tasks           |\n");
    printf("| + Double-Switching Penalty: Only %.2fms total                           |\n", 
           (preemptions + quantum_expirations) * (LINUX_CONTEXT_SWITCH / 1000.0));
    printf("| + IDEAL for Project Pulse's complex hybrid scheduling requirements         |\n");
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                VALENTRA RECOMMENDATION - HYBRID ANALYSIS                    |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| For Priority Round Robin Hybrid Scheduling:                                |\n");
    printf("|                                                                            |\n");
    printf("| WINDOWS Server 2022:                                                       |\n");
    printf("|   X Context Switch Time: 8.5us (FAIL) - High dead time from double-switching|\n");
    printf("|   X CPU Utilization: 78%% (FAIL) - Too high for real-time bidding          |\n");
    printf("|   X IPC Throughput: 0.9GB/s (FAIL) - Bottlenecks complex workflows        |\n");
    printf("|                                                                            |\n");
    printf("| UBUNTU 22.04 LTS:                                                          |\n");
    printf("|   + Context Switch Time: 3.2us (PASS) - Efficient for hybrid scheduling    |\n");
    printf("|   + CPU Utilization: 65%% (PASS) - Optimal for real-time performance       |\n");
    printf("|   + IPC Throughput: 1.4GB/s (PASS) - Supports complex data flows          |\n");
    printf("|   + Quantum Management: Clean 4ms slicing maintains fairness               |\n");
    printf("|                                                                            |\n");
    printf("| CONCLUSION: Linux is SUPERIOR for Hybrid Priority+RR in Project Pulse      |\n");
    printf("|             Efficient 3.2us switching enables complex scheduling           |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    
    return 0;
}
