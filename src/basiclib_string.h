#ifndef BASICLIB_STRING_H
#define BASICLIB_STRING_H

extern char* uniqueASCIICharInStr(char* str);
extern int charInStr(char c, char* str);
extern void termStrArr(char*** str, int size);
extern char** strtokSplit(char* str, char* delims);
extern char** strSplit(char* str, char* substr);
extern char* strJoin(char** arr, char* str);
extern char* substr(char* str, int start, int length);
extern int* strFind(char* str, char* substr);
extern int* strOverlapFind(char* str, char* substr);
extern char* strReplace(char* str, char* old, char* replacement, int count);
extern int strtoint(char* str);
extern int strArrLen(char** strArr);
extern char* upString(char* str);
extern int intLen(int n);

#endif
