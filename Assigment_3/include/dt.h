/************************************************************
* File name:      dt.h
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the boolean data type for the project.
 *   If the standard bool type is not available, it defines 'bool'
 *   as a short integer and provides boolean constants for true and false.
 ************************************************************/

#ifndef DT_H
#define DT_H

// If 'bool' is not defined, define it along with boolean constants
#ifndef bool
    typedef short bool;   // Define bool as a short integer
#define true 1        // Define true as 1
#define false 0       // Define false as 0
#endif

// Additional boolean macros for clarity
#define TRUE true
#define FALSE false

#endif // DT_H