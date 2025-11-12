#pragma once

#include "../error.h"
#include "types.h"


/**
 * @api
 */
typedef struct {
    char* ptr;
    usize len;
} String;

/**
 * @api
 */
typedef struct {
    char* str;
    usize len;
} StringSlice;

// TODO: Add _constructor and _destructor methodos. Use for reference
// src/types/signal.h/signal_constructor and src/types/singal.h/signal_destructor
// THEY ARE NON-API


/**
 * @brief create empty String with null-terminator
 *
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
String* string_new(void);

/**
 * @brief create String by char*
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
String* string_from(const char* c_str);

/**
 * @brief clone String by existing String
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
String* string_clone(const String* c_str);

/**
 * @brief update String by char*
 * @param c_str pointer on new content of String
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_set_cstr(String* self, const char* c_str);

/**
 * @brief update String by existing String
 * @param str  copied String
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_set(String* self, const String* str);

/**
 * @brief get String by way char*
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
 */
char* string_cstr(const String* self);

/**
 * @brief get len of the passed String
 * @param dest_len place where needed to write len
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
 */
u64 string_len(const String* const c_str, boolean* success);

/**
 * @brief add src-String in the end of self-String
 * @param src  String that need to add
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_push_back(String* self, const String* src);

/**
 * @brief add  char* in the end of self-String
 * @param src  char* that need to add
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_push_back_cstr(String* self, const char* src);

/**
 * @brief add  src-String in the front of self-String
 * @param src  String that need to add
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_push_front(String* self, const String* src);

/**
 * @brief add  char* in the front of self-String
 * @param src  char* that need to add
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_push_front_cstr(String* self, const char* src);


/**
 * @brief insert char* in self-String
 * @param src char* that need to insert
 * @param i symbol index
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_insert_cstr(String* self, const char* src, const u64 i);

/**
 * @brief insert src-String in self-String
 * @param src String that need to insert
 * @param i symbol index
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_insert(String* self, const String* src, const u64 i);

/**
 * @brief remove symbol under symbol index
 * @param i symbol index
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_remove(String* self, const u64 i);


/**
 * @brief remove n of symbols since symbol  under symbol index
 * @warning perhaps being bug, need check
 * @param i symbol index
 * @param n count removed symbols
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_remove_n(String* self, const u64 i, const u64 n);


/**
 * @brief check on equality Strings
 * @param str1 fisrt String
 * @param str2 second String
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
 */
boolean string_equals(const String* str1, const String* str2);

/**
 * @brief check on equality String and char*
 * @param str checking String
 * @param c_str checking char*
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
 */
boolean string_equals_cstr(const String* str, const char* c_str);

/**
 * @breif Compare String and StringSlice
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
 */
boolean string_equals_slice(const String* str, StringSlice* slice);

/**
 * @brief free memory of passed String
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
 */
boolean string_free(String* self);


/**
 * @brief get StringSlice from String. Use for your allocated data
 * @param from start point
 * @param to end point
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_get_slice_constructor(StringSlice* self, const String* str, const u64 from, const u64 to);

/**
 * @brief get StringSlice from const char*. Use for your allocated data
 * @param size -1 if string is null-treminated and size should be automaticly calculated.
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_slice_from_cstr_constructor(StringSlice* self, const char* c_str, const i64 size);


/**
 * @brief get StringSlice from String
 * @param from start point
 * @param to end point
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
StringSlice* string_get_slice(const String* str, const u64 from, const u64 to);

/**
 * @brief get StringSlice from const char*
 * @param size 0 if string is null-treminated and size should be automaticly calculated.
 *
 * @api
 */
StringSlice* string_slice_from_cstr(const char* c_str, const i64 size);

/**
 * @brief String from StringSlice
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
String* string_from_slice(const StringSlice* str_sl);

/**
 * @brief check an equality between two StringSlice
 * @param str_sl_1 fisrt StringSlice
 * @param str_sl_2 second StringSlice
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
 */
boolean string_slice_equals(StringSlice* str_sl_1, StringSlice* str_sl_2);

/**
 * @brief add StringSlice in the end of the self-String
 * @param src StringSlice that will be added
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_push_back_slice(String* self, const StringSlice* src);

/**
 * @brief add StringSlice in front of self-String
 * @param src StringSlice that will be added
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_push_front_slice(String* self, const StringSlice* src);

/**
 * @brief insert StringSlice in String under symbol index i
 * @param src StringSlice that will be inserted
 * @param i   symbol index
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
boolean string_insert_slice(String* self, const StringSlice* src, const u64 i);

/**
 * @brief free memory of passed StringSLice
 *
 * @error ERROR_INVALID_ARGUMENT
 * @api
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

// TODO: Add _constructor and _destructor methodos. Use for reference
// src/types/signal.h/signal_constructor and src/types/singal.h/signal_destructor

/**
 * @brief create empty StringUTF8 with null-terminator
 *
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
StringUTF8* string_utf8_new(void);

/**
 * @brief create StringUTF8 by char*
 * @param c_str input data
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
StringUTF8* string_utf8_from(const char* c_str);

/**
 * @brief convert StringUTF8 to String
 *
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 * @api
 */
String* string_utf8_to_string(const StringUTF8* self);

/**
 * @api
 * @brief get len of passed StringUTF8
 * @error ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_len(const StringUTF8* self, u64* const dest_len);

/**
 * @api
 * @brief get size of passed StringUTF8
 * @error ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_size(const StringUTF8* self, u64* const dest_len);


/**
 * @api
 * @brief get copy StringUTF8
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
StringUTF8* string_utf8_clone(const StringUTF8* str);


/**
 * @api
 * @brief add char* in last of  StringUTF8
 * @param src input data by way char*
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_back_cstr(StringUTF8* self, char* src);

/**
 * @api
 * @brief add StringUTF8 in last of StringUTF8
 * @param src input data by way StringUTF8
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_back(StringUTF8* self, const StringUTF8* src);


/**
 * @api
 * @brief add 'src' by way StringUTF8 in 'self' under symbol index
 * @param src input data by way StringUTF8
 * @param i   symbol index
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_insert(StringUTF8* self, const StringUTF8* src, const usize i);

/**
 * @api
 * @brief add StringUTF8 in front of StringUTF8
 * @param src input data by way StringUTF8
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_front(StringUTF8* self, const StringUTF8* src);

/**
 * @api
 * @brief add char* in front of  StringUTF8
 * @param src input data by way char*
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_push_front_cstr(StringUTF8* self, char* src);

/**
 * @api
 * @brief add 'src' by way char* in 'self' under symbol index
 * @param src input data by way char*
 * @param i   symbol index
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_insert_cstr(StringUTF8* self, const char* src, const u64 i);

/**
 * @api
 * @brief remove symbol under symbol index
 * @param i symbol index
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_remove(StringUTF8* self, const u64 i);

/**
 * @api
 * @brief remove n of symbols since symbol under symbol index
 * @param i symbol index
 * @param n count removed symbols
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
boolean string_utf8_remove_n(StringUTF8* self, const u64 i, const u64 n);

/**
 * @api
 * @brief converting String to StrinfUTF8
 * @error ERROR_INVALID_ARGUMENT
 * @error ERROR_ALLOCATION_FAILED
 */
StringUTF8* string_to_string_utf8(const String* str);

/**
 * @api
 * @brief check on equality Strings-UTF8
 * @param str1 fisrt StringUTF8
 * @param str2 second StringUTF8
 * @error ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_equals(const StringUTF8* str1, const StringUTF8* str2);

/**
 * @api
 * @error ERROR_INVALID_ARGUMENT
 */
boolean string_utf8_free(StringUTF8* self);
