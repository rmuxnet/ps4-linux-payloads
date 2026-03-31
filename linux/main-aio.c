/*
 * main-aio.c — All-In-One PS4 Linux payload
 *
 * Supports all 18 firmware variants:
 *   505, 672, 700, 750, 800, 850, 900, 903, 960,
 *   1000, 1050, 1100, 1102, 1150, 1200, 1250, 1300, 1302
 *
 * Compile-time flags:
 *   -DAIO_BAIKAL=1   embed PS4 Pro (Baikal) kexec blobs instead of standard
 *   -DVRAM_MB_DEFAULT=<n>   default VRAM size in MB (required)
 *   -DVRAM_MB_MIN=<n>       minimum VRAM size in MB
 *   -DVRAM_MB_MAX=<n>       maximum VRAM size in MB
 */

#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/thr.h>
#include <time.h>

#include "aio_types.h"
#include "fw_detect.h"

/* ═══════════════════════════════════════════════════════════════════
 *   Viską nxj kexec blobus įdedam kompiliavimo metu - nu nxj. neaišku kas čia nxj daros
 *     AIO_BAIKAL pasirenka PS4 Pro ar paprastus blobus, nxj.
 * ═══════════════════════════════════════════════════════════════════ */

#ifndef AIO_BAIKAL
#define AIO_BAIKAL 0
#endif

static unsigned long long str_len(const char *s)
{
    unsigned long long len = 0;

    while (s && s[len])
        len++;

    return len;
}

static void log_msg(const char *msg)
{
    unsigned long long len = str_len(msg);

    if (len)
        write(1, msg, len);
    if (!len || msg[len - 1] != '\n')
        write(1, "\n", 1);
}

#if AIO_BAIKAL
asm("kexec_505:\n.incbin \"kexec-build/baikal/505/kexec.bin\"\nkexec_505_end:\n");
asm("kexec_672:\n.incbin \"kexec-build/baikal/672/kexec.bin\"\nkexec_672_end:\n");
asm("kexec_700:\n.incbin \"kexec-build/baikal/700/kexec.bin\"\nkexec_700_end:\n");
asm("kexec_750:\n.incbin \"kexec-build/baikal/750/kexec.bin\"\nkexec_750_end:\n");
asm("kexec_800:\n.incbin \"kexec-build/baikal/800/kexec.bin\"\nkexec_800_end:\n");
asm("kexec_850:\n.incbin \"kexec-build/baikal/850/kexec.bin\"\nkexec_850_end:\n");
asm("kexec_900:\n.incbin \"kexec-build/baikal/900/kexec.bin\"\nkexec_900_end:\n");
asm("kexec_903:\n.incbin \"kexec-build/baikal/903/kexec.bin\"\nkexec_903_end:\n");
asm("kexec_960:\n.incbin \"kexec-build/baikal/960/kexec.bin\"\nkexec_960_end:\n");
asm("kexec_1000:\n.incbin \"kexec-build/baikal/1000/kexec.bin\"\nkexec_1000_end:\n");
asm("kexec_1050:\n.incbin \"kexec-build/baikal/1050/kexec.bin\"\nkexec_1050_end:\n");
asm("kexec_1100:\n.incbin \"kexec-build/baikal/1100/kexec.bin\"\nkexec_1100_end:\n");
asm("kexec_1102:\n.incbin \"kexec-build/baikal/1102/kexec.bin\"\nkexec_1102_end:\n");
asm("kexec_1150:\n.incbin \"kexec-build/baikal/1150/kexec.bin\"\nkexec_1150_end:\n");
asm("kexec_1200:\n.incbin \"kexec-build/baikal/1200/kexec.bin\"\nkexec_1200_end:\n");
asm("kexec_1250:\n.incbin \"kexec-build/baikal/1250/kexec.bin\"\nkexec_1250_end:\n");
asm("kexec_1300:\n.incbin \"kexec-build/baikal/1300/kexec.bin\"\nkexec_1300_end:\n");
asm("kexec_1302:\n.incbin \"kexec-build/baikal/1302/kexec.bin\"\nkexec_1302_end:\n");
#else
asm("kexec_505:\n.incbin \"kexec-build/normal/505/kexec.bin\"\nkexec_505_end:\n");
asm("kexec_672:\n.incbin \"kexec-build/normal/672/kexec.bin\"\nkexec_672_end:\n");
asm("kexec_700:\n.incbin \"kexec-build/normal/700/kexec.bin\"\nkexec_700_end:\n");
asm("kexec_750:\n.incbin \"kexec-build/normal/750/kexec.bin\"\nkexec_750_end:\n");
asm("kexec_800:\n.incbin \"kexec-build/normal/800/kexec.bin\"\nkexec_800_end:\n");
asm("kexec_850:\n.incbin \"kexec-build/normal/850/kexec.bin\"\nkexec_850_end:\n");
asm("kexec_900:\n.incbin \"kexec-build/normal/900/kexec.bin\"\nkexec_900_end:\n");
asm("kexec_903:\n.incbin \"kexec-build/normal/903/kexec.bin\"\nkexec_903_end:\n");
asm("kexec_960:\n.incbin \"kexec-build/normal/960/kexec.bin\"\nkexec_960_end:\n");
asm("kexec_1000:\n.incbin \"kexec-build/normal/1000/kexec.bin\"\nkexec_1000_end:\n");
asm("kexec_1050:\n.incbin \"kexec-build/normal/1050/kexec.bin\"\nkexec_1050_end:\n");
asm("kexec_1100:\n.incbin \"kexec-build/normal/1100/kexec.bin\"\nkexec_1100_end:\n");
asm("kexec_1102:\n.incbin \"kexec-build/normal/1102/kexec.bin\"\nkexec_1102_end:\n");
asm("kexec_1150:\n.incbin \"kexec-build/normal/1150/kexec.bin\"\nkexec_1150_end:\n");
asm("kexec_1200:\n.incbin \"kexec-build/normal/1200/kexec.bin\"\nkexec_1200_end:\n");
asm("kexec_1250:\n.incbin \"kexec-build/normal/1250/kexec.bin\"\nkexec_1250_end:\n");
asm("kexec_1300:\n.incbin \"kexec-build/normal/1300/kexec.bin\"\nkexec_1300_end:\n");
asm("kexec_1302:\n.incbin \"kexec-build/normal/1302/kexec.bin\"\nkexec_1302_end:\n");
#endif

/* Forward deklaracijos visiems blob simboliams, nxj */
extern char kexec_505[], kexec_505_end[];
extern char kexec_672[], kexec_672_end[];
extern char kexec_700[], kexec_700_end[];
extern char kexec_750[], kexec_750_end[];
extern char kexec_800[], kexec_800_end[];
extern char kexec_850[], kexec_850_end[];
extern char kexec_900[], kexec_900_end[];
extern char kexec_903[], kexec_903_end[];
extern char kexec_960[], kexec_960_end[];
extern char kexec_1000[], kexec_1000_end[];
extern char kexec_1050[], kexec_1050_end[];
extern char kexec_1100[], kexec_1100_end[];
extern char kexec_1102[], kexec_1102_end[];
extern char kexec_1150[], kexec_1150_end[];
extern char kexec_1200[], kexec_1200_end[];
extern char kexec_1250[], kexec_1250_end[];
extern char kexec_1300[], kexec_1300_end[];
extern char kexec_1302[], kexec_1302_end[];

/* ═══════════════════════════════════════════════════════════════════
 * §2  Runtime firmware offsetų nxj lentelė
 *     Visi offsetai patikrinti su freebsd-headers/ps4-offsets/*.h
 *     ir magic.h, nxj.
 * ═══════════════════════════════════════════════════════════════════ */

typedef struct {
    u16 fw_ver;           /* Normalised version (key) */
    u64 xfast_syscall;    /* kernel_offset_xfast_syscall */
    u64 printf_off;       /* kernel_offset_printf        */
    u64 kmem_alloc;       /* kernel_offset_kmem_alloc    */
    u64 kernel_map;       /* kernel_offset_kernel_map    */
    u64 patch1;           /* kernel_patch_kmem_alloc_1   */
    u64 patch2;           /* kernel_patch_kmem_alloc_2   */
    u64 pstate;           /* kern_off_pstate_before_shutdown */
} fw_offsets_t;

static fw_offsets_t fw_table[] = {
    /*  ver    xfast  printf       kmem_alloc  kernel_map   patch1       patch2       pstate     */
    {  505, 0x1c0, 0x436040,   0x0FCC80,   0x1AC60E0,  0x0FCD48,   0x0FCD56,   0x10D97E  },
    {  672, 0x1c0, 0x123280,   0x250730,   0x220DFC0,  0x2507F5,   0x250803,   0x20734E  },
    {  700, 0x1c0, 0x0BC730,   0x1170F0,   0x21C8EE0,  0x1171BE,   0x1171C6,   0x2CDD6E  },
    {  750, 0x1c0, 0x26F740,   0x1753E0,   0x21405B8,  0x1754AC,   0x1754B4,   0x0D2ED0  },
    {  800, 0x1c0, 0x430AE0,   0x01B3F0,   0x1B243E0,  0x01B4BC,   0x01B4C4,   0x155B50  },
    {  850, 0x1c0, 0x15D570,   0x2199A0,   0x1C64228,  0x219A6C,   0x219A74,   0x40BA10  },
    {  900, 0x1c0, 0x0B7A30,   0x37BE70,   0x2268D48,  0x37BF3C,   0x37BF44,   0x29A970  },
    {  903, 0x1c0, 0x0B79E0,   0x37A070,   0x2264D48,  0x37A13C,   0x37A144,   0x29A5F0  },
    {  960, 0x1c0, 0x205470,   0x1889D0,   0x2147830,  0x188A9C,   0x188AA4,   0x3323E0  },
    { 1000, 0x1c0, 0x0C50F0,   0x33B040,   0x227BEF8,  0x33B10C,   0x33B114,   0x4812D0  },
    { 1050, 0x1c0, 0x450E80,   0x36E120,   0x21A9250,  0x428A2C,   0x428A33,   0x45DCC0  },
    { 1100, 0x1c0, 0x2FCBD0,   0x245E10,   0x21FF130,  0x245EDC,   0x245EE4,   0x198650  },
    { 1102, 0x1c0, 0x2FCBF0,   0x245E30,   0x21FF130,  0x245EFC,   0x245F04,   0x198670  },
    { 1150, 0x1c0, 0x2E01A0,   0x4657A0,   0x22D1D50,  0x46586C,   0x465874,   0x3A2120  },
    { 1200, 0x1c0, 0x2E03E0,   0x4659E0,   0x22D1D50,  0x465AAC,   0x465AB4,   0x3A2360  },
    { 1250, 0x1c0, 0x2E0420,   0x465A20,   0x22D1D50,  0x465AEC,   0x465AF4,   0x3A23A0  },
    { 1300, 0x1c0, 0x2E0440,   0x465A40,   0x22D1D50,  0x465B0C,   0x465B14,   0x3A23C0  },
    { 1302, 0x1c0, 0x2E0450,   0x465A50,   0x22D1D50,  0x465B1C,   0x465B24,   0x3A23D0  },
    { 0 }   /* sentinel */
};

/* ═══════════════════════════════════════════════════════════════════
 * §3  Globalai kuriuos main() nxj nustato, kernel_main() naudoja
 *     kernel_main() veikia kernel kontekste bet vis dar gali skaityt
 *     payload globalus, nes payloado nxj fizinė atmintis lieka mapped.
 * ═══════════════════════════════════════════════════════════════════ */

static fw_offsets_t *g_fw       = (fw_offsets_t *)0;
static char         *g_kexec_s  = (char *)0;
static char         *g_kexec_e  = (char *)0;

/* ═══════════════════════════════════════════════════════════════════
 * §4  Pagalbinės nxj funkcijos
 * ═══════════════════════════════════════════════════════════════════ */

static fw_offsets_t *find_offsets_by_fw(u16 fw)
{
    u16 norm = normalize_fw_ver(fw);
    for (int i = 0; fw_table[i].fw_ver != 0; i++) {
        if (fw_table[i].fw_ver == norm)
            return &fw_table[i];
    }
    return (fw_offsets_t *)0;
}

/*
 * get_kexec_blob — grąžina pradžią/pabaigą įdėto kexec
 * binaro pagal normalizuotą firmware versiją. Switchas,
 * kad nebūtų nxj problemų su linkeriais ir adresais.
 */
static void get_kexec_blob(u16 norm_fw, char **start, char **end)
{
    switch (norm_fw) {
    case  505: *start = kexec_505;  *end = kexec_505_end;  break;
    case  672: *start = kexec_672;  *end = kexec_672_end;  break;
    case  700: *start = kexec_700;  *end = kexec_700_end;  break;
    case  750: *start = kexec_750;  *end = kexec_750_end;  break;
    case  800: *start = kexec_800;  *end = kexec_800_end;  break;
    case  850: *start = kexec_850;  *end = kexec_850_end;  break;
    case  900: *start = kexec_900;  *end = kexec_900_end;  break;
    case  903: *start = kexec_903;  *end = kexec_903_end;  break;
    case  960: *start = kexec_960;  *end = kexec_960_end;  break;
    case 1000: *start = kexec_1000; *end = kexec_1000_end; break;
    case 1050: *start = kexec_1050; *end = kexec_1050_end; break;
    case 1100: *start = kexec_1100; *end = kexec_1100_end; break;
    case 1102: *start = kexec_1102; *end = kexec_1102_end; break;
    case 1150: *start = kexec_1150; *end = kexec_1150_end; break;
    case 1200: *start = kexec_1200; *end = kexec_1200_end; break;
    case 1250: *start = kexec_1250; *end = kexec_1250_end; break;
    case 1300: *start = kexec_1300; *end = kexec_1300_end; break;
    case 1302: *start = kexec_1302; *end = kexec_1302_end; break;
    default:   *start = (char *)0;  *end = (char *)0;      break;
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * §5  Platformos nxj pagalbinės AIO paleidimo keliui
 * ═══════════════════════════════════════════════════════════════════ */

asm("kexec_load:\nmov %rcx, %r10\nmov $153, %rax\nsyscall\nret");

int kexec_load(char *kernel, unsigned long long kernel_size,
               char *initrd,  unsigned long long initrd_size,
               char *cmdline, int vram_mb);

int read_file(char *path, char **ptr, unsigned long long *sz)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    *sz = lseek(fd, 0, SEEK_END);
    *ptr = mmap(NULL, *sz, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char *p = *ptr;
    unsigned long long l = *sz;
    lseek(fd, 0, SEEK_SET);
    while (l) {
        unsigned long long chk = read(fd, p, l);
        if (chk <= 0) return -1;
        p += chk;
        l -= chk;
    }
    close(fd);
    return 0;
}

/* evf_open, evf_cancel, evf_close - kažkokie nxj eventai rebootui */
int evf_open(char *);
void evf_cancel(int, unsigned long long, unsigned long long);
void evf_close(int);

void reboot_thread(void *_)
{
    // Palauk nxj biškį ir tada rebootink
    nanosleep((const struct timespec *)
              "\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", NULL);
    int evf = evf_open("SceSysCoreReboot");
    evf_cancel(evf, 0x4000, 0);
    evf_close(evf);
    kill(1, SIGUSR1);
}

int my_atoi(const char *s)
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

static int pack_kexec_vram_arg(int vram_mb, u16 fw_ver)
{
    return (int)(((u32)fw_ver << 16) | ((u32)vram_mb & 0xFFFFu));
}

/* ═══════════════════════════════════════════════════════════════════
 * §6  kernel_main() — kernel kontekste po kexec nxj exploito
 *     Skaito g_fw / g_kexec_s / g_kexec_e kuriuos main() uždeda, nxj.
 * ═══════════════════════════════════════════════════════════════════ */

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
    // kernel_base nxj suskaičiuojam
    unsigned long long kernel_base = get_syscall() - g_fw->xfast_syscall;

    /* Išjungiam write-protect kad galėtume patchint kernelį, nxj */
    asm volatile("cli\nmov %%cr0, %%rax\nbtc $16, %%rax\nmov %%rax, %%cr0"
                 : : : "rax");

    *(char *)(kernel_base + g_fw->patch1) = 0x07;
    *(char *)(kernel_base + g_fw->patch2) = 0x07;
    /* pstate nustatom prieš shutdown (reik PS4 Pro), nxj */
    *(char *)(kernel_base + g_fw->pstate)  = 0x03;

    asm volatile("mov %%cr0, %%rax\nbts $16, %%rax\nmov %%rax, %%cr0\nsti"
                 : : : "rax");

    unsigned long long early_printf = kernel_base + g_fw->printf_off;
    unsigned long long kmem_alloc   = kernel_base + g_fw->kmem_alloc;
    unsigned long long kernel_map   = kernel_base + g_fw->kernel_map;

    /* Kernel memory alloc ir nukopijuojam kexec blobą, nxj */
    char *new_kexec = ((char *(*)(unsigned long long, unsigned long long))
                       kmem_alloc)(*(unsigned long long *)kernel_map,
                                   g_kexec_e - g_kexec_s);
    for (int i = 0; g_kexec_s + i != g_kexec_e; i++)
        new_kexec[i] = g_kexec_s[i];

    /* Paleidžiam kexec blobą, nxj */
    ((void (*)(void *, void *))new_kexec)((void *)early_printf, NULL);
}

/* ═══════════════════════════════════════════════════════════════════
 * §7  VRAM nxj defaultai
 * ═══════════════════════════════════════════════════════════════════ */

#ifndef VRAM_MB_DEFAULT
#define VRAM_MB_DEFAULT 1024
#endif
#ifndef VRAM_MB_MIN
#define VRAM_MB_MIN 32
#endif
#ifndef VRAM_MB_MAX
#define VRAM_MB_MAX 4609
#endif
#ifndef HDD_BOOT_PATH
#define HDD_BOOT_PATH "/data/linux/boot/"
#endif
#ifndef HDD_SECOND_BOOT_PATH
#define HDD_SECOND_BOOT_PATH "/user/system/boot/"
#endif

/* ═══════════════════════════════════════════════════════════════════
 * §8  main() — user-space entry pointas, nxj
 * ═══════════════════════════════════════════════════════════════════ */

int main(void)
{
    // Signalus nxj ignoruojam, kad niekas nenudėtų proceso
    struct sigaction sa = { .sa_handler = SIG_IGN, .sa_flags = 0 };
    sigaction(SIGSTOP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);

    /* ── Firmware detekcija ir blobų pasirinkimas, nxj ─────────────── */
    u16 fw_ver = get_firmware();
    u16 norm   = normalize_fw_ver(fw_ver);

    g_fw = find_offsets_by_fw(norm);
    if (!g_fw) {
        log_msg("AIO: Unsupported firmware version - no offset table entry.");
        return 1;
    }

    get_kexec_blob(norm, &g_kexec_s, &g_kexec_e);
    if (!g_kexec_s || g_kexec_s == g_kexec_e) {
        log_msg("AIO: No kexec blob found for this firmware.");
        return 1;
    }

    /* ── Linux failų krovimas iš USB ar HDD, nxj ───────────────────── */
    char *kernel = NULL; unsigned long long kernel_size = 0;
    char *initrd = NULL; unsigned long long initrd_size = 0;
    char *cmdline = NULL; unsigned long long cmdline_size = 0;
    char *vramstr = NULL; unsigned long long vramstr_size = 0;
    int vram_mb = 0;

#define L(name, where, wheresz, is_fatal) \
    if (read_file("/mnt/usb0/" name, where, wheresz) \
     && read_file("/mnt/usb1/" name, where, wheresz) \
     && read_file(HDD_BOOT_PATH name, where, wheresz) \
     && read_file(HDD_SECOND_BOOT_PATH name, where, wheresz) \
     && is_fatal) { \
        log_msg("Failed to load file: " name "\nChecked paths:\n" \
                "/mnt/usb0/" name "\n/mnt/usb1/" name "\n" \
                HDD_BOOT_PATH name "\n" HDD_SECOND_BOOT_PATH name); \
        return 1; \
    }

    L("bzImage",            &kernel,  &kernel_size,  1);
    L("initramfs.cpio.gz",  &initrd,  &initrd_size,  1);
    L("bootargs.txt",       &cmdline, &cmdline_size,  0);

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

    /* ── Paleidžiam kernel exploitą → kernel_main(), nxj ───────────── */
    kexec(kernel_main, (void *)0);

    /* ── Paleidžiam reboot watchdog threadą, tada Linux loaderį, nxj ─ */
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
    kexec_load(kernel, kernel_size, initrd, initrd_size, cmdline,
               pack_kexec_vram_arg(vram_mb, fw_ver));
    for (;;);
}
