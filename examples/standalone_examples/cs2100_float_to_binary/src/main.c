#include <ti/getcsc.h>
#include <ti/screen.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Set maximum size of input and output buffers */
#define INPUT_SIZE  20
#define RESP_SIZE   100

void floatToBinary(int integerPart, int decimalPart, int leadingZeros, char *binaryStr) {
    unsigned int sign = 0, exponent = 0, mantissa = 0;
    float absNum = integerPart + (float)decimalPart / pow(10, (int)log10(decimalPart) + 1 + leadingZeros);
    
    char floatStr[RESP_SIZE];
    if (leadingZeros > 0) {
        snprintf(floatStr, RESP_SIZE, "Float: %d.%0*d%d", integerPart, leadingZeros, 0, decimalPart);
    } else {
        snprintf(floatStr, RESP_SIZE, "Float: %d.%d", integerPart, decimalPart);
    }
    os_PutStrFull(floatStr);
    os_NewLine();
    // Determine the sign bit: 1 for negative, 0 for positive
    sign = (integerPart < 0) ? 1 : 0;
    os_PutStrFull(sign ? "Sign: 1 (negative)" : "Sign: 0 (positive)");
    os_NewLine();

    // Normalize the absolute value of the number to the range [1, 2)
    exponent = 0;
    while (absNum >= 2.0f) {
        absNum /= 2.0f;
        exponent++;
    }
    while (absNum < 1.0f && absNum != 0.0f) {
        absNum *= 2.0f;
        exponent--;
    }

    // Display the exponent without bias
    char exponentNoBiasStr[10];
    sprintf(exponentNoBiasStr, "%d", exponent);
    os_PutStrFull("E:");
    os_PutStrFull(exponentNoBiasStr);
    os_PutStrFull("+127=");
    // Adjust the exponent for the IEEE 754 bias
    exponent += 127;
    char exponentStr[10];
    sprintf(exponentStr, "%u", exponent);
    os_PutStrFull(exponentStr);
    os_PutStrFull("(");

    // Convert and display the exponent with bias in binary
    char exponentBinary[9];
    for (int i = 7; i >= 0; i--) {
        exponentBinary[7 - i] = (exponent & (1 << i)) ? '1' : '0';
    }
    exponentBinary[8] = '\0';
    os_PutStrFull(exponentBinary);
    os_PutStrFull(")");
    os_NewLine();

    // Remove the leading 1 for mantissa calculation
    absNum -= 1.0f;
    mantissa = 0;
    // Calculate the mantissa by multiplying by 2 and checking the integer part
    for (int i = 0; i < 23; i++) {
        absNum *= 2.0f;
        if (absNum >= 1.0f) {
            mantissa |= (1 << (22 - i));
            absNum -= 1.0f;
        }
    }
    mantissa += 1; // Round up the mantissa

    os_PutStrFull("M:");
    char mantissaStr[10];
    sprintf(mantissaStr, "%u", mantissa);
    os_PutStrFull(mantissaStr);
    os_PutStrFull("(");

    // Convert and display the mantissa in binary
    char mantissaBinary[24];
    for (int i = 22; i >= 0; i--) {
        mantissaBinary[22 - i] = (mantissa & (1 << i)) ? '1' : '0';
    }
    mantissaBinary[23] = '\0';
    os_PutStrFull(mantissaBinary);
    os_PutStrFull(")");
    os_NewLine();

    // Convert the sign, exponent, and mantissa to a binary string
    binaryStr[0] = sign ? '1' : '0';
    for (int i = 7; i >= 0; i--) {
        binaryStr[8 - i] = (exponentBinary[7 - i] == '1') ? '1' : '0';
    }
    for (int i = 22; i >= 0; i--) {
        binaryStr[31 - i] = (mantissaBinary[22 - i] == '1') ? '1' : '0';
    }
    binaryStr[32] = '\0';
}

void binaryToHex(const char *binaryStr, char *hexStr) {
    for (int i = 0; i < 8; i++) {
        unsigned int hexDigit = 0;
        for (int j = 0; j < 4; j++) {
            if (binaryStr[i * 4 + j] == '1') {
                hexDigit |= (1 << (3 - j));
            }
        }
        if (hexDigit < 10) {
            hexStr[i] = '0' + hexDigit;
        } else {
            hexStr[i] = 'a' + (hexDigit - 10);
        }
    }
    hexStr[8] = '\0';
}

void stringToFloatParts(const char *str, int *integerPart, int *decimalPart, int *leadingZeros) {
    *integerPart = 0;
    *decimalPart = 0;
    *leadingZeros = 0;
    int sign = 1;
    int i = 0;

    // Handle sign
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Convert integer part
    while (str[i] != '\0' && (str[i] >= '0' && str[i] <= '9')) {
        *integerPart = *integerPart * 10 + (str[i] - '0');
        i++;
    }

    // Convert decimal part if present
    if (str[i] == '.') {
        i++;
        int decimalDivisor = 1;
        while (str[i] != '\0' && (str[i] >= '0' && str[i] <= '9')) {
            if (*decimalPart == 0 && str[i] == '0') {
                (*leadingZeros)++;
            } else {
                *decimalPart = *decimalPart * 10 + (str[i] - '0');
            }
            decimalDivisor *= 10;
            i++;
        }
    }

    // Apply sign
    *integerPart *= sign;
}

int main(void)
{
    char inputBuffer[INPUT_SIZE];
    char binaryStr[33];
    char hexStr[9];
    char response[RESP_SIZE];
    int integerPart = 0, decimalPart = 0, leadingZeros = 0;

    /* Clear the homescreen */
    os_ClrHome();

    /* Ask the user to type a float number */
    os_PutStrFull("DM @tch1000 for bugs thx");
    os_NewLine();
    os_GetStringInput("Enter a float: ", inputBuffer, INPUT_SIZE);
    os_ClrHome();
    stringToFloatParts(inputBuffer, &integerPart, &decimalPart, &leadingZeros);
    floatToBinary(integerPart, decimalPart, leadingZeros, binaryStr);
    binaryToHex(binaryStr, hexStr);

    /* Clear the homescreen and display the formatted output */
    
    /* Display each piece of information on its own line */
    
    os_PutStrFull("B:");
    for (int j = 0; binaryStr[j] != '\0'; j += 4) {
        os_PutStrFull((char[]){binaryStr[j], binaryStr[j+1], binaryStr[j+2], binaryStr[j+3], ' ', '\0'});
    }
    os_NewLine();
    
    os_PutStrFull("H:");
    os_PutStrFull(hexStr);

    /* Waits for a key */
    while (!os_GetCSC());

    return 0;
}
