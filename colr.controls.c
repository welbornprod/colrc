/*! \file colr.c
    Implements everything in the colr.controls.h header.

    \internal
    \author Christopher Welborn
    \date 02-29-20
    \endinternal
*/

#include "colr.controls.h"

/*! Creates an allocated ColorResult that hides the cursor when printed.

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
    return ColorResult_to_ptr(ColorResult_from_str(COLR_ESC "?25l"));
}

/*! Creates an allocated ColorResult that shows the cursor when printed.

    \return \parblock
                An allocated ColorResult.
                \maybenullalloc
                \colrmightfree
            \endparblock
*/
ColorResult* Colr_cursor_show(void) {
    return ColorResult_to_ptr(ColorResult_from_str(COLR_ESC "?25h"));
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
    char* methstr = EraseMethod_to_str(method);
    if (!methstr) return NULL;
    size_t finallen = COLR_ESC_LEN + 2;
    char codes[finallen];
    snprintf(codes, finallen, COLR_ESC "%sJ", methstr);
    if (method < 4) return ColorResult_to_ptr(ColorResult_from_str(codes));
    return Colr_join(";", Colr_erase_display(ALL_MOVE), Colr_erase_display(ALL_ERASE));
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
            Colr_erase_line(ALL_MOVE),
            Colr_move_return(),
            Colr("This is a blue line.", fore(BLUE))
        );
    }
    \endexamplecode

*/
ColorResult* Colr_erase_line(EraseMethod method) {
    assert(method < 3);
    char* methstr = EraseMethod_to_str(method);
    if (!methstr) return NULL;
    size_t finallen = COLR_ESC_LEN + 2;
    char codes[finallen];
    snprintf(codes, finallen, COLR_ESC "%sK", methstr);
    return ColorResult_to_ptr(ColorResult_from_str(codes));

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
    return ColorResult_to_ptr(ColorResult_from_str("\r"));
}

/*! Returns an allocated ColorResult that will move the cursor to a specific
    column when printed.

    \details
    Columns start at `1`.

    \pi columns \parblock
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

/*! Creates an allocated ColorResult that restores a previously saved cursor
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
    return ColorResult_to_ptr(ColorResult_from_str(COLR_ESC "u"));
}

/*! Creates an allocated ColorResult that saves the cursor position when printed.

    \details
    This only saves the column position, not the line position.

    \return \parblock
                An allocated ColorResult.
                \maybenullalloc
                \colrmightfree
            \endparblock
*/
ColorResult* Colr_pos_save(void) {
    return ColorResult_to_ptr(ColorResult_from_str(COLR_ESC "s"));
}

/* ------------------------- Colr Control Objects ------------------------- */

/*! A ColrCursor object with methods to return allocated ColorResults.

    \examplecodefor{ColrCursor,.c}
    #include "colr.controls.h"

    int main(void) {
        puts("Hiding the cursor.");
        colr_control(ColrCursor.hide());
        puts("About to show the cursor.");
        colr_control(ColrCursor.show());
    }
    \endexamplecode
*/
struct ColrCursor ColrCursor = {
    Colr_cursor_hide,
    Colr_cursor_show,
};

/*! A ColrErase object with methods to return allocated ColorResults.

    \examplecodefor{ColrErase,.c}
    #include "colr.controls.h"

    int main(void) {
        colr_control(
            "About to erase the display.",
            ColrErase.display(ALL_MOVE),
            "About to replace this line.",
            ColrErase.line(ALL_MOVE),
            ColrMove.ret(),
            Colr("Line was erased.", fore(RED))
        );
    }
    \endexamplecode
*/
struct ColrErase ColrErase = {
    Colr_erase_display,
    Colr_erase_line,
};

/*! A ColrMove object with methods to return allocated ColorResults.

    \examplecodefor{ColrMove,.c}
    #include "colr.controls.h"

    int main(void) {
        colr_control(
            "Colr was",
            ColrMove.backward(3),
            "is awesome.",
            ColrMove.ret(),
            ColrMove.forward(4),
            Colr("C", fore(BLUE), style(BRIGHT)), " is even better!\n"
        );
    }
    \endexamplecode
*/
struct ColrMove ColrMove = {
    Colr_move_back,
    Colr_move_return,
    Colr_move_column,
    Colr_move_down,
    Colr_move_forward,
    Colr_move_next,
    Colr_move_pos,
    Colr_move_prev,
    Colr_move_up,
};

/*! A ColrPosition object with methods to return allocated ColorResults.

    \examplecodefor{ColrPosition,.c}
    #include "colr.controls.h"

    int main(void) {
        printf("Writing some text...");
        colr_control(ColrPosition.save());
        printf("Writing some more.");
        colr_control(
            ColrPosition.restore(),
            "<--",
            Colr("..And we're back.", fore(RED))
        );
    }
    \endexamplecode
*/
struct ColrPosition ColrPosition = {
    Colr_pos_restore,
    Colr_pos_save,
};
