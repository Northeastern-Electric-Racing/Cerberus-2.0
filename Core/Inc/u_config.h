#ifndef __U_CONFIG_H
#define __U_CONFIG_H

#include <string.h>
#include <stdio.h>

/* General-purpose macros that can be used throughout the project. */
#define U_SUCCESS   0
#define U_ERROR     1

/* Debugging Macros */
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) /* Gets the name of the file. */
#define U_DEBUG /* Comment out to enable/disable debugging. */
#ifdef U_DEBUG
    #define DEBUG_PRINT(message, ...) printf("[%s/%s()] " message "\n", __FILENAME__, __func__, ##__VA_ARGS__) /* Prints an error message in the format: "[file_name.c/function()] {message}"*/
#else
    #define DEBUG_PRINT(message, ...) /* If debugging is turned off, macro doesn't need to expand to anything. */
#endif

#endif /* u_config.h */