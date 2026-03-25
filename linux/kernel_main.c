#include <sys/types.h>
#include <stddef.h>
#include <ps4-offsets/kernel.h>
#include "magic.h"

#define STR(x) #x
#define XSTR(x) STR(x)
#define CONCAT(a, b) a ## _ ## b
#define FNAME(name, fw) CONCAT(name, fw)

extern char FNAME(ps4kexec, FW_SUFFIX)[];
extern char FNAME(ps4kexec_end, FW_SUFFIX)[];

asm(".global " XSTR(FNAME(ps4kexec, FW_SUFFIX)) "\n" XSTR(FNAME(ps4kexec, FW_SUFFIX)) ":\n.incbin \"" XSTR(KEXEC_PATH) "\"\n"
    ".global " XSTR(FNAME(ps4kexec_end, FW_SUFFIX)) "\n" XSTR(FNAME(ps4kexec_end, FW_SUFFIX)) ":\n");

static inline unsigned long long get_syscall(void)
{
    unsigned int eax, ecx, edx;
    ecx = 0xc0000082;
    asm volatile("rdmsr":"=a"(eax),"=d"(edx):"c"(ecx));
    return ((unsigned long long)edx) << 32 | eax;
}

void FNAME(kernel_main, FW_SUFFIX)()
{
    unsigned long long kernel_base = get_syscall() - kernel_offset_xfast_syscall;
    asm volatile("cli\nmov %%cr0, %%rax\nbtc $16, %%rax\nmov %%rax, %%cr0":::"rax");
    *(char*)(kernel_base + kernel_patch_kmem_alloc_1) = 0x07;
    *(char*)(kernel_base + kernel_patch_kmem_alloc_2) = 0x07;
    *(char*)(kernel_base + kern_off_pstate_before_shutdown) = 0x03;
    asm volatile("mov %%cr0, %%rax\nbts $16, %%rax\nmov %%rax, %%cr0\nsti":::"rax");

    unsigned long long early_printf = kernel_base + kernel_offset_printf;
    unsigned long long kmem_alloc = kernel_base + kernel_offset_kmem_alloc;
    unsigned long long kernel_map = kernel_base + kernel_offset_kernel_map;
    
    char* ps4kexec = FNAME(ps4kexec, FW_SUFFIX);
    char* ps4kexec_end = FNAME(ps4kexec_end, FW_SUFFIX);

    char* new_ps4_kexec = ((char*(*)(unsigned long long, unsigned long long))kmem_alloc)(*(unsigned long long*)kernel_map, ps4kexec_end-ps4kexec);
    for(int i = 0; ps4kexec + i != ps4kexec_end; i++)
        new_ps4_kexec[i] = ps4kexec[i];
    ((void(*)(void*, void*))new_ps4_kexec)((void*)early_printf, NULL);
}
