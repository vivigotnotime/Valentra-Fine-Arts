#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ITERATIONS 100000
int highest_bid = 0;
int bid_counter = 0;

void* bidding_engine(void* param) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // ========== RACE CONDITION ==========
        int current_bid = highest_bid;
        current_bid++;
        highest_bid = current_bid;
        // ====================================
        bid_counter++;
    }
    return NULL;
}

void* audit_engine(void* param) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // ========== RACE CONDITION ==========
        int current_bid = highest_bid;
        current_bid++;
        highest_bid = current_bid;
        // ====================================
        bid_counter++;
    }
    return NULL;
}

int main() {
    printf("================================================================\n");
    printf("    VALENTRA - DOUBLE-BID PROBLEM (Linux Race Condition)\n");
    printf("================================================================\n\n");
    
    printf("Simulation: Two auction threads updating highest_bid simultaneously\n");
    printf("Expected: 200,000 bids | No synchronization = RACE CONDITION\n\n");
    
    pthread_t threads[2];
    
    // Reset counters
    highest_bid = 0;
    bid_counter = 0;
    
    // Create threads (simulating P2 and P5)
    pthread_create(&threads[0], NULL, bidding_engine, NULL);
    pthread_create(&threads[1], NULL, audit_engine, NULL);
    
    // Wait for both threads to complete
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    
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
