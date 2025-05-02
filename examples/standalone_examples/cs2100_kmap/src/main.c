#include <ti/getcsc.h>
#include <ti/screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <debug.h>

/* Set maximum size of input and output buffers */
#define INPUT_SIZE  100
#define RESP_SIZE   100
#define MAX_COUNT   64  // Updated to handle up to 6 boolean variables

/* Macro to enable or disable debugging */
#define DEBUG 1

// Function prototypes
void simplifySOP(char *minterms[], int count);
char* MergeMinterms(const char *minterm1, const char *minterm2);
int CheckDashesAlign(const char *minterm1, const char *minterm2);
int CheckMintermDifference(const char *minterm1, const char *minterm2);
char* ConvertToRegularExpression(const char *primeImplicant);
void convertToExpressionFormat(char *primeImplicants[], int primeCount);

void simplifySOP(char *minterms[], int count) {
    // Debug: Print the current level of recursion and the minterms being processed
    #if DEBUG
    dbg_printf("Entering simplifySOP with %d minterms:\n", count);
    for (int i = 0; i < count; i++) {
        dbg_printf("Minterm %d: %s (Pointer: %p)\n", i, minterms[i], (void*)minterms[i]);
    }
    #endif

    // Step 1: Compute the prime implicants from the list of minterms
    char *newMinterms[MAX_COUNT] = {NULL};
    int merges[MAX_COUNT] = {0};
    int numberOfMerges = 0;
    int newCount = 0;
    for (int i = 0; i < count; i++) {
        for (int c = i + 1; c < count; c++) {
            if (CheckDashesAlign(minterms[i], minterms[c]) && CheckMintermDifference(minterms[i], minterms[c])) {
                char *mergedMinterm = strdup(MergeMinterms(minterms[i], minterms[c]));
                
                // Check for duplicate minterms before adding
                int isDuplicate = 0;
                for (int j = 0; j < newCount; j++) {
                    if (strcmp(newMinterms[j], mergedMinterm) == 0) {
                        isDuplicate = 1;
                        break;
                    }
                }

                if (!isDuplicate) {
                    // Debug: Print the merging process
                    #if DEBUG
                    dbg_printf("Merging minterms %s and %s into %s\n", minterms[i], minterms[c], mergedMinterm);
                    #endif

                    newMinterms[newCount++] = mergedMinterm;
                    numberOfMerges++;
                    merges[i] = 1;
                    merges[c] = 1;
                } else {
                    free(mergedMinterm);
                }
            }
        }
    }
    #if DEBUG
    dbg_printf("Number of merges performed: %d\n", numberOfMerges);
    #endif

    int nonMergedCount = 0;
    for (int j = 0; j < count; j++) {
        if (!merges[j]) {
            nonMergedCount++;
            #if DEBUG
            dbg_printf("non-merged minterm found: %s (Pointer: %p)\n", minterms[j], (void*)minterms[j]);
            #endif
        }
    }

    // Check if all newMinterms are prime by attempting further simplification
    if (numberOfMerges > 0) {
        simplifySOP(newMinterms, newCount);
    } else {
        // If no merges were possible, all minterms are prime implicants
        #if DEBUG
        dbg_printf("All minterms are prime implicants:\n");
        for (int k = 0; k < count; k++) {
            if (!merges[k]) {
                dbg_printf("Prime Implicant %d: %s (Pointer: %p)\n", k, minterms[k], (void*)minterms[k]);
            }
        }
        #endif
        for (int k = 0; k < count; k++) {
            if (!merges[k]) {
                os_PutStrFull(minterms[k]);
                os_NewLine();
            }
        }
    }

    // Free allocated memory for newMinterms
    for (int i = 0; i < newCount; i++) {
        free(newMinterms[i]);
    }
}

char* MergeMinterms(const char *minterm1, const char *minterm2) {
    static char mergedMinterm[10];
    size_t len = strlen(minterm1);
    for (size_t i = 0; i < len; i++) {
        if (minterm1[i] != minterm2[i]) {
            mergedMinterm[i] = '-';
        } else {
            mergedMinterm[i] = minterm1[i];
        }
    }
    mergedMinterm[len] = '\0';
    return mergedMinterm;
}

int CheckDashesAlign(const char *minterm1, const char *minterm2) {
    size_t len = strlen(minterm1);
    for (size_t i = 0; i < len; i++) {
        if (minterm1[i] != '-' && minterm2[i] == '-') {
            return 0;
        }
    }
    return 1;
}

// This function checks if two minterms differ by exactly one bit position.
// It converts each minterm from a string to an integer, ignoring '-' characters.
// The XOR operation (m1 ^ m2) is used to find differing bits between the two minterms.
// The result is checked to ensure it is a power of two, which indicates a single bit difference.
int CheckMintermDifference(const char *minterm1, const char *minterm2) {
    int m1 = 0, m2 = 0;
    size_t len = strlen(minterm1);
    for (size_t i = 0; i < len; i++) {
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

void convertToExpressionFormat(char *primeImplicants[], int primeCount) {
    char variables[] = "ABCDEF";
    int effectiveVariables = 0;

    for (int i = 0; i < primeCount; i++) {
        char term[20] = "";
        int termVariables = 0;
        size_t len = strlen(primeImplicants[i]);
        for (size_t j = 0; j < len; j++) {
            if (primeImplicants[i][j] != '-') {
                char var[3] = {variables[j], '\0', '\0'};
                if (primeImplicants[i][j] == '0') {
                    var[1] = '\'';
                }
                strcat(term, var);
                termVariables++;
            }
        }
        if (i > 0) {
            os_PutStrFull(" + ");
        }
        os_PutStrFull(term);
        if (termVariables > effectiveVariables) {
            effectiveVariables = termVariables;
        }
    }
    os_NewLine();
    char effectiveVarStr[50];
    sprintf(effectiveVarStr, "Effective Variables: %d", effectiveVariables);
    os_PutStrFull(effectiveVarStr);
}

int main(void)
{
    char inputBuffer[INPUT_SIZE];
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
        for (int i = 0; i < count; i++) {
            dbg_printf("%s\n", binaryMinterms[i]);
        }
    }
    if (DEBUG) {
        dbg_printf("Parsed %d minterms\n", count);
    }

    /* Simplify the SOP using the minterms */
    simplifySOP(minterms, count);

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}