#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <math.h>

#define NUM_PROCESSES 6
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
    int executed;
    double interrupt_latency;
    double context_switch_time;
    double ipc_throughput;
    double scheduling_jitter;
} Process;

typedef struct {
    char metric[30];
    double target_threshold;
    double windows_value;
    char reason[100];
} MetricThreshold;

void simulate_task(Process *p, int process_num) {
    printf("| Executing %s - %s\n", p->pid, p->description);
    
    // Simulate realistic Windows behavior
    switch(process_num) {
        case 0: // P1: Web Ingestion
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.95;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.1;
            break;
        case 1: // P2: Real-Time Price Calc
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.85;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 0.9;
            break;
        case 2: // P3: DB Check
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.2;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.15;
            break;
        case 3: // P4: WebSocket Push
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.8;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.05;
            break;
        case 4: // P5: Security Audit
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.15;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.2;
            break;
        case 5: // P6: Background Logging
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.3;
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.25;
            break;
    }
    
    // Simulate OS-specific variations
    p->ipc_throughput = WINDOWS_IPC_THROUGHPUT * (0.85 + 0.3 * (rand() / (double)RAND_MAX));
    p->scheduling_jitter = WINDOWS_SCHED_JITTER * (0.9 + 0.4 * (rand() / (double)RAND_MAX));
    
    Sleep(p->burst_time);  // Simulate work
}

// Function to find next process for SJF
int find_next_sjf_process(Process processes[], int current_time) {
    int min_burst = 9999;
    int selected_index = -1;
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (!processes[i].executed && processes[i].arrival_time <= current_time) {
            if (processes[i].burst_time < min_burst) {
                min_burst = processes[i].burst_time;
                selected_index = i;
            }
        }
    }
    
    return selected_index;
}

int main() {
    srand(time(NULL));
    
    Process processes[NUM_PROCESSES] = {
        {"P1", "Incoming Bid Stream (Web Ingestion)", 0, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P2", "Bidding Engine: Real-Time Price Calc", 2, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P3", "Inventory DB Check (Provenance Verify)", 3, 10, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P4", "Auction UI Broadcast (WebSocket Push)", 5, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P5", "Transaction Integrity Audit (Security)", 6, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P6", "Historical Trend Logging (Background)", 8, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    MetricThreshold thresholds[] = {
        {"Interrupt Latency", 50.0, WINDOWS_INTERRUPT_LATENCY, "To 're-check' queue after short tasks finish"},
        {"Scheduling Jitter", 1.0, WINDOWS_SCHED_JITTER, "Ensures price calculations stay stable"},
        {"CPU Utilization", 70.0, 60.0, "SJF leaves more headroom for short bursts"},
        {"Context Switch Time", 5.0, WINDOWS_CONTEXT_SWITCH, "Impacts SJF's frequent queue re-evaluations"},
        {"IPC Throughput", 1.0, WINDOWS_IPC_THROUGHPUT, "Short jobs need fast data access"}
    };
    
    int current_time = 0;
    int completed = 0;
    float total_tat = 0, total_wt = 0, total_rt = 0;
    double total_cpu_utilization = 0;
    
    printf("==================================================================\n");
    printf("        VALENTRA FINE ARTS - PROJECT PULSE (SJF SCHEDULING)      \n");
    printf("                   Windows Server 2022 Simulation                \n");
    printf("==================================================================\n\n");
    
    printf("------------------------------------------------------------------\n");
    printf("                        PROCESS TABLE (SJF)                         \n");
    printf("------------------------------------------------------------------\n");
    printf(" PID |          Description               |   AT  |   BT  | Priority\n");
    printf("------------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf(" %-3s | %-34s | %5d | %5d | %7d \n",
               processes[i].pid,
               processes[i].description,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority);
    }
    printf("------------------------------------------------------------------\n\n");
    
    printf("------------------------------------------------------------------\n");
    printf("                      SJF EXECUTION TIMELINE                        \n");
    printf("------------------------------------------------------------------\n");
    
    // SJF Scheduling Algorithm
    while (completed < NUM_PROCESSES) {
        int next_index = find_next_sjf_process(processes, current_time);
        
        if (next_index == -1) {
            // No process available, advance time
            printf("| [Time %3dms] CPU Idle - Waiting for processes...                \n", current_time);
            current_time++;
            continue;
        }
        
        processes[next_index].start_time = current_time;
        processes[next_index].response_time = current_time - processes[next_index].arrival_time;
        
        printf("| [Time %3dms] SELECTED: %s (Shortest BT: %dms)              \n",
               current_time, processes[next_index].pid, processes[next_index].burst_time);
        
        simulate_task(&processes[next_index], next_index);
        
        processes[next_index].completion_time = current_time + processes[next_index].burst_time;
        processes[next_index].turnaround_time = processes[next_index].completion_time - processes[next_index].arrival_time;
        processes[next_index].waiting_time = processes[next_index].start_time - processes[next_index].arrival_time;
        
        current_time = processes[next_index].completion_time;
        processes[next_index].executed = 1;
        completed++;
        
        total_tat += processes[next_index].turnaround_time;
        total_wt += processes[next_index].waiting_time;
        total_rt += processes[next_index].response_time;
        total_cpu_utilization += processes[next_index].burst_time;
    }
    
    printf("------------------------------------------------------------------\n\n");
    
    printf("------------------------------------------------------------------\n");
    printf("                                         SJF PERFORMANCE METRICS                                          \n");
    printf("------------------------------------------------------------------\n");
    printf(" PID |   AT   |   BT   |   ST   |   CT   |   TAT  |   WT   |   RT   | Int.Lat(us)| CtxSw(us) | IPC(GB/s) | Jitter(ms)\n");
    printf("------------------------------------------------------------------\n");
    
    // Sort by completion time for display
    for (int i = 0; i < NUM_PROCESSES - 1; i++) {
        for (int j = i + 1; j < NUM_PROCESSES; j++) {
            if (processes[j].completion_time < processes[i].completion_time) {
                Process temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf(" %-3s | %6d | %6d | %6d | %6d | %6d | %6d | %6d | %10.1f | %10.1f | %10.2f | %10.2f \n",
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
    printf("------------------------------------------------------------------\n\n");
    
    printf("------------------------------------------------------------------\n");
    printf("                         SYSTEM-WIDE STATISTICS                     \n");
    printf("------------------------------------------------------------------\n");
    printf(" Average Turnaround Time: %8.2f ms                                \n", total_tat / NUM_PROCESSES);
    printf(" Average Waiting Time:    %8.2f ms                                \n", total_wt / NUM_PROCESSES);
    printf(" Average Response Time:   %8.2f ms                                \n", total_rt / NUM_PROCESSES);
    
    double total_time = current_time;
    double cpu_utilization = (total_cpu_utilization / total_time) * 100;
    printf(" CPU Utilization:         %8.1f%%                                \n", cpu_utilization);
    printf(" SJF Efficiency Gain:     %8.1f%% (vs theoretical FCFS)          \n", (1 - (total_wt/NUM_PROCESSES)/(total_tat/NUM_PROCESSES))*100);
    printf("------------------------------------------------------------------\n\n");
    
    printf("------------------------------------------------------------------\n");
    printf("                     PROJECT PULSE THRESHOLD ANALYSIS (Windows SJF)                           \n");
    printf("------------------------------------------------------------------\n");
    printf(" Metric                                     | Target  | Win Value   | Status     | Why It Matters for SJF             \n");
    printf("------------------------------------------------------------------\n");
    
    for (int i = 0; i < 5; i++) {
        char status[10];
        char symbol;
        
        if (i == 2) {  // CPU Utilization
            if (thresholds[i].windows_value <= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = 'Y';
            } else {
                sprintf(status, "FAIL");
                symbol = 'N';
            }
        } else if (i == 4) {  // IPC Throughput
            if (thresholds[i].windows_value >= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = 'Y';
            } else {
                sprintf(status, "FAIL");
                symbol = 'N';
            }
        } else {
            if (thresholds[i].windows_value <= thresholds[i].target_threshold) {
                sprintf(status, "PASS");
                symbol = 'Y';
            } else {
                sprintf(status, "FAIL");
                symbol = 'N';
            }
        }
        
        printf(" %-40s | %7.1f | %11.1f | %c %-8s | %-36s \n",
               thresholds[i].metric,
               thresholds[i].target_threshold,
               thresholds[i].windows_value,
               symbol,
               status,
               thresholds[i].reason);
    }
    printf("------------------------------------------------------------------\n\n");
    
    printf("------------------------------------------------------------------\n");
    printf("                             GANTT CHART                            \n");
    printf("------------------------------------------------------------------\n");
    printf(" ");
    
    // Display Gantt Chart
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (i > 0 && processes[i].start_time > processes[i-1].completion_time) {
            printf("[IDLE:%dms]", processes[i].start_time - processes[i-1].completion_time);
        }
        printf("[%s:%dms]", processes[i].pid, processes[i].burst_time);
    }
    printf("\n");
    printf(" 0");
    
    int pos = 3;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pos += 8 + (processes[i].burst_time >= 10 ? 1 : 0) + (processes[i].burst_time >= 100 ? 1 : 0);
        printf("%*d", pos - strlen(processes[i].pid) - 2, processes[i].completion_time);
        pos = 0;
    }
    printf("\n");
    printf("------------------------------------------------------------------\n\n");
    
    printf("------------------------------------------------------------------\n");
    printf("                    WINDOWS SJF OBSERVATIONS                        \n");
    printf("------------------------------------------------------------------\n");
    printf(" * SJF reduces convoy effect but requires frequent queue re-checks  \n");
    printf(" * Higher context-switch overhead (8.5us) affects SJF efficiency    \n");
    printf(" * Interrupt latency (45us) delays recognition of new shortest jobs \n");
    printf(" * Background services cause scheduling jitter (1.2ms avg)          \n");
    printf(" * Stable priority subsystem helps maintain order despite SJF       \n");
    printf("------------------------------------------------------------------\n");
    
    return 0;
}
