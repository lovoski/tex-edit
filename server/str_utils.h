#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <stdio.h>

/**
 * keep in mind that the return value of a function
 * is very important, as the value of the variable counld
 * change during the called function
 */

/* convert int32_t to char array end with '\0' and returns str */
char *itostr(int a, char *str);
/* concatenate src to back of dest and returns dest */
char *strconcat(char *dest, char *src);
/* convert src to a string inside heap */
char *init_str_from_stack(unsigned int size, char *src);
/* change the string in heap to the src string */
char *reset_string(char *target, char *src);
/* compare two string until '\0' is met */
int compare_string(char *str1, char *str2);

#endif