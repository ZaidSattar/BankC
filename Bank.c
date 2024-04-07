#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ACCOUNTS 4 // define the NUM_ACCOUNTS as 4
#define NUM_THREADS 4 // 4 threads
#define MAX_TRANSACTIONS 50 // max transactions is just set to 50 so that it can process all of our transactions (less value here was causing the program to be interupted halfway)

int bankAccounts[NUM_ACCOUNTS];
pthread_mutex_t account_locks[NUM_ACCOUNTS];

typedef struct {
    int threadID;
    int transactions[MAX_TRANSACTIONS][2];
} ThreadArg;

void* transactionThread(void* arg) { // this will go through each of the transactions
    ThreadArg* threadArg = (ThreadArg*) arg;
    for (int i = 0; i < MAX_TRANSACTIONS && threadArg->transactions[i][0] != 0; ++i) {
        int accountIdx = threadArg->transactions[i][0] - 1;
        int bankAmount = threadArg->transactions[i][1];

        pthread_mutex_lock(&account_locks[accountIdx]);

        if (bankAmount < 0 && bankAccounts[accountIdx] + bankAmount < 0) { // checks if the bankAmount is less than 0 and shows insufficient funds
            printf("Thread %d: *** INSUFFICIENT FUNDS *** Account %d: $%d\n", threadArg->threadID, accountIdx + 1, bankAccounts[accountIdx]);
        } else {
            bankAccounts[accountIdx] += bankAmount;
            printf("Thread %d: %s $%d %s Account %d\n", threadArg->threadID, bankAmount < 0 ? "Withdraw" : "Deposit", abs(bankAmount), bankAmount < 0 ? "from" : "to", accountIdx + 1);
            printf("Account %d: $%d\n", accountIdx + 1, bankAccounts[accountIdx]);
        }

        pthread_mutex_unlock(&account_locks[accountIdx]);
    }

    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) { // this will just check if the correct amouunt of arguments are given
        fprintf(stderr, "Usage: %s <transaction file>\n", argv[0]); // if it is not, it will reiterate the format to the user
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file"); // error if the file cannot open
        return 1;
    }

    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        pthread_mutex_init(&account_locks[i], NULL); // the mutex for each account
        fscanf(file, "%d", &bankAccounts[i]);
    }

    printf("Balance\n"); // print statement for the balance
    for (int i = 0; i < NUM_ACCOUNTS; ++i) { // for loop that will print the account info
        printf("    Account %d: $%d\n", i + 1, bankAccounts[i]);
    }

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        ThreadArg* arg = (ThreadArg*)malloc(sizeof(ThreadArg));
        arg->threadID = i + 1;
        for (int j = 0; j < MAX_TRANSACTIONS; ++j) {
            if (fscanf(file, "%d %d", &arg->transactions[j][0], &arg->transactions[j][1]) != 2) {
                arg->transactions[j][0] = 0;
                break;
            }
        }
        pthread_create(&threads[i], NULL, transactionThread, arg);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL); // placeholder put so that it waits for all of the threads to finish
    }

    printf("Balance\n");
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        printf("    Account %d: $%d\n", i + 1, bankAccounts[i]);
    }

    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        pthread_mutex_destroy(&account_locks[i]); // destroy mutex
    }

    fclose(file); // close file
    return 0;
}


// Cited work: https://chat.openai.com/share/59123103-4560-4622-8164-31a3317bb96c
