/* Tests for the ColorResult struct and it's related functions.

    -Christopher Welborn 09-01-2019
*/

#include "test_ColrC.h"

describe(ColorResult) {
subdesc(ColorResult_empty) {
    it("creates an empty ColorResult") {
        ColorResult empty = ColorResult_empty();
        assert_size_eq_repr(empty.length, (size_t)-1, empty);
    }
}
subdesc(ColorResult_eq) {
    it("compares ColorResults") {
        struct {
            ColorResult a;
            ColorResult b;
            bool expected;
        } tests[] = {
            {ColorResult_empty(), ColorResult_empty(), true},
            {ColorResult_new("test"), ColorResult_new("test"), true},
            {ColorResult_new("test"), ColorResult_new("not"), false},
            {ColorResult_new("test"), ColorResult_new("not"), false},
            {ColorResult_new("test"), ColorResult_new("not"), false},
        };
        for_each(tests, i) {
            asserteq(ColorResult_eq(tests[i].a, tests[i].b), tests[i].expected);
        }
    }
}
subdesc(ColorResult_free) {
    it("frees it's resources") {
        ColorResult* p = Colr_join("-", Colr("test", fore(RED)), fore(WHITE));
        ColorResult_free(p);
    }
}
subdesc(ColorResult_is_ptr) {
    it("detects ColorResult pointers") {
        ColorArg carg = ColorArg_empty();
        ColorResult cres = ColorResult_empty();
        char* s = "test";
        struct {
            void* p;
            bool expected;
        } tests[] = {
            {&carg, false},
            {&cres, true},
            {s, false},
        };
        for_each(tests, i) {
            asserteq(ColorResult_is_ptr(tests[i].p), tests[i].expected);
        }
    }
}
subdesc(ColorResult_length) {
    it("calculates the length of a ColorResult") {
        struct {
            ColorResult* cres;
            size_t expected;
        } tests[] = {
            {Colr_join(NULL, "[", "]"), 1},
            {Colr_join("test", NULL), 1},
            {Colr_join("test", "[", "]"), 7},
        };
        for_each(tests, i) {
            size_t length = ColorResult_length(*tests[i].cres);
            assert_size_eq_repr(length, tests[i].expected, *tests[i].cres);
            ColorResult_free(tests[i].cres);
        }
    }
}
subdesc(ColorResult_new) {
    it("creates new ColorResults") {
        ColorResult cres = ColorResult_new("test");
        asserteq(cres.marker, COLORRESULT_MARKER);
        assert_size_eq_repr(cres.length, 5, cres);
    }
}
subdesc(ColorResult_repr) {
    it("creates a ColorResult repr") {
        char* s = ColorResult_repr(ColorResult_empty());
        free(s);
    }
}
subdesc(ColorResult_to_ptr) {
    it("allocates a ColorResult") {
        ColorResult* cres = Colr_join("test", "[", "]");
        assert(ColorResult_is_ptr(cres));
        ColorResult_free(cres);
    }
}
subdesc(ColorResult_to_str) {
    it("returns a string") {
        // Nothing to test here. Future development may change that though.
        char* s = "test";
        ColorResult cres = ColorResult_new(s);
        assert_ptr_eq_repr(ColorResult_to_str(cres), s, cres);
    }
}
}
