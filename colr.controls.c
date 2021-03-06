/*! \file colr.controls.c
    Implements everything in the colr.controls.h header.

    \internal
    \author Christopher Welborn
    \date 02-29-20
    \endinternal

    \details
    To use ColrC Controls in your project, you will need to include colr.controls.h
    and compile both colr.c and colr.controls.c with the rest of your files.

    <em>Don't forget to compile with `colr.c` and `-lm`</em>.
    \code{.sh}
    gcc -std=c11 -c your_program.c colr.c colr.controls.c -lm
    \endcode

*/

#include "colr.controls.h"

/*! Returns an allocated ColorResult that hides the cursor when printed.

    \return \parblock
                An allocated ColorResult.
                \maybenullalloc
                \colrmightfree
            \endparblock

    \examplecodefor{Colr_cursor_hide,.c}
    #include "colr.controls.h"

    int main(void) {
        printf("Hiding the cursor. ");
        colr_control(Colr_cursor_hide());
        puts("About to show the cursor.");
        colr_control(Colr_cursor_show());
    }
    \endexamplecode
*/
ColorResult* Colr_cursor_hide(void) {
    return ColorResult_from_stra(COLR_ESC "?25l");
}

/*! Returns an allocated ColorResult that shows the cursor when printed.

    \return \parblock
                An allocated ColorResult.
                \maybenullalloc
                \colrmightfree
            \endparblock
*/
ColorResult* Colr_cursor_show(void) {
    return ColorResult_from_stra(COLR_ESC "?25h");
}

/*! Returns an allocated ColorResult that will erase the display or part of the display
    when printed.

    \pi method  The erase method.
    \return     \parblock
                    An allocated ColorResult, or `NULL` if the EraseMethod was
                    invalid.
                    \maybenullalloc
                    \colrmightfree
                \endparblock

    \examplecodefor{Colr_erase_display,.c}
    #include "colr.controls.h"

    int main(void) {
        // Force flush with colr_control, so we see the first line.
        colr_control("This is a line.");
        colr_control(Colr_erase_display(ALL_MOVE));
        colr_puts(Colr("This is a blue line.", fore(BLUE)));
    }
    \endexamplecode
*/
ColorResult* Colr_erase_display(EraseMethod method) {
    if (method == ALL) method = ALL_MOVE;
    if (method == ALL_MOVE_ERASE) {
        // ALL_MOVE + ALL_ERASE
        // finallen *= 2;
        // char dblcodes[finallen];
        char* methstr_move = EraseMethod_to_str(ALL_MOVE);
        char* methstr_erase = EraseMethod_to_str(ALL_ERASE);
        return Colr_cat(COLR_ESC, methstr_move, "J;" COLR_ESC, methstr_erase, "J");
    }
    char* methstr = EraseMethod_to_str(method);
    if (!methstr) return NULL;
    size_t finallen = COLR_ESC_LEN + 2;
    char codes[finallen];
    snprintf(codes, finallen, COLR_ESC "%sJ", methstr);
    return ColorResult_from_stra(codes);
}

/*! Returns an allocated ColorResult that will erase line or part of a line when printed.

    \pi method  The erase method.
    \return     \parblock
                    An allocated ColorResult, or `NULL` if the EraseMethod was
                    invalid.
                    \maybenullalloc
                    \colrmightfree
                \endparblock

    \examplecodefor{Colr_erase_line,.c}
    #include "colr.controls.h"

    int main(void) {
        // Force flush with colr_control, so we see the first line.
        colr_control(
            "This is a line.",
            Colr_erase_line(ALL),
            Colr_move_return(),
            Colr("This is a blue line.", fore(BLUE))
        );
    }
    \endexamplecode

*/
ColorResult* Colr_erase_line(EraseMethod method) {
    if (method == ALL) method = ALL_MOVE;
    assert((method == END) || (method == START) || (method == ALL_MOVE));
    char* methstr = EraseMethod_to_str(method);
    if (!methstr) return NULL;
    size_t finallen = COLR_ESC_LEN + 2;
    char codes[finallen];
    snprintf(codes, finallen, COLR_ESC "%sK", methstr);
    return ColorResult_from_stra(codes);

}

/*! Returns an allocated ColorResult that will move the cursor back a number
    of columns when printed.

    \pi columns \parblock
                    The number of columns to move.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_back(unsigned int columns) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dD", columns ? columns : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that will move the cursor back to the
    beginning of the line with a carriage return character when printed.

    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_return(void) {
    return ColorResult_from_stra("\r");
}

/*! Returns an allocated ColorResult that will move the cursor to a specific
    column when printed.

    \details
    Columns start at `1`.

    \pi column  \parblock
                    The column to move to.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_column(unsigned int column) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dG", column ? column : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that will move the cursor down a number
    of lines when printed.

    \pi lines   \parblock
                    The number of lines to move.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_down(unsigned int lines) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dB", lines ? lines : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that will move the cursor forward a number
    of columns when printed.

    \pi columns \parblock
                    The number of columns to move.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_forward(unsigned int columns) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dC", columns ? columns : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that will move the cursor down a number
    of lines, at the start of the line, when printed.

    \pi lines   \parblock
                    The number of lines to move.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_next(unsigned int lines) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dE", lines ? lines : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}


/*! Returns an allocated ColorResult that will position the cursor on the desired
    line and column when printed.

    \details
    Positions start at `1`.

    \pi line    \parblock
                    The line to move to.
                    Using `0` is the same as using `1`.
                \endparblock
    \pi column  \parblock
                    The column to move to.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_pos(unsigned int line, unsigned int column) {
    char* codes = NULL;
    asprintf_or_return(
        NULL,
        &codes,
        COLR_ESC "%d;%dH",
        line ? line : 1,
        column ? column : 1
    );
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that will move the cursor up a number of
    lines, at the start of the line, when printed.

    \pi lines   \parblock
                    The number of lines to move.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_prev(unsigned int lines) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dF", lines ? lines : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that will move the cursor up a number of
    lines when printed.

    \details
    Positions start at `1`.

    \pi lines   \parblock
                    The number of lines to move.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_move_up(unsigned int lines) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dA", lines ? lines : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that restores a previously saved cursor
    position when printed.

    \details
    This only restores the column position, not the line position.

    \return \parblock
                An allocated ColorResult.
                \maybenullalloc
                \colrmightfree
            \endparblock

    \examplecodefor{Colr_pos_restore,.c}
    #include "colr.controls.h"

    int main(void) {
        printf("Saving the cursor.");
        colr_control(Colr_pos_save());
        printf("About to restore the cursor.");
        colr_control(Colr_pos_restore());
        puts(" <-- I'm back to where I started.");
    }
    \endexamplecode
*/
ColorResult* Colr_pos_restore(void) {
    return ColorResult_from_stra(COLR_ESC "u");
}

/*! Returns an allocated ColorResult that saves the cursor position when printed.

    \details
    This only saves the column position, not the line position.

    \return \parblock
                An allocated ColorResult.
                \maybenullalloc
                \colrmightfree
            \endparblock
*/
ColorResult* Colr_pos_save(void) {
    return ColorResult_from_stra(COLR_ESC "s");
}

/*! Returns an allocated ColorResult that will scroll the cursor down a number
    of lines when printed. New lines are added to the top.

    \pi lines   \parblock
                    The number of lines to scroll.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock

    \examplecodefor{Colr_scroll_down,.c}
    #include "colr.controls.h"

    int main(void) {
        colr_control("Start here.\n");
        for (int i = 0; i < 5; i++) {
            printf("Line: 1-%d\n", i);
        }
        colr_control(Colr_scroll_down(5), "Back again.\n");
        for (int i = 0; i < 5; i++) {
            printf("Line: 2-%d\n", i);
        }
    }
    \endexamplecode
*/
ColorResult* Colr_scroll_down(unsigned int lines) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dT", lines ? lines : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}

/*! Returns an allocated ColorResult that will scroll the cursor up a number
    of lines when printed. New lines are added to the bottom.

    \pi lines   \parblock
                    The number of lines to scroll.
                    Using `0` is the same as using `1`.
                \endparblock
    \return     \parblock
                    An allocated ColorResult.
                    \maybenullalloc
                    \colrmightfree
                \endparblock
*/
ColorResult* Colr_scroll_up(unsigned int lines) {
    char* codes = NULL;
    asprintf_or_return(NULL, &codes, COLR_ESC "%dS", lines ? lines : 1);
    return ColorResult_to_ptr(ColorResult_new(codes));
}
