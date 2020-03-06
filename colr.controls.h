/*! \file colr.controls.h
    Declarations for ColrC cursor controls.

    \internal
    \author Christopher Welborn
    \date 02-29-20
    \endinternal

    \details
    To use ColrC in your project, you will need to include colr.controls.h
    and compile both colr.c and colr.controls.c with the rest of your files.

    <em>Don't forget to compile with `colr.c` and `-lm`</em>.
    \code{.sh}
    gcc -std=c11 -c your_program.c colr.c colr.controls.c -lm
    \endcode
*/

#ifndef COLR_CONTROLS_H
#define COLR_CONTROLS_H

#include "colr.h"

//! Escape sequence to use when creating escape codes.
#define COLR_ESC "\x1b["
//! Length of COLR_ESC, with `NULL`-terminator.
#define COLR_ESC_LEN 3

/*! Like colr_fprint(stdout, ...) except it flushes stdout after it prints.

    \pi ... Arguments for colr_fprintf.
*/
#define colr_control(...) \
    do { \
        colr_fprint(stdout, __VA_ARGS__); \
        fflush(stdout); \
    } while (0)

/*! \def colr_print_inplace
    Save the cursor position, print exactly like colr_control(), and then restore
    the cursor position.

    \pi ... Arguments for colr_control() to print.
*/
#define colr_print_inplace(...) \
    do { \
        colr_control(Colr_pos_save(), __VA_ARGS__, Colr_pos_restore()); \
    } while (0)

/*! \def colr_print_overwrite
    Erase the current line, move to column 1 and print exactly like colr_control().

    \pi ... Arguments for colr_control() to print.
*/
#define colr_print_overwrite(...) \
    do { \
        colr_control(Colr_erase_line(ALL), Colr_move_column(1), __VA_ARGS__); \
    } while (0)

/*! Returns a static string representation for an EraseMethod.

    \pi method The EraseMethod value to get a string representation for.
    \return    A static string with the result, or `NULL` if the method was unknown.
*/
#define EraseMethod_to_str(method) ( \
        method == END ? "0" : \
        method == START ? "1" : \
        method == ALL_MOVE ? "2" : \
        method == ALL_ERASE ? "3" : \
        method == ALL_MOVE_ERASE ? "4" : \
        method == ALL ? "2" : \
        NULL \
    )

//! Methods to erase text.
typedef enum EraseMethod {
    //! Clear cursor to the end of the line/screen (depending on erase function used).
    END,
    //! Clear cursor to the start of the line/screen (depending on erase function used).
    START,
    //! Clear all, and move home for display, or clear entire the line when doing a line erase.
    ALL_MOVE,
    //! Clear all, and erase scrollback buffer.
    ALL_ERASE,
    /*! Clear all, move home, and erase scrollback buffer.
        This is a feature of ColrC. It is not standard.
    */
    ALL_MOVE_ERASE,
    //! This is an alias for ALL_MOVE, when using the erase_line functions.
    ALL
} EraseMethod;

ColorResult* Colr_cursor_hide(void);
ColorResult* Colr_cursor_show(void);
ColorResult* Colr_erase_display(EraseMethod method);
ColorResult* Colr_erase_line(EraseMethod method);
ColorResult* Colr_move_back(unsigned int columns);
ColorResult* Colr_move_return(void);
ColorResult* Colr_move_column(unsigned int column);
ColorResult* Colr_move_down(unsigned int lines);
ColorResult* Colr_move_forward(unsigned int columns);
ColorResult* Colr_move_next(unsigned int lines);
ColorResult* Colr_move_pos(unsigned int line, unsigned int column);
ColorResult* Colr_move_prev(unsigned int lines);
ColorResult* Colr_move_up(unsigned int lines);
ColorResult* Colr_pos_restore(void);
ColorResult* Colr_pos_save(void);
ColorResult* Colr_scroll_down(unsigned int lines);
ColorResult* Colr_scroll_up(unsigned int lines);

#endif // COLR_CONTROLS_H
