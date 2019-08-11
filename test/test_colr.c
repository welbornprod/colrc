/*! Tests for the colr* macros/functions.

    \author Christopher Welborn
    \date   08-05-2019
*/

#include "test_ColrC.h"

describe(colr) {
// colr
subdesc(colr) {
    it("handles NULL") {
        char* s = colr(NULL, "that");
        assert_str_empty(s);
        free(s);
        char* s2 = colr("this", NULL);
        assert_str_eq(s2, "this", "Should act like strdup() with a NULL argument.");
        free(s2);
        char* s3 = colr("this", NULL, "that");
        assert_str_eq(s3, "thisthat", "Should act like strdup()/strcat() with a NULL argument.");
        free(s3);
    }
    it("handles sentinel value") {
        char* s = _colr("test", "this", _ColrLastArg);
        assert_str_eq(s, "testthis", "Failed to stop on sentinel value!");
        free(s);
    }
    it("handles custom sentinel value") {
        struct _ColrLastArg_s* sent = malloc(sizeof(struct _ColrLastArg_s));
        *sent = _ColrLastArgValue;
        char* s = _colr("test", "this", "thing", sent);
        free(sent);
        assert_str_eq(s, "testthisthing", "Failed to stop on custom allocated sentinel value!");
        free(s);
    }
    it("joins strings") {
        char* s = colr("this", "that", "the other");
        assert_str_eq(s, "thisthatthe other", "Failed to join strings");
        free(s);
    }
    it("joins ColorArgs") {
        char* s = colr(
            fore(RED),
            back(XWHITE),
            fore(rgb(255, 255, 255))
        );
        assert_str_eq(
            s,
            "\x1b[31m\x1b[48;5;7m\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs"
        );
        free(s);
    }
    it("joins ColorTexts") {
        char* s = colr(
            Colr("this", fore(RED)),
            Colr("that", back(XWHITE)),
            Colr("the other", fore(rgb(255, 255, 255)))
        );
        assert_str_eq(
            s,
            "\x1b[31mthis\x1b[0m\x1b[48;5;7mthat\x1b[0m\x1b[38;2;255;255;255mthe other\x1b[0m",
            "Failed to join ColorTexts"
        );
        free(s);
    }
} // subdesc(colr)
// colr_join
subdesc(colr_join) {
    it("handles NULL") {
        char* result = colr_join(NULL, "this", "that");
        assert_str_empty(result);
        free(result);
    }
    it("joins strings by a string") {
        char* s = colr_join("-", "this", "that", "the other");
        assert_str_eq(
            s,
            "this-that-the other",
            "Failed to join strings by a string"
        );
        free(s);
    }
    it("joins strings by a ColorArg") {
        char* ca = colr_join(fore(RED), "this", "that", "the other");
        assert_str_eq(
            ca,
            "this\x1b[31mthat\x1b[31mthe other\x1b[0m",
            "Failed to join strings by a ColorArg"
        );
        free(ca);
    }
    it("joins strings by a ColorText") {
        char* ct = colr_join(Colr("X", fore(RED)), "this", "that", "the other");
        assert_str_eq(
            ct,
            "this\x1b[31mX\x1b[0mthat\x1b[31mX\x1b[0mthe other\x1b[0m",
            "Failed to join strings by a string"
        );
        free(ct);
    }
    it("joins ColorArgs by a string") {
        char* s = colr_join(
            "-",
            fore(RED),
            back(XWHITE),
            fore(rgb(255, 255, 255))
        );
        assert_str_eq(
            s,
            "\x1b[31m-\x1b[48;5;7m-\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a string"
        );
        free(s);
    }
    it("joins ColorArgs by a ColorArg") {
        char* ca = colr_join(
            fore(GREEN),
            fore(RED),
            back(XWHITE),
            fore(rgb(255, 255, 255))
        );
        assert_str_eq(
            ca,
            "\x1b[31m\x1b[32m\x1b[48;5;7m\x1b[32m\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a ColorArg"
        );
        free(ca);
    }
    it("joins ColorArgs by a ColorText") {
        char* ct = colr_join(
            Colr("X", fore(GREEN)),
            fore(RED),
            back(XWHITE),
            fore(rgb(255, 255, 255))
        );
        assert_str_eq(
            ct,
            "\x1b[31m\x1b[32mX\x1b[0m\x1b[48;5;7m\x1b[32mX\x1b[0m\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a ColorText"
        );
        free(ct);

    }
    it("joins ColorTexts by a string") {
        char* s = colr_join(
            "-",
            Colr("this", fore(RED)),
            Colr("that", back(XWHITE)),
            Colr("the other", fore(rgb(255, 255, 255)))
        );
        assert_str_eq(
            s,
            "\x1b[31mthis\x1b[0m-\x1b[48;5;7mthat\x1b[0m-\x1b[38;2;255;255;255mthe other\x1b[0m",
            "Failed to join ColorTexts by a string"
        );
        free(s);
    }
    it("joins ColorTexts by a ColorArg") {
        char* ca = colr_join(
            fore(GREEN),
            Colr("this", fore(RED)),
            Colr("that", back(XWHITE)),
            Colr("the other", fore(rgb(255, 255, 255)))
        );
        assert_str_eq(
            ca,
            "\x1b[31mthis\x1b[0m\x1b[32m\x1b[48;5;7mthat\x1b[0m\x1b[32m\x1b[38;2;255;255;255mthe other\x1b[0m",
            "Failed to join ColorTexts by a ColorArg"
        );
        free(ca);
    }
    it("joins ColorTexts by a ColorText") {
        char* ct = colr_join(
            Colr("X", fore(GREEN)),
            Colr("this", fore(RED)),
            Colr("that", back(XWHITE)),
            Colr("the other", fore(rgb(255, 255, 255)))
        );
        assert_str_eq(
            ct,
            "\x1b[31mthis\x1b[0m\x1b[32mX\x1b[0m\x1b[48;5;7mthat\x1b[0m\x1b[32mX\x1b[0m\x1b[38;2;255;255;255mthe other\x1b[0m",
            "Failed to join ColorTexts by a ColorText"
        );
        free(ct);
    }
} // subdesc(colr_join)
// colr_join_array
subdesc(colr_join_array) {
    it("joins string arrays by strings") {
        char* j = "-";
        char* words[] = {
            "this",
            "that",
            "the other",
            NULL
        };
        char* s = colr_join_array(j, words);
        assert_str_eq(
            s,
            "this-that-the other",
            "Failed to join strings by a string."
        );
        free(s);
    }
    it("joins string arrays by ColorArgs") {
        ColorArg* cargp = fore(RED);
        char* words[] = {
            "this",
            "that",
            "the other",
            NULL
        };
        char* s = colr_join_array(cargp, words);
        assert_str_eq(
            s,
            "this\x1b[31mthat\x1b[31mthe other\x1b[0m",
            "Failed to join strings by a ColorArg."
        );
        colr_free(cargp);
        free(s);
    }
    it("joins string arrays by ColorTexts") {
        ColorText* ctextp = Colr("X", fore(RED));
        char* words[] = {
            "this",
            "that",
            "the other",
            NULL
        };
        char* s = colr_join_array(ctextp, words);
        assert_str_eq(
            s,
            "this\x1b[31mX\x1b[0mthat\x1b[31mX\x1b[0mthe other\x1b[0m",
            "Failed to join strings by a ColorText."
        );
        colr_free(ctextp);
        free(s);
    }
    it("joins ColorArg arrays by strings") {
        char* j = "-";
        ColorArg* cargs[] = {
            fore(RED),
            fore(XWHITE),
            fore(rgb(255, 255, 255)),
            NULL
        };
        char* s = colr_join_array(j, cargs);
        assert_str_eq(
            s,
            "\x1b[31m-\x1b[38;5;7m-\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a string."
        );
        free(s);
        for_not_null(cargs, i) colr_free(cargs[i]);
    }
    it("joins ColorArg arrays by ColorArgs") {
        ColorArg* cargp = fore(RED);
        ColorArg* cargs[] = {
            fore(RED),
            fore(XWHITE),
            fore(rgb(255, 255, 255)),
            NULL
        };
        char* s = colr_join_array(cargp, cargs);
        assert_str_eq(
            s,
            "\x1b[31m\x1b[31m\x1b[38;5;7m\x1b[31m\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a ColorArg."
        );
        colr_free(cargp);
        for_not_null(cargs, i) colr_free(cargs[i]);
        free(s);
    }
    it("joins ColorArg arrays by ColorTexts") {
        ColorText* ctextp = Colr("X", fore(RED));
        ColorArg* cargs[] = {
            fore(RED),
            fore(XWHITE),
            fore(rgb(255, 255, 255)),
            NULL
        };
        char* s = colr_join_array(ctextp, cargs);
        assert_str_eq(
            s,
            "\x1b[31m\x1b[31mX\x1b[0m\x1b[38;5;7m\x1b[31mX\x1b[0m\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a ColorText."
        );
        colr_free(ctextp);
        for_not_null(cargs, i) colr_free(cargs[i]);
        free(s);
    }
    it("joins ColorText arrays by strings") {
        char* j = "-";
        ColorText* ctexts[] = {
            Colr("X", fore(RED)),
            Colr("X", fore(XWHITE)),
            Colr("X", fore(rgb(255, 255, 255))),
            NULL
        };
        char* s = colr_join_array(j, ctexts);
        assert_str_eq(
            s,
            "\x1b[31mX\x1b[0m-\x1b[38;5;7mX\x1b[0m-\x1b[38;2;255;255;255mX\x1b[0m",
            "Failed to join ColorTexts by a string."
        );
        free(s);
        for_not_null(ctexts, i) colr_free(ctexts[i]);
    }
    it("joins ColorText arrays by ColorArgs") {
        ColorArg* cargp = fore(RED);
        ColorText* ctexts[] = {
            Colr("A", fore(RED)),
            Colr("B", fore(XWHITE)),
            Colr("C", fore(rgb(255, 255, 255))),
            NULL
        };
        char* s = colr_join_array(cargp, ctexts);
        assert_str_eq(
            s,
            "\x1b[31mA\x1b[0m\x1b[31m\x1b[38;5;7mB\x1b[0m\x1b[31m\x1b[38;2;255;255;255mC\x1b[0m",
            "Failed to join ColorTexts by a ColorArg."
        );
        colr_free(cargp);
        for_not_null(ctexts, i) colr_free(ctexts[i]);
        free(s);
    }
    it("joins ColorText arrays by ColorTexts") {
        ColorText* ctextp = Colr("X", fore(RED));
        ColorText* ctexts[] = {
            Colr("A", fore(RED)),
            Colr("B", fore(XWHITE)),
            Colr("C", fore(rgb(255, 255, 255))),
            NULL
        };
        char* s = colr_join_array(ctextp, ctexts);
        assert_str_eq(
            s,
            "\x1b[31mA\x1b[0m\x1b[31mX\x1b[0m\x1b[38;5;7mB\x1b[0m\x1b[31mX\x1b[0m\x1b[38;2;255;255;255mC\x1b[0m",
            "Failed to join ColorTexts by a ColorText."
        );
        colr_free(ctextp);
        for_not_null(ctexts, i) colr_free(ctexts[i]);
        free(s);
    }
}
subdesc(colr_replace) {
    it("replaces with strings") {
        // helpers.colr_str_replace already tests colr_str_replace.
        // Since colr_replace is just a wrapper around that function, these
        // tests will just ensure that the correct function is called.
        char* result = colr_replace("test", "s", "z");
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
            {"apple", "p", fore(RED), "a\x1b[31m\x1b[31mle"},
            {
                " this has spaces ",
                " ",
                fore(RED),
                "\x1b[31mthis\x1b[31mhas\x1b[31mspaces\x1b[31m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            colr_free(tests[i].repl);
            assert_str_eq(result, tests[i].expected, "Failed on ColorArg");
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
            {"apple", "p", Colr("test", fore(RED)), "a\x1b[31mtest\x1b[0m\x1b[31mtest\x1b[0mle"},
            {
                " this has spaces ",
                " ",
                Colr("test", fore(RED)),
                "\x1b[31mtest\x1b[0mthis\x1b[31mtest\x1b[0mhas\x1b[31mtest\x1b[0mspaces\x1b[31mtest\x1b[0m"
            }
        };
        for_each(tests, i) {
            char* result = colr_replace(
                tests[i].s,
                tests[i].target,
                tests[i].repl
            );
            colr_free(tests[i].repl);
            assert_str_eq(result, tests[i].expected, "Failed on ColorText");
            free(result);
        }
    }
}
} // describe(colr_api)
