#pragma once

#include "../error.h"
#include "types.h"


/**
 * @api
 */
typedef struct {
    char* ptr;
    u64 len;
} String;

/**
 * @api
 */
typedef struct {
    char* s;
    u64 len;
} StringSlice;

/**
 * @api
 * @brief create empty String with null-terminator
 * @warning ERROR_ALLOCATION_FAILED
 */
String* string_new(void);

/**
 * @api
 * @brief create String by char*
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
String* string_from(const char* c_str);

/**
 * @api
 * @brief clone String by existing String
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
String* string_clone(const String* c_str);

/**
 * @api
 * @brief update String by char*
 * @param c_str pointer on new content of String
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_set_cstr(String* self, const char* c_str);

/**
 * @api
 * @brief update String by existing String
 * @param str  copied String
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_set(String* self, const String* str);

/**
 * @api
 * @brief get String by way char*
 * @warning ERROR_INVALID_ARGUMENT
 */
char* string_cstr(const String* self);

/**
 * @api
 * @brief get len of the passed String
 * @param dest_len place where needed to write len
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_len(const String* const c_str, u64* const dest_len);

/**
 * @api
 * @brief get size of the passed String
 * @return return 0 if was passed c_str is NULL
 * @warning ERROR_INVALID_ARGUMENT
 */
u64 string_size(const String* c_str);

/**
 * @api
 * @brief add src-String in the end of self-String
 * @param src  String that need to add
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_push_back(String* self, const String* src);

/**
 * @api
 * @brief add  char* in the end of self-String
 * @param src  char* that need to add
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_push_back_cstr(String* self, const char* src);

/**
 * @api
 * @brief add  src-String in the front of self-String
 * @param src  String that need to add
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_push_front(String* self, const String* src);

/**
 * @api
 * @brief add  char* in the front of self-String
 * @param src  char* that need to add
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_push_front_cstr(String* self, const char* src);


/**
 * @api
 * @brief insert char* in self-String
 * @param src char* that need to insert
 * @param i symbol index
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_insert_cstr(String* self, const char* src, const u64 i);

/**
 * @api
 * @brief insert src-String in self-String
 * @param src String that need to insert
 * @param i symbol index
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_insert(String* self, const String* src, const u64 i);

/**
 * @api
 * @brief remove symbol under symbol index
 * @param i symbol index
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_remove(String* self, const u64 i);


/**
 * @api
 * @brief remove n of symbols since symbol  under symbol index
 * @warning perhaps being bag, need check
 * @param i symbol index
 * @param n count removed symbols
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_remove_n(String* self, const u64 i, const u64 n);


/**
 * @api
 * @brief check on equality Strings
 * @param str1 fisrt String
 * @param str2 second String
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_equals(const String* str1, const String* str2);

/**
 * @api
 * @brief check on equality String and char*
 * @param str checking String
 * @param c_str checking char*
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_equals_cstr(const String* str, const char* c_str);

/**
 * @api
 * @brief free memory of passed String
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_free(String* self);

/**
 * @api
 * @brief get StringSlice from String
 * @param s start point
 * @param e end point
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
StringSlice* string_get_slice(const String* str, const u64 s, const u64 e);

/**
 * @api
 * @brief String from StringSlice
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
String* string_from_slice(const StringSlice* str_sl);

/**
 * @api
 * @brief check on equality StringsSlice
 * @param str_sl_1 fisrt StringSlice
 * @param str_sl_2 second StringSlice
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_equals_slice(StringSlice* str_sl_1, StringSlice* str_sl_2);

/**
 * @api
 * @brief add StringSlice in last of self-String
 * @param src StringSlice that will be added
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_push_back_slice(String* self, const StringSlice* src);

/**
 * @api
 * @brief add StringSlice in front of self-String
 * @param src StringSlice that will be added
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_push_front_slice(String* self, const StringSlice* src);

/**
 * @api
 * @brief insert StringSlice in String under symbol index i
 * @param src StringSlice that will be inserted
 * @param i   symbol index
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_insert_slice(String* self, const StringSlice* src, const usize i);

/**
 * @api
 * @brief free memory of passed StringSLice
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_slice_free(StringSlice* self);

//<--------------------------- UTF-8 --------------------------->

/**
 * @api
 */
typedef struct {
    u32* ptr;
    usize len;
} StringUTF8;

/**
 * @api
 * @brief create empty StringUTF8 with null-terminator
 * @warning ERROR_ALLOCATION_FAILED
 */
StringUTF8* string_utf8_new(void);

/**
 * @api
 * @brief create StringUTF8 by char*
 * @param c_str input data
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
StringUTF8* string_utf8_from(const char* c_str);

/**
 * @api
 * @brief convert StringUTF8 to String
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
String* string_utf8_to_string(const StringUTF8* self);

/**
 * @api
 * @brief get len of passed StringUTF8
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_len(const StringUTF8* self, u64* const dest_len);

/**
 * @api
 * @brief get size of passed StringUTF8
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_size(const StringUTF8* self, u64* const dest_len);


/**
 * @api
 * @brief get copy StringUTF8
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
StringUTF8* string_utf8_clone(const StringUTF8* str);


/**
 * @api
 * @brief add char* in last of  StringUTF8
 * @param src input data by way char*
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_back_cstr(StringUTF8* self, char* src);

/**
 * @api
 * @brief add StringUTF8 in last of StringUTF8
 * @param src input data by way StringUTF8
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_back(StringUTF8* self, const StringUTF8* src);


/**
 * @api
 * @brief add 'src' by way StringUTF8 in 'self' under symbol index
 * @param src input data by way StringUTF8
 * @param i   symbol index
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_insert(StringUTF8* self, const StringUTF8* src, const usize i);

/**
 * @api
 * @brief add StringUTF8 in front of StringUTF8
 * @param src input data by way StringUTF8
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_front(StringUTF8* self, const StringUTF8* src);

/**
 * @api
 * @brief add char* in front of  StringUTF8
 * @param src input data by way char*
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_front_cstr(StringUTF8* self, char* src);

/**
 * @api
 * @brief add 'src' by way char* in 'self' under symbol index
 * @param src input data by way char*
 * @param i   symbol index
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_insert_cstr(StringUTF8* self, const char* src, const u64 i);

/**
 * @api
 * @brief remove symbol under symbol index
 * @param i symbol index
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_remove(StringUTF8* self, const u64 i);

/**
 * @api
 * @brief remove n of symbols since symbol under symbol index
 * @param i symbol index
 * @param n count removed symbols
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_remove_n(StringUTF8* self, const u64 i, const u64 n);

/**
 * @api
 * @brief converting String to StrinfUTF8
 * @warning ERROR_INVALID_ARGUMENT
 * @warning ERROR_ALLOCATION_FAILED
 */
StringUTF8* string_to_string_utf8(const String* str);

/**
 * @api
 * @brief check on equality Strings-UTF8
 * @param str1 fisrt StringUTF8
 * @param str2 second StringUTF8
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_equals(const StringUTF8* str1, const StringUTF8* str2);

/**
 * @api
 * @warning ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_free(StringUTF8* self);
