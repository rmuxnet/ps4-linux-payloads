/*
 * main-aio.c — All-In-One Linux kexec payload
 *
 * A single binary that detects the running PS4 firmware at boot time,
 * selects the matching kexec blob and kernel offset table, then proceeds
 * identically to the per-firmware main.c / main-baikal.c payloads.
 *
 * Build variants (controlled by preprocessor):
 *   default   → embeds the normal    ps4-kexec-<fw>/kexec.bin blobs
 *   -DAIO_BAIKAL → embeds the baikal ps4-kexec-<fw>-baikal/kexec.bin blobs
 *
 * Example Makefile targets produced by the Makefile additions below:
 *   fw_aio/payload-aio-256mb-normal.bin
 *   fw_aio/payload-aio-256mb-baikal.bin
 *
 * Supported firmware: 505 672 700 750 900 903 960
 *                     1000 1050 1100 1102 1150 1200 1250 1300 1302
 */

#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/thr.h>
#include <time.h>
#include <stdio.h>

#include "fw_detect.h"   /* get_firmware()          */
#include "fw_offsets.h"  /* FwOffsets, find_offsets_by_fw() */

/* ─────────────────────────────────────────────────────────────────────────
 * Embed all kexec blobs for the selected variant.
 *
 * Each blob lands in .text as a labeled byte sequence.  The matching
 * extern char[] declarations give us the start/end pointers at runtime.
 * The blobs are selected by the `kexec_blob_table` below.
 * ───────────────────────────────────────────────────────────────────────── */

#ifdef AIO_BAIKAL
asm(
    "kexec_blob_505_start:\n"  ".incbin \"ps4-kexec-505-baikal/kexec.bin\"\n"  "kexec_blob_505_end:\n"
    "kexec_blob_672_start:\n"  ".incbin \"ps4-kexec-672-baikal/kexec.bin\"\n"  "kexec_blob_672_end:\n"
    "kexec_blob_700_start:\n"  ".incbin \"ps4-kexec-700-baikal/kexec.bin\"\n"  "kexec_blob_700_end:\n"
    "kexec_blob_750_start:\n"  ".incbin \"ps4-kexec-750-baikal/kexec.bin\"\n"  "kexec_blob_750_end:\n"
    "kexec_blob_900_start:\n"  ".incbin \"ps4-kexec-900-baikal/kexec.bin\"\n"  "kexec_blob_900_end:\n"
    "kexec_blob_903_start:\n"  ".incbin \"ps4-kexec-903-baikal/kexec.bin\"\n"  "kexec_blob_903_end:\n"
    "kexec_blob_960_start:\n"  ".incbin \"ps4-kexec-960-baikal/kexec.bin\"\n"  "kexec_blob_960_end:\n"
    "kexec_blob_1000_start:\n" ".incbin \"ps4-kexec-1000-baikal/kexec.bin\"\n" "kexec_blob_1000_end:\n"
    "kexec_blob_1050_start:\n" ".incbin \"ps4-kexec-1050-baikal/kexec.bin\"\n" "kexec_blob_1050_end:\n"
    "kexec_blob_1100_start:\n" ".incbin \"ps4-kexec-1100-baikal/kexec.bin\"\n" "kexec_blob_1100_end:\n"
    "kexec_blob_1102_start:\n" ".incbin \"ps4-kexec-1102-baikal/kexec.bin\"\n" "kexec_blob_1102_end:\n"
    "kexec_blob_1150_start:\n" ".incbin \"ps4-kexec-1150-baikal/kexec.bin\"\n" "kexec_blob_1150_end:\n"
    "kexec_blob_1200_start:\n" ".incbin \"ps4-kexec-1200-baikal/kexec.bin\"\n" "kexec_blob_1200_end:\n"
    "kexec_blob_1250_start:\n" ".incbin \"ps4-kexec-1250-baikal/kexec.bin\"\n" "kexec_blob_1250_end:\n"
    "kexec_blob_1300_start:\n" ".incbin \"ps4-kexec-1300-baikal/kexec.bin\"\n" "kexec_blob_1300_end:\n"
    "kexec_blob_1302_start:\n" ".incbin \"ps4-kexec-1302-baikal/kexec.bin\"\n" "kexec_blob_1302_end:\n"
);
#else
asm(
    "kexec_blob_505_start:\n"  ".incbin \"ps4-kexec-505/kexec.bin\"\n"  "kexec_blob_505_end:\n"
    "kexec_blob_672_start:\n"  ".incbin \"ps4-kexec-672/kexec.bin\"\n"  "kexec_blob_672_end:\n"
    "kexec_blob_700_start:\n"  ".incbin \"ps4-kexec-700/kexec.bin\"\n"  "kexec_blob_700_end:\n"
    "kexec_blob_750_start:\n"  ".incbin \"ps4-kexec-750/kexec.bin\"\n"  "kexec_blob_750_end:\n"
    "kexec_blob_900_start:\n"  ".incbin \"ps4-kexec-900/kexec.bin\"\n"  "kexec_blob_900_end:\n"
    "kexec_blob_903_start:\n"  ".incbin \"ps4-kexec-903/kexec.bin\"\n"  "kexec_blob_903_end:\n"
    "kexec_blob_960_start:\n"  ".incbin \"ps4-kexec-960/kexec.bin\"\n"  "kexec_blob_960_end:\n"
    "kexec_blob_1000_start:\n" ".incbin \"ps4-kexec-1000/kexec.bin\"\n" "kexec_blob_1000_end:\n"
    "kexec_blob_1050_start:\n" ".incbin \"ps4-kexec-1050/kexec.bin\"\n" "kexec_blob_1050_end:\n"
    "kexec_blob_1100_start:\n" ".incbin \"ps4-kexec-1100/kexec.bin\"\n" "kexec_blob_1100_end:\n"
    "kexec_blob_1102_start:\n" ".incbin \"ps4-kexec-1102/kexec.bin\"\n" "kexec_blob_1102_end:\n"
    "kexec_blob_1150_start:\n" ".incbin \"ps4-kexec-1150/kexec.bin\"\n" "kexec_blob_1150_end:\n"
    "kexec_blob_1200_start:\n" ".incbin \"ps4-kexec-1200/kexec.bin\"\n" "kexec_blob_1200_end:\n"
    "kexec_blob_1250_start:\n" ".incbin \"ps4-kexec-1250/kexec.bin\"\n" "kexec_blob_1250_end:\n"
    "kexec_blob_1300_start:\n" ".incbin \"ps4-kexec-1300/kexec.bin\"\n" "kexec_blob_1300_end:\n"
    "kexec_blob_1302_start:\n" ".incbin \"ps4-kexec-1302/kexec.bin\"\n" "kexec_blob_1302_end:\n"
);
#endif

/* Declare all blob boundary symbols so C can reference them. */
extern char kexec_blob_505_start[],  kexec_blob_505_end[];
extern char kexec_blob_672_start[],  kexec_blob_672_end[];
extern char kexec_blob_700_start[],  kexec_blob_700_end[];
extern char kexec_blob_750_start[],  kexec_blob_750_end[];
extern char kexec_blob_900_start[],  kexec_blob_900_end[];
extern char kexec_blob_903_start[],  kexec_blob_903_end[];
extern char kexec_blob_960_start[],  kexec_blob_960_end[];
extern char kexec_blob_1000_start[], kexec_blob_1000_end[];
extern char kexec_blob_1050_start[], kexec_blob_1050_end[];
extern char kexec_blob_1100_start[], kexec_blob_1100_end[];
extern char kexec_blob_1102_start[], kexec_blob_1102_end[];
extern char kexec_blob_1150_start[], kexec_blob_1150_end[];
extern char kexec_blob_1200_start[], kexec_blob_1200_end[];
extern char kexec_blob_1250_start[], kexec_blob_1250_end[];
extern char kexec_blob_1300_start[], kexec_blob_1300_end[];
extern char kexec_blob_1302_start[], kexec_blob_1302_end[];

/* Blob dispatch table — parallel to fw_table in fw_offsets.c. */
typedef struct {
    u16   fw;
    char *start;
    char *end;
} KexecBlobEntry;

static const KexecBlobEntry kexec_blob_table[] = {
    {  505, kexec_blob_505_start,  kexec_blob_505_end  },
    {  672, kexec_blob_672_start,  kexec_blob_672_end  },
    {  700, kexec_blob_700_start,  kexec_blob_700_end  },
    {  750, kexec_blob_750_start,  kexec_blob_750_end  },
    {  900, kexec_blob_900_start,  kexec_blob_900_end  },
    {  903, kexec_blob_903_start,  kexec_blob_903_end  },
    {  960, kexec_blob_960_start,  kexec_blob_960_end  },
    { 1000, kexec_blob_1000_start, kexec_blob_1000_end },
    { 1050, kexec_blob_1050_start, kexec_blob_1050_end },
    { 1100, kexec_blob_1100_start, kexec_blob_1100_end },
    { 1102, kexec_blob_1102_start, kexec_blob_1102_end },
    { 1150, kexec_blob_1150_start, kexec_blob_1150_end },
    { 1200, kexec_blob_1200_start, kexec_blob_1200_end },
    { 1250, kexec_blob_1250_start, kexec_blob_1250_end },
    { 1300, kexec_blob_1300_start, kexec_blob_1300_end },
    { 1302, kexec_blob_1302_start, kexec_blob_1302_end },
    {    0, 0, 0 } /* sentinel */
};

static const KexecBlobEntry *find_blob_by_fw(u16 fw_ver)
{
    for (int i = 0; kexec_blob_table[i].fw != 0; i++)
        if (kexec_blob_table[i].fw == fw_ver)
            return &kexec_blob_table[i];
    return (const KexecBlobEntry *)0; /* NULL */
}

/* ─────────────────────────────────────────────────────────────────────────
 * Runtime-selected globals — written by main() before kexec() is called,
 * read by kernel_main() after the exploit runs.
 * These live in the payload data segment which remains accessible from
 * kernel context (the exploit grants kernel RW to userland mappings).
 * ───────────────────────────────────────────────────────────────────────── */
static const FwOffsets      *g_offsets    = (const FwOffsets *)0;
static const char           *g_kexec_start = (const char *)0;
static const char           *g_kexec_end   = (const char *)0;

/* ─────────────────────────────────────────────────────────────────────────
 * Kernel-mode stage — runs after kexec() delivers us into ring-0.
 * Must not call any userspace functions (printf, malloc, …).
 * All kernel addresses are computed from g_offsets.
 * ───────────────────────────────────────────────────────────────────────── */
void kexec(void *f, void *u);

static unsigned long long get_syscall(void)
{
    unsigned int eax, ecx, edx;
    ecx = 0xc0000082;
    asm volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
    return ((unsigned long long)edx) << 32 | eax;
}

void kernel_main(void)
{
    unsigned long long kernel_base =
        get_syscall() - g_offsets->xfast_syscall;

    /* Disable write-protect so we can patch kernel memory. */
    asm volatile("cli\n"
                 "mov %%cr0, %%rax\n"
                 "btc $16, %%rax\n"
                 "mov %%rax, %%cr0\n"
                 ::: "rax");

    *(char *)(kernel_base + g_offsets->patch_kmem_alloc_1) = 0x07;
    *(char *)(kernel_base + g_offsets->patch_kmem_alloc_2) = 0x07;

    /* Set pstate before shutdown (needed for PS4 Pro). */
    *(char *)(kernel_base + g_offsets->pstate_before_shutdown) = 0x03;

    /* Re-enable write-protect. */
    asm volatile("mov %%cr0, %%rax\n"
                 "bts $16, %%rax\n"
                 "mov %%rax, %%cr0\n"
                 "sti\n"
                 ::: "rax");

    unsigned long long early_printf = kernel_base + g_offsets->printf_off;
    unsigned long long kmem_alloc   = kernel_base + g_offsets->kmem_alloc;
    unsigned long long kernel_map   = kernel_base + g_offsets->kernel_map;

    /* Allocate kernel memory, copy the kexec blob, then jump into it. */
    char *new_kexec = ((char *(*)(unsigned long long, unsigned long long))
                       kmem_alloc)(*(unsigned long long *)kernel_map,
                                   g_kexec_end - g_kexec_start);

    for (int i = 0; g_kexec_start + i != g_kexec_end; i++)
        new_kexec[i] = g_kexec_start[i];

    ((void (*)(void *, void *))new_kexec)((void *)early_printf, (void *)0);
}

/* ─────────────────────────────────────────────────────────────────────────
 * Syscall wrappers and utilities (identical to main.c / main-baikal.c)
 * ───────────────────────────────────────────────────────────────────────── */
asm("kexec_load:\n"
    "mov %rcx, %r10\n"
    "mov $153, %rax\n"
    "syscall\n"
    "ret\n");

int kexec_load(char *kernel, unsigned long long kernel_size,
               char *initrd,  unsigned long long initrd_size,
               char *cmdline, int vram_mb);

static int read_file(const char *path, char **ptr, unsigned long long *sz)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;
    *sz  = lseek(fd, 0, SEEK_END);
    *ptr = mmap(NULL, *sz, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char            *p = *ptr;
    unsigned long long l = *sz;
    lseek(fd, 0, SEEK_SET);
    while (l) {
        unsigned long long chk = read(fd, p, l);
        if (chk <= 0)
            return -1;
        p += chk;
        l -= chk;
    }
    close(fd);
    return 0;
}

int  evf_open(char *);
void evf_cancel(int, unsigned long long, unsigned long long);
void evf_close(int);

static void reboot_thread(void *_)
{
    nanosleep((const struct timespec *)
              "\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", NULL);
    int evf = evf_open("SceSysCoreReboot");
    evf_cancel(evf, 0x4000, 0);
    evf_close(evf);
    kill(1, SIGUSR1);
}

void *dlopen(const char *, int);
void *dlsym(void *, const char *);

void alert(const char *msg)
{
    static int (*sceSysUtilSendSystemNotificationWithText)(int, const char *);
    if (!sceSysUtilSendSystemNotificationWithText) {
        void *handle = dlopen("/system/common/lib/libSceSysUtil.sprx", 0);
        sceSysUtilSendSystemNotificationWithText =
            dlsym(handle, "sceSysUtilSendSystemNotificationWithText");
    }
    sceSysUtilSendSystemNotificationWithText(222, msg);
}

static int my_atoi(const char *s)
{
    int ret = 0, neg = 0;
    while (*s == ' ') s++;
    neg = (*s == '-') ? 1 : 0;
    for (; *s; s++) {
        char c = *s;
        if ('0' <= c && c <= '9') { ret *= 10; ret += c - '0'; }
        else break;
    }
    return neg ? -ret : ret;
}

/* ── Debug helpers ── */
static void u16_to_dec(u16 val, char *buf)
{
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[8]; int i = 0;
    while (val) { tmp[i++] = '0' + (val % 10); val /= 10; }
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
}

static void itohex(u64 val, char *buf)
{
    const char *hex = "0123456789abcdef";
    buf[0] = '0'; buf[1] = 'x';
    int out = 2;
    /* Write all 16 nibbles (skip leading zeros but keep at least one). */
    int started = 0;
    for (int shift = 60; shift >= 0; shift -= 4) {
        int nibble = (int)((val >> shift) & 0xf);
        if (nibble || started || shift == 0) {
            buf[out++] = hex[nibble];
            started = 1;
        }
    }
    buf[out] = '\0';
}

/* ─────────────────────────────────────────────────────────────────────────
 * VRAM defaults — injected by Makefile, with safe fallbacks.
 * ───────────────────────────────────────────────────────────────────────── */
#ifndef VRAM_MB_DEFAULT
#define VRAM_MB_DEFAULT 256
#endif
#ifndef VRAM_MB_MIN
#define VRAM_MB_MIN 32
#endif
#ifndef VRAM_MB_MAX
#define VRAM_MB_MAX 4096
#endif
#ifndef HDD_BOOT_PATH
#define HDD_BOOT_PATH "/data/linux/boot/"
#endif

/* ─────────────────────────────────────────────────────────────────────────
 * Entry point
 * ───────────────────────────────────────────────────────────────────────── */
int main(void)
{
    struct sigaction sa = {
        .sa_handler = SIG_IGN,
        .sa_flags   = 0,
    };
    sigaction(SIGSTOP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);

    /* ── Step 1: detect firmware ─────────────────────────────────────── */
    u16 fw_ver = get_firmware();

    /* Normalization: map minor/incremental versions to the supported base entries. */
    if (fw_ver == 506 || fw_ver == 507) fw_ver = 505;
    if (fw_ver == 701 || fw_ver == 702) fw_ver = 700;
    if (fw_ver >= 750 && fw_ver <= 755) fw_ver = 750;
    if (fw_ver == 800) fw_ver = 750; /* Some use 7.50 offsets? */
    if (fw_ver == 904) fw_ver = 903;
    if (fw_ver == 1001) fw_ver = 1000;
    if (fw_ver == 1070 || fw_ver == 1071) fw_ver = 1050;
    if (fw_ver == 1151 || fw_ver == 1152) fw_ver = 1150;
    if (fw_ver == 1201 || fw_ver == 1202) fw_ver = 1200;
    if (fw_ver == 1251 || fw_ver == 1252) fw_ver = 1250;

#ifdef AIO_DEBUG
    {
        char decbuf[8];
        u16_to_dec(fw_ver, decbuf);
        alert(decbuf); /* e.g. "700" */
    }
#endif

    if (!fw_ver) {
        alert("AIO: failed to detect firmware version.");
        return 1;
    }

    /* ── Step 2: look up offsets ─────────────────────────────────────── */
    g_offsets = find_offsets_by_fw(fw_ver);

#ifdef AIO_DEBUG
    if (!g_offsets) {
        char decbuf[8];
        u16_to_dec(fw_ver, decbuf);
        alert("AIO: find_offsets_by_fw returned NULL for:");
        alert(decbuf);
        return 1;
    }
    alert("AIO: offsets found!");

    /* Dump one offset as a sanity check. */
    {
        char addr_str[20];
        char final_msg[128];
        itohex(g_offsets->copyinstr, addr_str);
        /* manual strcat — no libc */
        int i = 0;
        const char *prefix = "copyinstr offset: ";
        while (prefix[i]) { final_msg[i] = prefix[i]; i++; }
        int j = 0;
        while (addr_str[j]) final_msg[i++] = addr_str[j++];
        final_msg[i] = '\0';
        alert(final_msg);
    }
#else
    if (!g_offsets) {
        alert("AIO: unsupported firmware.");
        return 1;
    }
#endif

    /* ── Step 3: look up the matching kexec blob ─────────────────────── */
    const KexecBlobEntry *blob = find_blob_by_fw(fw_ver);
    if (!blob) {
        alert("AIO: no kexec blob for this firmware.");
        return 1;
    }
    g_kexec_start = blob->start;
    g_kexec_end   = blob->end;

    /* ── Step 4: load kernel, initrd, cmdline (same as main.c) ──────── */
    char            *kernel      = (char *)0;
    unsigned long long kernel_size = 0;
    char            *initrd      = (char *)0;
    unsigned long long initrd_size = 0;
    char            *cmdline     = (char *)0;
    unsigned long long cmdline_size = 0;
    char            *vramstr     = (char *)0;
    unsigned long long vramstr_size = 0;
    int              vram_mb     = 0;

#define L(name, where, wheresz, is_fatal)                                    \
    if (   read_file("/mnt/usb0/" name, where, wheresz)                      \
        && read_file("/mnt/usb1/" name, where, wheresz)                      \
        && read_file(HDD_BOOT_PATH name, where, wheresz)                     \
        && is_fatal)                                                          \
    {                                                                         \
        alert("Failed to load file: " name ".\n"                             \
              "Paths checked:\n"                                              \
              "/mnt/usb0/" name "\n"                                          \
              "/mnt/usb1/" name "\n"                                          \
              HDD_BOOT_PATH name);                                            \
        return 1;                                                             \
    }

    L("bzImage",            &kernel,  &kernel_size,  1);
    L("initramfs.cpio.gz",  &initrd,  &initrd_size,  1);
    L("bootargs.txt",       &cmdline, &cmdline_size, 0);

    if (cmdline && cmdline_size) {
        for (int i = 0; i < (int)cmdline_size; i++)
            if (cmdline[i] == '\n') { cmdline[i] = '\0'; break; }
    } else {
        cmdline = "panic=0 clocksource=tsc consoleblank=0 net.ifnames=0 "
                  "radeon.dpm=0 amdgpu.dpm=0 drm.debug=0 "
                  "console=ttyS0,115200n8 console=tty0 "
                  "video=HDMI-A-1:1920x1080@60";
    }

    L("vram.txt", &vramstr, &vramstr_size, 0);
    if (vramstr && vramstr_size) {
        vram_mb = my_atoi(vramstr);
        if (vram_mb < VRAM_MB_MIN || vram_mb > VRAM_MB_MAX)
            vram_mb = VRAM_MB_DEFAULT;
    } else {
        vram_mb = VRAM_MB_DEFAULT;
    }

    /* ── Step 5: Generate info.txt dynamically ── */
    {
        int info_fd = open("info.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (info_fd >= 0) {
            char info_buf[256];
            int info_len = snprintf(info_buf, sizeof(info_buf),
                "PS4 Linux Payload Info\n"
                "========================\n"
                "VRAM Applied: %d MB\n"
                "Variant: %s\n"
                "Detected FW: %d\n",
                vram_mb,
#ifdef AIO_BAIKAL
                "AIO-Baikal",
#else
                "AIO-Normal",
#endif
                fw_ver);
            write(info_fd, info_buf, info_len);
            close(info_fd);
        }
    }


    /* ── Step 6: exploit → kernel_main → kexec blob ─────────────────── */
    kexec(kernel_main, (void *)0);

    long x, y;
    struct thr_param thr = {
        .start_func = reboot_thread,
        .arg        = NULL,
        .stack_base = mmap(NULL, 16384, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0),
        .stack_size = 16384,
        .tls_base   = NULL,
        .tls_size   = 0,
        .child_tid  = &x,
        .parent_tid = &y,
        .flags      = 0,
        .rtp        = NULL,
    };
    thr_new(&thr, sizeof(thr));
    kexec_load(kernel, kernel_size, initrd, initrd_size, cmdline, vram_mb);

    for (;;);
}
