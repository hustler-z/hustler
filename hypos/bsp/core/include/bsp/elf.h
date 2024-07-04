/**
 * Hustler's Project
 *
 * File:  elf.h
 * Date:  2024/05/21
 * Usage:
 */

#ifndef _BSP_ELF_H
#define _BSP_ELF_H
// --------------------------------------------------------------

/* Symbol table index */
#define STN_UNDEF           0       /* undefined */

/* Extract symbol info - st_info */
#define ELF32_ST_BIND(x)    ((x) >> 4)
#define ELF32_ST_TYPE(x)    (((unsigned int)(x)) & 0xf)
#define ELF32_ST_INFO(b,t)  (((b) << 4) + ((t) & 0xf))

#define ELF64_ST_BIND(x)    ((x) >> 4)
#define ELF64_ST_TYPE(x)    (((unsigned int)(x)) & 0xf)
#define ELF64_ST_INFO(b,t)  (((b) << 4) + ((t) & 0xf))

/* Symbol Binding - ELF32_ST_BIND - st_info */
#define STB_LOCAL           0       /* Local symbol */
#define STB_GLOBAL          1       /* Global symbol */
#define STB_WEAK            2       /* like global - lower precedence */
#define STB_NUM             3       /* number of symbol bindings */
#define STB_LOPROC          13      /* reserved range for processor */
#define STB_HIPROC          15      /*  specific symbol bindings */

/* Symbol type - ELF32_ST_TYPE - st_info */
#define STT_NOTYPE          0       /* not specified */
#define STT_OBJECT          1       /* data object */
#define STT_FUNC            2       /* function */
#define STT_SECTION         3       /* section */
#define STT_FILE            4       /* file */
#define STT_NUM             5       /* number of symbol types */
#define STT_LOPROC          13      /* reserved range for processor */
#define STT_HIPROC          15      /*  specific symbol types */

// --------------------------------------------------------------
#endif /* _BSP_ELF_H */
