#include <stdio.h>
#include <windows.h>
#include <time.h>

#define PACKET_SIZE 2048  // 2KB bid packets
#define NUM_PACKETS 100000
#define PIPE_SIZE 65536

double get_time_ms() {
    return (double)GetTickCount();
}

int main() {
    printf("================================================================\n");
    printf("    VALENTRA - SLOW IPC PROBLEM (Windows Named Pipes)\n");
    printf("================================================================\n\n");
    
    printf("Simulation: P1 → P2 bid packet transfer (2KB each)\n");
    printf("Method: Named Pipes (High overhead, data copying)\n\n");
    
    char data[PACKET_SIZE];
    char buffer[PACKET_SIZE];
    HANDLE hPipe;
    DWORD bytesWritten, bytesRead;
    double start_time, end_time;
    
    // Create pipe
    hPipe = CreateNamedPipe(
        "\\\\.\\pipe\\ValentraBidPipe",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1, PIPE_SIZE, PIPE_SIZE, 0, NULL);
    
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("Error creating pipe\n");
        return 1;
    }
    
    // Prepare test data
    for (int i = 0; i < PACKET_SIZE; i++) {
        data[i] = (char)(i % 256);  // Test pattern
    }
    
    printf("Starting IPC throughput test...\n");
    start_time = get_time_ms();
    
    // Simulate P1 sending to P2
    for (int i = 0; i < NUM_PACKETS; i++) {
        // P1 writes to pipe (kernel copy)
        WriteFile(hPipe, data, PACKET_SIZE, &bytesWritten, NULL);
        
        // P2 reads from pipe (another kernel copy)
        ReadFile(hPipe, buffer, PACKET_SIZE, &bytesRead, NULL);
    }
    
    end_time = get_time_ms();
    CloseHandle(hPipe);
    
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
    printf("• Data copied: P1 → Kernel → P2 (2 copies)\n");
    printf("• Kernel transitions slow down transfer\n");
    printf("• Throughput limited by pipe buffer sizes\n");
    printf("• Not suitable for real-time bidding\n");
    
    return 0;
}

