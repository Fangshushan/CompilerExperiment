#ifndef _VM_VM_H
#define _VM_VM_H
#include "common.h"
// 这么设计是为了设计多线程
struct vm{
    uint32_t allocatedBytes;//累积分配的内存
    Parser* curParser; // 当前词法分析器
};

void initVM(VM* vm);
VM* newVM(void);

#endif
