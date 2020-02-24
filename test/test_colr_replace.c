/*! Tests for the colr_replace* macros/functions.

    \author Christopher Welborn
    \date   12-29-2019
*/

#include "test_colr_replace.h"

describe(colr_replace) {
subdesc(colr_replace_str) {
    /*
        strings
    */
    it("replaces with strings") {
        // helpers.colr_str_replace already tests colr_str_replace.
        // Since colr_replace is just a wrapper around that function, these
        // tests will just ensure that the correct function is called.
        char* result = colr_replace("test", "s", "z");
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "tezt", "Failed to call colr_str_replace()");
        free(result);
    }
    it("replaces ColorArgs") {
        struct {
            char* s;
            char* target;
            ColorArg* repl;
            char* expected;
        } tests[] = {
            // Null/empty string and/or target.
            {NULL, "", NULL, NULL},
            {"", "", NULL, NULL},
            {"a", NULL, NULL, NULL},
            {"a", "", NULL, NULL},
            // Empty replacements.
            {"a", "a", NULL, ""},
            // ColorArgs.
            {"apple", "a", fore(RED), "\x1b[31mpple"},
            {"apple", "e", fore(RED), "appl\x1b[31m"},
            {"apple", "p", fore(RED), "a\x1b[31mple"},
            {
                " this has spaces ",
                " ",
                fore(RED),
                "\x1b[31mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorArg");
            free(result);
        }
    }
    it("replaces ColorResults") {
        struct {
            char* s;
            char* target;
            ColorResult* repl;
            char* expected;
        } tests[] = {
            // Null/empty string and/or target.
            {NULL, "", NULL, NULL},
            {"", "", NULL, NULL},
            {"a", NULL, NULL, NULL},
            {"a", "", NULL, NULL},
            // Empty replacements.
            {"a", "a", NULL, ""},
            // ColorResults.
            {
                "apple",
                "a",
                Colr_join("test", fore(RED), fore(RED)),
                "\x1b[31mtest\x1b[31m\x1b[0mpple"
            },
            {
                "apple",
                "e",
                Colr_join("test", "[", "]"),
                "appl[test]"
            },
            {
                "apple",
                "p",
                Colr_join("test", fore(RED), fore(RED)),
                "a\x1b[31mtest\x1b[31m\x1b[0mple"
            },
            {
                " this has spaces ",
                " ",
                Colr_join("test", "[", "]"),
                "[test]this has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorResult");
            free(result);
        }
    }
    it("replaces ColorTexts") {
        struct {
            char* s;
            char* target;
            ColorText* repl;
            char* expected;
        } tests[] = {
            // Null/empty string and/or target.
            {NULL, "", NULL, NULL},
            {"", "", NULL, NULL},
            {"a", NULL, NULL, NULL},
            {"a", "", NULL, NULL},
            // Empty replacements.
            {"a", "a", NULL, ""},
            // ColorTexts.
            {"apple", "a", Colr("test", fore(RED)), "\x1b[31mtest\x1b[0mpple"},
            {"apple", "e", Colr("test", fore(RED)), "appl\x1b[31mtest\x1b[0m"},
            {"apple", "p", Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0mple"},
            {
                " this has spaces ",
                " ",
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorText");
            free(result);
        }
    }
} // subdesc(colr_replace_str)
subdesc(colr_replace_regex) {
    /*
        regex_t
    */
    it("replaces regex_t with strings") {
        regex_t pat;
        compile_re(pat, "s");
        char* result = colr_replace("test", &pat, "z");
        regfree(&pat);
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "tezt", "colr_replace(regex_t) failed.");
        free(result);
    }
    it("replaces regex_t with ColorArgs") {
        struct {
            char* s;
            regex_t target;
            ColorArg* repl;
            char* expected;
        } tests[] = {
            // ColorArgs.
            {"apple", compiled_re("a"), fore(RED), "\x1b[31mpple"},
            {"apple", compiled_re("e"), fore(RED), "appl\x1b[31m"},
            {"apple", compiled_re("p"), fore(RED), "a\x1b[31mple"},
            {
                " this has spaces ",
                compiled_re(" "),
                fore(RED),
                "\x1b[31mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                &tests[i].target,
                tests[i].repl
            );
            regfree(&tests[i].target);
            assert_str_eq(result, tests[i].expected, "colr_replace(regex_t) failed on ColorArg");
            free(result);
        }
    }
    it("replaces regex_t with ColorResults") {
        struct {
            char* s;
            regex_t target;
            ColorResult* repl;
            char* expected;
        } tests[] = {
            // ColorResults.
            {
                "apple",
                compiled_re("a"),
                Colr_join("test", fore(RED), fore(RED)),
                "\x1b[31mtest\x1b[31m\x1b[0mpple"
            },
            {
                "apple",
                compiled_re("e"),
                Colr_join("test", "[", "]"),
                "appl[test]"
            },
            {
                "apple",
                compiled_re("p"),
                Colr_join("test", fore(RED), fore(RED)),
                "a\x1b[31mtest\x1b[31m\x1b[0mple"
            },
            {
                " this has spaces ",
                compiled_re(" "),
                Colr_join("test", "[", "]"),
                "[test]this has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                &tests[i].target,
                tests[i].repl
            );
            regfree(&tests[i].target);
            assert_str_eq(result, tests[i].expected, "colr_replace(regex_t) failed on ColorResult");
            free(result);
        }
    }
    it("replaces regex_t with ColorTexts") {
        struct {
            char* s;
            regex_t target;
            ColorText* repl;
            char* expected;
        } tests[] = {
            // ColorTexts.
            {"apple", compiled_re("a"), Colr("test", fore(RED)), "\x1b[31mtest\x1b[0mpple"},
            {"apple", compiled_re("e"), Colr("test", fore(RED)), "appl\x1b[31mtest\x1b[0m"},
            {"apple", compiled_re("p"), Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0mple"},
            {
                " this has spaces ",
                compiled_re(" "),
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                &tests[i].target,
                tests[i].repl
            );
            regfree(&tests[i].target);
            assert_str_eq(result, tests[i].expected, "colr_replace(regex_t) failed on ColorText");
            free(result);
        }
    }
} // subdesc(colr_replace_regex)
subdesc(colr_replace_regmatch) {
    /*
        regmatch_t
    */
    it("replaces regmatch_t with strings") {
        char* s = "test";
        regmatch_t matches[1];
        compile_match(matches, 1, "s", s);
        char* result = colr_replace(s, matches, "z");
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "tezt", "colr_replace(regmatch_t) failed.");
        free(result);
    }
    it("replaces regmatch_t with ColorArgs") {
        char* str = "apple";
        char* str_spaces = " this has spaces ";
        regmatch_t a_matches[1], e_matches[1], p_matches[1], sp_matches[1];
        compile_match(a_matches, 1, "a", str);
        compile_match(e_matches, 1, "e", str);
        compile_match(p_matches, 1, "p", str);
        compile_match(sp_matches, 1, " ", str_spaces);

        struct {
            char* s;
            regmatch_t* target;
            ColorArg* repl;
            char* expected;
        } tests[] = {
            // ColorArgs.
            {str, a_matches, fore(RED), "\x1b[31mpple"},
            {str, e_matches, fore(RED), "appl\x1b[31m"},
            {str, p_matches, fore(RED), "a\x1b[31mple"},
            {
                str_spaces,
                sp_matches,
                fore(RED),
                "\x1b[31mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            assert_not_null(result);
            assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "colr_replace(regmatch_t) failed on ColorArg");
            free(result);
        }
    }
    it("replaces regmatch_t with ColorResults") {
        char* str = "apple";
        char* str_spaces = " this has spaces ";
        regmatch_t a_matches[1], e_matches[1], p_matches[1], sp_matches[1];
        compile_match(a_matches, 1, "a", str);
        compile_match(e_matches, 1, "e", str);
        compile_match(p_matches, 1, "p", str);
        compile_match(sp_matches, 1, " ", str_spaces);

        struct {
            char* s;
            regmatch_t* target;
            ColorResult* repl;
            char* expected;
        } tests[] = {
            // ColorResults.
            {
                str,
                a_matches,
                Colr_join("test", fore(RED), fore(RED)),
                "\x1b[31mtest\x1b[31m\x1b[0mpple"
            },
            {
                str,
                e_matches,
                Colr_join("test", "[", "]"),
                "appl[test]"
            },
            {
                str,
                p_matches,
                Colr_join("test", fore(RED), fore(RED)),
                "a\x1b[31mtest\x1b[31m\x1b[0mple"
            },
            {
                str_spaces,
                sp_matches,
                Colr_join("test", "[", "]"),
                "[test]this has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            assert_not_null(result);
            assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "colr_replace(regmatch_t) failed on ColorResult");
            free(result);
        }
    }
    it("replaces regmatch_t with ColorTexts") {
        char* str = "apple";
        char* str_spaces = " this has spaces ";
        regmatch_t a_matches[1], e_matches[1], p_matches[1], sp_matches[1];
        compile_match(a_matches, 1, "a", str);
        compile_match(e_matches, 1, "e", str);
        compile_match(p_matches, 1, "p", str);
        compile_match(sp_matches, 1, " ", str_spaces);

        struct {
            char* s;
            regmatch_t* target;
            ColorText* repl;
            char* expected;
        } tests[] = {
            // ColorTexts.
            {str, a_matches, Colr("test", fore(RED)), "\x1b[31mtest\x1b[0mpple"},
            {str, e_matches, Colr("test", fore(RED)), "appl\x1b[31mtest\x1b[0m"},
            {str, p_matches, Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0mple"},
            {
                str_spaces,
                sp_matches,
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            assert_not_null(result);
            assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "colr_replace(regmatch_t) failed on ColorText");
            free(result);
        }
    }

} // subdesc(colr_replace_regmatch)
subdesc(colr_replace_re) {
    it("replaces regex strings with strings") {
        char* result = colr_replace_re("test", "s", "z", 0);
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "tezt", "Failed to call colr_str_replace_re()");
        free(result);
    }
    it("returns NULL for bad patterns") {
        char* result = colr_replace_re("test", "(s", "z", 0);
        assert_null(result);
    }
    it("returns NULL for missing matches") {
        char* result = colr_replace_re("test", "apple", "z", 0);
        assert_null(result);
    }
    it("replaces regex strings with ColorArgs") {
        struct {
            char* s;
            char* target;
            ColorArg* repl;
            char* expected;
        } tests[] = {
            // Null/empty string and/or target.
            {NULL, "", NULL, NULL},
            {"", "", NULL, NULL},
            {"a", NULL, NULL, NULL},
            {"a", "", NULL, NULL},
            // Empty replacements.
            {"a", "a", NULL, ""},
            // ColorArgs.
            {"apple", "a", fore(RED), "\x1b[31mpple"},
            {"apple", "e", fore(RED), "appl\x1b[31m"},
            {"apple", "p+", fore(RED), "a\x1b[31mle"},
            {
                " this has spaces ",
                " ",
                fore(RED),
                "\x1b[31mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_re(
                tests[i].s,
                tests[i].target,
                tests[i].repl,
                0
            );
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorArg");
            free(result);
        }
    }
    it("replaces regex strings with ColorResults") {
        struct {
            char* s;
            char* target;
            ColorResult* repl;
            char* expected;
        } tests[] = {
            // Null/empty string and/or target.
            {NULL, "", NULL, NULL},
            {"", "", NULL, NULL},
            {"a", NULL, NULL, NULL},
            {"a", "", NULL, NULL},
            // Empty replacements.
            {"a", "a", NULL, ""},
            // ColorResults.
            {
                "apple",
                "a",
                Colr_join("test", fore(RED), fore(RED)),
                "\x1b[31mtest\x1b[31m\x1b[0mpple"
            },
            {
                "apple",
                "e",
                Colr_join("test", "[", "]"),
                "appl[test]"
            },
            {
                "apple",
                "p+",
                Colr_join("test", fore(RED), fore(RED)),
                "a\x1b[31mtest\x1b[31m\x1b[0mle"
            },
            {
                " this has spaces ",
                " ",
                Colr_join("test", "[", "]"),
                "[test]this has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_re(
                tests[i].s,
                tests[i].target,
                tests[i].repl,
                0
            );
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorResult");
            free(result);
        }
    }
    it("replaces regex strings with ColorTexts") {
        struct {
            char* s;
            char* target;
            ColorText* repl;
            char* expected;
        } tests[] = {
            // Null/empty string and/or target.
            {NULL, "", NULL, NULL},
            {"", "", NULL, NULL},
            {"a", NULL, NULL, NULL},
            {"a", "", NULL, NULL},
            // Empty replacements.
            {"a", "a", NULL, ""},
            // ColorTexts.
            {"apple", "a", Colr("test", fore(RED)), "\x1b[31mtest\x1b[0mpple"},
            {"apple", "e", Colr("test", fore(RED)), "appl\x1b[31mtest\x1b[0m"},
            {"apple", "p+", Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0mle"},
            {
                " this has spaces ",
                " ",
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis has spaces "
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_re(
                tests[i].s,
                tests[i].target,
                tests[i].repl,
                0
            );
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorText");
            free(result);
        }
    }
} // subdesc(colr_replace_re)
} // describe(colr_re)
