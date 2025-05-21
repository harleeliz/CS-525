/*******************************************************************
 *  File Name:     dt.h
 * CS 525 Advanced Database Organization (Spring 2025)
 * Harlee Ramos, Jisun Yun, Baozhu Xie
 *  Description:   Boolean type definitions and compatibility macros
 ********************************************************************/

#ifndef DT_H
#define DT_H

// If this file is used in C++, ensure the functions and definitions use C linkage
#ifdef __cplusplus
extern "C" {
#endif

    /*-------------------------------------------------------------
     * Boolean Type Definitions
     *-------------------------------------------------------------*/

    /*
     * In C99 and later, <stdbool.h> defines `bool`, `true`, and `false`.
     * For compatibility with older compilers or non-C99 environments,
     * we define them manually if needed.
     */
#if !defined(__cplusplus) && !defined(_Bool)
    typedef short bool;
#define true  1
#define false 0
#endif

    /*-------------------------------------------------------------
     * Boolean Macro Definitions
     *-------------------------------------------------------------*/

    // Define TRUE and FALSE macros for legacy support and convenience
#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifdef __cplusplus
}
#endif

#endif // DT_H