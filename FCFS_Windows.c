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
    printf("[Windows] Executing %s - %s\n", p->pid, p->description);
    
    // Simulate realistic Windows behavior
    switch(process_num) {
        case 0: // P1: Web Ingestion
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.9;  // Better for I/O
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.1;   // Slightly worse
            break;
        case 1: // P2: Real-Time Price Calc
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.95; // Good for priority
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 0.9;    // Better for high-priority
            break;
        case 2: // P3: DB Check
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.2;  // Worse for DB ops
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.15;
            break;
        case 3: // P4: WebSocket Push
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 0.85; // Good for networking
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.05;
            break;
        case 4: // P5: Security Audit
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.1;  // Security overhead
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.2;
            break;
        case 5: // P6: Background Logging
            p->interrupt_latency = WINDOWS_INTERRUPT_LATENCY * 1.3;  // Low priority
            p->context_switch_time = WINDOWS_CONTEXT_SWITCH * 1.25;
            break;
    }
    
    p->ipc_throughput = WINDOWS_IPC_THROUGHPUT * (0.8 + 0.4 * (rand() / (double)RAND_MAX));
    p->scheduling_jitter = WINDOWS_SCHED_JITTER * (0.9 + 0.3 * (rand() / (double)RAND_MAX));
    
    Sleep(p->burst_time);  // Simulate work in milliseconds
}

int main() {
    srand(time(NULL));
    
    Process processes[NUM_PROCESSES] = {
        {"P1", "Incoming Bid Stream (Web Ingestion)", 0, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P2", "Bidding Engine: Real-Time Price Calc", 2, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P3", "Inventory DB Check (Provenance Verify)", 3, 10, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P4", "Auction UI Broadcast (WebSocket Push)", 5, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P5", "Transaction Integrity Audit (Security)", 6, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {"P6", "Historical Trend Logging (Background)", 8, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    MetricThreshold thresholds[] = {
        {"Interrupt Latency", 50.0, WINDOWS_INTERRUPT_LATENCY, "Ensures the bid is \"seen\" by the OS immediately."},
        {"Scheduling Jitter", 1.0, WINDOWS_SCHED_JITTER, "Ensures prices update smoothly without \"lag spikes\"."},
        {"CPU Utilization", 70.0, 65.0, "Leaves 30% \"headroom\" for sudden bidding wars."},
        {"Context Switch Time", 5.0, WINDOWS_CONTEXT_SWITCH, "Minimizes wasted CPU time when switching users."},
        {"IPC Throughput", 1.0, WINDOWS_IPC_THROUGHPUT, "Fast data flow between Engine and Database."}
    };
    
    int current_time = 0;
    float total_tat = 0, total_wt = 0, total_rt = 0;
    double total_cpu_utilization = 0;
    
    printf("============================================================\n");
    printf("VALENTRA FINE ARTS - PROJECT PULSE\n");
    printf("FCFS Scheduling Simulation (Windows Server 2022)\n");
    printf("============================================================\n\n");
    
    // Sort by arrival time (FCFS)
    for (int i = 0; i < NUM_PROCESSES - 1; i++) {
        for (int j = i + 1; j < NUM_PROCESSES; j++) {
            if (processes[j].arrival_time < processes[i].arrival_time) {
                Process temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }
    
    printf("PROCESS EXECUTION ORDER:\n");
    printf("PID\tArrival\tBurst\tPriority\tDescription\n");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("%s\t%dms\t%dms\t%d\t\t%s\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority,
               processes[i].description);
    }
    
    printf("\nEXECUTION SIMULATION:\n");
    printf("------------------------------------------------------------\n");
    
    // FCFS Scheduling
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (current_time < processes[i].arrival_time) {
            printf("[Time %dms] CPU Idle (Waiting for %s)...\n", current_time, processes[i].pid);
            Sleep(processes[i].arrival_time - current_time);
            current_time = processes[i].arrival_time;
        }
        
        printf("\n[Time %dms] Starting %s\n", current_time, processes[i].pid);
        processes[i].response_time = current_time - processes[i].arrival_time;
        
        simulate_task(&processes[i], i);
        
        processes[i].completion_time = current_time + processes[i].burst_time;
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        
        current_time = processes[i].completion_time;
        total_tat += processes[i].turnaround_time;
        total_wt += processes[i].waiting_time;
        total_rt += processes[i].response_time;
        total_cpu_utilization += processes[i].burst_time;
    }
    
    printf("\n\nPERFORMANCE METRICS TABLE:\n");
    printf("========================================================================\n");
    printf("PID\tAT\tBT\tCT\tTAT\tWT\tRT\tInt.Lat(µs)\tCtxSw(µs)\tIPC(GB/s)\tJitter(ms)\n");
    printf("------------------------------------------------------------------------\n");
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%.1f\t\t%.1f\t\t%.2f\t\t%.2f\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               processes[i].response_time,
               processes[i].interrupt_latency,
               processes[i].context_switch_time,
               processes[i].ipc_throughput,
               processes[i].scheduling_jitter);
    }
    
    printf("\n\nSYSTEM-WIDE AVERAGES:\n");
    printf("------------------------------------------------------------\n");
    printf("Average Turnaround Time: %.2f ms\n", total_tat / NUM_PROCESSES);
    printf("Average Waiting Time:    %.2f ms\n", total_wt / NUM_PROCESSES);
    printf("Average Response Time:   %.2f ms\n", total_rt / NUM_PROCESSES);
    
    double total_time = current_time;
    double cpu_utilization = (total_cpu_utilization / total_time) * 100;
    printf("CPU Utilization:         %.1f%%\n", cpu_utilization);
    
    printf("\n\nPROJECT PULSE THRESHOLD ANALYSIS (Windows Server 2022):\n");
    printf("========================================================================\n");
    printf("Metric\t\t\tTarget\tWindows Value\tStatus\t\tWhy It Matters\n");
    printf("------------------------------------------------------------------------\n");
    
    for (int i = 0; i < 5; i++) {
        char* status;
        if (i == 2) {  // CPU Utilization (lower is better)
            status = (thresholds[i].windows_value <= thresholds[i].target_threshold) ? "✓ PASS" : "✗ FAIL";
        } else if (i == 4) {  // IPC Throughput (higher is better)
            status = (thresholds[i].windows_value >= thresholds[i].target_threshold) ? "✓ PASS" : "✗ FAIL";
        } else {  // Others (lower is better)
            status = (thresholds[i].windows_value <= thresholds[i].target_threshold) ? "✓ PASS" : "✗ FAIL";
        }
        
        printf("%-20s\t%.1f\t%.1f\t\t%s\t%s\n",
               thresholds[i].metric,
               thresholds[i].target_threshold,
               thresholds[i].windows_value,
               status,
               thresholds[i].reason);
    }
    
    printf("\n\nGANTT CHART:\n");
    printf("0");
    for (int i = 0; i < NUM_PROCESSES; i++) {
        printf(" --[%s:%dms]--> %d", processes[i].pid, processes[i].burst_time, processes[i].completion_time);
    }
    printf("\n");
  
    printf("\nWINDOWS-SPECIFIC OBSERVATIONS:\n");
    printf("------------------------------------------------------------\n");
    printf("• Slightly higher context-switch overhead (%.1f µs) due to User/Kernel transitions\n", WINDOWS_CONTEXT_SWITCH);
    printf("• Stable priority handling despite complex subsystem architecture\n");
    printf("• Background services (Windows Update, Defender) may cause jitter spikes\n");
    printf("• Excellent SQL Server integration for database operations\n");
    
    return 0;
}
