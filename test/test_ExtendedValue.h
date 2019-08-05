/*! Tests for ExtendedValue, and it's related functions.
    \author Christopher Welborn
    \date   07-26-2019
*/


#ifndef TEST_EXTENDEDVALUE_H
#define TEST_EXTENDEDVALUE_H

// Assert ExtendedValue_from_str(s) == val, with a better message on failure.
#define assert_ext_from_str_eq(s, val, msg) \
    do { \
        int _a_e_f_s_e_eval = ExtendedValue_from_str(s); \
        char* _a_e_f_s_e_msg; \
        if_not_asprintf(&_a_e_f_s_e_msg, "%s (%s)", msg, s) { \
            fail("Cannot allocate memory for the failure message!"); \
        } \
        assert_ext_eq(_a_e_f_s_e_eval, val, _a_e_f_s_e_msg); \
        free(_a_e_f_s_e_msg); \
    } while (0)

// Assert ExtendedValue_from_str(s) != val, with a better message on failure.
#define assert_ext_from_str_neq(s, val, msg) \
    do { \
        int _a_e_f_s_ne_eval = ExtendedValue_from_str(s); \
        char* _a_e_f_s_ne_msg; \
        if_not_asprintf(&_a_e_f_s_ne_msg, "%s (%s)", msg, s) { \
            fail("Allocation failed for failure message!"); \
        }\
        assert_ext_neq(_a_e_f_s_ne_eval, val, _a_e_f_s_ne_msg); \
        free(_a_e_f_s_ne_msg); \
    } while (0)

// Assert two ExtendedValues are equal, with a better message on failure.
#define assert_ext_eq(a, b, msg) \
    do { \
        char* _a_e_e_msg; \
        char* _a_e_e_eval_repr_a = ExtendedValue_repr(a); \
        char* _a_e_e_eval_repr_b = ExtendedValue_repr(b); \
        if_not_asprintf(&_a_e_e_msg, "%s: %s != %s", msg, _a_e_e_eval_repr_a, _a_e_e_eval_repr_b) { \
            fail("Allocation failed for failure message!"); \
        }\
        free(_a_e_e_eval_repr_a); \
        free(_a_e_e_eval_repr_b); \
        if (a != b) { \
            fail("%s", _a_e_e_msg); \
        } \
        free(_a_e_e_msg); \
    } while (0)

// Assert two ExtendedValues are not equal, with a better message on failure.
#define assert_ext_neq(a, b, msg) \
    do { \
        char* _a_e_ne_msg; \
        char* _a_e_ne_eval_repr_a = ExtendedValue_repr(a); \
        char* _a_e_ne_eval_repr_b = ExtendedValue_repr(b); \
        if_not_asprintf(&_a_e_ne_msg, "%s: %s == %s", msg, _a_e_ne_eval_repr_a, _a_e_ne_eval_repr_b) { \
            fail("Allocation failed for failure message!"); \
        }\
        free(_a_e_ne_eval_repr_a); \
        free(_a_e_ne_eval_repr_b); \
        if (a == b) { \
            fail("%s", _a_e_ne_msg); \
        } \
        free(_a_e_ne_msg); \
    } while (0)

#endif // TEST_EXTENDEDVALUE_H
