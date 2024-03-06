# Stack unwind

## 1. 概述

    Stack unwind（栈回溯）技术是一种在软件开发中广泛使用的机制，旨在优化和管理程序的异常处理和调用栈展开。该技术的核心是提供一套有效的方法来追踪和还原程序在执行时的调用栈状态，特别是在异常或错误发生时。

## 2. 功能简介

.
├── funwind.c
└── funwind.h

本驱动为开发者提供了特性：

1. 提供异常错误时栈回溯功能
2. 提供业务代码中自定义栈追溯功能

## 3. 错误码定义

```
#define FUNWIND_SUCCESS                FT_SUCCESS
#define FUNWIND_IDX_NOT_FOUND          FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 1)
#define FUNWIND_IDX_CANTUNWIND         FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 2)
#define FUNWIND_INSN_ERROR             FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 3)
#define FUNWIND_FAILURE                FT_MAKE_ERRCODE(ErrModBsp, ErrCommGeneral, 4)
```


## 4. 应用示例

example\system\unwind

## 5. API参考


### 1. FUnwindBacktrace

- 该函数是一个用于执行堆栈回溯（Unwind Backtrace）的函数，通常用于调试目的，以确定在异常或其他关键事件发生时程序的调用栈状态。

```c
_WEAK void FUnwindBacktrace(void * regs);
```

    Note:
        当前提供的版本仅适用于裸跑工作环境，若要基于其他rtos 应用场景使用，请进行重构覆盖。

    Input:
    - regs，如果需要自定义栈追溯起点，请按照struct FUnwindStackFrame * 数据结构进行传入。

    Return:
    - void
