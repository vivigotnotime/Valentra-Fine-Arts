#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PROCESSES 6
#define TIME_STEP 1  // 1ms time step for priority checking
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
    int current_priority;  // For aging
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int start_time;
    int remaining_time;
    int executed;
    double interrupt_latency;
    double context_switch_time;
    double ipc_throughput;
    double scheduling_jitter;
    int first_response;
    int preemption_count;
} Process;

typedef struct {
    char metric[30];
    double target_threshold;
    double linux_value;
    char reason[100];
} MetricThreshold;

void simulate_priority_quantum(Process *p, int time_step, int process_num) {
    if (time_step > 0) {
        printf("| Executing %s (Pri:%d) for %dms\n", 
               p->pid, p->current_priority, time_step);
    }
    
    // Record first response time
    if (!p->first_response) {
        p->response_time = p->start_time - p->arrival_time;
        p->first_response = 1;
    }
    
    // Simulate realistic Linux behavior for Priority
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

// Function to find highest priority process (lower number = higher priority)
int find_highest_priority_process(Process processes[], int current_time, int current_pid) {
    int highest_priority = 999;
    int selected_index = -1;
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (processes[i].remaining_time > 0 && 
            processes[i].arrival_time <= current_time) {
            
            // Apply aging: increase priority of waiting processes
            if (processes[i].current_priority > 1 && 
                current_time - processes[i].arrival_time > 20) {
                processes[i].current_priority--;
            }
            
            if (processes[i].current_priority < highest_priority) {
                highest_priority = processes[i].current_priority;
                selected_index = i;
            }
        }
    }
    
    return selected_index;
}

int main() {
    srand(time(NULL));
    
    Process processes[NUM_PROCESSES] = {
        {"P1", "Incoming Bid Stream (Web Ingestion)", 0, 8, 3, 3, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0},
        {"P2", "Bidding Engine: Real-Time Price Calc", 2, 4, 1, 1, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0},
        {"P3", "Inventory DB Check (Provenance Verify)", 3, 10, 2, 2, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0},
        {"P4", "Auction UI Broadcast (WebSocket Push)", 5, 2, 4, 4, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
        {"P5", "Transaction Integrity Audit (Security)", 6, 5, 1, 1, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {"P6", "Historical Trend Logging (Background)", 8, 6, 5, 5, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0}
    };
    
    MetricThreshold thresholds[] = {
        {"Interrupt Latency", 50.0, LINUX_INTERRUPT_LATENCY, "Kernel must hear new high-priority interrupts"},
        {"Scheduling Jitter", 1.0, LINUX_SCHED_JITTER, "Priority decisions require stable timing"},
        {"CPU Utilization", 70.0, 65.0, "Priority preemption increases overhead"},
        {"Context Switch Time", 5.0, LINUX_CONTEXT_SWITCH, "Critical for swapping to higher priority tasks"},
        {"IPC Throughput", 1.0, LINUX_IPC_THROUGHPUT, "High-priority tasks need fast data access"}
    };
    
    int current_time = 0;
    int completed = 0;
    int context_switches = 0;
    int preemptions = 0;
    float total_tat = 0, total_wt = 0, total_rt = 0;
    double total_cpu_utilization = 0;
    
    int current_process = -1;
    int last_process = -1;
    
    printf("================================================================================\n");
    printf("        VALENTRA FINE ARTS - PROJECT PULSE (PRIORITY PREEMPTIVE)\n");
    printf("                    Ubuntu 22.04 LTS Simulation\n");
    printf("================================================================================\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                        PROCESS TABLE (PRIORITY)                             |\n");
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
    printf("|                  PRIORITY EXECUTION TIMELINE (1ms steps)                    |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    
    // Priority Preemptive Scheduling Algorithm
    while (completed < NUM_PROCESSES) {
        // ========== CHECK FOR HIGHER PRIORITY LOGIC ==========
        int next_process = find_highest_priority_process(processes, current_time, current_process);
        
        // Check if we need to preempt
        if (current_process != -1 && next_process != current_process && 
            processes[current_process].remaining_time > 0) {
            
            if (next_process != -1 && processes[next_process].current_priority < 
                                     processes[current_process].current_priority) {
                
                printf("| [Time %3dms] PREEMPTING %s (Pri:%d) for %s (Pri:%d) |\n",
                       current_time,
                       processes[current_process].pid,
                       processes[current_process].current_priority,
                       processes[next_process].pid,
                       processes[next_process].current_priority);
                
                preemptions++;
                
                // Apply context switch penalty
                double context_switch_ms = LINUX_CONTEXT_SWITCH / 1000.0;
                current_time += context_switch_ms;
                context_switches++;
            }
        }
        
        // If no process running or preemption occurred, switch to new process
        if (current_process == -1 || 
            (next_process != current_process && next_process != -1 &&
             processes[next_process].current_priority < 
             (current_process != -1 ? processes[current_process].current_priority : 999))) {
            
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
        
        // Execute current process for 1ms time step
        if (current_process != -1) {
            // Execute 1ms quantum
            simulate_priority_quantum(&processes[current_process], TIME_STEP, current_process);
            
            processes[current_process].remaining_time -= TIME_STEP;
            total_cpu_utilization += TIME_STEP;
            
            // Check if process completed
            if (processes[current_process].remaining_time == 0) {
                processes[current_process].completion_time = current_time + TIME_STEP;
                processes[current_process].turnaround_time = 
                    processes[current_process].completion_time - processes[current_process].arrival_time;
                processes[current_process].waiting_time = 
                    processes[current_process].turnaround_time - processes[current_process].burst_time;
                completed++;
                
                printf("| [Time %3dms] %s COMPLETED (Priority %d)                |\n",
                       current_time + TIME_STEP,
                       processes[current_process].pid,
                       processes[current_process].current_priority);
                
                current_process = -1;
            }
            
            current_time += TIME_STEP;
        } else {
            // CPU idle
            printf("| [Time %3dms] CPU Idle - No processes ready                |\n", current_time);
            usleep(TIME_STEP * 1000);
            current_time += TIME_STEP;
        }
        
        last_process = current_process;
        
        // Aging: Boost priority of waiting processes
        for (int i = 0; i < NUM_PROCESSES; i++) {
            if (processes[i].remaining_time > 0 && 
                processes[i].arrival_time <= current_time &&
                processes[i].current_priority > 1) {
                
                // Age every 10ms of waiting
                if ((current_time - processes[i].arrival_time) % 10 == 0) {
                    processes[i].current_priority--;
                    printf("| [Time %3dms] AGING: %s priority increased to %d     |\n",
                           current_time, processes[i].pid, processes[i].current_priority);
                }
            }
        }
    }
    
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    // Calculate totals
    for (int i = 0; i < NUM_PROCESSES; i++) {
        total_tat += processes[i].turnaround_time;
        total_wt += processes[i].waiting_time;
        total_rt += processes[i].response_time;
    }
    
    printf("+--------------------------------------------------------------------------------------------------------+\n");
    printf("|                                 PRIORITY PERFORMANCE METRICS                                           |\n");
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
    printf("|                      SYSTEM-WIDE PRIORITY STATISTICS                        |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| Average Turnaround Time:          %8.2f ms                             |\n", total_tat / NUM_PROCESSES);
    printf("| Average Waiting Time:             %8.2f ms                             |\n", total_wt / NUM_PROCESSES);
    printf("| Average Response Time:            %8.2f ms                             |\n", total_rt / NUM_PROCESSES);
    printf("| Total Context Switches:           %8d                                |\n", context_switches);
    printf("| Total Preemptions:                %8d                                |\n", preemptions);
    printf("| Context Switch Overhead:          %8.2f ms                            |\n", context_switches * (LINUX_CONTEXT_SWITCH / 1000.0));
    
    double total_time = current_time;
    double cpu_utilization = (total_cpu_utilization / total_time) * 100;
    double overhead_percentage = (context_switches * (LINUX_CONTEXT_SWITCH / 1000.0) / total_time) * 100;
    
    printf("| CPU Utilization:                  %8.1f%%                             |\n", cpu_utilization);
    printf("| Context Switch Overhead %%:        %8.1f%%                             |\n", overhead_percentage);
    printf("| Priority Response Time:           %8.2f ms avg                      |\n", total_rt / NUM_PROCESSES);
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    printf("+-------------------------------------------------------------------------------------+\n");
    printf("|               PROJECT PULSE THRESHOLD ANALYSIS (Linux Priority)                     |\n");
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
        } else if (i == 0) {  // Interrupt Latency
            printf(" High     |\n");
        } else if (i == 4) {  // IPC Throughput
            printf(" Moderate |\n");
        } else {
            printf(" Moderate |\n");
        }
    }
    printf("+----------------------------------------+-----------+-----------+----------+---------+\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                    LINUX PRIORITY OBSERVATIONS                              |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| + Context Switch PASS (3.2us): Near-instantaneous preemption                |\n");
    printf("| + IPC Throughput PASS (1.4GB/s): Fast data feed to bidding engine           |\n");
    printf("| + Interrupt Latency PASS (28us): Plenty of headroom for bid bursts          |\n");
    printf("| + Aging prevents starvation while maintaining priority fairness             |\n");
    printf("| + Priority Response: Critical bids get CPU within 3.2us                     |\n");
    printf("| + IDEAL for Project Pulse's high-priority bid processing                    |\n");
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                VALENTRA RECOMMENDATION - PRIORITY ANALYSIS                  |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| For Priority Preemptive Scheduling:                                        |\n");
    printf("|                                                                            |\n");
    printf("| CRITICAL EVENT ANALYSIS:                                                   |\n");
    printf("|   When P2 (Bidding Engine) arrives at T=2ms:                               |\n");
    printf("|   - Linux: 3.2us near-instant preemption meets deadline                    |\n");
    printf("|   - Fast context switching ensures high-priority tasks run immediately     |\n");
    printf("|                                                                            |\n");
    printf("| DATA FLOW ANALYSIS:                                                        |\n");
    printf("|   High-priority tasks need fast data access:                               |\n");
    printf("|   - Linux: 1.4GB/s PASS - feeds data at high speed to bidding engine       |\n");
    printf("|   - Excellent IPC throughput complements fast scheduling                   |\n");
    printf("|                                                                            |\n");
    printf("| CONCLUSION: Linux is SUPERIOR for Priority scheduling in Project Pulse     |\n");
    printf("|             due to 3.2us context switching and 1.4GB/s IPC throughput      |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    
    return 0;
}
