/*! Tests for the colr_replace* macros/functions.

    \author Christopher Welborn
    \date   12-29-2019
*/

#include "test_ColrC.h"

#define compile_match(name, nmatches, pat_str, target_str) \
    do { \
        regex_t _c_m_pat; \
        compile_re(_c_m_pat, pat_str); \
        if (regexec(&_c_m_pat, target_str, nmatches, name, 0)) { \
            char* _c_m_pat_repr = colr_repr(pat_str); \
            char* _c_m_target_repr = colr_repr(target_str); \
            fail( \
                "Failed to match with regex: %s\n  In: %s", \
                _c_m_pat_repr, \
                _c_m_target_repr \
            ); \
        } \
        regfree(&_c_m_pat); \
    } while (0)

#define compile_re(name, pat_str) \
    do { \
        if (regcomp(&name, pat_str, 0)) { \
            regfree(&name); \
            char* _c_r_pat_repr = colr_repr(pat_str); \
            if (!_c_r_pat_repr) fail("Failed to allocate for pattern repr."); \
            fail("Failed to compile regex pattern: %s", _c_r_pat_repr); \
            free(_c_r_pat_repr); \
        } \
    } while (0)

#define compiled_matches(target_str, pat_str) \
    __extension__ ({ \
        regmatch_t** _c_m_m = NULL; \
        if (target_str && pat_str) { \
            regex_t _c_m_re; \
            compile_re(_c_m_re, pat_str); \
            _c_m_m = colr_re_matches(target_str, &_c_m_re); \
            regfree(&_c_m_re); \
        } \
        _c_m_m; \
    })

#define compiled_re(pat_str) \
    __extension__ ({ \
        regex_t _c_r_pat; \
        compile_re(_c_r_pat, pat_str); \
        _c_r_pat; \
    })

/*! \def test_match_item
    Returns comma-separated members of the test item struct in test_colr_replace_all.c.

    \pi s        (char*) The string to search.
    \pi pat_str  (char*) The pattern string to compile.
    \pi repl     (char*, ColorArg*, ColorResult*, ColorText*) The replacement.
    \pi expected (char*) The expected result.

    \return      Comma separated members/initializers.
*/
#define test_match_item(s, pat_str, repl, expected) \
    s, compiled_matches(s, pat_str), repl, expected
