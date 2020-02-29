/*! \file colr.controls.h
    Declarations for ColrC cursor controls.

    \internal
    \author Christopher Welborn
    \date 02-29-20

    \details
    To use ColrC in your project, you will need to include colr.h
    and compile colr.c with the rest of your files.

    <em>Don't forget to compile with `colr.c` and `-lm`</em>.
    \code{.sh}
    gcc -std=c11 -c your_program.c colr.c -lm
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
        NULL \
    )

//! Methods to erase text.
typedef enum EraseMethod {
    //! Clear cursor to the end of the line/screen (depending on erase function used).
    END,
    //! Clear cursor to the start of the line/screen (depending on erase function used).
    START,
    //! Clear all, and move home, or clear the line when doing a line erase.
    ALL_MOVE,
    //! Clear all, and erase scrollback buffer.
    ALL_ERASE,
    /*! Clear all, move home, and erase scrollback buffer.
        This is a feature of ColrC. It is not standard.
    */
    ALL_MOVE_ERASE
} EraseMethod;

ColorResult* Colr_cursor_hide(void);
ColorResult* Colr_cursor_show(void);
ColorResult* Colr_erase_display(EraseMethod method);
ColorResult* Colr_erase_line(EraseMethod method);


/* ---------------------------- Function Types ---------------------------- */

/*! A function type that knows how to return an allocated ColorResult without arguments.
*/
typedef ColorResult* (*ColorResult_void_func)(void);

/*! A function type that knows how to return an allocated ColorResult with an EraseMethod.
*/
typedef ColorResult* (*ColorResult_erase_func)(EraseMethod);

/* ------------------------ Colr Control Objects. ------------------------ */

/*! An object that has a hide() and show() method, both returning allocated `ColorResult`s.

    \details
    This is used to implement ColrCursor. You should use it instead.
*/
struct ColrCursor {
    ColorResult_void_func hide;
    ColorResult_void_func show;
};

/*! An object that has a display() and line() method, both accepting an
    EraseMethod and returning allocated `ColorResult`s.

    \details
    This is used to implement ColrErase. You should use it instead.
*/
struct ColrErase {
    ColorResult_erase_func display;
    ColorResult_erase_func line;
};

//! An initialized ColrCursor struct. This is set in colr.controls.c.
extern struct ColrCursor ColrCursor;
//! An initialized ColrErase struct. This is set in colr.controls.c.
extern struct ColrErase ColrErase;

#endif // COLR_CONTROLS_H
