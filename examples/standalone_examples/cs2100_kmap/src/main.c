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

// Global variable for original minterms
char *originalMinterms[MAX_COUNT];
int originalMintermCount = 0;

// Function prototypes
void simplifySOP(char *minterms[], int count, char *xterms[], int xCount);
char* MergeMinterms(const char *minterm1, const char *minterm2);
int CheckDashesAlign(const char *minterm1, const char *minterm2);
int CheckMintermDifference(const char *minterm1, const char *minterm2);
void convertToExpressionFormat(char *primeImplicants[], int primeCount);
void CreatePrimeImplicantChart(char *primeImplicants[], int primeCount, char *chart[]);
void getEssentialPrimeImplicants(char *chart[], int chartSize, char *primeImplicants[], int primeCount, char *essentialPrimeImplicants[], int *essentialCount);
void calculateNonEssentialPrimeImplicants(char *chart[], int chartSize, char *primeImplicants[], int primeCount, char *nonEssentialPrimeImplicants[], int *nonEssentialCount);

void simplifySOP(char *minterms[], int count, char *xterms[], int xCount) {
    // Debug: Print the current level of recursion and the minterms being processed
    #if DEBUG
    dbg_printf("Entering simplifySOP with %d minterms and %d xterms:\n", count, xCount);
    for (int i = 0; i < count; i++) {
        dbg_printf("Minterm %d: %s (Pointer: %p)\n", i, minterms[i], (void*)minterms[i]);
    }
    for (int i = 0; i < xCount; i++) {
        dbg_printf("Xterm %d: %s (Pointer: %p)\n", i, xterms[i], (void*)xterms[i]);
    }
    #endif

    // Step 1: Compute the prime implicants from the list of minterms
    char *primeImplicants[MAX_COUNT] = {NULL};
    int merges[MAX_COUNT] = {0};
    int numberOfMerges = 0;
    int primeCount = 0;

    for (int i = 0; i < count; i++) {
        for (int c = i + 1; c < count; c++) {
            char *minterm1 = minterms[i];
            char *minterm2 = minterms[c];
            if (CheckDashesAlign(minterm1, minterm2) && CheckMintermDifference(minterm1, minterm2)) {
                char *mergedMinterm = strdup(MergeMinterms(minterm1, minterm2));
                int isDuplicate = 0;
                for (int j = 0; j < primeCount; j++) {
                    if (strcmp(primeImplicants[j], mergedMinterm) == 0) {
                        isDuplicate = 1;
                        break;
                    }
                }
                if (!isDuplicate) {
                    primeImplicants[primeCount++] = strdup(mergedMinterm);
                    numberOfMerges++;
                    merges[i] = 1;
                    merges[c] = 1;
                }
            }
        }
    }

    for (int i = 0; i < count; i++) {
        if (!merges[i]) {
            int isDuplicate = 0;
            for (int j = 0; j < primeCount; j++) {
                if (strcmp(primeImplicants[j], minterms[i]) == 0) {
                    isDuplicate = 1;
                    break;
                }
            }
            if (!isDuplicate) {
                primeImplicants[primeCount++] = strdup(minterms[i]);
            }
        }
    }

    if (numberOfMerges == 0) {
        // If no merges were possible, all minterms are prime implicants
        #if DEBUG
        dbg_printf("All minterms are prime implicants:\n");
        for (int k = 0; k < primeCount; k++) {
            dbg_printf("Prime Implicant %d: %s (Pointer: %p)\n", k, primeImplicants[k], (void*)primeImplicants[k]);
        }
        #endif

        // Create a prime implicant chart using the original set of minterms
        char *chart[MAX_COUNT];
        CreatePrimeImplicantChart(primeImplicants, primeCount, chart);

        // Get essential prime implicants
        char *essentialPrimeImplicants[MAX_COUNT];
        int essentialCount = 0;
        getEssentialPrimeImplicants(chart, originalMintermCount, primeImplicants, primeCount, essentialPrimeImplicants, &essentialCount);
        dbg_printf("Essential Prime Implicants (EPI):");
        dbg_printf("\n");
        for (int i = 0; i < essentialCount; i++) {
            dbg_printf("%s\n", essentialPrimeImplicants[i]);
        }

        // Print essential prime implicants
        for (int i = 0; i < essentialCount; i++) {
            os_PutStrFull(essentialPrimeImplicants[i]);
            os_NewLine();
        }
        os_NewLine();

        // Calculate and print the non-essential prime implicants
        char *nonEssentialPrimeImplicants[MAX_COUNT];
        int nonEssentialCount = 0;
        calculateNonEssentialPrimeImplicants(chart, originalMintermCount, primeImplicants, primeCount, nonEssentialPrimeImplicants, &nonEssentialCount);
        dbg_printf("Non-Essential Prime Implicants (NEPI):");
        dbg_printf("\n");
        for (int i = 0; i < nonEssentialCount; i++) {
            dbg_printf("%s\n", nonEssentialPrimeImplicants[i]);
        }

        // Convert and print in SOP format
        convertToExpressionFormat(essentialPrimeImplicants, essentialCount);

        // Free chart memory
        for (int i = 0; i < originalMintermCount; i++) {
            free(chart[i]);
        }
    } else {
        simplifySOP(primeImplicants, primeCount, xterms, xCount);
    }

    // Free allocated memory for primeImplicants
    for (int i = 0; i < primeCount; i++) {
        free(primeImplicants[i]);
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
    size_t commonLeadingZeros = SIZE_MAX;

    // Determine the number of common leading zeros across all prime implicants
    for (int i = 0; i < primeCount; i++) {
        size_t len = strlen(primeImplicants[i]);
        size_t leadingZeros = 0;
        for (size_t j = 0; j < len; j++) {
            if (primeImplicants[i][j] == '0') {
                leadingZeros++;
            } else {
                break;
            }
        }
        if (leadingZeros < commonLeadingZeros) {
            commonLeadingZeros = leadingZeros;
        }
    }

    for (int i = 0; i < primeCount; i++) {
        char term[20] = "";
        size_t len = strlen(primeImplicants[i]);

        // Construct the term by processing each character in the prime implicant
        for (size_t j = commonLeadingZeros; j < len; j++) {
            if (primeImplicants[i][j] != '-') {
                // Adjust the variable index to start from 'A' after leading zeros
                char var[3] = {variables[j - commonLeadingZeros], '\0', '\0'};
                if (primeImplicants[i][j] == '0') {
                    var[1] = '\'';
                }
                strcat(term, var);
            }
        }

        if (i > 0) {
            os_PutStrFull(" + ");
        }
        os_PutStrFull(term);
    }
    os_NewLine();
}

void CreatePrimeImplicantChart(char *primeImplicants[], int primeCount, char *chart[]) {
    #if DEBUG
    dbg_printf("Original Minterms (columns) we are interested in:\n");
    for (int j = 0; j < originalMintermCount; j++) {
        dbg_printf("Minterm %d: %s\n", j, originalMinterms[j]);
    }
    #endif
    for (int i = 0; i < primeCount; i++) {
        char *primeImplicant = primeImplicants[i];
        chart[i] = (char *)malloc(originalMintermCount + 1);
        for (int j = 0; j < originalMintermCount; j++) {
            int match = 1;
            size_t primeImplicantLength = strlen(primeImplicant);
            size_t matchCount = 0;
            for (size_t k = 0; k < primeImplicantLength; k++) {
                if (primeImplicant[k] == '-' || primeImplicant[k] == originalMinterms[j][k]) {
                    matchCount++;
                }
            }
            if (matchCount < primeImplicantLength) {
                match = 0;
            }
            chart[i][j] = match ? '1' : '0';
        }
        chart[i][originalMintermCount] = '\0';
        
        #if DEBUG
        dbg_printf("Chart row %d for prime implicant %s: %s\n", i, primeImplicants[i], chart[i]);
        #endif
    }
}

void getEssentialPrimeImplicants(char *chart[], int chartSize, char *primeImplicants[], int primeCount, char *essentialPrimeImplicants[], int *essentialCount) {
    #if DEBUG
    dbg_printf("Prime Implicant Chart:\n");
    for (int i = 0; i < primeCount; i++) {
        dbg_printf("Prime Implicant %d: %s\n", i, chart[i]);
    }
    #endif
    *essentialCount = 0;
    for (int j = 0; j < chartSize; j++) {
        int count = 0;
        int lastIndex = -1;
        for (int i = 0; i < primeCount; i++) {
            if (chart[i][j] == '1') {
                count++;
                lastIndex = i;
            }
        }
        if (count == 1) {
            essentialPrimeImplicants[(*essentialCount)++] = primeImplicants[lastIndex];
        }
    }
}

void calculateNonEssentialPrimeImplicants(char *chart[], int chartSize, char *primeImplicants[], int primeCount, char *nonEssentialPrimeImplicants[], int *nonEssentialCount) {
    *nonEssentialCount = 0;
    int isEssential[MAX_COUNT] = {0};
    int covered[MAX_COUNT] = {0};

    // Mark essential prime implicants and the minterms they cover
    for (int j = 0; j < chartSize; j++) {
        int count = 0;
        int lastIndex = -1;
        for (int i = 0; i < primeCount; i++) {
            if (chart[i][j] == '1') {
                count++;
                lastIndex = i;
            }
        }
        if (count == 1) {
            isEssential[lastIndex] = 1;
            covered[j] = 1;
        }
    }

    // Collect non-essential prime implicants that cover uncovered minterms
    for (int j = 0; j < chartSize; j++) {
        if (!covered[j]) {
            for (int i = 0; i < primeCount; i++) {
                if (!isEssential[i] && chart[i][j] == '1') {
                    nonEssentialPrimeImplicants[(*nonEssentialCount)++] = primeImplicants[i];
                    // Mark all minterms covered by this non-essential prime implicant
                    for (int k = 0; k < chartSize; k++) {
                        if (chart[i][k] == '1') {
                            covered[k] = 1;
                        }
                    }
                    break; // Choose only one non-essential implicant for this uncovered minterm
                }
            }
        }
    }
}

int main(void)
{
    char inputBuffer[INPUT_SIZE];
    char xInputBuffer[INPUT_SIZE];
    char *minterms[MAX_COUNT];
    char *xterms[MAX_COUNT];
    char *combinedTerms[MAX_COUNT * 2];
    char binaryMinterms[MAX_COUNT][7]; // 6 bits + null terminator
    char binaryXterms[MAX_COUNT][7]; // 6 bits + null terminator
    int count = 0;
    int xCount = 0;
    int combinedCount = 0;

    /* Clear the homescreen */
    os_ClrHome();

    /* Display message for bug reports */
    os_PutStrFull("DM @tch1000 for bugs thx");
    os_NewLine();
    os_PutStrFull("Works with up to 6 vars");
    os_NewLine();

    /* Ask the user to type the minterms */
    os_PutStrFull("minterms separated by ,:");
    os_NewLine();
    os_GetStringInput("", inputBuffer, INPUT_SIZE);
    os_NewLine();

    /* Parse the input string to extract minterms */
    char *token = strtok(inputBuffer, ",");
    while (token != NULL && count < MAX_COUNT) {
        int mintermValue = atoi(token);
        convertToBinaryString(mintermValue, binaryMinterms[count], 6);
        minterms[count] = binaryMinterms[count];
        originalMinterms[originalMintermCount++] = binaryMinterms[count];
        combinedTerms[combinedCount++] = binaryMinterms[count];
        count++;
        token = strtok(NULL, ",");
    }

    /* Ask the user to type the X terms */
    os_PutStrFull("xterms separated by ,:");
    os_NewLine();
    os_GetStringInput("", xInputBuffer, INPUT_SIZE);
    os_NewLine();

    /* Parse the input string to extract X terms */
    token = strtok(xInputBuffer, ",");
    while (token != NULL && xCount < MAX_COUNT) {
        int xtermValue = atoi(token);
        convertToBinaryString(xtermValue, binaryXterms[xCount], 6);
        xterms[xCount] = binaryXterms[xCount];
        combinedTerms[combinedCount++] = binaryXterms[xCount];
        xCount++;
        token = strtok(NULL, ",");
    }

    if (DEBUG) {
        for (int i = 0; i < count; i++) {
            dbg_printf("Minterm: %s\n", binaryMinterms[i]);
        }
        for (int i = 0; i < xCount; i++) {
            dbg_printf("X term: %s\n", binaryXterms[i]);
        }
        dbg_printf("Parsed %d minterms\n", count);
        dbg_printf("Parsed %d X terms\n", xCount);
    }

    /* Simplify the SOP using the combined terms */
    simplifySOP(combinedTerms, combinedCount, xterms, xCount);

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}