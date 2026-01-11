#include <core/types/string.h>
#include <string.h>
#include <core/error.h>
#include <core/types/types.h>
#include <core/platform/memory.h>
#include <core/log.h>

static inline boolean string_set_cstr_ex(String*, const char*, const u64);
static inline boolean string_push_back_cstr_ex(String*, const char*, const u64);
static inline boolean string_utf8_push_back_ex(StringUTF8*, const StringUTF8*);
static inline boolean string_insert_cstr_ex(String*, const char*, const u64, const u64);
static inline boolean string_utf8_dec(StringUTF8* self, const char*, const u64);

boolean string_destructor(String* self) {
    ERROR_ARG_CHECK(self, { return false; });
    tfree(self->ptr);
    return true;
}

boolean string_constructor(String* self) {
    ERROR_ARG_CHECK(self, { return false; });

    self->ptr = tmalloc(sizeof(char));
    ERROR_ALLOC_CHECK(self->ptr, { return false; });

    *(self->ptr) = '\0';
    self->len = 0;

    return false;
}

boolean string_from_constructor(String* self, const char* c_str) {
    ERROR_ARGS_CHECK_2(self, c_str, { return false; });

    self->len = strlen(c_str);
    self->ptr = tmalloc(self->len + 1);
    ERROR_ALLOC_CHECK(self->ptr, { return false; });

    memcpy(self->ptr, c_str, self->len + 1);

    return true;
}

boolean string_from_slice_constructor(String* self, const StringSlice* slice) {
    ERROR_ARGS_CHECK_2(self, slice, { return false; });

    self->ptr = tmalloc(slice->len + 1);
    ERROR_ALLOC_CHECK(self->ptr, { return false; });

    memcpy(self->ptr, slice->str, slice->len);
    self->len = slice->len;
    *(self->ptr + self->len) = '\0';

    return true;
}

boolean string_clone_constructor(String* self, const String* string) {
    ERROR_ARGS_CHECK_2(self, string, { return false; });

    self->len = string->len;
    self->ptr = tmalloc(string->len + 1);
    ERROR_ALLOC_CHECK(self->ptr, { return false; });

    memcpy(self->ptr, string->ptr, string->len + 1);

    return true;
}


boolean string_free(String* self) {
    ERROR_ARG_CHECK(self, { return false; });
    if (!string_destructor(self))
        return false;

    tfree(self);
    return true;
}

#define CONSTRUCTOR_CHECK(fn, var, bad_block)                                                           \
    do {                                                                                                \
        boolean status = fn;                                                                            \
        if (status) {                                                                                   \
            return var;                                                                                 \
        } else {                                                                                        \
            bad_block                                                                                   \
        }                                                                                               \
    } while (0)

String* string_new(void) {
    String* str_new = tmalloc(sizeof(String));
    ERROR_ALLOC_CHECK(str_new, { return NULL; });

    CONSTRUCTOR_CHECK(string_constructor(str_new), str_new, {
        tfree(str_new);
        return NULL;
    });
}

String* string_from(const char* c_str) {
    ERROR_ARG_CHECK(c_str, { return NULL; });

    String* str = tmalloc(sizeof(String));
    ERROR_ALLOC_CHECK(str, { return NULL; });

    CONSTRUCTOR_CHECK(string_from_constructor(str, c_str), str, {
        tfree(str);
        return NULL;
    });
}

String* string_from_slice(const StringSlice* slice) {
    ERROR_ARG_CHECK(slice, { return NULL; });

    String* str = tmalloc(sizeof(String));
    ERROR_ALLOC_CHECK(str, { return NULL; });


    CONSTRUCTOR_CHECK(string_from_slice_constructor(str, slice), str, {
        tfree(str);
        return NULL;
    });
}

String* string_clone(const String* other_string) {
    ERROR_ARG_CHECK(other_string, { return NULL; });

    String* str = tmalloc(sizeof(String));
    ERROR_ALLOC_CHECK(str, { return NULL; });

    CONSTRUCTOR_CHECK(string_clone_constructor(str, other_string), str, {
        tfree(str);
        return NULL;
    });
}

static inline boolean string_set_cstr_ex(String* self, const char* c_str, const u64 len_c_str) {
    if (c_str >= self->ptr && c_str <= self->ptr + self->len)
        return false;

    char* tmp_ptr = trealloc(self->ptr, len_c_str + 1);
    ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

    self->ptr = tmp_ptr;
    self->len = len_c_str;
    memcpy(self->ptr, c_str, len_c_str + 1);

    return true;
}

boolean string_set(String* self, const String* str) {
    ERROR_ARGS_CHECK_2(self, str, { return false; });
    return string_set_cstr_ex(self, str->ptr, str->len);
}

boolean string_set_cstr(String* self, const char* c_str) {
    ERROR_ARGS_CHECK_2(self, c_str, { return false; });
    return string_set_cstr_ex(self, c_str, strlen(c_str));
}

boolean string_set_slice(String* self, const StringSlice* str) {
    ERROR_ARGS_CHECK_2(self, str, { return false; });
    return string_set_cstr_ex(self, str->str, str->len);
}

char* string_cstr(const String* self) {
    ERROR_ARG_CHECK(self, { return NULL; });
    return self->ptr;
}

u64 string_len(const String* const c_str, boolean* success) {
    ERROR_ARGS_CHECK_1(c_str, {
        if (success) {
            *success = false;
        }
        return 0;
    });
    //*dest_len = c_str->len;
    if (success) {
        *success = true;
    }
    return c_str->len;
}


boolean string_push_back(String* self, const String* src) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });
    return string_push_back_cstr_ex(self, src->ptr, src->len);
}

boolean string_push_back_cstr(String* self, const char* src) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });
    return string_push_back_cstr_ex(self, src, strlen(src));
}

boolean string_push_back_slice(String* self, const StringSlice* src) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });
    return string_push_back_cstr_ex(self, src->str, src->len);
}

static inline boolean string_push_back_cstr_ex(String* self, const char* src, const u64 len_src) {
    char* tmp_ptr = trealloc(self->ptr, self->len + len_src + 1);
    ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

    if (src >= self->ptr && src <= self->ptr + self->len - 1)
        memcpy(tmp_ptr + self->len, tmp_ptr + (src - self->ptr), len_src);
    else
        memcpy(tmp_ptr + self->len, src, len_src);

    self->ptr = tmp_ptr;
    self->len += len_src;
    *(self->ptr + self->len) = '\0';

    return true;
}


boolean string_insert(String* self, const String* src, const u64 i) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });
    return string_insert_cstr_ex(self, src->ptr, i, src->len);
}

boolean string_insert_cstr(String* self, const char* src, const u64 i) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });
    return string_insert_cstr_ex(self, src, i, strlen(src));
}

boolean string_insert_slice(String* self, const StringSlice* src, const u64 i) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });
    return string_insert_cstr_ex(self, src->str, i, src->len);
}

static inline boolean string_insert_cstr_ex(
        String* self, const char* src, const u64 i, const u64 len_src
) {
    if (i > self->len) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid argument (output for boundary): i");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    // if (i == 0)
    //     return string_push_front_cstr_ex(self, src, len_src);

    if (i == self->len)
        return string_push_back_cstr_ex(self, src, len_src);

    char* tmp_ptr = trealloc(self->ptr, self->len + len_src + 1);
    ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

    if (src >= self->ptr && src <= self->ptr + self->len - 1) {
        memmove(tmp_ptr + len_src + i, tmp_ptr + i, self->len - i + 1);

        if (src - self->ptr >= i)
            memcpy(tmp_ptr + i, tmp_ptr + (src - self->ptr) + len_src, len_src);
        else
            memmove(tmp_ptr + i, tmp_ptr + (src - self->ptr), len_src);
    } else {
        memmove(tmp_ptr + len_src + i, tmp_ptr + i, self->len - i + 1);
        memcpy(tmp_ptr + i, src, len_src);
    }

    self->ptr = tmp_ptr;
    self->len += len_src;

    return true;
}


boolean string_remove(String* self, const u64 i) {
    ERROR_ARG_CHECK(self, { return false; });

    if (i >= self->len) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid argument (output for boundary): i");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    char* tmp_ptr = trealloc(self->ptr, self->len);
    ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

    self->ptr = tmp_ptr;
    memmove(self->ptr + i, self->ptr + i + 1, self->len - i + 1);
    self->len -= 1;
    *(self->ptr + self->len) = '\0';

    return true;
}

boolean string_remove_n(String* self, const u64 i, const u64 n) {
    if (n == 0)
        return false;

    if (n == 1)
        return string_remove(self, i);

    ERROR_ARG_CHECK(self, { return false; });

    if (i + n > self->len) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid arguments (output for boundary): i or n");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    char* tmp_ptr;

    if (i + n == self->len) { // если мы удаляем до конца строки начиная с i-индекса
        tmp_ptr =
                trealloc(self->ptr, i + 1); // в этом случае просто оставляем место для i кол-во символов
        ERROR_ALLOC_CHECK(tmp_ptr, { return false; });
        *(tmp_ptr + i) = '\0';
    } else {
        const u64 rem = self->len - i - n + 1;
        const u64 len_cache = rem > n ? n : rem;

        char* cache = tmalloc(len_cache);
        ERROR_ALLOC_CHECK(cache, { return false; });

        const u64 len_tmp = self->len - n + 1;
        memcpy(cache, self->ptr + self->len - len_cache + 1, len_cache);
        tmp_ptr = trealloc(self->ptr, len_tmp);
        ERROR_ALLOC_CHECK(tmp_ptr, {
            tfree(cache);
            return false;
        });

        if (rem > n)
            memmove(tmp_ptr + i, tmp_ptr + i + n, rem - n);

        memcpy(tmp_ptr + len_tmp - len_cache, cache, len_cache);
        tfree(cache);
    }

    self->ptr = tmp_ptr;
    self->len -= n;

    return true;
}

boolean string_equals(const String* str1, const String* str2) {
    ERROR_ARGS_CHECK_2(str1, str2, { return false; });
    if (str1->len != str2->len)
        return false;

    return memcmp(str1->ptr, str2->ptr, str1->len) ? false : true;
}

boolean string_equals_cstr(const String* str, const char* c_str) {
    ERROR_ARGS_CHECK_2(str, c_str, { return false; });
    if (str->len != strlen(c_str))
        return false;

    return memcmp(str->ptr, c_str, str->len) ? false : true;
}

boolean string_equals_slice(const String* str, StringSlice* slice) {
    ERROR_ARGS_CHECK_2(str, slice, { return false; });
    if (str->len != slice->len)
        return false;

    return memcmp(str->ptr, slice->str, str->len) ? false : true;
}


boolean string_get_slice_constructor(
        StringSlice* self, const String* str, const u64 from, const u64 to
) {
    ERROR_ARG_CHECK(str, { return false; });

    if (from >= str->len || to < from || to >= str->len) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid argument (output for boundary): s or e");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    self->len = to - from + 1;
    self->str = str->ptr + from;

    return true;
}

boolean string_slice_from_cstr_constructor(StringSlice* self, const char* c_str, const i64 size) {
    ERROR_ARGS_CHECK_2(self, c_str, { return false; });

    // Automaticly size calculation
    if (size == -1)
        self->len = strlen(c_str);
    else
        self->len = size;

    // StringSlice is guarantees, that self->s is read-only
    self->str = (char*) c_str;
    return true;
}

StringSlice* string_get_slice(const String* str, const u64 from, const u64 to) {
    StringSlice* self = tmalloc(sizeof(StringSlice));
    ERROR_ALLOC_CHECK(self, { return NULL; });

    boolean status = string_get_slice_constructor(self, str, from, to);
    if (status) {
        return self;
    } else {
        tfree(self);
        return NULL;
    }
}

StringSlice* string_slice_from_cstr(const char* c_str, const i64 size) {
    StringSlice* self = tmalloc(sizeof(StringSlice));
    ERROR_ALLOC_CHECK(self, { return NULL; });

    boolean status = string_slice_from_cstr_constructor(self, c_str, size);
    if (status) {
        return self;
    } else {
        tfree(self);
        return NULL;
    }
}

boolean string_slice_equals(const StringSlice* str_sl_1, const StringSlice* str_sl_2) {
    ERROR_ARGS_CHECK_2(str_sl_1, str_sl_2, { return false; });
    if (str_sl_1->len != str_sl_2->len)
        return false;

    return memcmp(str_sl_1->str, str_sl_2->str, str_sl_1->len) ? false : true;
}

boolean string_slice_equals_cstr(const StringSlice* str_sl_1, const char* c_str) {
    ERROR_ARGS_CHECK_1(str_sl_1, { return false; });

    StringSlice tmp_slice;
    boolean status = string_slice_from_cstr_constructor(&tmp_slice, c_str, -1);
    if (status) {
        return string_slice_equals(str_sl_1, &tmp_slice);
    } else {
        return false;
    }
}

boolean string_slice_free(StringSlice* self) {
    ERROR_ARG_CHECK(self, { return false; });
    tfree(self);
    return true;
}

//<--------------------------- UTF-8 --------------------------->


static inline boolean string_utf8_dec(StringUTF8* self, const char* c_str, const u64 len_c_str) {
    if (len_c_str == 0) {
        return string_utf8_constructor(self);
    }

    u32* cache = tmalloc(sizeof(u32) * len_c_str);
    ERROR_ALLOC_CHECK(cache, { return false; });

    u32* tmp_ptr = cache;

    while (*c_str != 0) { // итерируемся до '\0'
        if ((*c_str & 0x80) == 0) { // маска для 1 байта
            *(tmp_ptr) = *(c_str++);
            memset((u8*) (tmp_ptr++) + 1, 0, 3);
        } else if ((c_str[0] & 0xE0) == 0xC0 && (c_str[1] & 0xC0) == 0x80) { // маска для 2 байт
            memcpy(tmp_ptr, c_str, 2);
            memset((u8*) (tmp_ptr++) + 2, 0, 2);
            c_str += 2; // это сдвиг на кол-во добавляемых байтов
        } else if ((c_str[0] & 0xF0) == 0xE0 && (c_str[1] & 0xC0) == 0x80 &&
                   (c_str[2] & 0xC0) == 0x80) { // маска для 3 байт
            memcpy(tmp_ptr, c_str, 3);
            memset((u8*) (tmp_ptr++) + 3, 0, 1);
            c_str += 3;
        } else if ((c_str[0] & 0xF8) == 0xF0 && (c_str[1] & 0xC0) == 0x80 && (c_str[2] & 0xC0) == 0x80 &&
                   (c_str[3] & 0xC0) == 0x80) { // маска для 4 байт
            memcpy(tmp_ptr++, c_str, 4);
            c_str += 4;
        } else {
            LOG_ERROR_OR_DEBUG_FATAL("Invalid argument 'c_str': Incorrect UTF8-format");
            set_error(ERROR_INVALID_ARGUMENT);
            tfree(cache);
            return false;
        }
    }

    usize size_new_str = sizeof(u32) * (tmp_ptr - cache); // размер получившейся строки в байтах
    self->ptr = trealloc(cache, size_new_str);
    ERROR_ALLOC_CHECK(self->ptr, {
        tfree(cache);
        return false;
    });

    self->len = tmp_ptr - cache;

    return true;
}

//

boolean string_utf8_destructor(StringUTF8* self) {
    ERROR_ARG_CHECK(self, { return false; });
    tfree(self->ptr);

    return true;
}

boolean string_utf8_constructor(StringUTF8* self) {
    ERROR_ARG_CHECK(self, { return false; });
    self->len = 0;
    self->ptr = NULL;
    return true;
}

boolean string_utf8_from_constructor(StringUTF8* self, const char* c_str) {
    ERROR_ARGS_CHECK_2(self, c_str, { return false; });
    return string_utf8_dec(self, c_str, strlen(c_str));
}

boolean string_utf8_from_string_constructor(StringUTF8* self, const String* string) {
    ERROR_ARGS_CHECK_2(self, string, { return false; });
    return string_utf8_dec(self, string->ptr, string->len);
}

boolean string_utf8_from_slice_constructor(StringUTF8* self, const StringSlice* slice) {
    ERROR_ARGS_CHECK_2(self, slice, { return false; });
    return string_utf8_dec(self, slice->str, slice->len);
}

boolean string_utf8_clone_constructor(StringUTF8* self, const StringUTF8* str) {
    ERROR_ARGS_CHECK_2(self, str, { return false; });

    u64 size_str = sizeof(u32) * str->len;
    self->ptr = tmalloc(size_str);
    ERROR_ALLOC_CHECK(self->ptr, { return false; });

    self->len = str->len;
    memcpy(self->ptr, str->ptr, size_str);

    return true;
}


boolean string_utf8_free(StringUTF8* self) {
    ERROR_ARG_CHECK(self, { return false; });
    if (!string_utf8_destructor(self))
        return false;
    tfree(self);

    return true;
}

StringUTF8* string_utf8_new(void) {
    StringUTF8* str_new = tmalloc(sizeof(StringUTF8));
    ERROR_ALLOC_CHECK(str_new, { return NULL; });

    CONSTRUCTOR_CHECK(string_utf8_constructor(str_new), str_new, {
        tfree(str_new);
        return NULL;
    });
}


StringUTF8* string_utf8_from(const char* c_str) {
    ERROR_ARG_CHECK(c_str, { return NULL; });
    StringUTF8* str_new = tmalloc(sizeof(StringUTF8));
    ERROR_ALLOC_CHECK(str_new, { return NULL; });

    CONSTRUCTOR_CHECK(string_utf8_from_constructor(str_new, c_str), str_new, {
        tfree(str_new);
        return NULL;
    });
}

StringUTF8* string_utf8_from_string(const String* string) {
    ERROR_ARG_CHECK(string, { return NULL; });
    StringUTF8* str_new = tmalloc(sizeof(StringUTF8));
    ERROR_ALLOC_CHECK(str_new, { return NULL; });

    CONSTRUCTOR_CHECK(string_utf8_from_string_constructor(str_new, string), str_new, {
        tfree(str_new);
        return NULL;
    });
}

StringUTF8* string_utf8_from_slice(const StringSlice* slice) {
    ERROR_ARG_CHECK(slice, { return NULL; });
    StringUTF8* str_new = tmalloc(sizeof(StringUTF8));
    ERROR_ALLOC_CHECK(str_new, { return NULL; });

    CONSTRUCTOR_CHECK(string_utf8_from_slice_constructor(str_new, slice), str_new, {
        tfree(str_new);
        return NULL;
    });
}

StringUTF8* string_utf8_clone(const StringUTF8* str) {
    ERROR_ARG_CHECK(str, { return NULL; });

    StringUTF8* str_new = tmalloc(sizeof(String));
    ERROR_ALLOC_CHECK(str_new, { return NULL; });

    CONSTRUCTOR_CHECK(string_utf8_clone_constructor(str_new, str), str_new, {
        tfree(str_new);
        return NULL;
    });
}

String* string_utf8_to_string(const StringUTF8* self) {
    ERROR_ARG_CHECK(self, { return NULL; });

    u64 len_str = self->len;
    if (len_str == 0) // если кол-во элементов равно 0, то можно просто вызвать 'string_new'
        return string_new();

    u64 size_str = 4 * self->len;

    char* cache = tmalloc(size_str); // тут я очень упрощенно создаю кеш, взяв худшую ситуацию, что у нас
                                     // элементы все по 4 байта (без лишних)
    ERROR_ALLOC_CHECK(cache, { return NULL; });

    char* tmp_ptr = cache;

    String* str_new = tmalloc(sizeof(String));
    ERROR_ALLOC_CHECK(str_new, {
        tfree(cache);
        return NULL;
    });

    for (u64 i = 0; i < len_str; i++) {
        char c = *(char*) (self->ptr + i); // получаем байт в текущей итерации

        /*
            у нас есть 4 ситуации, когда символ требует 1, 2, 3, 4 байта;
            чтобы итерироваться быстрее, будем брать старший байт и по нему определять кол-во байтов для
           символа; так как в 'string_decode' я использую memcpy для вставки то меня не волнует вопрос о
           little или big endian, потому что это гарантиурет что старший байт будет первым; остается
           только научиться определять кол-во символов по старшему байту, для этого надо почитать по ним
           доку, тогда все станет ясно пример (кол-во единиц говорит о кол-ве байт): [0]100 0101 -> 1
           byte [110]1 0101 -> 2 byte [1110] 0101 -> 3 byte [11110] 101 -> 4 byte

            также подчеркну что последующие байты после старшего (кроме символа равному 1 байту) будут
           иметь маску [10]** **** можно было бы исползовать условия, но мне кажется это медленнее чем
           расчетный способ
        */
        char b7 = (c & 0x80) >> 7; // получение 7 бита (отсчет с нуля)
        char b6 = (c & 0x40) >> 6; // получение 6 бита
        char b5 = (c & 0x20) >> 5; // получение 5 бита
        char b4 = (c & 0x10) >> 4; // получение 4 бита

        /*
            cnt (count) - кол-во байт в числе

            если мы просто сложим биты то это может привести к неправильному определению
            пример:

            7654 3210 (индексы)
            [0101] 0000

            как видно у нас символ должен хранить байт, но неправильный способ скажет что у нас 2 байта
            я решил считать биты не просто так, а именно слева на право
            в случае с верхним примером мы поступим так:
            1) добавим b7 (чтобы данные правильно интерпретировались, надо вместо него сложить 1)
            2) затем будем добавлять другие биты, но умножим все их на b7, это приводит к тому, что если
           7 бит равен 0,то он просто обнулит остальные
        */
        char cnt = 1 + b7 * (b6 + b6 * (b5 + b5 * b4));

        memcpy(tmp_ptr, self->ptr + i, cnt);
        tmp_ptr += cnt;
    }

    u64 size_new_str = tmp_ptr - cache + 1;
    str_new->ptr = trealloc(cache, size_new_str);
    ERROR_ALLOC_CHECK(str_new->ptr, {
        tfree(cache);
        tfree(str_new);
        return NULL;
    });

    str_new->len = size_new_str - 1;
    *(str_new->ptr + str_new->len) = '\0';

    return str_new;
}


u64 string_utf8_len(const StringUTF8* self, boolean* success) {
    ERROR_ARGS_CHECK_1(self, {
        if (success)
            *success = false;
        return 0;
    });

    if (success)
        *success = true;

    return self->len;
}

u64 string_utf8_size(const StringUTF8* self, boolean* success) {
    ERROR_ARGS_CHECK_1(self, {
        if (success)
            *success = false;
        return 0;
    });

    if (success)
        *success = true;

    return sizeof(u32) * self->len;
}


static inline boolean string_utf8_push_back_ex(StringUTF8* self, const StringUTF8* src) {
    const u64 size_dest = 4 * self->len;
    const u64 size_src = 4 * src->len;

    u32* tmp_ptr = trealloc(self->ptr, size_dest + size_src);
    ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

    if (src->ptr >= self->ptr && src->ptr < self->ptr + self->len)
        memcpy(tmp_ptr + self->len, tmp_ptr + (src->ptr - self->ptr), size_src);
    else
        memcpy(tmp_ptr + self->len, src->ptr, size_src);

    self->ptr = tmp_ptr;
    self->len += src->len;

    return true;
}

boolean string_utf8_push_back(StringUTF8* self, const StringUTF8* src) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });
    return string_utf8_push_back_ex(self, src);
}

boolean string_utf8_push_back_cstr(StringUTF8* self, const char* src) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });

    StringUTF8 str_new;
    boolean status = string_utf8_from_constructor(&str_new, src);
    if (status == false)
        return false;

    status = string_utf8_push_back_ex(self, &str_new);
    string_utf8_destructor(&str_new);

    return status;
}

boolean string_utf8_push_back_string(StringUTF8* self, const String* src) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });

    StringUTF8 str_new;
    boolean status = string_utf8_from_string_constructor(&str_new, src);
    if (status == false)
        return false;

    status = string_utf8_push_back_ex(self, &str_new);
    string_utf8_destructor(&str_new);

    return status;
}

boolean string_utf8_push_back_slice(StringUTF8* self, const StringSlice* src) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });

    StringUTF8 str_new;
    boolean status = string_utf8_from_slice_constructor(&str_new, src);
    if (status == false)
        return false;

    status = string_utf8_push_back_ex(self, &str_new);
    string_utf8_destructor(&str_new);

    return status;
}

static inline boolean string_utf8_insert_ex(StringUTF8* self, const StringUTF8* src, const usize i) {
    if (i > self->len) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid argument (output for boundary): i");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    // if (i == 0)
    //     return string_utf8_push_front_ex(self, src);

    if (i == self->len)
        return string_utf8_push_back_ex(self, src);

    const u64 size_dest = 4 * self->len;
    const u64 size_src = 4 * src->len;

    u32* tmp_ptr = trealloc(self->ptr, size_dest + size_src);
    ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

    if (src->ptr >= self->ptr && src->ptr <= self->ptr + self->len - 1) {
        memmove(tmp_ptr + src->len + i, tmp_ptr + i, 4 * (self->len - i));

        if (src->ptr - self->ptr >= i)
            memcpy(tmp_ptr + i, tmp_ptr + (src->ptr - self->ptr) + src->len, size_src);
        else
            memmove(tmp_ptr + i, tmp_ptr + (src->ptr - self->ptr), size_src);
    } else {
        memmove(tmp_ptr + src->len + i, tmp_ptr + i, 4 * (self->len - i));
        memcpy(tmp_ptr + i, src->ptr, size_src);
    }

    self->ptr = tmp_ptr;
    self->len += src->len;

    return true;
}


boolean string_utf8_insert(StringUTF8* self, const StringUTF8* src, const u64 i) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });

    return string_utf8_insert_ex(self, src, i);
}


boolean string_utf8_insert_cstr(StringUTF8* self, const char* src, const u64 i) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });

    StringUTF8 str_new;
    boolean status = string_utf8_from_constructor(&str_new, src);
    if (status == false)
        return false;

    status = string_utf8_insert_ex(self, &str_new, i);
    string_utf8_destructor(&str_new);

    return status;
}

boolean string_utf8_insert_string(StringUTF8* self, const String* src, const u64 i) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });

    StringUTF8 str_new;
    boolean status = string_utf8_from_string_constructor(&str_new, src);
    if (status == false)
        return false;

    status = string_utf8_insert_ex(self, &str_new, i);
    string_utf8_destructor(&str_new);

    return status;
}

boolean string_utf8_insert_slice(StringUTF8* self, const StringSlice* src, const u64 i) {
    ERROR_ARGS_CHECK_2(self, src, { return false; });

    StringUTF8 str_new;
    boolean status = string_utf8_from_slice_constructor(&str_new, src);
    if (status == false)
        return false;

    status = string_utf8_insert_ex(self, &str_new, i);
    string_utf8_destructor(&str_new);

    return status;
}

boolean string_utf8_remove(StringUTF8* self, const u64 i) {
    ERROR_ARG_CHECK(self, { return false; });

    if (i >= self->len) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid argument (output for boundary): i");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    const u64 size_self = 4 * self->len;

    if (i + 1 == self->len) {
        u32* tmp_ptr = trealloc(self->ptr, size_self - 4);
        ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

        self->ptr = tmp_ptr;
        self->len -= 1;
    } else {
        u32 cache = *(self->ptr + self->len - 1);
        u32* tmp_ptr = trealloc(self->ptr, size_self - 4);
        ERROR_ALLOC_CHECK(tmp_ptr, { return false; });

        self->ptr = tmp_ptr;
        memmove(self->ptr + i, self->ptr + i + 1, size_self - 4 * (i + 2));
        self->len -= 1;
        *(self->ptr + self->len - 1) = cache;
    }

    return true;
}

boolean string_utf8_remove_n(StringUTF8* self, const u64 i, const u64 n) {
    if (n == 0)
        return false;

    if (n == 1)
        return string_utf8_remove(self, i);

    ERROR_ARG_CHECK(self, { return false; });

    if (i + n > self->len) {
        LOG_ERROR_OR_DEBUG_FATAL("Invalid arguments (output for boundary): i or n");
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }

    u32* tmp_ptr;

    if (i + n == self->len) {
        tmp_ptr = trealloc(self->ptr, 4 * i);
        ERROR_ALLOC_CHECK(tmp_ptr, { return false; });
    } else {
        u64 rem = self->len - i - n;
        u64 len_cache = rem > n ? n : rem;

        u32* cache = tmalloc(4 * len_cache);
        ERROR_ALLOC_CHECK(cache, { return false; });

        const u64 len_tmp = self->len - n;
        memcpy(cache, self->ptr + self->len - len_cache, 4 * len_cache);
        tmp_ptr = trealloc(self->ptr, 4 * len_tmp);
        ERROR_ALLOC_CHECK(tmp_ptr, {
            tfree(cache);
            return false;
        });

        if (rem > n)
            memmove(tmp_ptr + i, tmp_ptr + i + n, 4 * (rem - n));

        memcpy(tmp_ptr + i, cache, 4 * len_cache);
        tfree(cache);
    }

    self->ptr = tmp_ptr;
    self->len -= n;

    return true;
}


boolean string_utf8_equals(const StringUTF8* str1, const StringUTF8* str2) {
    ERROR_ARGS_CHECK_2(str1, str2, { return false; });

    if (str1->len != str2->len)
        return false;

    return memcmp(str1->ptr, str2->ptr, 4 * str1->len) ? false : true;
}
