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

#define compiled_re(pat_str) \
    __extension__ ({ \
        regex_t _c_r_pat; \
        compile_re(_c_r_pat, pat_str); \
        _c_r_pat; \
    })
