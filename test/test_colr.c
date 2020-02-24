/*! Tests for the colr* macros/functions.

    \author Christopher Welborn
    \date   08-05-2019
*/

#include "test_ColrC.h"

describe(colr) {
// colr
subdesc(colr) {
    it("handles NULL") {
        char* s = colr_cat(NULL, "that");
        assert_str_eq(s, "that", "Should act like strdup() with a NULL argument.");
        free(s);
        char* s2 = colr_cat("this", NULL);
        assert_str_eq(s2, "this", "Should act like strdup() with a NULL argument.");
        free(s2);
        char* s3 = colr_cat("this", NULL, "that");
        assert_str_eq(s3, "thisthat", "Should act like strdup()/strcat() with a NULL argument.");
        free(s3);
        char* s4 = colr_cat("", NULL);
        assert_str_empty(s4);
        free(s4);
        char* s5 = colr_cat(NULL, NULL);
        assert_not_null(s5);
        assert_str_empty(s5);
        free(s5);
    }
    it("joins strings") {
        char* s = colr_cat("this", "that", "the other");
        assert_str_eq(s, "thisthatthe other", "Failed to join strings");
        free(s);
    }
    it("joins ColorArgs") {
        char* s = colr_cat(
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
    it("joins ColorResults") {
        char* s = colr_cat(
            Colr_join("this", "[", "]"),
            Colr_join("that", "[", "]"),
            Colr_join("the other", "[", "]")
        );
        assert_str_eq(
            s,
            "[this][that][the other]",
            "Failed to join ColorResults"
        );
        free(s);
    }
    it("joins ColorTexts") {
        char* s = colr_cat(
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
// colr_asprintf
subdesc(colr_asprintf) {
    it("handles alternate form") {
        char* mystring = NULL;
        colr_asprintf(&mystring, "%" COLR_FMT, Colr("Test", fore(RED)));
        assert_not_null(mystring);
        assert(colr_str_has_codes(mystring));
        free(mystring);
        // Now do the same thing, using the "alternate form".
        colr_asprintf(&mystring, "%#" COLR_FMT, Colr("Test", fore(RED)));
        assert_not_null(mystring);
        // Should not contain escape codes.
        assert(!colr_str_has_codes(mystring));
        free(mystring);
    }
    it("handles escape modifier") {
        char* escaped = NULL;
        colr_asprintf(&escaped, "%" COLR_FMT_MOD_ESC COLR_FMT, Colr("Test", fore(RED)));
        assert_not_null(escaped);
        assert_str_eq(escaped, "\"\\x1b[31mTest\\x1b[0m\"", "Did not escape.");
        free(escaped);
    }
    it("handles justification") {
        // center
        char* mycenter = NULL;
        colr_asprintf(&mycenter, "% 8" COLR_FMT, Colr("test", fore(RED)));
        assert_not_null(mycenter);
        assert_str_starts_with(mycenter, "  ");
        assert_str_ends_with(mycenter, "  ");
        free(mycenter);

        // left
        char* myleft = NULL;
        colr_asprintf(&myleft, "%-8" COLR_FMT, Colr("test", fore(RED)));
        assert_not_null(myleft);
        assert_str_ends_with(myleft, "    ");
        free(myleft);
        // right
        char* myright = NULL;
        colr_asprintf(&myright, "%8" COLR_FMT, Colr("test", fore(RED)));
        assert_not_null(myright);
        assert_str_starts_with(myright, "    ");
        free(myright);
    }
    it("handles strings") {
        char* tests[] = {
            "test",
            "this thing",
            "\nout\n",
        };
        for_each(tests, i) {
            char* mystring = NULL;
            colr_asprintf(&mystring, "%" COLR_FMT, tests[i]);
            assert_str_eq(mystring, tests[i], "Printf malformed a plain string.");
            free(mystring);
        }
    }
    it("handles colr objects") {
        struct {
            void* obj;
            char* original;
        } tests[] = {
            {
                Colr("This is a string.", fore(RED), style(UNDERLINE)),
                "This is a string."
            },
            {fore(RED), NULL},
            {back(WHITE), NULL},
            {
                Colr_join("-", Colr("this", fore(BLUE)), Colr("that", style(BRIGHT))),
                "this-that"
            },
        };
        for_each(tests, i) {
            char* to_str = NULL;
            if (ColorArg_is_ptr(tests[i].obj)) {
                ColorArg* copiedp = tests[i].obj;
                ColorArg copied = *copiedp;
                to_str = colr_to_str(copied);
            } else if (ColorResult_is_ptr(tests[i].obj)) {
                ColorResult* copiedp = tests[i].obj;
                copiedp = ColrResult(strdup(ColorResult_to_str(*copiedp)));
                ColorResult copied = *copiedp;
                to_str = strdup(colr_to_str(copied));
                colr_free(copiedp);
           } else if (ColorText_is_ptr(tests[i].obj)) {
                ColorText* copiedp = tests[i].obj;
                ColorText copied = *copiedp;
                to_str = colr_to_str(copied);
            } else {
                fail("Did not detect pointer type, and that is unforgivable.");
            }
            char* mystring = NULL;
            colr_asprintf(&mystring, "%" COLR_FMT, tests[i].obj);
            assert(colr_str_has_codes(mystring));
            assert_str_eq(mystring, to_str, "Printf output doesn't match to_str");
            free(to_str);
            char* stripped = colr_str_strip_codes(mystring);
            free(mystring);
            if (tests[i].original) {
                assert_str_eq(stripped, tests[i].original, "Stripped output doesn't match the input");
            } else {
                assert_str_empty(stripped);
            }
            free(stripped);
        }

    }
}
// colr_join
subdesc(colr_join) {
    it("handles NULL/empty") {
        char* result = colr_join(NULL, "this", "that");
        assert_str_null(result);
        result = colr_join("", NULL);
        assert_str_empty(result);
        free(result);
        result = colr_join("", NULL, NULL);
        assert_str_empty(result);
        free(result);
        result = colr_join("", "");
        assert_str_empty(result);
        free(result);
    }

    it("handles sentinel value") {
        char* s = _colr_join("", "test", "this", _ColrLastArg);
        assert_str_eq(s, "testthis", "Failed to stop on sentinel value!");
        free(s);
    }
    it("handles custom sentinel value") {
        struct _ColrLastArg_s* sent = malloc(sizeof(struct _ColrLastArg_s));
        *sent = _ColrLastArgValue;
        char* s = _colr_join("", "test", "this", "thing", sent);
        free(sent);
        assert_str_eq(s, "testthisthing", "Failed to stop on custom allocated sentinel value!");
        free(s);
    }
    // string
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
    it("joins strings by a ColorResult") {
        char* ct = colr_join(Colr_join("X", "[", "]"), "this", "that", "the other");
        assert_str_eq(
            ct,
            "this[X]that[X]the other",
            "Failed to join strings by a string"
        );
        free(ct);
    }
    it("joins strings by a ColorText") {
        char* ct = colr_join(Colr("X", fore(RED)), "this", "that", "the other");
        assert_str_eq(
            ct,
            "this\x1b[31mX\x1b[0mthat\x1b[31mX\x1b[0mthe other",
            "Failed to join strings by a string"
        );
        free(ct);
    }
    // ColorArg
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
    it("joins ColorArgs by a ColorResult") {
        char* ct = colr_join(
            Colr_join("X", "[", "]"),
            fore(RED),
            back(XWHITE),
            fore(rgb(255, 255, 255))
        );
        assert_str_eq(
            ct,
            "\x1b[31m[X]\x1b[48;5;7m[X]\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a ColorResult"
        );
        free(ct);
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
    // ColorResult
    it("joins ColorResults by a string") {
        char* s = colr_join(
            "-",
            Colr_join("A", "[", "]"),
            Colr_join("B", "[", "]"),
            Colr_join("C", "[", "]")
        );
        assert_str_eq(
            s,
            "[A]-[B]-[C]",
            "Failed to join ColorResults by a string"
        );
        free(s);
    }
    it("joins ColorResults by a ColorArg") {
        char* ca = colr_join(
            fore(GREEN),
            Colr_join("A", "[", "]"),
            Colr_join("B", "[", "]"),
            Colr_join("C", "[", "]")
        );
        assert_str_eq(
            ca,
            "[A]\x1b[32m[B]\x1b[32m[C]\x1b[0m",
            "Failed to join ColorResults by a ColorArg"
        );
        free(ca);
    }
    it("joins ColorResults by a ColorResult") {
        char* cr = colr_join(
            Colr_join("X", "[", "]"),
            Colr_join("A", "[", "]"),
            Colr_join("B", "[", "]"),
            Colr_join("C", "[", "]")
        );
        assert_str_eq(
            cr,
            "[A][X][B][X][C]",
            "Failed to join ColorResults by a ColorResult"
        );
        free(cr);
    }
    it("joins ColorResults by a ColorText") {
        char* ct = colr_join(
            Colr("X", fore(GREEN)),
            Colr_join("A", "[", "]"),
            Colr_join("B", "[", "]"),
            Colr_join("C", "[", "]")
        );
        assert_str_eq(
            ct,
            "[A]\x1b[32mX\x1b[0m[B]\x1b[32mX\x1b[0m[C]",
            "Failed to join ColorResults by a ColorText"
        );
        free(ct);
    }
    // ColorText
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
    it("joins ColorTexts by a ColorResult") {
        char* ct = colr_join(
            Colr_join("X", "[", "]"),
            Colr("this", fore(RED)),
            Colr("that", back(XWHITE)),
            Colr("the other", fore(rgb(255, 255, 255)))
        );
        assert_str_eq(
            ct,
            "\x1b[31mthis\x1b[0m[X]\x1b[48;5;7mthat\x1b[0m[X]\x1b[38;2;255;255;255mthe other\x1b[0m",
            "Failed to join ColorTexts by a ColorResult"
        );
        free(ct);
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
    it("handles NULL") {
        char* j = "joiner";
        char* null_words[] = {NULL, NULL, NULL, NULL};
        char* s = colr_join_array(j, null_words);
        assert_null(s);

        char* empty_words[] = {"", "", NULL};
        s = colr_join_array(j, empty_words);
        assert_str_eq(s, "joiner", "Empty string lists should be like strdup(joiner)");
        free(s);
    }
    it("handles empty objects") {
        char* j = "joiner";
        ColorArg** cargs = NULL;
        ColorArgs_array_fill(
            cargs,
            ColorArg_to_ptr(ColorArg_empty()),
            ColorArg_to_ptr(ColorArg_empty()),
            ColorArg_to_ptr(ColorArg_empty())
        );
        char* s = colr_join_array(j, cargs);
        ColorArgs_array_free(cargs);
        assert_str_eq(s, "joiner", "Empty arg lists should be like strdup(joiner)");
        free(s);
    }
    // string arrays
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
            "Failed to join strings by a string"
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
            "Failed to join strings by a ColorArg"
        );
        colr_free(cargp);
        free(s);
    }
    it("joins string arrays by ColorResults") {
        ColorResult* cresp = Colr_join("X", "[", "]");
        char* words[] = {
            "this",
            "that",
            "the other",
            NULL
        };
        char* s = colr_join_array(cresp, words);
        assert_str_eq(
            s,
            "this[X]that[X]the other",
            "Failed to join strings by a ColorResult"
        );
        colr_free(cresp);
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
            "Failed to join strings by a ColorText"
        );
        colr_free(ctextp);
        free(s);
    }
    // ColorArg arrays
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
            "Failed to join ColorArgs by a string"
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
            "Failed to join ColorArgs by a ColorArg"
        );
        colr_free(cargp);
        for_not_null(cargs, i) colr_free(cargs[i]);
        free(s);
    }
    it("joins ColorArg arrays by ColorResults") {
        ColorResult* cresp = Colr_join("X", "[", "]");
        ColorArg* cargs[] = {
            fore(RED),
            fore(XWHITE),
            fore(rgb(255, 255, 255)),
            NULL
        };
        char* s = colr_join_array(cresp, cargs);
        assert_str_eq(
            s,
            "\x1b[31m[X]\x1b[38;5;7m[X]\x1b[38;2;255;255;255m\x1b[0m",
            "Failed to join ColorArgs by a ColorResult"
        );
        colr_free(cresp);
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
            "Failed to join ColorArgs by a ColorText"
        );
        colr_free(ctextp);
        for_not_null(cargs, i) colr_free(cargs[i]);
        free(s);
    }
    // ColorResult arrays
    it("joins ColorResult arrays by strings") {
        char* j = "-";
        ColorResult* cress[] = {
            Colr_join("X", "[", "]"),
            Colr_join("X", "<", ">"),
            Colr_join("X", "(", ")"),
            NULL
        };
        char* s = colr_join_array(j, cress);
        assert_str_eq(
            s,
            "[X]-<X>-(X)",
            "Failed to join ColorResults by a string"
        );
        free(s);
        for_not_null(cress, i) colr_free(cress[i]);
    }
    it("joins ColorResult arrays by ColorArgs") {
        ColorArg* cargp = fore(RED);
        ColorResult* cress[] = {
            Colr_join("A", "[", "]"),
            Colr_join("B", "[", "]"),
            Colr_join("C", "[", "]"),
            NULL
        };
        char* s = colr_join_array(cargp, cress);
        assert_str_eq(
            s,
            "[A]\x1b[31m[B]\x1b[31m[C]\x1b[0m",
            "Failed to join ColorResults by a ColorArg"
        );
        colr_free(cargp);
        for_not_null(cress, i) colr_free(cress[i]);
        free(s);
    }
    it("joins ColorResult arrays by ColorResults") {
        ColorResult* cresp = Colr_join("X", "[", "]");
        ColorResult* cress[] = {
            Colr_join("A", "[", "]"),
            Colr_join("B", "[", "]"),
            Colr_join("C", "[", "]"),
            NULL
        };
        char* s = colr_join_array(cresp, cress);
        assert_str_eq(
            s,
            "[A][X][B][X][C]",
            "Failed to join ColorResults by a ColorResult"
        );
        colr_free(cresp);
        for_not_null(cress, i) colr_free(cress[i]);
        free(s);
    }
    it("joins ColorResult arrays by ColorTexts") {
        ColorText* ctextp = Colr("X", fore(RED));
        ColorResult* cress[] = {
            Colr_join("A", "[", "]"),
            Colr_join("B", "[", "]"),
            Colr_join("C", "[", "]"),
            NULL
        };
        char* s = colr_join_array(ctextp, cress);
        assert_str_eq(
            s,
            "[A]\x1b[31mX\x1b[0m[B]\x1b[31mX\x1b[0m[C]\x1b[0m",
            "Failed to join ColorResults by a ColorText"
        );
        colr_free(ctextp);
        for_not_null(cress, i) colr_free(cress[i]);
        free(s);
    }
    // ColorText arrays
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
            "Failed to join ColorTexts by a string"
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
            "Failed to join ColorTexts by a ColorArg"
        );
        colr_free(cargp);
        for_not_null(ctexts, i) colr_free(ctexts[i]);
        free(s);
    }
    it("joins ColorText arrays by ColorResults") {
        ColorResult* cresp = Colr_join("X", "[", "]");
        ColorText* ctexts[] = {
            Colr("A", fore(RED)),
            Colr("B", fore(XWHITE)),
            Colr("C", fore(rgb(255, 255, 255))),
            NULL
        };
        char* s = colr_join_array(cresp, ctexts);
        assert_str_eq(
            s,
            "\x1b[31mA\x1b[0m[X]\x1b[38;5;7mB\x1b[0m[X]\x1b[38;2;255;255;255mC\x1b[0m",
            "Failed to join ColorTexts by a ColorResult"
        );
        colr_free(cresp);
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
            "Failed to join ColorTexts by a ColorText"
        );
        colr_free(ctextp);
        for_not_null(ctexts, i) colr_free(ctexts[i]);
        free(s);
    }
}
subdesc(colr_repr) {
    it("sends arguments to the correct _repr function") {
        ColorArg** colrargs = NULL;
        ColorArgs_array_fill(colrargs, fore(RED), back(BLUE));

        ColorArg* cargp = fore(YELLOW);
        ColorResult* cresp = Colr_cat("This is a ", Colr("test", fore(BLUE)), ".");
        ColorText* ctextp = Colr("test", fore(RED));

        char* s = "this is a regular string.";

        char* tests[] = {
            colr_repr(fore_arg(RED)),
            colr_repr(colrargs),
            colr_repr(ColorJustify_new(JUST_LEFT, 4, ' ')),
            colr_repr((ColorJustifyMethod)JUST_RIGHT),
            colr_repr(ColorResult_new("testing")),
            colr_repr(*ctextp),
            colr_repr(color_val(rgb(55, 0, 155))),
            colr_repr(FORE),
            colr_repr(TYPE_EXTENDED),
            colr_repr(BLUE),
            colr_repr(ext(35)),
            colr_repr(rgb(55, 0, 155)),
            colr_repr(BRIGHT),
            colr_repr(((TermSize){.rows= 5, .columns= 10})),
            colr_repr("this is a constant string."),
            colr_repr(s),
            colr_repr((const char)'X'),
            colr_repr((char)'Y'),
            // Test against _colr_ptr_repr().
            colr_repr((void*)cargp),
            colr_repr((void*)cresp),
            colr_repr((void*)ctextp),
            colr_repr((void*)s),
        };
        // Free ColorArg list.
        ColorArgs_array_free(colrargs);
        // Free helper objects.
        colr_free(cargp);
        colr_free(ctextp);
        colr_free(cresp);

        for_each(tests, i) {
            assert_not_null(tests[i]);
            assert_str_not_empty(tests[i]);
            free(tests[i]);
        }
    }
}
subdesc(colr_snprintf) {
    it("handles alternate form") {
        char* s = "Test";
        size_t length = CODE_ANY_LEN + strlen(s);
        char mystring[length + 1];
        colr_snprintf(mystring, length + 1, "%" COLR_FMT, Colr(s, fore(RED)));
        assert_not_null(mystring);
        assert(colr_str_has_codes(mystring));
        // Now do the same thing, using the "alternate form".
        colr_snprintf(mystring, length + 1, "%#" COLR_FMT, Colr(s, fore(RED)));
        assert_not_null(mystring);
        // Should not contain escape codes.
        assert(!colr_str_has_codes(mystring));
    }
    it("handles escape modifier") {
        char* s = "Test";
        size_t length = CODE_ANY_LEN + strlen(s);
        char escaped[length + 1];
        colr_snprintf(
            escaped,
            length + 1,
            "%" COLR_FMT_MOD_ESC COLR_FMT,
            Colr("Test", fore(RED))
        );
        assert_str_not_empty(escaped);
        assert_str_eq(escaped, "\"\\x1b[31mTest\\x1b[0m\"", "Did not escape.");
    }
    it("handles justification") {
        ColorText* ctext = Colr("test", fore(RED));
        size_t length = ColorText_length(*ctext) + 8;
        colr_free(ctext);
        char mystring[length];
        // left
        colr_snprintf(mystring, length, "%-8" COLR_FMT, Colr("test", fore(RED)));
        assert_str_ends_with(mystring, "    ");
        // right
        colr_snprintf(mystring, length, "%8" COLR_FMT, Colr("test", fore(RED)));
        assert_str_starts_with(mystring, "    ");
        // center
        colr_snprintf(mystring, length, "% 8" COLR_FMT, Colr("test", fore(RED)));
        assert_str_starts_with(mystring, "  ");
        assert_str_ends_with(mystring, "  ");
    }
    it("handles strings") {
        char* tests[] = {
            "test",
            "this thing",
            "\nout\n",
        };
        for_each(tests, i) {
            char mystring[25];
            colr_snprintf(mystring, strlen(tests[i]) + 1, "%" COLR_FMT, tests[i]);
            assert_str_eq(mystring, tests[i], "Printf malformed a plain string");
        }
    }
    it("handles colr objects") {
        struct {
            void* obj;
            char* original;
        } tests[] = {
            {
                Colr("This is a string.", fore(RED), style(UNDERLINE)),
                "This is a string."
            },
            {fore(RED), NULL},
            {back(WHITE), NULL},
            {
                Colr_join("-", Colr("this", fore(BLUE)), Colr("that", style(BRIGHT))),
                "this-that"
            },
        };
        for_each(tests, i) {
            size_t color_len = 0;
            char* to_str = NULL;
            if (ColorArg_is_ptr(tests[i].obj)) {
                ColorArg* copiedp = tests[i].obj;
                ColorArg copied = *copiedp;
                color_len = colr_length(copied);
                to_str = colr_to_str(copied);
            } else if (ColorResult_is_ptr(tests[i].obj)) {
                ColorResult* copiedp = tests[i].obj;
                copiedp = ColrResult(strdup(ColorResult_to_str(*copiedp)));
                ColorResult copied = *copiedp;
                color_len = colr_length(copied);
                to_str = strdup(colr_to_str(copied));
                colr_free(copiedp);
           } else if (ColorText_is_ptr(tests[i].obj)) {
                ColorText* copiedp = tests[i].obj;
                ColorText copied = *copiedp;
                color_len = colr_length(copied);
                to_str = colr_to_str(copied);
            } else {
                fail("Did not detect pointer type, and that is unforgivable.");
            }
            char mystring[color_len];
            colr_snprintf(mystring, color_len, "%" COLR_FMT, tests[i].obj);
            assert(colr_str_has_codes(mystring));
            assert_str_eq(mystring, to_str, "Printf output doesn't match to_str");
            free(to_str);
            char* stripped = colr_str_strip_codes(mystring);
            if (tests[i].original) {
                assert_str_eq(stripped, tests[i].original, "Stripped output doesn't match the input");
            } else {
                assert_str_empty(stripped);
            }
            free(stripped);
        }

    }
} // subdesc(colr_sprintf)
subdesc(colr_to_str) {
    it("sends arguments to the correct _to_str function") {
        ColorArg* cargp = fore(RED);
        ColorText* ctextp = Colr("test", fore(BLUE));
        RGB rgbval = rgb(55, 0, 155);

        char* tests[] = {
            colr_to_str(FORE),
            colr_to_str(BLUE),
            colr_to_str(fore_arg(RED)),
            colr_to_str(*ctextp),
            colr_to_str(TYPE_EXTENDED),
            colr_to_str(ext(35)),
            colr_to_str(BRIGHT),
            colr_to_str(rgbval),
            colr_to_str((void*)cargp),
            colr_to_str((void*)ctextp),
        };

        colr_free(cargp);
        colr_free(ctextp);

        for_each(tests, i) {
            assert_not_null(tests[i]);
            assert_str_not_empty(tests[i]);
            free(tests[i]);
        }
        // ColorResult_to_str is special. Can't free it until we've used the
        // the resulting string.
        ColorResult* cresp = Colr_cat("This is a ", Colr("test", fore(RED)), ".");
        char* colrresult = colr_to_str(*cresp);
        assert_not_null(colrresult);
        assert_str_not_empty(colrresult);
        // This will actually be the same string, not leaking here.
        colrresult = colr_to_str((void*)cresp);
        assert_not_null(colrresult);
        assert_str_not_empty(colrresult);
        colr_free(cresp);
    }
}
} // describe(colr)
