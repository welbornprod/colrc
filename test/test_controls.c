/*! Tests for colr.controls.c

    \author Christopher Welborn
    \date   03-10-2020
*/
#include "test_ColrC.h"

describe(controls) {
// Colr_cursor_hide
subdesc(Colr_cursor_hide) {
    it("basically works") {
        // Not much to test here.
        ColorResult* cres = Colr_cursor_hide();
        assert_not_null(cres);
        char* s = ColorResult_to_str(*cres);
        assert_not_null(s);
        assert_str_not_empty(s);
        colr_free(cres);
    }
}
// Colr_cursor_show
subdesc(Colr_cursor_show) {
    it("basically works") {
        // Not much to test here.
        ColorResult* cres = Colr_cursor_show();
        assert_not_null(cres);
        char* s = ColorResult_to_str(*cres);
        assert_not_null(s);
        assert_str_not_empty(s);
        colr_free(cres);
    }
}
// Colr_erase_display
subdesc(Colr_erase_display) {
    it("handles all EraseMethods") {
        EraseMethod tests[] = {
            END,
            START,
            ALL_MOVE,
            ALL_ERASE,
            ALL_MOVE_ERASE,
            ALL,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_erase_display(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            colr_free(cres);
        }
    }
    it("handles ALL -> ALL_MOVE") {
        ColorResult* cres_all_move = Colr_erase_display(ALL_MOVE);
        ColorResult* cres_all = Colr_erase_display(ALL);
        assert_colr_eq(*cres_all_move, *cres_all);
        colr_free(cres_all_move);
        colr_free(cres_all);
    }
}
// Colr_erase_line
subdesc(Colr_erase_line) {
    it("handles all EraseMethods") {
        EraseMethod tests[] = {
            END,
            START,
            ALL_MOVE,
            ALL,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_erase_line(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            colr_free(cres);
        }
    }
    it("handles ALL -> ALL_MOVE") {
        ColorResult* cres_all_move = Colr_erase_line(ALL_MOVE);
        ColorResult* cres_all = Colr_erase_line(ALL);
        assert_colr_eq(*cres_all_move, *cres_all);
        colr_free(cres_all_move);
        colr_free(cres_all);
    }

}
// Colr_move_back
subdesc(Colr_move_back) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_back(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_move_column
subdesc(Colr_move_column) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_column(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_move_down
subdesc(Colr_move_down) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_down(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_move_forward
subdesc(Colr_move_forward) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_forward(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_move_next
subdesc(Colr_move_next) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_next(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_move_pos
subdesc(Colr_move_pos) {
    it("basically works") {
        // Not much to test here.
        struct {
            unsigned int line;
            unsigned int column;
        } tests[] = {
            {0, 555},
            {777, 0},
            {1, 888},
            {444, 1},
            {3, 6},
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_pos(tests[i].line, tests[i].column);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            if (tests[i].line) {
                char* linestr = asserted_asprintf("%d", tests[i].line);
                assert_str_contains(s, linestr);
                free(linestr);
            }
            if (tests[i].column) {
                char* colstr = asserted_asprintf("%d", tests[i].column);
                assert_str_contains(s, colstr);
                free(colstr);
            }
            colr_free(cres);
        }
    }
}
// Colr_move_prev
subdesc(Colr_move_prev) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_prev(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_move_return
subdesc(Colr_move_return) {
    it("basically works") {
        // Not much to test here.
        ColorResult* cres = Colr_move_return();
        assert_not_null(cres);
        char* s = ColorResult_to_str(*cres);
        assert_not_null(s);
        assert_str_not_empty(s);
        colr_free(cres);
    }
}
// Colr_move_up
subdesc(Colr_move_up) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_move_up(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_pos_restore
subdesc(Colr_pos_restore) {
    it("basically works") {
        // Not much to test here.
        ColorResult* cres = Colr_pos_restore();
        assert_not_null(cres);
        char* s = ColorResult_to_str(*cres);
        assert_not_null(s);
        assert_str_not_empty(s);
        colr_free(cres);
    }
}
// Colr_pos_save
subdesc(Colr_pos_save) {
    it("basically works") {
        // Not much to test here.
        ColorResult* cres = Colr_pos_save();
        assert_not_null(cres);
        char* s = ColorResult_to_str(*cres);
        assert_not_null(s);
        assert_str_not_empty(s);
        colr_free(cres);
    }
}
// Colr_scroll_down
subdesc(Colr_scroll_down) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_scroll_down(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// Colr_scroll_up
subdesc(Colr_scroll_up) {
    it("basically works") {
        // Not much to test here.
        unsigned int tests[] = {
            0,
            1,
            999,
            88,
            -1,
        };
        for_each(tests, i) {
            ColorResult* cres = Colr_scroll_up(tests[i]);
            assert_not_null(cres);
            char* s = ColorResult_to_str(*cres);
            assert_not_null(s);
            assert_str_not_empty(s);
            char* numstr = asserted_asprintf("%d", tests[i]);
            if ((tests[i] > 0) && (tests[i] < (unsigned int)-1)) assert_str_contains(s, numstr);
            free(numstr);
            colr_free(cres);
        }
    }
}
// EraseMethod_to_str
subdesc(EraseMethod_to_str) {
    it("handles bad EraseMethods") {
        assert_null(EraseMethod_to_str(99));
    }
}
} // describe(controls)
