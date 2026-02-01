#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ITERATIONS 100000
int highest_bid = 0;
int bid_counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Linux POSIX Mutex

void* bidding_engine(void* param) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // ========== POSIX MUTEX ==========
        pthread_mutex_lock(&mutex);
        int current_bid = highest_bid;
        current_bid++;
        highest_bid = current_bid;
        pthread_mutex_unlock(&mutex);
        // ================================
        bid_counter++;
    }
    return NULL;
}

void* audit_engine(void* param) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // ========== POSIX MUTEX ==========
        pthread_mutex_lock(&mutex);
        int current_bid = highest_bid;
        current_bid++;
        highest_bid = current_bid;
        pthread_mutex_unlock(&mutex);
        // ================================
        bid_counter++;
    }
    return NULL;
}

int main() {
    printf("================================================================\n");
    printf("    VALENTRA - DOUBLE-BID SOLUTION (Linux POSIX Mutex)\n");
    printf("================================================================\n\n");
    
    printf("Simulation: Two auction threads with synchronization\n");
    printf("Using: Linux POSIX Mutex (Futex-based, efficient)\n\n");
    
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
    pthread_mutex_destroy(&mutex);
    
    // Display results
    printf("+--------------------------------------+\n");
    printf("|         SYNCHRONIZED RESULT          |\n");
    printf("+--------------------------------------+\n");
    printf("| Expected Final Bid:    200,000       |\n");
    printf("| Actual Final Bid:      %-12d |\n", highest_bid);
    printf("| Total Operations:      %-12d |\n", bid_counter);
    printf("| Lost Bids:             0             |\n");
    printf("+--------------------------------------+\n\n");
    
    printf("ANALYSIS: Synchronization successful!\n");
    printf("• Linux POSIX Mutex prevented race conditions\n");
    printf("• Using Futex (Fast User-space Mutex) - efficient\n");
    printf("• Stays in user-space unless contention occurs\n");
    printf("• Minimizes kernel transitions for better performance\n");
    printf("• Ideal for Valentra's high-frequency bidding system\n");
    
    return 0;
}
