// Assumes all string arrays passed into functions ends with -1
#include <stdio.h>
#include <stdlib.h>     // For malloc, realloc, calloc
#include <string.h>     // For strlen, strtok, strstr, strcat

#include "basiclib_string.h"

// Find all unique ASCII characters in the string
char* uniqueASCIICharInStr(char* str) {
    // Calloc to init everything as 0
    int* chars = calloc(128, sizeof(int));

    for (int i = 0; i < strlen(str); i++) {
        chars[str[i]]++;
    }

    char* out = malloc(1);
    int outSize = 1;
    for (int i = 0; i < 128; i++) {
        if (chars[i] > 0) {
            outSize++;
            out = realloc(out, outSize*sizeof(char));
            out[outSize-2] = i;
        }
    }
    out[outSize-1] = '\0';

    return out;
}

// Find if a character is in a string
int charInStr(char c, char* str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == c) {
            return 1;
        }
    }
    return 0;
}

// Terminate string array by adding -1 as last string
// Allows getting size of array without storing in a variable
void termStrArr(char*** str, int size) {
    *str = realloc(*str, (size+1) * sizeof(char*));
    (*str)[size] = malloc(sizeof(char));
    (*str)[size][0] = -1;
}

// Split string by characters matching any in delimiter
char** strtokSplit(char* str, char* delimiters) {
    char** out = malloc(0);
    int outSize = 0;

    char * token = strtok(str, delimiters);		// TODO: strtok() does not work with multi-char
    while( token != NULL ) {
      outSize++;
      out = realloc(out, outSize*sizeof(char*));
      out[outSize-1] = token;

      token = strtok(NULL, delimiters);
    }
    termStrArr(&out, outSize);

    return out;
}

// Split string by matching substrings
char** strSplit(char* str, char* delim) {
	char** out = malloc(0);
	int outSize = 0;

	char* strFront;
	char* strAfter;

	strAfter = strstr(str, delim);
	while (strAfter != NULL) {
		// Get all characters before delimiter is found
		strFront = malloc(((int)(strAfter-str) + 1) * sizeof(char));
		for (int i=0; i<((int)(strAfter - str)); i++) {
			strFront[i] = str[i];
		}
        strFront[((int)(strAfter - str))] = '\0';
        // If not two back to back delimiters
        if (strFront[0] != '\0') {
            // Append characters to output
            outSize++;
            out = realloc(out, outSize * sizeof(char*));
            out[outSize-1] = strFront;
        }
		// Let string start from after delimiter
		str = str + ((int)(strAfter-str)) + strlen(delim);
		strAfter = strstr(str, delim);
	}
	// If string does not end on delimiter
	if (strlen(str) > 0) {
		outSize++;
		out = realloc(out, outSize * sizeof(char*));
		out[outSize-1] = str;
	}

    termStrArr(&out, outSize);
	return out;
}


char* strJoin(char** arr, char* str) {
    int outSize = strlen(arr[0]);
    char* out = malloc(outSize * sizeof(char));
    strcat(out, arr[0]);

    int i=1;
    while (arr[i][0] != -1) {
        outSize += strlen(str) + strlen(arr[i]);
        out = realloc(out, outSize * sizeof(char));

        strcat(out, str);
        strcat(out, arr[i]);
        i++;
    }
    strcat(out, "\0");

    return out;
}

char* substr(char* str, int start, int length) {
    char* out = malloc((length+1) * sizeof(char));
    for (int i = 0; i < length; i++) {
        out[i] = str[i+start];
    }
    out[length] = '\0';
    return out;
}

// Return array of positions where first element is number found with no overlap
int* strFind(char* str, char* substr) {
    int* out = malloc(sizeof(int));
    int outSize = 1;
    
    int n = -strlen(substr);
    char* ptr = strstr(str, substr);
    while (ptr != NULL) {
        int pos = (ptr - str);
        n += pos + strlen(substr);

        outSize++;
        out = realloc(out, outSize * sizeof(int));
        out[outSize-1] = n;

        str = (str + strlen(substr) + pos);
        ptr = strstr(str, substr);
    }

    out[0] = outSize-1;
    return out;
}

int* strOverlapFind(char* str, char* substr) {
    int* out = malloc(sizeof(int));
    int outSize = 1;
    
    int n = -1;
    char* ptr = strstr(str, substr);
    while (ptr != NULL) {
        int pos = (ptr - str);
        n += pos + 1;

        outSize++;
        out = realloc(out, outSize * sizeof(int));
        out[outSize-1] = n;

        str = (str + pos + 1);
        ptr = strstr(str, substr);
    }

    out[0] = outSize-1;
    return out;
}

char* strReplace(char* str, char* old, char* replacement, int count) {
    int* pos = strFind(str, old);
    // All occurences
    if (count == -1) {
        count = pos[0];
    }
    else if (pos[0] < count) {
        count = pos[0];
    }
    
    char* out = malloc((strlen(str) + count*(strlen(replacement) - strlen(old)) + 1) * sizeof(char));

    int idx = 0;
    int outIdx = 0;
    for (int i=0; i<count; i++) {
        for (int j=idx; j<pos[i+1]; j++) {
            out[outIdx] = str[j];
            outIdx++;
        }
        for (int j=0; j<strlen(replacement); j++) {
            out[outIdx] = replacement[j];
            outIdx++;
        }
        idx = pos[i+1] + strlen(old);
    }
    for (int j=idx; j<strlen(str); j++) {
        out[outIdx] = str[j];
        outIdx++;
    }
	out[outIdx] = '\0';
    
    return out;
}

// Converts number from a string to an integer
int strtoint(char* str) {
	// int out = 0;

	// for (int i=0; i<strlen(str); i++) {
	// 	if (str[i] >= '0' && str[i] <= '9')
	// 		// Move original by a tenth unit and add new digit
	// 		out = out*10 + (str[i]-'0');
	// 	else
	// 		// If character is not a number
	// 		break;
	// }

	// return out;

	return (int)strtol(str, NULL, 0);
}

// Find length of null terminated string array
int strArrLen(char **strArr) {
	int length = 0;
	while (strArr[length][0] != -1) {
		length++;
	}
	return length;
}

