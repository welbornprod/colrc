/*! Tests for the colr_replace* macros/functions.

    \author Christopher Welborn
    \date   12-29-2019
*/

#include "test_colr_replace.h"

describe(colr_replace_all) {
subdesc(colr_replace_str_all) {
    /*
        strings
    */
    it("replaces with strings") {
        // helpers.colr_str_replace already tests colr_str_replace.
        // Since colr_replace is just a wrapper around that function, these
        // tests will just ensure that the correct function is called.
        char* result = colr_replace_all("test", "t", "z");
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "zesz", "Failed to call colr_str_replace_all()");
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
            {"appliance", "a", NULL, "pplince"},
            {"good ending", "g", NULL, "ood endin"},
            // ColorArgs.
            {"apple", "a", fore(RED), "\x1b[31mpple"},
            {"apple", "e", fore(RED), "appl\x1b[31m"},
            {"apple", "p", fore(RED), "a\x1b[31m\x1b[31mle"},
            {
                " this has spaces ",
                " ",
                fore(RED),
                "\x1b[31mthis\x1b[31mhas\x1b[31mspaces\x1b[31m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
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
            {"appliance", "a", NULL, "pplince"},
            {"good ending", "g", NULL, "ood endin"},
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
                "a\x1b[31mtest\x1b[31m\x1b[0m\x1b[31mtest\x1b[31m\x1b[0mle"
            },
            {
                " this has spaces ",
                " ",
                Colr_join("test", "[", "]"),
                "[test]this[test]has[test]spaces[test]"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
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
            {"appliance", "a", NULL, "pplince"},
            {"good ending", "g", NULL, "ood endin"},
            // ColorTexts.
            {"apple", "a", Colr("test", fore(RED)), "\x1b[31mtest\x1b[0mpple"},
            {"apple", "e", Colr("test", fore(RED)), "appl\x1b[31mtest\x1b[0m"},
            {"apple", "p", Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0m\x1b[31mtest\x1b[0mle"},
            {
                " this has spaces ",
                " ",
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis\x1b[31mtest\x1b[0mhas\x1b[31mtest\x1b[0mspaces\x1b[31mtest\x1b[0m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
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
} // subdesc(colr_replace_str_all)
subdesc(colr_replace_regex_all) {
    /*
        regex_t
    */
    it("replaces regex_t with strings") {
        regex_t pat;
        compile_re(pat, "t");
        char* result = colr_replace_all("test", &pat, "z");
        regfree(&pat);
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "zesz", "colr_replace_all(regex_t) failed.");
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
            {"apple", compiled_re("p"), fore(RED), "a\x1b[31m\x1b[31mle"},
            {
                " this has spaces ",
                compiled_re(" "),
                fore(RED),
                "\x1b[31mthis\x1b[31mhas\x1b[31mspaces\x1b[31m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
                tests[i].s,
                &tests[i].target,
                tests[i].repl
            );
            regfree(&tests[i].target);
            assert_str_eq(result, tests[i].expected, "colr_replace_all(regex_t) failed on ColorArg");
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
                "a\x1b[31mtest\x1b[31m\x1b[0m\x1b[31mtest\x1b[31m\x1b[0mle"
            },
            {
                " this has spaces ",
                compiled_re(" "),
                Colr_join("test", "[", "]"),
                "[test]this[test]has[test]spaces[test]"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
                tests[i].s,
                &tests[i].target,
                tests[i].repl
            );
            regfree(&tests[i].target);
            assert_str_eq(result, tests[i].expected, "colr_replace_all(regex_t) failed on ColorResult");
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
            {"apple", compiled_re("p"), Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0m\x1b[31mtest\x1b[0mle"},
            {
                " this has spaces ",
                compiled_re(" "),
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis\x1b[31mtest\x1b[0mhas\x1b[31mtest\x1b[0mspaces\x1b[31mtest\x1b[0m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
                tests[i].s,
                &tests[i].target,
                tests[i].repl
            );
            regfree(&tests[i].target);
            assert_str_eq(result, tests[i].expected, "colr_replace(regex_t) failed on ColorText");
            free(result);
        }
    }
} // subdesc(colr_replace_regex_all)
    /*
        regex pattern strings
    */
subdesc(colr_replace_re_all) {
    it("replaces regex strings with strings") {
        char* result = colr_replace_re_all("test", "t", "z", 0);
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "zesz", "Failed to call colr_str_replace_re()");
        free(result);
    }
    it("returns NULL for bad patterns") {
        char* result = colr_replace_re_all("test", "(s", "z", 0);
        assert_null(result);
    }
    it("returns NULL for missing matches") {
        char* result = colr_replace_re_all("test", "apple", "z", 0);
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
            {"appliance", "a", NULL, "pplince"},
            {"good ending", "g", NULL, "ood endin"},
            // ColorArgs.
            {"apple", "a", fore(RED), "\x1b[31mpple"},
            {"apple", "e", fore(RED), "appl\x1b[31m"},
            {"apple", "p", fore(RED), "a\x1b[31m\x1b[31mle"},
            {
                " this has spaces ",
                " ",
                fore(RED),
                "\x1b[31mthis\x1b[31mhas\x1b[31mspaces\x1b[31m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_re_all(
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
            {"appliance", "a", NULL, "pplince"},
            {"good ending", "g", NULL, "ood endin"},
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
                "a\x1b[31mtest\x1b[31m\x1b[0m\x1b[31mtest\x1b[31m\x1b[0mle"
            },
            {
                " this has spaces ",
                " ",
                Colr_join("test", "[", "]"),
                "[test]this[test]has[test]spaces[test]"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_re_all(
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
            {"appliance", "a", NULL, "pplince"},
            {"good ending", "g", NULL, "ood endin"},
            // ColorTexts.
            {"apple", "a", Colr("test", fore(RED)), "\x1b[31mtest\x1b[0mpple"},
            {"apple", "e", Colr("test", fore(RED)), "appl\x1b[31mtest\x1b[0m"},
            {"apple", "p", Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0m\x1b[31mtest\x1b[0mle"},
            {
                " this has spaces ",
                " ",
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis\x1b[31mtest\x1b[0mhas\x1b[31mtest\x1b[0mspaces\x1b[31mtest\x1b[0m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace_re_all(
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
} // subdesc(colr_replace_re_all)
    /*
        regex matches
    */
subdesc(colr_replace_matches_all) {
    it("replaces regmatch_t** with strings") {
        char* s = "test";
        regmatch_t** matches = compiled_matches(s, "t");
        assert_not_null(matches);
        char* result = colr_replace_all(s, matches, "z");
        colr_free(matches);
        assert_not_null(result);
        assert_str_not_empty(result);
        assert_str_eq(result, "zesz", "Failed for regmatch_t**!");
        free(result);
    }
    it("returns NULL for missing matches") {
        char* s = "test";
        regmatch_t** matches = compiled_matches(s, "apple");
        assert_null(matches);
        char* result = colr_replace_all("test", matches, "z");
        assert_null(result);
    }
    it("replaces regmatch_t** with ColorArgs") {
        struct {
            char* s;
            regmatch_t** target;
            ColorArg* repl;
            char* expected;
        } tests[] = {
            // Null/empty string and/or target.
            {NULL, NULL, NULL, NULL},
            {"", NULL, NULL, NULL},
            {"a", NULL, NULL, NULL},
            // Empty replacements.
            {test_match_item("a", "a", NULL, "")},
            {test_match_item("appliance", "a", NULL, "pplince")},
            {test_match_item("good ending", "g", NULL, "ood endin")},
            // ColorArgs.
            {test_match_item("apple", "a", fore(RED), "\x1b[31mpple")},
            {test_match_item("apple", "e", fore(RED), "appl\x1b[31m")},
            {test_match_item("apple", "p", fore(RED), "a\x1b[31m\x1b[31mle")},
            {test_match_item(
                " this has spaces ",
                " ",
                fore(RED),
                "\x1b[31mthis\x1b[31mhas\x1b[31mspaces\x1b[31m"
            )}
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            // Free the matches.
            if (tests[i].target) colr_free(tests[i].target);
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorArg");
            free(result);
        }
    }
    it("replaces regmatch_t** with ColorResults") {
        struct {
            char* s;
            regmatch_t** target;
            ColorResult* repl;
            char* expected;
        } tests[] = {
            // Empty replacements.
            {test_match_item("a", "a", NULL, "")},
            {test_match_item("appliance", "a", NULL, "pplince")},
            {test_match_item("good ending", "g", NULL, "ood endin")},
            // ColorResults.
            {test_match_item(
                "apple",
                "a",
                Colr_join("test", fore(RED), fore(RED)),
                "\x1b[31mtest\x1b[31m\x1b[0mpple"
            }),
            {test_match_item(
                "apple",
                "e",
                Colr_join("test", "[", "]"),
                "appl[test]"
            }),
            {test_match_item(
                "apple",
                "p",
                Colr_join("test", fore(RED), fore(RED)),
                "a\x1b[31mtest\x1b[31m\x1b[0m\x1b[31mtest\x1b[31m\x1b[0mle"
            }),
            {test_match_item(
                " this has spaces ",
                " ",
                Colr_join("test", "[", "]"),
                "[test]this[test]has[test]spaces[test]"
            )}
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            // Free matches.
            if (tests[i].target) colr_free(tests[i].target);
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorResult");
            free(result);
        }
    }
    it("replaces regmatch_t** with ColorTexts") {
        struct {
            char* s;
            regmatch_t** target;
            ColorText* repl;
            char* expected;
        } tests[] = {
            // Empty replacements.
            {test_match_item("a", "a", NULL, "")},
            {test_match_item("appliance", "a", NULL, "pplince")},
            {test_match_item("good ending", "g", NULL, "ood endin")},
            // ColorTexts.
            {test_match_item("apple", "a", Colr("test", fore(RED)), "\x1b[31mtest\x1b[0mpple")},
            {test_match_item("apple", "e", Colr("test", fore(RED)), "appl\x1b[31mtest\x1b[0m")},
            {test_match_item("apple", "p", Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0m\x1b[31mtest\x1b[0mle")},
            {test_match_item(
                " this has spaces ",
                " ",
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis\x1b[31mtest\x1b[0mhas\x1b[31mtest\x1b[0mspaces\x1b[31mtest\x1b[0m"
            )}
        };
        for_each(tests, i) {
            char* result = colr_replace_all(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            // Free matches.
            if (tests[i].target) colr_free(tests[i].target);
            if (tests[i].expected) assert_not_null(result);
            if (tests[i].expected && tests[i].expected[0] != '\0') assert_str_not_empty(result);
            assert_str_eq(result, tests[i].expected, "Failed on ColorText");
            free(result);
        }
    }
} // subdesc(colr_replace_matches_all)
} // describe(colr_replace_all)
