#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/thr.h>
#include <time.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long long u64;

struct kmain_entry {
    int fw;
    void (*kmain)(void);
};

extern void kernel_main_505();
extern void kernel_main_672();
extern void kernel_main_700();
extern void kernel_main_900();
extern void kernel_main_903();
extern void kernel_main_960();
extern void kernel_main_1000();
extern void kernel_main_1050();
extern void kernel_main_1100();
extern void kernel_main_1102();
extern void kernel_main_1150();
extern void kernel_main_1200();
extern void kernel_main_1250();
extern void kernel_main_1300();
extern void kernel_main_1302();

/* fw values match get_firmware() decimal output: 505, 672, 700 ... */
struct kmain_entry kmain_entries[] = {
    {505,  kernel_main_505},
    {672,  kernel_main_672},
    {700,  kernel_main_700},
    {900,  kernel_main_900},
    {903,  kernel_main_903},
    {960,  kernel_main_960},
    {1000, kernel_main_1000},
    {1050, kernel_main_1050},
    {1100, kernel_main_1100},
    {1102, kernel_main_1102},
    {1150, kernel_main_1150},
    {1200, kernel_main_1200},
    {1250, kernel_main_1250},
    {1300, kernel_main_1300},
    {1302, kernel_main_1302},
    {0,    NULL}
};

void kexec(void* f, void* u);

asm("kexec_load:\nmov %rcx, %r10\nmov $153, %rax\nsyscall\nret");

int kexec_load(char* kernel, unsigned long long kernel_size, char* initrd, unsigned long long initrd_size, char* cmdline, int vram_mb);

int read_file(char* path, char** ptr, unsigned long long* sz)
{
    int fd = open(path, O_RDONLY);
    if(fd < 0)
        return -1;
    *sz = lseek(fd, 0, SEEK_END);
    *ptr = mmap(NULL, *sz, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    char* p = *ptr;
    unsigned long long l = *sz;
    lseek(fd, 0, SEEK_SET);
    while(l)
    {
        unsigned long long chk = read(fd, p, l);
        if(chk <= 0)
            return -1;
        p += chk;
        l -= chk;
    }
    close(fd);
    return 0;
}

int evf_open(char*);
void evf_cancel(int, unsigned long long, unsigned long long);
void evf_close(int);

void reboot_thread(void* _)
{
    nanosleep((const struct timespec*)"\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", NULL);
    int evf = evf_open("SceSysCoreReboot");
    evf_cancel(evf, 0x4000, 0);
    evf_close(evf);
    kill(1, SIGUSR1);
}

/* ------------------------------------------------------------------ *
 * Firmware detection via libc.sprx SELF/SCE header (no SDK needed)  *
 * Source: Al-Azif/ps4-skeleton (MIT) / psdevwiki SELF_File_Format   *
 * ------------------------------------------------------------------ */
typedef struct { u32 props; u32 reserved; u64 offset; u64 file_size; u64 memory_size; } SelfEntry;
typedef struct {
    u32 magic; u8 version; u8 mode; u8 endian; u8 attr;
    u8 content_type; u8 program_type; u16 padding;
    u16 header_size; u16 signature_size; u64 self_size;
    u16 num_of_segments; u16 flags; u32 reserved2;
} SelfHeader;
typedef struct {
    unsigned char e_ident[16]; u16 e_type; u16 e_machine; u32 e_version;
    u64 e_entry; u64 e_phoff; u64 e_shoff; u32 e_flags;
    u16 e_ehsize; u16 e_phentsize; u16 e_phnum;
    u16 e_shentsize; u16 e_shnum; u16 e_shstrndx;
} SelfElf64_Ehdr;
typedef struct {
    u64 program_authority_id; u64 program_type; u64 app_version;
    u64 fw_version; /* bytes [5:4] of u64 = major.minor BCD */
    unsigned char digest[0x20];
} SceHeader;

static u16 g_firmware = 0;

static u16 get_firmware(void)
{
    if (g_firmware)
        return g_firmware;

    int fd = open("/system/common/lib/libc.sprx", O_RDONLY);
    if (fd < 0)
        return 0;

    SelfHeader self_hdr;
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &self_hdr, sizeof(self_hdr)) != (long)sizeof(self_hdr))
        goto fail;

    u64 elf_off = sizeof(self_hdr) + (u64)self_hdr.num_of_segments * sizeof(SelfEntry);

    SelfElf64_Ehdr elf_hdr;
    lseek(fd, (long)elf_off, SEEK_SET);
    if (read(fd, &elf_hdr, sizeof(elf_hdr)) != (long)sizeof(elf_hdr))
        goto fail;

    u64 sce_off = elf_off + elf_hdr.e_ehsize + (u64)elf_hdr.e_phnum * elf_hdr.e_phentsize;
    while (sce_off % 0x10 != 0) sce_off++;

    SceHeader sce_hdr;
    lseek(fd, (long)sce_off, SEEK_SET);
    if (read(fd, &sce_hdr, sizeof(sce_hdr)) != (long)sizeof(sce_hdr))
        goto fail;

    close(fd);

    /* fw_version: byte at shift 40 = major BCD, byte at shift 32 = minor BCD */
    u8 major = (u8)((sce_hdr.fw_version >> 40) & 0xFF);
    u8 minor = (u8)((sce_hdr.fw_version >> 32) & 0xFF);
    u32 maj_dec = ((major >> 4) * 10) + (major & 0xF);
    u32 min_dec = ((minor >> 4) * 10) + (minor & 0xF);
    g_firmware = (u16)(maj_dec * 100 + min_dec);
    return g_firmware;

fail:
    close(fd);
    return 0;
}

void alert(const char* msg)
{
    static int(*sceSysUtilSendSystemNotificationWithText)(int, const char*);
    if(!sceSysUtilSendSystemNotificationWithText)
    {
        void* handle = dlopen("/system/common/lib/libSceSysUtil.sprx", 0);
        sceSysUtilSendSystemNotificationWithText = dlsym(handle, "sceSysUtilSendSystemNotificationWithText");
    }
    sceSysUtilSendSystemNotificationWithText(222, msg);
}

int my_atoi(const char *s)
{
    int ret = 0;
    int neg = 0;
    while(*s == ' ')
        s++;
    neg = (*s == '-') ? 1 : 0;
    for(; *s; s++)
    {
        char c = *s;
        if('0' <= c && c <= '9')
        {
            ret *= 10;
            ret += c-'0';
        }
        else
            break;
    }
    return (neg) ? (-ret) : (ret);
}

#ifndef VRAM_MB_DEFAULT
#define VRAM_MB_DEFAULT 1024
#endif

#ifndef VRAM_MB_MIN
#define VRAM_MB_MIN 256
#endif

// VRAM_MB_MAX is injected by the Makefile 
// Provide a safe fallback only if somehow not set (should not happen in normal builds).
#ifndef VRAM_MB_MAX
#define VRAM_MB_MAX 5120
#endif

#ifndef HDD_BOOT_PATH
#define HDD_BOOT_PATH "/data/linux/boot/"
#endif



int main()
{
    struct sigaction sa = {
        .sa_handler = SIG_IGN,
        .sa_flags = 0,
    };
    // note: overriding SIGSTOP and SIGKILL requires a kernel patch
    sigaction(SIGSTOP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);
    char* kernel = NULL;
    unsigned long long kernel_size = 0;
    char* initrd = NULL;
    unsigned long long initrd_size = 0;
    char* cmdline = NULL;
    unsigned long long cmdline_size = 0;
    char* vramstr = NULL;
    unsigned long long vramstr_size = 0;
    int vram_mb = 0;

#define L(name, where, wheresz, is_fatal)\
    if(read_file("/mnt/usb0/" name, where, wheresz)\
    && read_file("/mnt/usb1/" name, where, wheresz)\
    && read_file(HDD_BOOT_PATH name, where, wheresz)\
    && is_fatal)\
    {\
        alert("Failed to load file: " name ".\nPaths checked:\n/mnt/usb0/" name "\n/mnt/usb1/" name "\n" HDD_BOOT_PATH name);\
        return 1;\
    }
    L("bzImage", &kernel, &kernel_size, 1);
    L("initramfs.cpio.gz", &initrd, &initrd_size, 1);

    L("bootargs.txt", &cmdline, &cmdline_size, 0);

    if(cmdline && cmdline_size)
    {
        for(int i = 0; i < cmdline_size; i++)
            if(cmdline[i] == '\n')
            {
                cmdline[i] = '\0';
                break;
            }
    }
    else
        cmdline = "panic=0 clocksource=tsc consoleblank=0 net.ifnames=0 radeon.dpm=0 amdgpu.dpm=0 drm.debug=0 console=ttyS0,115200n8 console=tty0 video=HDMI-A-1:1920x1080@60";

    // video=HDMI-A-1:1920x1080-24@60      drm.edid_firmware=edid/1920x1080.bin  drm.edid_firmware=edid/my_edid.bin

    L("vram.txt", &vramstr, &vramstr_size, 0);
    if(vramstr && vramstr_size)
    {
        vram_mb = my_atoi(vramstr);
        if(vram_mb < VRAM_MB_MIN || vram_mb > VRAM_MB_MAX)
            vram_mb = VRAM_MB_DEFAULT;
    }
    else
        vram_mb = VRAM_MB_DEFAULT;

    int fw = (int)get_firmware();
    if (!fw) {
        alert("Failed to detect PS4 firmware version!");
        return 1;
    }

    void (*kmain)(void) = NULL;
    for (int i = 0; kmain_entries[i].fw != 0; i++) {
        if (kmain_entries[i].fw == fw) {
            kmain = kmain_entries[i].kmain;
            break;
        }
    }

    if (!kmain) {
        alert("Unsupported firmware version! Expected a payload matching this firmware.");
        return 1;
    }

    kexec(kmain, (void*)0);
    long x, y;
    struct thr_param thr = {
        .start_func = reboot_thread,
        .arg = NULL,
        .stack_base = mmap(NULL, 16384, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0),
        .stack_size = 16384,
        .tls_base = NULL,
        .tls_size = 0,
        .child_tid = &x,
        .parent_tid = &y,
        .flags = 0,
        .rtp = NULL
    };
    thr_new(&thr, sizeof(thr));
    kexec_load(kernel, kernel_size, initrd, initrd_size, cmdline, vram_mb);
    for(;;);
}
