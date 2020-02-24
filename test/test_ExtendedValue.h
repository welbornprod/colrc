/*! Tests for ExtendedValue, and it's related functions.
    \author Christopher Welborn
    \date   07-26-2019
*/


#ifndef TEST_EXTENDEDVALUE_H
#define TEST_EXTENDEDVALUE_H
#pragma clang diagnostic ignored "-Wempty-translation-unit"
#pragma clang diagnostic ignored "-Wvariadic-macros"

// Assert ExtendedValue_from_str(s) == val, with a better message on failure.
#define assert_ext_from_str_eq(s, val, msg...) \
    do { \
        int _a_e_f_s_e_eval = ExtendedValue_from_str(s); \
        if (_a_e_f_s_e_eval != val) { \
            char* _a_e_e_msg = "" msg; \
            if (_a_e_e_msg[0] == '\0') _a_e_e_msg = "Not equal"; \
            char* _a_e_e_eval_repr_a = ExtendedValue_repr(_a_e_f_s_e_eval); \
            char* _a_e_e_eval_repr_b = ExtendedValue_repr(val); \
            fail("%s: %s != %s", _a_e_e_msg, _a_e_e_eval_repr_a, _a_e_e_eval_repr_b); \
            free(_a_e_e_eval_repr_a); \
            free(_a_e_e_eval_repr_b); \
        } \
    } while (0)

// Assert ExtendedValue_from_str(s) != val, with a better message on failure.
#define assert_ext_from_str_neq(s, val, msg...) \
    do { \
        int _a_e_f_s_ne_eval = ExtendedValue_from_str(s); \
        if (_a_e_f_s_ne_eval == val) { \
            char* _a_e_e_msg = "" msg; \
            if (_a_e_e_msg[0] == '\0') _a_e_e_msg = "Equal"; \
            char* _a_e_e_eval_repr_a = ExtendedValue_repr(_a_e_f_s_ne_eval); \
            char* _a_e_e_eval_repr_b = ExtendedValue_repr(val); \
            fail("%s: %s != %s", _a_e_e_msg, _a_e_e_eval_repr_a, _a_e_e_eval_repr_b); \
            free(_a_e_e_eval_repr_a); \
            free(_a_e_e_eval_repr_b); \
        } \
    } while (0)

#endif // TEST_EXTENDEDVALUE_H
