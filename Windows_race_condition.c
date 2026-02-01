#include <stdio.h>
#include <windows.h>

#define NUM_ITERATIONS 100000
int highest_bid = 0;
int bid_counter = 0;

DWORD WINAPI bidding_engine(LPVOID param) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // ========== RACE CONDITION ==========
        int current_bid = highest_bid;
        current_bid++;
        highest_bid = current_bid;
        // ====================================
        bid_counter++;
    }
    return 0;
}

DWORD WINAPI audit_engine(LPVOID param) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // ========== RACE CONDITION ==========
        int current_bid = highest_bid;
        current_bid++;
        highest_bid = current_bid;
        // ====================================
        bid_counter++;
    }
    return 0;
}

int main() {
    printf("================================================================\n");
    printf("    VALENTRA - DOUBLE-BID PROBLEM (Windows Race Condition)\n");
    printf("================================================================\n\n");
    
    printf("Simulation: Two auction threads updating highest_bid simultaneously\n");
    printf("Expected: 200,000 bids | No synchronization = RACE CONDITION\n\n");
    
    HANDLE threads[2];
    
    // Reset counters
    highest_bid = 0;
    bid_counter = 0;
    
    // Create threads (simulating P2 and P5)
    threads[0] = CreateThread(NULL, 0, bidding_engine, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, audit_engine, NULL, 0, NULL);
    
    // Wait for both threads to complete
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    
    // Display results
    printf("+--------------------------------------+\n");
    printf("|           RACE CONDITION RESULT      |\n");
    printf("+--------------------------------------+\n");
    printf("| Expected Final Bid:    200,000       |\n");
    printf("| Actual Final Bid:      %-12d |\n", highest_bid);
    printf("| Total Operations:      %-12d |\n", bid_counter);
    printf("| Lost Bids:             %-12d |\n", 200000 - highest_bid);
    printf("+--------------------------------------+\n\n");
    
    printf("ANALYSIS: Race Condition detected!\n");
    printf("• Bids were lost due to concurrent access\n");
    printf("• Final value should be 200,000 but is %d\n", highest_bid);
    printf("• %d bids disappeared due to preemption\n", 200000 - highest_bid);
    
    return 0;
}
