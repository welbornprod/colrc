/* Tests for the ColorValue struct and it's related functions.

    -Christopher Welborn 08-10-2019
*/
#include "test_ColrC.h"

#define assert_colorval_from_str_eq(s, val) \
    do { \
        ColorValue _a_cv_f_s_e_cval = ColorValue_from_str(s); \
        if (!colr_eq(_a_cv_f_s_e_cval, val)) { \
            char* _a_cv_f_s_e_cval_repr = colr_repr(_a_cv_f_s_e_cval); \
            char* _a_cv_f_s_e_val_repr = colr_repr(val); \
            char* _a_cv_f_s_e_str_repr = colr_repr(s); \
            fail( \
                "ColorValue_from_str(%s) failed:\n     %s\n  != %s", \
                _a_cv_f_s_e_str_repr, \
                _a_cv_f_s_e_cval_repr, \
                _a_cv_f_s_e_val_repr \
            ); \
            free(_a_cv_f_s_e_str_repr); \
            free(_a_cv_f_s_e_cval_repr); \
            free(_a_cv_f_s_e_val_repr); \
        } \
    } while (0)

#define assert_colorval_from_str_eq_type(s, colrtype) \
    do { \
        ColorValue _a_cv_f_s_e_cval = ColorValue_from_str(s); \
        if (!colr_eq(_a_cv_f_s_e_cval.type, colrtype)) { \
            char* _a_cv_f_s_e_cval_repr = colr_repr(_a_cv_f_s_e_cval.type); \
            char* _a_cv_f_s_e_val_repr = colr_repr(colrtype); \
            char* _a_cv_f_s_e_str_repr = colr_repr(s); \
            fail( \
                "ColorValue_from_str(%s) failed for type:\n     %s\n  != %s", \
                _a_cv_f_s_e_str_repr, \
                _a_cv_f_s_e_cval_repr, \
                _a_cv_f_s_e_val_repr \
            ); \
            free(_a_cv_f_s_e_str_repr); \
            free(_a_cv_f_s_e_cval_repr); \
            free(_a_cv_f_s_e_val_repr); \
        } \
    } while (0)
