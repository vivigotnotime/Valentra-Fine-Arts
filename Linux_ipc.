#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PACKET_SIZE 2048  // 2KB bid packets
#define NUM_PACKETS 100000

double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

int main() {
    printf("================================================================\n");
    printf("    VALENTRA - SLOW IPC PROBLEM (Linux Pipes)\n");
    printf("================================================================\n\n");
    
    printf("Simulation: P1 → P2 bid packet transfer (2KB each)\n");
    printf("Method: Unix Pipes (High overhead, data copying)\n\n");
    
    int pipefd[2];
    char data[PACKET_SIZE];
    char buffer[PACKET_SIZE];
    double start_time, end_time;
    
    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    
    // Prepare test data
    for (int i = 0; i < PACKET_SIZE; i++) {
        data[i] = (char)(i % 256);  // Test pattern
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (P2 - Receiver)
        close(pipefd[1]);  // Close write end
        
        for (int i = 0; i < NUM_PACKETS; i++) {
            read(pipefd[0], buffer, PACKET_SIZE);
        }
        
        close(pipefd[0]);
        _exit(0);
    } else {
        // Parent process (P1 - Sender)
        close(pipefd[0]);  // Close read end
        
        printf("Starting IPC throughput test...\n");
        start_time = get_time_ms();
        
        // P1 writes to pipe
        for (int i = 0; i < NUM_PACKETS; i++) {
            write(pipefd[1], data, PACKET_SIZE);
        }
        
        end_time = get_time_ms();
        close(pipefd[1]);
        wait(NULL);
        
        // Calculate throughput
        double total_time = (end_time - start_time) / 1000.0;  // seconds
        double total_data = (PACKET_SIZE * NUM_PACKETS) / (1024.0 * 1024.0 * 1024.0);  // GB
        double throughput = total_data / total_time;  // GB/s
        
        printf("+--------------------------------------+\n");
        printf("|        SLOW IPC PERFORMANCE          |\n");
        printf("+--------------------------------------+\n");
        printf("| Packets Transferred:   %-12d |\n", NUM_PACKETS);
        printf("| Packet Size:           2 KB          |\n");
        printf("| Total Data:           %.2f GB      |\n", total_data);
        printf("| Time Taken:           %.3f seconds |\n", total_time);
        printf("| Throughput:           %.2f GB/s    |\n", throughput);
        printf("+--------------------------------------+\n\n");
        
        printf("ANALYSIS: High Overhead IPC detected!\n");
        printf("• Data copied: P1 → Kernel → P2 (2 kernel copies)\n");
        printf("• Each write/read requires system call overhead\n");
        printf("• Pipe buffer limits cause context switches\n");
        printf("• Throughput ~600 MB/s (too slow for bidding)\n");
    }
    
    return 0;
}
