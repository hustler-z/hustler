/**
 * Hustler's Project
 *
 * File:  linker.h
 * Date:  2024/05/20
 * Usage: linker list
 */

#ifndef _ASM_LINKER_H
#define _ASM_LINKER_H
// --------------------------------------------------------------
#define entry_sym(_type, _name, _list)                       \
    ((_type *)&_hypos_list_2_##_list##_2_##_name)

#define entry_start(_type) ({								 \
    static char start[0] __aligned(4) __attribute__((unused))\
        __section("__hypos_list_1");				         \
    _type * tmp = (_type *)&start;					         \
    asm("":"+r"(tmp));						                 \
    tmp;								                     \
})

#define entry_end(_type) ({									 \
    static char end[0] __aligned(4) __attribute__((unused))	 \
        __section("__hypos_list_3");				         \
    _type * tmp = (_type *)&end;					         \
    asm("":"+r"(tmp));						                 \
    tmp;								                     \
})

#define _entry_start(_type, _list) ({					     \
    static char start[0] __aligned(8)                        \
        __attribute__((unused))					             \
        __section("__hypos_list_2_"#_list"_1");			     \
    _type * tmp = (_type *)&start;					         \
    asm("":"+r"(tmp));						                 \
    tmp;								                     \
})

#define _entry_end(_type, _list) ({					         \
    static char end[0] __aligned(4) __attribute__((unused))	 \
        __section("__hypos_list_2_"#_list"_3");			     \
    _type * tmp = (_type *)&end;					         \
    asm("":"+r"(tmp));						                 \
    tmp;								                     \
})

#define _entry_count(_type, _list) ({			             \
    _type *start = _entry_start(_type, _list);               \
    _type *end = _entry_end(_type, _list);	                 \
    unsigned int __offset = end - start;			         \
    __offset;						                         \
})

#define _entry_declare(_type, _name, _list)                  \
    _type _hypos_list_2_##_list##_2_##_name __aligned(4)     \
            __attribute__((unused))                          \
            __section("__hypos_list_2_"#_list"_2_"#_name)

#define _entry_declare_list(_type, _name, _list)             \
    _type _hypos_list_2_##_list##_2_##_name[] __aligned(4)   \
            __attribute__((unused))                          \
            __section("__hypos_list_2_"#_list"_2_"#_name)

#define _entry_acquire(_type, _name, _list) ({               \
    extern _type _hypos_list_2_##_list##_2_##_name;          \
    _type *__entry =                                         \
        &_hypos_list_2_##_list##_2_##_name;                  \
    __entry;                                                 \
})

#define _entry_reference(_type, _name, _list)                \
    ((_type *)&_hypos_list_2_##_list##_2_##_name)
// --------------------------------------------------------------
#endif /* _ASM_LINKER_H */
