/* Tests for the ColorJustify struct and it's related functions.

    -Christopher Welborn 08-01-2019
*/

#include "test_ColrC.h"

describe(ColorJustify) {
subdesc(ColorJustify_empty) {
    it("creates an empty ColorJustify") {
        ColorJustify empty = ColorJustify_empty();
        assert(ColorJustify_is_empty(empty));
    }
}
subdesc(ColorJustify_eq) {
    it("compares ColorJustifys") {
        struct {
            ColorJustify a;
            ColorJustify b;
            bool expected;
        } tests[] = {
            {ColorJustify_empty(), ColorJustify_empty(), true},
            {(ColorJustify){JUST_LEFT, 4, ' '}, (ColorJustify){JUST_LEFT, 4, ' '}, true},
            {(ColorJustify){JUST_LEFT, 4, ' '}, (ColorJustify){JUST_RIGHT, 4, ' '}, false},
            {(ColorJustify){JUST_LEFT, 4, ' '}, (ColorJustify){JUST_LEFT, 4, 'X'}, false},
            {(ColorJustify){JUST_LEFT, 4, ' '}, (ColorJustify){JUST_NONE, 4, ' '}, false},
        };
        for_each(tests, i) {
            asserteq(ColorJustify_eq(tests[i].a, tests[i].b), tests[i].expected);
        }
    }
}
subdesc(ColorJustify_is_empty) {
    it("detects empty ColorJustifys") {
        ColorJustify empty = ColorJustify_empty();
        assert(ColorJustify_is_empty(empty));
        ColorJustify cjust = (ColorJustify){JUST_LEFT, 0, 0};
        assert(!ColorJustify_is_empty(cjust));
    }
}
subdesc(ColorJustify_repr) {
    it("creates a ColorJustify repr") {
        char* s = ColorJustify_repr(ColorJustify_empty());
        free(s);
    }
}
subdesc(ColorJustifyMethod_repr) {
    it("creates a ColorJustifyMethod_repr") {
        char* s = ColorJustifyMethod_repr(JUST_LEFT);
        assert_str_eq(s, "JUST_LEFT", "JUST_LEFT repr was bad.");
        free(s);
        s = ColorJustifyMethod_repr(JUST_RIGHT);
        assert_str_eq(s, "JUST_RIGHT", "JUST_RIGHT repr was bad.");
        free(s);
        s = ColorJustifyMethod_repr(JUST_CENTER);
        assert_str_eq(s, "JUST_CENTER", "JUST_CENTER repr was bad.");
        free(s);
        s = ColorJustifyMethod_repr(JUST_NONE);
        assert_str_eq(s, "JUST_NONE", "JUST_NONE repr was bad.");
        free(s);
    }
}
}
