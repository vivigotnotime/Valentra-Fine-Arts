#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <math.h>

#define NUM_PROCESSES 6
#define TIME_QUANTUM 4  // 4ms time quantum for Project Pulse
#define WINDOWS_CONTEXT_SWITCH 8.5    // microseconds
#define WINDOWS_INTERRUPT_LATENCY 45  // microseconds
#define WINDOWS_IPC_THROUGHPUT 0.9    // GB/s
#define WINDOWS_SCHED_JITTER 1.2      // ms

typedef struct {
    char pid[4];
    char description[50];
    int arrival_time;
    int burst_time;
    int priority;
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
    int first_response;  // Flag for first CPU allocation
} Process;

typedef struct {
    char metric[30];
    double target_threshold;
    double windows_value;
    char reason[100];
} MetricThreshold;

void simulate_rr_quantum(Process *p, int quantum, int process_num) {
    printf("| Executing %s for %dms quantum (Remaining: %dms)\n", 
           p->pid, quantum, p->remaining_time);
    
    // Record first response time
    if (!p->first_response) {
        p->response_time = p->start_time - p->arrival_time;
        p->first_response = 1;
    }
    
    // Simulate realistic Windows behavior for RR
    switch(process_num) {
        case 0: // P1: Web Ingestion
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.95;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.1;
            break;
        case 1: // P2: Real-Time Price Calc
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.85;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.0;
            break;
        case 2: // P3: DB Check
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.15;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.2;
            break;
        case 3: // P4: WebSocket Push
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.9;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.05;
            break;
        case 4: // P5: Security Audit
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.1;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.15;
            break;
        case 5: // P6: Background Logging
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.25;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.3;
            break;
    }
    
    // Simulate OS-specific variations
    p->ipc_throughput = WINDOWS_IPC_THROUGHPUT * (0.8 + 0.35 * (rand() / (double)RAND_MAX));
    p->scheduling_jitter = WINDOWS_SCHED_JITTER * (0.95 + 0.35 * (rand() / (double)RAND_MAX));
    
    Sleep(quantum);  // Simulate quantum execution
}

int main() {
    srand(time(NULL));
    
    Process processes[NUM_PROCESSES] = {
        {"P1", "Incoming Bid Stream (Web Ingestion)", 0, 8, 3, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0},
        {"P2", "Bidding Engine: Real-Time Price Calc", 2, 4, 1, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0},
        {"P3", "Inventory DB Check (Provenance Verify)", 3, 10, 2, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0},
        {"P4", "Auction UI Broadcast (WebSocket Push)", 5, 2, 4, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0},
        {"P5", "Transaction Integrity Audit (Security)", 6, 5, 1, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {"P6", "Historical Trend Logging (Background)", 8, 6, 5, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0}
    };
    
    MetricThreshold thresholds[] = {
        {"Interrupt Latency", 50.0, WINDOWS_INTERRUPT_LATENCY, "Required for timer interrupts in RR"},
        {"Scheduling Jitter", 1.0, WINDOWS_SCHED_JITTER, "Jitter ruins fairness of time quantum"},
        {"CPU Utilization", 70.0, 75.0, "RR overhead increases CPU usage"},
        {"Context Switch Time", 5.0, WINDOWS_CONTEXT_SWITCH, "Critical: RR switches tasks every TQ"},
        {"IPC Throughput", 1.0, WINDOWS_IPC_THROUGHPUT, "Preemption affects data flow consistency"}
    };
    
    int current_time = 0;
    int completed = 0;
    int context_switches = 0;
    float total_tat = 0, total_wt = 0, total_rt = 0;
    double total_cpu_utilization = 0;
    
    printf("================================================================================\n");
    printf("        VALENTRA FINE ARTS - PROJECT PULSE (ROUND ROBIN SCHEDULING)\n");
    printf("                    Windows Server 2022 Simulation (TQ=%dms)\n", TIME_QUANTUM);
    printf("================================================================================\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                          PROCESS TABLE (ROUND ROBIN)                        |\n");
    printf("+-----+---------------------------------------+--------+--------+-------------+\n");
    printf("| PID |           Description                 |   AT   |   BT   |   Priority  |\n");
    printf("+-----+---------------------------------------+--------+--------+-------------+\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("| %-3s | %-37s | %6d | %6d | %11d |\n",
               processes[i].pid,
               processes[i].description,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority);
    }
    printf("+-----+---------------------------------------+--------+--------+-------------+\n\n");
    
    printf("+-----------------------------------------------------------------------------+\n");
    printf("|                       RR EXECUTION TIMELINE (TQ=%dms)                       |\n", TIME_QUANTUM);
    printf("+-----------------------------------------------------------------------------+\n");
    
    // Round Robin Scheduling Algorithm
    while (completed < NUM_PROCESSES) {
        int all_idle = 1;
        
        for (int i = 0; i < NUM_PROCESSES; i++) {
            if (processes[i].remaining_time > 0 && processes[i].arrival_time <= current_time) {
                all_idle = 0;
                processes[i].start_time = processes[i].start_time == 0 ? current_time : processes[i].start_time;
                
                int exec_time = (processes[i].remaining_time > TIME_QUANTUM) ? TIME_QUANTUM : processes[i].remaining_time;
                
                printf("| [Time %3dms] Allocating CPU to %s for %dms (Quantum)   |\n", 
                       current_time, processes[i].pid, exec_time);
                
                simulate_rr_quantum(&processes[i], exec_time, i);
                
                // Apply context switch penalty (convert microseconds to milliseconds)
                double context_switch_ms = WINDOWS_CONTEXT_SWITCH / 1000.0;
                current_time += exec_time + context_switch_ms;
                context_switches++;
                
                processes[i].remaining_time -= exec_time;
                
                if (processes[i].remaining_time == 0) {
                    processes[i].completion_time = current_time;
                    processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
                    processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
                    completed++;
                    
                    printf("| -> %s COMPLETED at Time %dms                             |\n",
                           processes[i].pid, current_time);
                }
                
                if (completed == NUM_PROCESSES) break;
            }
        }
        
        if (all_idle) {
            printf("| [Time %3dms] CPU Idle - Waiting for processes...            |\n", current_time);
            current_time++;
        }
    }
    
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    // Calculate totals
    for (int i = 0; i < NUM_PROCESSES; i++) {
        total_tat += processes[i].turnaround_time;
        total_wt += processes[i].waiting_time;
        total_rt += processes[i].response_time;
        total_cpu_utilization += processes[i].burst_time;
    }
    
    printf("+--------------------------------------------------------------------------------------------------------+\n");
    printf("|                                     RR PERFORMANCE METRICS                                             |\n");
    printf("+-----+--------+--------+--------+--------+--------+--------+--------+------------+------------+------------+------------+\n");
    printf("| PID |   AT   |   BT   |   ST   |   CT   |   TAT  |   WT   |   RT   |Int.Lat(us) | CtxSw(us) | IPC(GB/s) |Jitter(ms) |\n");
    printf("+-----+--------+--------+--------+--------+--------+--------+--------+------------+------------+------------+------------+\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("| %-3s | %6d | %6d | %6d | %6d | %6d | %6d | %6d | %10.1f | %10.1f | %10.2f | %10.2f |\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].start_time,
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
    printf("|                         SYSTEM-WIDE RR STATISTICS                           |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| Average Turnaround Time:          %8.2f ms                             |\n", total_tat / NUM_PROCESSES);
    printf("| Average Waiting Time:             %8.2f ms                             |\n", total_wt / NUM_PROCESSES);
    printf("| Average Response Time:            %8.2f ms                             |\n", total_rt / NUM_PROCESSES);
    printf("| Total Context Switches:           %8d                                |\n", context_switches);
    printf("| Context Switch Overhead:          %8.2f ms                            |\n", context_switches * (WINDOWS_CONTEXT_SWITCH / 1000.0));
    
    double total_time = current_time;
    double cpu_utilization = (total_cpu_utilization / total_time) * 100;
    double overhead_percentage = (context_switches * (WINDOWS_CONTEXT_SWITCH / 1000.0) / total_time) * 100;
    
    printf("| CPU Utilization:                  %8.1f%%                             |\n", cpu_utilization);
    printf("| Context Switch Overhead %%:        %8.1f%%                             |\n", overhead_percentage);
    printf("+-----------------------------------------------------------------------------+\n\n");
    
    printf("+-------------------------------------------------------------------------------------+\n");
    printf("|                 PROJECT PULSE THRESHOLD ANALYSIS (Windows RR)                       |\n");
    printf("+----------------------------------------+-----------+-----------+----------+---------+\n");
    printf("| Metric                                 |  Target   | Win Value |  Status  | Impact  |\n");
    printf("+----------------------------------------+-----------+-----------+----------+---------+\n");
    
    for (int i = 0; i < 5; i++) {
        char status[10];
        char symbol;
        
        if (i == 2) {  // CPU Utilization
            if (thresholds[i].windows_value <= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = '+';
            } else {
                sprintf(status, "FAIL");
                symbol = 'X';
            }
        } else if (i == 4) {  // IPC Throughput
            if (thresholds[i].windows_value >= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = '+';
            } else {
                sprintf(status, "FAIL");
                symbol = 'X';
            }
        } else {
            if (thresholds[i].windows_value <= thresholds[i].target_threshold) {
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
               thresholds[i].windows_value,
               symbol,
               status);
        
        
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
    printf("|                    WINDOWS RR OBSERVATIONS                                  |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    printf("| X Context Switch FAIL (8.5us): High overhead degrades RR throughput         |\n");
    printf("| X Scheduling Jitter FAIL (1.2ms): Timer inconsistency affects fairness      |\n");
    printf("| + Interrupt Latency PASS (45us): Adequate for timer interrupts              |\n");
    printf("| - CPU Utilization HIGH (75%%): RR overhead consumes significant cycles      |\n");
    printf("| - Context Switch Overhead: %.2fms wasted on switching                   |\n", 
           context_switches * (WINDOWS_CONTEXT_SWITCH / 1000.0));
    printf("| - Not ideal for Project Pulse due to frequent preemption requirements       |\n");
    printf("+-----------------------------------------------------------------------------+\n");
    
    return 0;
}
