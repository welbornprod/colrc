/* Tests for the ArgType enum and it's related functions.

    -Christopher Welborn 08-01-2019
*/

#include "test_ColrC.h"

describe(ArgType) {
subdesc(ArgType_eq) {
    struct {
        ArgType a;
        ArgType b;
        bool expected;
    } tests[] = {
        {ARGTYPE_NONE, ARGTYPE_NONE, true},
        {FORE, FORE, true},
        {BACK, BACK, true},
        {ARGTYPE_NONE, FORE, false},
        {FORE, BACK, false},
        {BACK, ARGTYPE_NONE, false},
    };
    for_each(tests, i) {
        bool result = ArgType_eq(tests[i].a, tests[i].b);
        assert(result == tests[i].expected);
    }
}
subdesc(ArgType_repr) {
    it("creates a ArgType_repr") {
        char* s = ArgType_repr(ARGTYPE_NONE);
        assert_str_eq(s, "ARGTYPE_NONE", "ARGTYPE_NONE repr was bad.");
        free(s);
        s = ArgType_repr(FORE);
        assert_str_eq(s, "FORE", "FORE repr was bad.");
        free(s);
        s = ArgType_repr(BACK);
        assert_str_eq(s, "BACK", "BACK repr was bad.");
        free(s);
        s = ArgType_repr(STYLE);
        assert_str_eq(s, "STYLE", "STYLE repr was bad.");
        free(s);
    }
}
subdesc(ArgType_to_str) {
    it("creates a string from an ArgType") {
        char* s = ArgType_to_str(ARGTYPE_NONE);
        assert_str_eq(s, "none", "ARGTYPE_NONE string was bad.");
        free(s);
        s = ArgType_to_str(FORE);
        assert_str_eq(s, "fore", "FORE string was bad.");
        free(s);
        s = ArgType_to_str(BACK);
        assert_str_eq(s, "back", "BACK string was bad.");
        free(s);
        s = ArgType_to_str(STYLE);
        assert_str_eq(s, "style", "STYLE string was bad.");
        free(s);
    }
}
}
