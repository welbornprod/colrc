/*! \file dbug.h
    Debug macros, disabled and cost-free in release builds.

    \details
    These macros should cost nothing when building without DEBUG.

    \author Christopher Welborn
    \date 11-12-2017
*/
#ifndef DBUG_H
#define DBUG_H

/* Tell gcc to ignore this unused inclusion macro. */
#pragma GCC diagnostic ignored "-Wunused-macros"
/* Tell gcc to ignore clang pragmas, for linting. */
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
/* Tell clang to ignore this unused inclusion macro. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnounused-macros"
#pragma clang diagnostic ignored "-Wunused-macros"
//! Name of this library.
#define DBUG_NAME "dbug"
//! Version for this library.
#define DBUG_VERSION "0.0.3"
//! Long version string for this library.
#define DBUG_VERSION_STR DBUG_NAME " v. " DBUG_VERSION


#include <stdarg.h>
#include <stdio.h>

#ifdef DEBUG
    #ifndef DOXYGEN_SKIP
    #define DBUG_COLOR_NONE "\033[0m"
    #define DBUG_COLOR_BLUE "\033[34m"
    #define DBUG_COLOR_CYAN "\033[36m"
    #define DBUG_COLOR_GREEN "\033[32m"
    #define DBUG_COLOR_RED "\033[31m"
    #define DBUG_COLOR_YELLOW "\033[33m"
    #define DBUG_STYLE_BRIGHT "\033[1m"
    #endif
    //! Number of characters in the file, line, and lineno info.
    #define DBUG_DEBUGWIDTH 45
    //! Number of characters in the maximum line-count/line-number.
    #define DBUG_LINELEN 4
    //! Number of "other characters", such as ':', '()', and ' ' in the line info.
    #define DBUG_EXTRACHARS 6
    //! Computed max width for file name in line info.
    #define DBUG_FILELEN ((DBUG_DEBUGWIDTH - DBUG_LINELEN - DBUG_EXTRACHARS) / 2)
    //! Computed max width for function name in line info.
    #define DBUG_FUNCLEN ((DBUG_DEBUGWIDTH - DBUG_LINELEN - DBUG_EXTRACHARS) / 2)
    //! Macro for printing line information only, with no newline.
    #define dbug_lineinfo() \
        fprintf(stderr, "%s%*s%s:%s%s%-*d%s %s%*s()%s: " \
            ,DBUG_COLOR_YELLOW \
            ,DBUG_FILELEN, __FILE__ \
            ,DBUG_COLOR_NONE \
            ,DBUG_COLOR_BLUE \
            ,DBUG_STYLE_BRIGHT \
            ,DBUG_LINELEN, __LINE__ \
            ,DBUG_COLOR_NONE \
            ,DBUG_COLOR_BLUE \
            ,DBUG_FUNCLEN, __func__ \
            ,DBUG_COLOR_NONE \
        );
    /*! Macro for printing a regular message.

        \details
        Writes a debug message with no line information, and no newline.

        \pi ... Format string and all other arguments to satisfy the format string.
    */
    #define dbug_msg(...) \
        fprintf(stderr, "%s", DBUG_COLOR_GREEN); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "%s", DBUG_COLOR_NONE);

    /*! Macro for printing an error message.

        \details
        File and line information will be included.

        \pi ... Format string and all other arguments to satisfy the format string.
    */
    #define dbug_msg_err(...) \
        fprintf(stderr, "%s", DBUG_COLOR_RED); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "%s", DBUG_COLOR_NONE);

    /*! Macro for printing debug information.

        \details
        File and line information will be included.

        \pi ... Format string and all other arguments to satisfy the format string.
    */
    #define dbug(...) \
        dbug_lineinfo(); \
        dbug_msg(__VA_ARGS__);
    /*! Macro for printing debug error information.

        \details
        File and line information will be included.

        \pi ... Format string and all other arguments to satisfy the format string.
    */
    #define debugerr(...) \
        dbug_lineinfo(); \
        dbug_msg_err(__VA_ARGS__);

    /* Warn for any other unused macros, for gcc and clang. */
    #pragma clang diagnostic pop
    #pragma GCC diagnostic warning "-Wunused-macros"
    #pragma clang diagnostic push
    #pragma clang diagnostic warning "-Wunused-macros"
#else
    // Ignore unused macros (for linting).
    #pragma GCC diagnostic ignored "-Wunknown-pragmas"
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-macros"

    // In non-debug builds, any calls to the debug* macros are no-ops.
    #define dbug_lineinfo(...) ((void)0)
    #define dbug_msg(...) ((void)0)
    #define dbug_msg_err(...) ((void)0)
    #define dbug(...) ((void)0)
    #define dbugerr(...) ((void)0)

    #pragma clang diagnostic pop /* end ignore -Wunused-macros */
#endif

#pragma clang diagnostic pop /* end warning -Wunused-macros */
#endif /* DBUG_H */
