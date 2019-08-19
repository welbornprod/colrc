/*! Tests for the ColrC library.

    \author Christopher Welborn
    \date 06-29-2019
*/


#ifndef TEST_COLR_H
/* Tell gcc to ignore this unused inclusion macro. */
#pragma GCC diagnostic ignored "-Wunused-macros"
/* Tell gcc to ignore clang pragmas, for linting. */
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
/* Tell clang to ignore this unused inclusion macro. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#define TEST_COLR_H
#pragma clang diagnostic pop
/* Warn for any other unused macros, for gcc and clang. */
#pragma GCC diagnostic warning "-Wunused-macros"
#pragma clang diagnostic push
#pragma clang diagnostic warning "-Wunused-macros"
#pragma clang diagnostic ignored "-Wvariadic-macros"

#include "../colr.h"

// snow redefines the `assert()` macro. It's better though.
#undef assert
#define SNOW_ENABLED
#include "snow.h"

#define assert_ColorArg_eq(a, b) \
    do { \
        if (!ColorArg_eq(a, b)) { \
            char* _a_CA_eq_a = colr_repr(a); \
            char* _a_CA_eq_b = colr_repr(b); \
            fail("ColorArgs are not equal: (" #a ") %s != (" #b ") %s", _a_CA_eq_a, _a_CA_eq_b); \
            free(_a_CA_eq_a); \
            free(_a_CA_eq_b); \
        }\
    } while (0)

#define assert_ColorArg_neq(a, b) \
    do { \
        if (ColorArg_eq(a, b)) { \
            char* _a_CA_neq_a = colr_repr(a); \
            char* _a_CA_neq_b = colr_repr(b); \
            fail("ColorArgs are equal: (" #a ") %s == (" #b ") %s", _a_CA_neq_a, _a_CA_neq_b); \
            free(_a_CA_neq_a); \
            free(_a_CA_neq_b); \
        }\
    } while (0)

#define assert_ColorText_has_arg(clrtext, clrarg) \
    do { \
        if (!ColorText_has_arg(clrtext, clrarg)) { \
            char* _a_CT_h_a_clrtext_repr = colr_repr(clrtext); \
            char* _a_CT_h_a_clrarg_repr = colr_repr(clrarg); \
            fail( \
                "ColorText does not contain ColorArg:\n     (" #clrtext ") %s\n  -> (" #clrarg ") %s", \
                _a_CT_h_a_clrtext_repr, \
                _a_CT_h_a_clrarg_repr \
            ); \
        }\
    } while (0)

#define assert_ColorText_nothas_arg(clrtext, clrarg) \
    do { \
        if (ColorText_has_arg(clrtext, clrarg)) { \
            char* _a_CT_h_a_clrtext_repr = colr_repr(clrtext); \
            char* _a_CT_h_a_clrarg_repr = colr_repr(clrarg); \
            fail( \
                "ColorText does contain ColorArg:\n     (" #clrtext ") %s\n  -> (" #clrarg ") %s", \
                _a_CT_h_a_clrtext_repr, \
                _a_CT_h_a_clrarg_repr \
            ); \
            free(_a_CT_h_a_clrtext_repr); \
            free(_a_CT_h_a_clrarg_repr); \
        }\
    } while (0)

#define assert_ColorValue_has(cval, val) \
    do { \
        if (!ColorValue_has(cval, val)) { \
            char* _a_CT_h_a_clrval_repr = colr_repr(cval); \
            char* _a_CT_h_a_val_repr = colr_repr(val); \
            fail( \
                "ColorValue does not contain value:\n     (" #cval ") %s\n  -> (" #val ") %s", \
                _a_CT_h_a_clrval_repr, \
                _a_CT_h_a_val_repr \
            ); \
            free(_a_CT_h_a_clrval_repr); \
            free(_a_CT_h_a_val_repr); \
        } \
    } while (0)

#define assert_colr_eq(a, b) \
    do { \
        if (!colr_eq(a, b)) { \
            char* _a_c_eq_a = colr_repr(a); \
            char* _a_c_eq_b = colr_repr(b); \
            fail("Not equal: (" #a ") %s != (" #b ") %s", _a_c_eq_a, _a_c_eq_b); \
            free(_a_c_eq_a); \
            free(_a_c_eq_b); \
        }\
    } while (0)

#define assert_colr_neq(a, b) \
    do { \
        if (colr_eq(a, b)) { \
            char* _a_c_neq_a = colr_repr(a); \
            char* _a_c_neq_b = colr_repr(b); \
            fail("Equal: (" #a ") %s == (" #b ") %s", _a_c_neq_a, _a_c_neq_b); \
            free(_a_c_neq_a); \
            free(_a_c_neq_b); \
        }\
    } while (0)

/*! \def assert_colr_eq_repr
    Like assert_colr_eq(), but adds an extra argument to print the repr() for.

    \pi a       First value to compare.
    \pi b       Second value to compare.
    \pi colrobj Any object compatible with colr_repr(), to get the repr for.
*/
#define assert_colr_eq_repr(a, b, colrobj) \
    do { \
        if (!colr_eq(a, b)) { \
            char* _a_c_eq_a = colr_repr(a); \
            char* _a_c_eq_b = colr_repr(b); \
            char* _a_c_eq_repr = colr_repr(colrobj); \
            fail( \
                "Not equal: (" #a ") %s != (" #b ") %s\n    Extra Repr: %s", \
                _a_c_eq_a, \
                _a_c_eq_b, \
                _a_c_eq_repr \
            ); \
            free(_a_c_eq_a); \
            free(_a_c_eq_b); \
            free(_a_c_eq_repr); \
        }\
    } while (0)

/*! \def assert_colr_neq_repr
    Like assert_colr_neq(), but adds an extra argument to print the repr() for.

    \pi a       First value to compare.
    \pi b       Second value to compare.
    \pi colrobj Any object compatible with colr_repr(), to get the repr for.
*/
#define assert_colr_neq_repr(a, b, colrobj) \
    do { \
        if (colr_eq(a, b)) { \
            char* _a_c_neq_a = colr_repr(a); \
            char* _a_c_neq_b = colr_repr(b); \
            char* _a_c_neq_repr = colr_repr(colrobj); \
            fail( \
                "Equal: (" #a ") %s == (" #b ") %s\n    Extra Repr: %s", \
                _a_c_neq_a, \
                _a_c_neq_b, \
                _a_c_neq_repr \
            ); \
            free(_a_c_neq_a); \
            free(_a_c_neq_b); \
            free(_a_c_neq_repr); \
        }\
    } while (0)
// Assert two ExtendedValues are equal, with a better message on failure.
#define assert_ext_eq(a, b, msg) \
    do { \
        if (a != b) { \
            char* _a_e_e_eval_repr_a = ExtendedValue_repr(a); \
            char* _a_e_e_eval_repr_b = ExtendedValue_repr(b); \
            fail("%s: %s != %s", msg, _a_e_e_eval_repr_a, _a_e_e_eval_repr_b); \
            free(_a_e_e_eval_repr_a); \
            free(_a_e_e_eval_repr_b); \
        } \
    } while (0)

// Assert two ExtendedValues are not equal, with a better message on failure.
#define assert_ext_neq(a, b, msg) \
    do { \
        if (a == b) { \
            char* _a_e_ne_eval_repr_a = ExtendedValue_repr(a); \
            char* _a_e_ne_eval_repr_b = ExtendedValue_repr(b); \
            fail("%s: %s == %s", msg, _a_e_ne_eval_repr_a, _a_e_ne_eval_repr_b); \
            free(_a_e_ne_eval_repr_a); \
            free(_a_e_ne_eval_repr_b); \
        } \
    } while (0)

#define assert_fmt_op(a, op, b, fmt, msg) \
    do { \
        if (!(a op b)) { \
            fail("%s: (" #a ") " fmt " " #op " (" #b ") " fmt, msg, a, b); \
        }\
    } while (0)

#define assert_fmt_op_func(a, op, b, fmt, func, msg) \
    do { \
        if (!(a op b)) { \
            fail("%s: " #func "(" #a ") " fmt " " #op " " #func "(" #b ") " fmt, msg, a, b); \
        }\
    } while (0)

#define assert_from_esc_eq(s, val) \
    assert_colr_eq( \
        _Generic( \
            (val), \
            BasicValue: BasicValue_from_esc, \
            ColorArg: ColorArg_from_esc, \
            ColorValue: ColorValue_from_esc, \
            ExtendedValue: ExtendedValue_from_esc, \
            RGB: RGB_from_esc, \
            StyleValue: StyleValue_from_esc \
        )(s), \
        val \
    )

#define assert_from_esc_neq(s, val) \
    assert_colr_neq( \
        _Generic( \
            (val), \
            BasicValue: BasicValue_from_esc, \
            ColorArg: ColorArg_from_esc, \
            ColorValue: ColorValue_from_esc, \
            ExtendedValue: ExtendedValue_from_esc, \
            RGB: RGB_from_esc, \
            StyleValue: StyleValue_from_esc \
        )(s), \
        val \
    )

#define assert_from_str_eq(s, val) \
    assert_colr_eq( \
        _Generic( \
            (val), \
            BasicValue: BasicValue_from_str, \
            ColorArg: ColorArg_from_str, \
            ColorType: ColorType_from_str, \
            ColorValue: ColorValue_from_str, \
            ExtendedValue: ExtendedValue_from_str, \
            RGB: RGB_from_str, \
            StyleValue: StyleValue_from_str \
        )(s), \
        val \
    )

#define assert_from_str_neq(s, val) \
    assert_colr_neq( \
        _Generic( \
            (val), \
            BasicValue: BasicValue_from_str, \
            ColorArg: ColorArg_from_str, \
            ColorType: ColorType_from_str, \
            ColorValue: ColorValue_from_str, \
            ExtendedValue: ExtendedValue_from_str, \
            RGB: RGB_from_str, \
            StyleValue: StyleValue_from_str \
        )(s), \
        val \
    )

#define assert_hash_eq(a, b) assert_fmt_op(a, ==, b, COLR_HASH_FMT, "Hashes are not equal")
#define assert_hash_eq_func(a, b, func) assert_fmt_op_func(a, ==, b, COLR_HASH_FMT, func, "Hashes are not equal")
#define assert_hash_str_eq(a, b) assert_hash_str_op(a, ==, b, "Hashes are not equal")
#define assert_hash_str_neq(a, b) assert_hash_str_op(a, !=, b, "Hashes are equal")
#define assert_hash_str_op_func(a, op, b, func, msg) \
    do { \
        colr_hash _a_h_s_e_a = func(a); \
        colr_hash _a_h_s_e_b = func(b); \
        if (!(_a_h_s_e_a op _a_h_s_e_b)) { \
            char* _a_h_s_e_repr_a = colr_repr(a); \
            char* _a_h_s_e_repr_b = colr_repr(b); \
            fail( \
                "%s: " #func "(%s) " #op " " #func "(%s)", \
                msg, \
                _a_h_s_e_repr_a, \
                _a_h_s_e_repr_b \
            ); \
            free(_a_h_s_e_repr_a); \
            free(_a_h_s_e_repr_b); \
        } \
    } while (0)
#define assert_hash_str_op(a, op, b, msg) assert_hash_str_op_func(a, op, b, colr_str_hash, msg)

#define assert_hash_neq(a, b) assert_fmt_op(a, !=, b, COLR_HASH_FMT, "Hashes are equal")
#define assert_hash_neq_func(a, b, func) assert_fmt_op_func(a, !=, b, COLR_HASH_FMT, func, "Hashes are equal")

#define assert_int_eq(a, b) assert_fmt_op(a, ==, b, "%d", "Integers are not equal")
#define assert_int_neq(a, b) assert_fmt_op(a, ==, b, "%d", "Integers are equal")

#define assert_is_invalid(colrobj) \
    do { \
        if (!colr_is_invalid(colrobj)) { \
            char* _a_i_i_repr = colr_repr(colrobj); \
            fail( \
                "Supposed to be invalid: (" #colrobj ") %s", \
                _a_i_i_repr \
            ); \
            free(_a_i_i_repr); \
        } \
    } while (0)

#define assert_is_valid(colrobj) \
    do { \
        if (!colr_is_valid(colrobj)) { \
            char* _a_i_v_repr = colr_repr(colrobj); \
            fail( \
                "Supposed to be valid: (" #colrobj ") %s", \
                _a_i_v_repr \
            ); \
            free(_a_i_v_repr); \
        } \
    } while (0)

#define array_length(array) (sizeof(array) / sizeof(array[0]))

/*! Make sure a value is `NULL`.
    \details
    This actually checks boolean logic.
    It may be switched to `== NULL`.

    \pi x The value to check.
*/
#define assert_null(x) \
    do { \
        if (x) { \
            fail("Supposed to be NULL: " #x); \
        } \
    } while (0)

/*! Make sure a value is not `NULL`.
    \details
    This actually checks boolean logic.
    It may be switched to `!= NULL`.

    \pi x The value to check.
*/
#define assert_not_null(x) \
    do { \
        if (!x) { \
            fail("Not supposed to be NULL: " #x); \
        } \
    } while (0)

#define assert_RGB_eq(a, b) \
    do { \
        if (!RGB_eq(a, b)) { \
            char* _are_a = colr_repr(a); \
            char* _are_b = colr_repr(b); \
            fail("RGB values are not equal: %s != %s\n", _are_a, _are_b); \
            free(_are_a); \
            free(_are_b); \
        } \
    } while (0)

#define assert_range(x, xmin, xmax, msg) \
    do { \
        if (!in_range(x, xmin, xmax)) { \
            char* _a_r_msg = NULL; \
            char* _a_r_x_repr = test_repr(x); \
            char* _a_r_xmin_repr = test_repr(xmin); \
            char* _a_r_xmax_repr = test_repr(xmax); \
            if_not_asprintf(&_a_r_msg, "%s (%s): %s-%s", msg, _a_r_x_repr, _a_r_xmin_repr, _a_r_xmax_repr) { \
                fail("Allocation failed for failure message!"); \
            } \
            free(_a_r_x_repr); \
            free(_a_r_xmin_repr); \
            free(_a_r_xmax_repr); \
            fail("%s", _a_r_msg); \
            free(_a_r_msg); \
        } \
    } while (0)

#define assert_size_eq(a, b) \
    assert_size_op(a, ==, b, "Sizes are not equal")

#define assert_size_eq_full(a, b, colrobj) \
    assert_size_op_full(a, ==, b, colrobj, "Sizes are not equal")

#define assert_size_eq_repr(a, b, colrobj) \
    assert_size_op_repr(a, ==, b, colrobj, "Sizes are not equal")

#define assert_size_eq_str(a, b, colrobj) \
    assert_size_op_str(a, ==, b, colrobj, "Sizes are not equal")

#define assert_size_gt_full(a, b, colrobj) \
    assert_size_op_full(a, >, b, colrobj, "Size is not greater")

#define assert_size_gte_full(a, b, colrobj) \
    assert_size_op_full(a, >=, b, colrobj, "Size is not greater or equal")

#define assert_size_op(a, op, b, msg) \
    do { \
        if (!(a op b)) { \
            fail("%s: (" #a ") %zu " #op " (" #b ") %zu", msg, a, b); \
        }\
    } while (0)

#define assert_size_op_func(a, op, b, func, msg) \
    do { \
        if (!(a op b)) { \
            fail("%s: " #func "(" #a ") %zu " #op " " #func "(" #b ") %zu", msg, a, b); \
        }\
    } while (0)

#define assert_size_op_full(a, op,  b, colrobj, msg) \
    do { \
        if (!(a op b)) { \
            char* _a_s_op_f_repr = colr_repr(colrobj); \
            char* _a_s_op_f_str = colr_to_str(colrobj); \
            char* _a_s_op_f_strrepr = colr_repr(_a_s_op_f_str); \
            free(_a_s_op_f_str); \
            fail( \
                "%s: (" #a ") %zu " #op " (" #b ") %zu\n      Repr: %s\n    String: %s", \
                msg, \
                (size_t) a, \
                (size_t) b, \
                _a_s_op_f_repr, \
                _a_s_op_f_strrepr \
            ); \
            free(_a_s_op_f_repr); \
            free(_a_s_op_f_strrepr); \
        }\
    } while (0)

#define assert_size_op_repr(a, op, b, colrobj, msg) \
    do { \
        if (!(a op b)) { \
            char* _a_s_op_r_repr = colr_repr(colrobj); \
            fail( \
                "%s: (" #a ") %zu " #op " (" #b ") %zu\n      Repr: %s", \
                msg, \
                (size_t) a, \
                (size_t) b, \
                _a_s_op_r_repr \
            ); \
            free(_a_s_op_r_repr); \
        }\
    } while (0)

#define assert_size_op_str(a, op, b, colrobj, msg) \
    do { \
        if (!(a op b)) { \
            char* _a_s_op_s_str = colr_to_str(colrobj); \
            fail( \
                "%s: (" #a ") %zu " #op " (" #b ") %zu\n    String: %s", \
                msg, \
                (size_t) a, \
                (size_t) b, \
                _a_s_op_s_str \
            ); \
            free(_a_s_op_s_str); \
        }\
    } while (0)

#define assert_str_contains(s, needle) \
    do { \
        assert_not_null(s); \
        assert_not_null(needle); \
        assert_str_not_empty(s); \
        assert_str_not_empty(needle); \
        if (!strstr(s, needle)) { \
            char* _a_s_c_repr = colr_repr(s); \
            char* _a_s_c_needle = colr_repr(needle); \
            fail("String does not contain %s: %s", _a_s_c_needle, _a_s_c_repr); \
        } \
    } while (0)

#define assert_str_empty(s) \
    do { \
        assert(s, "Empty string was actually NULL: " #s); \
        char* _a_e_s_repr = colr_repr(s); \
        if (s[0] != '\0') { \
            fail("String was not empty: " #s " == %s", _a_e_s_repr); \
        } \
        free(_a_e_s_repr); \
    } while (0)


/*! \def assert_str_eq
    Assert that two strings are equal, with a nice message with string reprs.

    \pi s1  First string to compare.
    \pi s2  Second string to compare.
    \pi msg Message for failures.
*/
#define assert_str_eq(s1, s2, msg) \
    do { \
        char* _a_s_e_use_msg = msg ? (msg[0] == '\0' ? "Strings aren't equal" : msg) : "Strings aren't equal"; \
        if (s1 == NULL && s2 == NULL) { /* cppcheck-suppress literalWithCharPtrCompare */ \
            (void)0; \
        } else if (s1 != NULL && s2 == NULL) { /* cppcheck-suppress literalWithCharPtrCompare */ \
            char* _a_s_e_s1_repr = colr_str_repr(s1); \
            fail("%s:\n    %s\n  != NULL", _a_s_e_use_msg, _a_s_e_s1_repr); \
            free(_a_s_e_s1_repr); \
        } else if (s2 != NULL && s1 == NULL) { /* cppcheck-suppress literalWithCharPtrCompare */ \
            char* _a_s_e_s2_repr = colr_str_repr(s2); \
            fail("%s:\n    NULL\n  != %s", _a_s_e_use_msg, _a_s_e_s2_repr); \
            free(_a_s_e_s2_repr); \
        } else if (strcmp(s1, s2) != 0) { \
            char* _a_s_e_s1_repr = colr_str_repr(s1); \
            char* _a_s_e_s2_repr = colr_str_repr(s2); \
            fail( \
                "%s:\n     %s\n  != %s", \
                _a_s_e_use_msg, \
                _a_s_e_s1_repr, \
                _a_s_e_s2_repr \
            ); \
            free(_a_s_e_s1_repr); \
            free(_a_s_e_s2_repr); \
        } \
    } while (0)

#define assert_str_list_contains(lst, s) \
    do { \
        if (!colr_str_list_contains(lst, s)) { \
            char* _a_s_l_c_repr = colr_repr(s); \
            char* _a_s_l_l_repr = colr_str_list_repr(lst); \
            fail( \
                #lst " does not contain: %s\n    List: %s", \
                _a_s_l_c_repr, \
                _a_s_l_l_repr \
            ); \
            free(_a_s_l_c_repr); \
            free(_a_s_l_l_repr); \
        }\
    } while (0)

#define assert_str_list_not_contains(lst, s) \
    do { \
        if (colr_str_list_contains(lst, s)) { \
            char* _a_s_l_c_repr = colr_repr(s); \
            char* _a_s_l_l_repr = colr_str_list_repr(lst); \
            fail( \
                #lst " contains: %s\n    List: %s", \
                _a_s_l_c_repr, \
                _a_s_l_l_repr \
            ); \
            free(_a_s_l_c_repr); \
            free(_a_s_l_l_repr); \
        }\
    } while (0)

#define assert_str_not_empty(s) \
    do { \
        assert(s, "String was actually NULL: " #s); \
        if (s[0] == '\0') { \
            fail("String was empty: " #s " == \"\""); \
        } \
    } while (0)

#define assert_str_list_size_eq_repr(a, b, lst) \
    assert_str_list_size_op_repr(a, ==, b, lst, "List sizes are not equal")

#define assert_str_list_size_op_repr(a, op, b, lst, msg) \
    do { \
        if (!(a op b)) { \
            char* _a_s_op_r_repr = colr_str_list_repr(lst); \
            fail( \
                "%s: (" #a ") %zu " #op " (" #b ") %zu\n      Repr: %s", \
                msg, \
                (size_t) a, \
                (size_t) b, \
                _a_s_op_r_repr \
            ); \
            free(_a_s_op_r_repr); \
        }\
    } while (0)

#define in_range(x, xmin, xmax) ((bool)((x >= xmin) && (x <= xmax)))

/*! Construct a for-loop to iterate over an array, where `x` is the index.

    \details
    The index is declared `volatile` because a `longjmp` may occur while
    testing in `snow`.

    \pi array_name The name of the array to iterate over.
    \pi x          The index variable to use (usually `i`).
*/
#define for_each(array_name, x) \
    size_t _fe_ ## array_name ## _len = array_length(array_name); \
    for (volatile size_t x = 0; x < _fe_ ## array_name ## _len; x++)

/*! Construct a for-loop to iterate from `0` to `len`, where `x` is the index.

    \details
    The index is declared `volatile` because a `longjmp` may occur while
    testing in `snow`.

    \pi len Stopping point. The final index will be `len - 1`.
    \pi x   The index variable to use (usually `i`).
*/
#define for_len(len, x) \
    for (volatile size_t x = 0; x < len; x++)

/*! Construct a for-loop to iterate over an array until `NULL` is reached.

    \details
    The index is declared `volatile` because a `longjmp` may occur while
    testing in `snow`.

    \details
    The array <em>must contain a `NULL` element at the end</em>!

    \pi array_name The name of the array to iterate over.
    \pi x          Variable name for the for-loop (usually `i`).
*/
#define for_not_null(array_name, x) \
    for (volatile size_t x = 0; array_name[x]; x++)

/*! Use stack strings to allocate and fill a list of string pointers.

    \pi dsl_lstname The name of the variable to use (`char**`).
    \pi ...         Strings to use.
*/
#define str_list_fill(dsl_lstname, ...) \
    do { \
        char* _d_s_l_stack[] = { __VA_ARGS__, NULL }; \
        size_t _d_s_l_len = array_length(_d_s_l_stack); \
        dsl_lstname = calloc(_d_s_l_len, sizeof(char*)); \
        assert(dsl_lstname != NULL); \
        for (size_t i = 0; _d_s_l_stack[i]; i++) { \
            if (_d_s_l_stack[i][0] == '\0') { \
                dsl_lstname[i] = colr_empty_str(); \
            } else { \
                dsl_lstname[i] = strdup(_d_s_l_stack[i]); \
            } \
        } \
        dsl_lstname[_d_s_l_len - 1] = NULL; \
    } while (0)

/*! \def test_repr
    Calls the correct \<type\>_repr method using `_Generic`.

    \pi     x The value to get a string representation for.
    \return An allocated string with the result.\n
            \mustfree
*/
#define test_repr(x) \
    _Generic( \
        (x), \
        char: colr_char_repr, \
        char*: colr_str_repr, \
        char**: colr_str_list_repr, \
        int: int_repr, \
        unsigned int: uint_repr, \
        long: long_repr, \
        long long: long_long_repr, \
        unsigned long: ulong_repr, \
        unsigned long long: ulong_long_repr \
    )(x)

/*! Kinda like colr_str_repr, but nothing is escaped.
    If the string is NULL, then "NULL" is returned.
    If the string is empty, then "\"\"" is returned.
    Otherwise, the string itself is returned.

    \pi s   The string to get the repr for.
    \return Either the string, "NULL", or "\"\"".
*/
#define test_str_repr(s) (s ? ((s[0] == '\0') ? "\"\"" : s) : "NULL")

size_t colr_str_list_len(char** lst);
char* colr_str_list_repr(char** lst);
char* int_repr(int x);
char* long_repr(long x);
char* long_long_repr(long long x);
char* uint_repr(unsigned int x);
char* ulong_repr(unsigned long x);
char* ulong_long_repr(unsigned long long x);


#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#endif /* TEST_COLR_H */
