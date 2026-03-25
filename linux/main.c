#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/thr.h>
#include <time.h>
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

struct kmain_entry kmain_entries[] = {
    {0x0505, kernel_main_505},
    {0x0672, kernel_main_672},
    {0x0700, kernel_main_700},
    {0x0900, kernel_main_900},
    {0x0903, kernel_main_903},
    {0x0960, kernel_main_960},
    {0x1000, kernel_main_1000},
    {0x1050, kernel_main_1050},
    {0x1100, kernel_main_1100},
    {0x1102, kernel_main_1102},
    {0x1150, kernel_main_1150},
    {0x1200, kernel_main_1200},
    {0x1250, kernel_main_1250},
    {0x1300, kernel_main_1300},
    {0x1302, kernel_main_1302},
    {0, NULL}
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

void* dlopen(const char*, int);
void* dlsym(void*, const char*);

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

int get_fw_version(void)
{
    int fw = 0;
    void *libkernel = dlopen("/system/common/lib/libkernel.sprx", 0);
    if (libkernel) {
        int (*sceKernelGetSystemSwVersion)(void) = dlsym(libkernel, "sceKernelGetSystemSwVersion");
        if (sceKernelGetSystemSwVersion) {
            int ver = sceKernelGetSystemSwVersion();
            fw = ver >> 16;
        }
    }
    return fw;
}

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

    int fw = get_fw_version();
    if (!fw) {
        alert("Failed to detect PS4 firmware version dynamically!");
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
