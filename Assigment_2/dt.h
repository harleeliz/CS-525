/************************************************************
*     File name:                dt.h                        *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#ifndef DT_H  // Include guard to prevent multiple definitions if this file is included more than once
#define DT_H

// Define bool if not already defined. This handles cases where the C99 <stdbool.h> header might not be available.
#ifndef bool
typedef short bool; // Define bool as a short integer type
#define true 1      // Define true as 1
#define false 0     // Define false as 0
#endif

// Define TRUE and FALSE for convenience. These are equivalent to true and false, respectively.
#define TRUE true   // Define TRUE as the boolean value true
#define FALSE false // Define FALSE as the boolean value false

#endif // DT_H  // End of include guard
