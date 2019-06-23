/* dbug.h
    Debug print macro, for when building with DEBUG.
    These macros should cost nothing when building without DEBUG.
    -Christopher Welborn 11-12-2017
*/

#ifndef DBUG_H
#define DBUG_H
/*! \file
    Documentation for dbug.h.
*/

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
#define DBUG_VERSION "0.0.2"
//! Long version string for this library.
#define DBUG_VERSION_STR DBUG_NAME " v. " DBUG_VERSION

/* Warn for any other unused macros, for gcc and clang. */
#pragma clang diagnostic pop
#pragma GCC diagnostic warning "-Wunused-macros"
#pragma clang diagnostic push
#pragma clang diagnostic warning "-Wunused-macros"

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
    #define DBUG_DEBUGWIDTH 41
    //! Number of characters in the maximum line-count/line-number.
    #define DBUG_LINELEN 4
    //! Number of "other characters", such as ':', '()', and ' ' in the line info.
    #define DBUG_EXTRACHARS 6
    //! Computed max width for file name in line info.
    #define DBUG_FILELEN ((DBUG_DEBUGWIDTH - DBUG_LINELEN - DBUG_EXTRACHARS) / 2)
    //! Computed max width for function name in line info.
    #define DBUG_FUNCLEN ((DBUG_DEBUGWIDTH - DBUG_LINELEN - DBUG_EXTRACHARS) / 2)
    //! Macro for printing line information only.
    #define debug_lineinfo() \
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
    /*! \brief Macro for printing a regular message.

        Writes a debug message with no line information, and no newline.

        \param fmt Format string, like \c printf format strings.
        \param ... All other arguments to satisfy the format string.
    */
    #define debug_msg(...) \
        fprintf(stderr, "%s", DBUG_COLOR_GREEN); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "%s", DBUG_COLOR_NONE);

    /*! \brief Macro for printing an error message.

        File and line information will be included.

        \param fmt Format string, like \c printf format strings.
        \param ... All other arguments to satisfy the format string.
    */
    #define debug_msg_err(...) \
        fprintf(stderr, "%s", DBUG_COLOR_RED); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "%s", DBUG_COLOR_NONE);

    /*! \brief Macro for printing debug information.

        File and line information will be included.

        \param fmt Format string, like \c printf format strings.
        \param ... All other arguments to satisfy the format string.
    */
    #define debug(...) \
        debug_lineinfo(); \
        debug_msg(__VA_ARGS__);
    /*! \brief Macro for printing debug error information.

        File and line information will be included.

        \param fmt Format string, like \c printf format strings.
        \param ... All other arguments to satisfy the format string.
    */
    #define debugerr(...) \
        debug_lineinfo(); \
        debug_msg_err(__VA_ARGS__);
#else
    // Ignore unused macros (for linting).
    #pragma GCC diagnostic ignored "-Wunknown-pragmas"
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-macros"

    // In non-debug builds, any calls to the debug* macros are no-ops.
    #define debug_lineinfo(...) ((void)0)
    #define debug_msg(...) ((void)0)
    #define debug_msg_err(...) ((void)0)
    #define debug(...) ((void)0)
    #define debugerr(...) ((void)0)

    #pragma clang diagnostic pop /* end ignore -Wunused-macros */
#endif

#pragma clang diagnostic pop /* end warning -Wunused-macros */
#endif /* DBUG_H */
