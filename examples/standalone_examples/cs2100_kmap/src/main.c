#include <ti/getcsc.h>
#include <ti/screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Set maximum size of input and output buffers */
#define INPUT_SIZE  100
#define RESP_SIZE   100
#define MAX_COUNT   64  // Updated to handle up to 6 boolean variables

/* Macro to enable or disable debugging */
#define DEBUG 1

// Function prototypes
void simplifySOP(char *minterms[], int count, char *result);
char* MergeMinterms(const char *minterm1, const char *minterm2);
int CheckDashesAlign(const char *minterm1, const char *minterm2);
int CheckMintermDifference(const char *minterm1, const char *minterm2);
char* ConvertToRegularExpression(const char *primeImplicant);

void simplifySOP(char *minterms[], int count, char *result) {
    // Step 1: Compute the prime implicants from the list of minterms
    char *primeImplicants[MAX_COUNT] = {NULL};
    int merges[MAX_COUNT] = {0};
    int numberOfMerges = 0;
    int primeCount = 0;

    for (int i = 0; i < count; i++) {
        for (int c = i + 1; c < count; c++) {
            if (CheckDashesAlign(minterms[i], minterms[c]) && CheckMintermDifference(minterms[i], minterms[c])) {
                char *mergedMinterm = MergeMinterms(minterms[i], minterms[c]);
                int found = 0;
                for (int p = 0; p < primeCount; p++) {
                    if (strcmp(primeImplicants[p], mergedMinterm) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    primeImplicants[primeCount++] = mergedMinterm;
                }
                numberOfMerges++;
                merges[i] = 1;
                merges[c] = 1;
            }
        }
    }

    for (int j = 0; j < count; j++) {
        if (!merges[j]) {
            int found = 0;
            for (int p = 0; p < primeCount; p++) {
                if (strcmp(primeImplicants[p], minterms[j]) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                primeImplicants[primeCount++] = minterms[j];
            }
        }
    }

    if (numberOfMerges == 0) {
        strcpy(result, "Simplified SOP: ");
        for (int i = 0; i < primeCount; i++) {
            strcat(result, primeImplicants[i]);
            strcat(result, " ");
        }
        return;
    } else {
        simplifySOP(primeImplicants, primeCount, result);
    }
}

char* MergeMinterms(const char *minterm1, const char *minterm2) {
    static char mergedMinterm[10];
    for (int i = 0; i < strlen(minterm1); i++) {
        if (minterm1[i] != minterm2[i]) {
            mergedMinterm[i] = '-';
        } else {
            mergedMinterm[i] = minterm1[i];
        }
    }
    mergedMinterm[strlen(minterm1)] = '\0';
    return mergedMinterm;
}

int CheckDashesAlign(const char *minterm1, const char *minterm2) {
    for (int i = 0; i < strlen(minterm1); i++) {
        if (minterm1[i] != '-' && minterm2[i] == '-') {
            return 0;
        }
    }
    return 1;
}

int CheckMintermDifference(const char *minterm1, const char *minterm2) {
    int m1 = 0, m2 = 0;
    for (int i = 0; i < strlen(minterm1); i++) {
        if (minterm1[i] != '-') {
            m1 = (m1 << 1) | (minterm1[i] - '0');
        }
        if (minterm2[i] != '-') {
            m2 = (m2 << 1) | (minterm2[i] - '0');
        }
    }
    int res = m1 ^ m2;
    return res != 0 && (res & (res - 1)) == 0;
}

void convertToBinaryString(int number, char *binaryString, int length) {
    for (int i = length - 1; i >= 0; i--) {
        binaryString[i] = (number & 1) ? '1' : '0';
        number >>= 1;
    }
    binaryString[length] = '\0';
}

int main(void)
{
    char inputBuffer[INPUT_SIZE];
    char response[RESP_SIZE];
    char *minterms[MAX_COUNT];
    char binaryMinterms[MAX_COUNT][7]; // 6 bits + null terminator
    int count = 0;

    /* Clear the homescreen */
    os_ClrHome();

    /* Display message for bug reports */
    os_PutStrFull("DM @tch1000 for bugs thx");
    os_NewLine();

    /* Ask the user to type the minterms */
    os_PutStrFull("Enter 6-var minterms separated by ,:");
    os_NewLine();
    os_GetStringInput("", inputBuffer, INPUT_SIZE);
    os_ClrHome();

    /* Parse the input string to extract minterms */
    char *token = strtok(inputBuffer, ",");
    while (token != NULL && count < MAX_COUNT) {
        int mintermValue = atoi(token);
        convertToBinaryString(mintermValue, binaryMinterms[count], 6);
        minterms[count] = binaryMinterms[count];
        count++;
        token = strtok(NULL, ",");
    }
    if (DEBUG) {
        char debugStr[50];
        sprintf(debugStr, "Parsed %d minterms", count);
        os_PutStrFull(debugStr);
        os_NewLine();
    }

    /* Simplify the SOP using the minterms */
    simplifySOP(minterms, count, response);
    os_PutStrFull(response);

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}
