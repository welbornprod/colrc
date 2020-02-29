/*! \file colr.c
    Implements everything in the colr.controls.h header.

    \internal
    \author Christopher Welborn
    \date 02-29-20
    \endinternal
*/

#include "colr.controls.h"

/*! Creates an allocated ColorResult that hides the cursor when printed.

    \return An allocated ColorResult.

    \examplecodefor{colr_cursor_hide,.c}
    #include "colr.controls.h"

    int main(void) {
        puts("Hiding the cursor.");
        colr_control(colr_cursor_hide());
        puts("About to show the cursor.");
        sleep(2);
        colr_control(colr_cursor_show());
    }
    \endexamplecode
*/
ColorResult* Colr_cursor_hide(void) {
    return ColorResult_to_ptr(ColorResult_from_str(COLR_ESC "?25l"));
}

/*! Creates an allocated ColorResult that shows the cursor when printed.

    \return An allocated ColorResult.

    \examplecodefor{Colr_cursor_show,.c}
    #include "colr.controls.h"

    int main(void) {
        puts("Hiding the cursor.");
        colr_control(Colr_cursor_hide());
        puts("About to show the cursor.");
        sleep(2);
        colr_control(Colr_cursor_show());
    }
    \endexamplecode
*/
ColorResult* Colr_cursor_show(void) {
    return ColorResult_to_ptr(ColorResult_from_str(COLR_ESC "?25h"));
}

/*! Returns a ColorResult that will erase the display or part of the display
    when printed.

    \pi method  The erase method.
    \return     \parblock
                    An allocated ColorResult, or `NULL` if the EraseMethod was
                    invalid.
                \endparblock

    \examplecodefor{Colr_erase_display,.c}
    #include "colr.controls.h"

    int main(void) {
        // Force flush with colr_control, so we see the first line.
        colr_control("This is a line.");
        sleep(1);
        colr_control(Colr_erase_display(START));
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
    // Need to join methods ALL_MOVE(2) and ALL_ERASE(3).
    finallen *= 2;
    return Colr_join(";", Colr_erase_display(ALL_MOVE), Colr_erase_display(ALL_ERASE));
}

/*! Returns a ColorResult that will erase line or part of a line when printed.

    \pi method  The erase method.
    \return     \parblock
                    An allocated ColorResult, or `NULL` if the EraseMethod was
                    invalid.
                \endparblock

    \examplecodefor{Colr_erase_line,.c}
    #include "colr.controls.h"

    int main(void) {
        // Force flush with colr_control, so we see the first line.
        colr_control("This is a line.");
        sleep(1);
        colr_control(Colr_erase_line(START));
        colr_puts(Colr("This is a blue line.", fore(BLUE)));
    }
    \endexamplecode

*/
ColorResult* Colr_erase_line(EraseMethod method) {
    char* methstr = EraseMethod_to_str(method);
    if (!methstr) return NULL;
    size_t finallen = COLR_ESC_LEN + 2;
    char codes[finallen];
    snprintf(codes, finallen, COLR_ESC "%sK", methstr);
    if (method < 4) return ColorResult_to_ptr(ColorResult_from_str(codes));
    // Need to join methods ALL_MOVE(2) and ALL_ERASE(3).
    finallen *= 2;
    return Colr_join(";", Colr_erase_line(ALL_MOVE), Colr_erase_line(ALL_ERASE));

}

/*! A ColrCursor object with methods to return allocated ColorResults.

    \examplecodefor{ColrCursor,.c}
    #include "colr.controls.h"

    int main(void) {
        puts("Hiding the cursor.");
        colr_control(ColrCursor.hide());
        puts("About to show the cursor.");
        sleep(2);
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
        puts("About to erase the display.");
        colr_control(ColrErase.display());
        colr_control("About to replace this line.");
        sleep(2);
        colr_control(ColrErase.line());
        sleep(1);
        colr_puts(Colr("Line was erased.", fore(RED)));
    }
    \endexamplecode
*/
struct ColrErase ColrErase = {
    Colr_erase_display,
    Colr_erase_line,
};
