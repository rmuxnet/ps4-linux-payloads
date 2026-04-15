/*
 * ps4-kexec - a kexec() implementation for Orbis OS / FreeBSD
 *
 * Copyright (C) 2015-2016 shuffle2 <godisgovernment@gmail.com>
 * Copyright (C) 2015-2016 Hector Martin "marcan" <marcan@marcan.st>
 *
 * This code is licensed to you under the 2-clause BSD license. See the LICENSE
 * file for more information.
 */

#include "kernel.h"
#include "linux_boot.h"
#include "x86.h"
#include "kexec.h"
#include "firmware.h"
#include "string.h"
#include "acpi.h"
#include "../sb_detect.h"

u8 sb_id = 0;

#define LINUX_BOOT_FLAG_MAGIC 0xAA55
#define LINUX_HDR_MAGIC       0x53726448

static int bad_kptr(const void *p)
{
    uintptr_t v = (uintptr_t)p;
    return v == 0 || v == ~(uintptr_t)0;
}

static int image_looks_linux(const struct boot_params *bp)
{
    if (!bp)
        return 0;

    return bp->hdr.boot_flag == LINUX_BOOT_FLAG_MAGIC &&
           bp->hdr.header == LINUX_HDR_MAGIC;
}

static int k_copyin(const void *uaddr, void *kaddr, size_t len)
{
    if (!uaddr || !kaddr)
        return EFAULT;
    memcpy(kaddr, uaddr, len);
    return 0;
}

static int k_copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done)
{
    const char *ustr = (const char*)uaddr;
    char *kstr = (char*)kaddr;
    size_t ret;

    if (!uaddr || !kaddr)
        return EFAULT;

    ret = strlcpy(kstr, ustr, len);
    if (ret >= len) {
        if (done)
            *done = len;
        return ENAMETOOLONG;
    } else {
        if (done)
            *done = ret + 1;
    }
    return 0;
}

static int k_copyout(const void *kaddr, void *uaddr, size_t len)
{
    if (!uaddr || !kaddr)
        return EFAULT;
    memcpy(uaddr, kaddr, len);
    return 0;
}

#define KEXEC_VRAM_MASK 0xFFFFu
#define KEXEC_FW_MASK   0xFFFu
#define KEXEC_SB_MASK   0xFu

static u16 kexec_get_vram_mb(int packed)
{
    return (u16)((u32)packed & KEXEC_VRAM_MASK);
}

static u16 kexec_get_fw_ver(int packed)
{
    return (u16)(((u32)packed >> 16) & KEXEC_FW_MASK);
}

static u8 kexec_get_sb_id(int packed)
{
    return (u8)(((u32)packed >> 28) & KEXEC_SB_MASK);
}

static const char* kexec_get_sb_name(u8 id)
{
    switch (id) {
        case SB_AEOLIA:  return "Aeolia";
        case SB_BELIZE:  return "Belize";
        case SB_BAIKAL:  return "Baikal";
        case SB_BELIZE2: return "Belize2";
        default:         return "Unknown Southbridge";
    }
}

static void kexec_print_banner(u16 fw_ver, u16 vram_mb, u8 sb_id)
{
    kern.printf("========================================\n");
    kern.printf("PS4 Linux Payloads AIO\n");
    if (fw_ver) {
        kern.printf("FW %u.%02u (%u)\n",
            (unsigned int)(fw_ver / 100),
            (unsigned int)(fw_ver % 100),
            (unsigned int)fw_ver);
    } else {
        kern.printf("FW unknown\n");
    }
    kern.printf("VRAM %u MB\n", (unsigned int)vram_mb);
    kern.printf("Southbridge: %s\n", kexec_get_sb_name(sb_id));
    kern.printf("========================================\n");
}

int sys_kexec(void *td, struct sys_kexec_args *uap)
{
    int err = 0;
    size_t initramfs_size = uap->initramfs_size;
    void *image = NULL;
    void *initramfs = NULL;
    size_t firmware_size = 0;
    struct boot_params *bp = NULL;
    size_t cmd_line_maxlen = 0;
    char *cmd_line = NULL;
    const struct boot_params *img_bp = NULL;
    u16 vram_mb = kexec_get_vram_mb(uap->vram_gb);
    u16 fw_ver = kexec_get_fw_ver(uap->vram_gb);

    sb_id = kexec_get_sb_id(uap->vram_gb);

    int (*copyin)(const void *uaddr, void *kaddr, size_t len) =
        td ? kern.copyin : k_copyin;
    int (*copyinstr)(const void *uaddr, void *kaddr, size_t len, size_t *done) =
        td ? kern.copyinstr : k_copyinstr;
    int (*copyout)(const void *kaddr, void *uaddr, size_t len) =
        td ? kern.copyout : k_copyout;

    kern.printf("sys_kexec invoked\n");
    kexec_print_banner(fw_ver, vram_mb, sb_id);
    kern.printf("sys_kexec(%p, %zu, %p, %zu, \"%s\")\n", uap->image,
        uap->image_size, uap->initramfs, uap->initramfs_size, uap->cmd_line);

    // Look up our shutdown hook point
    void *icc_query_nowait = kern.icc_query_nowait;
    if (!icc_query_nowait) {
        err = ENOENT;
        goto cleanup;
    }

    // Set gpu frequencies and pstate
    if (kern.gpu_devid_is_9924()) {
        // PS4 PRO
        kern.set_gpu_freq(1, 853);
        kern.set_gpu_freq(2, 711);
        kern.set_gpu_freq(4, 911);
        kern.set_gpu_freq(5, 800);
        kern.set_gpu_freq(6, 984);

        kern.set_cu_power_gate(0x24);
    } else {
        // PS4 FAT/SLIM
        kern.set_pstate(3);
        kern.set_gpu_freq(0, 800);
        kern.set_gpu_freq(1, 673);
        kern.set_gpu_freq(2, 609);
        kern.set_gpu_freq(4, 800);
        kern.set_gpu_freq(5, 711);
        kern.set_gpu_freq(6, 711);

        kern.set_cu_power_gate(0x12);
    }

    // Both PRO & FAT/SLIM
    kern.set_pstate(3);
    kern.set_gpu_freq(0, 800);
    kern.set_gpu_freq(3, 800);
    kern.set_gpu_freq(7, 673);
    kern.update_vddnp(0x12);

    // Copy in kernel image
    image = kernel_alloc_contig(uap->image_size);
    if (!image || bad_kptr(image)) {
        kern.printf("Failed to allocate valid image buffer: %p\n", image);
        err = ENOMEM;
        goto cleanup;
    }

    err = copyin(uap->image, image, uap->image_size);
    if (err) {
        kern.printf("Failed to copy in image\n");
        goto cleanup;
    }

    img_bp = (const struct boot_params *)image;
    if (!image_looks_linux(img_bp)) {
        kern.printf("Invalid Linux image header: boot_flag=%#x header=%#x version=%#x\n",
                    img_bp->hdr.boot_flag, img_bp->hdr.header,
                    img_bp->hdr.version);
        err = EINVAL;
        goto cleanup;
    }

    kern.printf("Linux image header:\n");
    kern.printf("    boot_flag:    %#x\n", img_bp->hdr.boot_flag);
    kern.printf("    header:       %#x\n", img_bp->hdr.header);
    kern.printf("    version:      %#x\n", img_bp->hdr.version);
    kern.printf("    cmdline_size: %u\n", img_bp->hdr.cmdline_size);
    kern.printf("    init_size:    %u\n", img_bp->hdr.init_size);
    kern.printf("    pref_address: %#llx\n",
                (unsigned long long)img_bp->hdr.pref_address);

    // Copy in initramfs
    initramfs = kernel_alloc_contig(initramfs_size + FW_CPIO_SIZE);
    if (!initramfs || bad_kptr(initramfs)) {
        kern.printf("Failed to allocate valid initramfs buffer: %p\n", initramfs);
        err = ENOMEM;
        goto cleanup;
    }

    err = firmware_extract((u8 *)initramfs);
    if (err < 0) {
        kern.printf("Failed to extract GPU firmware - continuing anyway\n");
    } else {
        firmware_size = (size_t)err;
    }

    if (initramfs_size) {
        err = copyin(uap->initramfs, (u8 *)initramfs + firmware_size, initramfs_size);
        if (err) {
            kern.printf("Failed to copy in initramfs\n");
            goto cleanup;
        }
    }
    initramfs_size += firmware_size;

    // Copy in cmdline
    cmd_line_maxlen = (size_t)img_bp->hdr.cmdline_size + 1;
    if (cmd_line_maxlen < 2 || cmd_line_maxlen > 0x10000) {
        kern.printf("Bad cmdline_size: %zu\n", cmd_line_maxlen);
        err = EINVAL;
        goto cleanup;
    }

    cmd_line = kernel_alloc_contig(cmd_line_maxlen);
    if (!cmd_line || bad_kptr(cmd_line)) {
        kern.printf("Failed to allocate valid cmdline buffer: %p\n", cmd_line);
        err = ENOMEM;
        goto cleanup;
    }

    err = copyinstr(uap->cmd_line, cmd_line, cmd_line_maxlen, NULL);
    if (err) {
        kern.printf("Failed to copy in cmdline\n");
        goto cleanup;
    }
    cmd_line[cmd_line_maxlen - 1] = 0;

    kern.printf("\nkexec parameters:\n");
    kern.printf("    Kernel image size:   %zu bytes\n", uap->image_size);
    kern.printf("    Initramfs size:      %zu bytes (%zu from user)\n",
                initramfs_size, uap->initramfs_size);
    kern.printf("    Kernel command line: %s\n", cmd_line);
    kern.printf("    Kernel image buffer: %p\n", image);
    kern.printf("    Initramfs buffer:    %p\n", initramfs);
    kern.printf("    Cmdline buffer:      %p\n", cmd_line);

    // Allocate our boot params
    bp = kernel_alloc_contig(sizeof(*bp));
    if (!bp || bad_kptr(bp)) {
        kern.printf("Failed to allocate valid bp: %p\n", bp);
        err = ENOMEM;
        goto cleanup;
    }

    set_nix_info(image, bp, initramfs, initramfs_size, cmd_line, (int)vram_mb);
    prepare_boot_params(bp, image);

    kern.printf("Prepared boot params:\n");
    kern.printf("    bp:                %p\n", bp);
    kern.printf("    boot_flag:         %#x\n", bp->hdr.boot_flag);
    kern.printf("    header:            %#x\n", bp->hdr.header);
    kern.printf("    version:           %#x\n", bp->hdr.version);
    kern.printf("    cmd_line_ptr:      %#x\n", bp->hdr.cmd_line_ptr);
    kern.printf("    ramdisk_image:     %#x\n", bp->hdr.ramdisk_image);
    kern.printf("    ramdisk_size:      %#x\n", bp->hdr.ramdisk_size);
    kern.printf("    ext_ramdisk_image: %#x\n", bp->ext_ramdisk_image);
    kern.printf("    ext_ramdisk_size:  %#x\n", bp->ext_ramdisk_size);
    kern.printf("    type_of_loader:    %#x\n", bp->hdr.type_of_loader);
    kern.printf("    hardware_subarch:  %u\n", bp->hdr.hardware_subarch);

    if (!image_looks_linux(bp)) {
        kern.printf("Prepared boot params contain invalid header: boot_flag=%#x header=%#x\n",
                    bp->hdr.boot_flag, bp->hdr.header);
        err = EINVAL;
        goto cleanup;
    }

    if (!kernel_hook_install(hook_icc_query_nowait, icc_query_nowait)) {
        kern.printf("Failed to install shutdown hook\n");
        err = EINVAL;
        goto cleanup;
    }

    kern.printf("******************************************************\n");
    kern.printf("kexec successfully armed. Please shut down the system.\n");
    kern.printf("******************************************************\n\n");

    return 0;

cleanup:
    kernel_free_contig(cmd_line, cmd_line_maxlen);
    kernel_free_contig(bp, sizeof(*bp));
    kernel_free_contig(image, uap->image_size);
    kernel_free_contig(initramfs, initramfs_size);
    return err;

    copyout(NULL, NULL, 0);
}

int kexec_init(void *_early_printf, sys_kexec_t *sys_kexec_ptr)
{
    int rv = 0;

    // potentially needed to write early_printf
    u64 flags = intr_disable();
    u64 wp = write_protect_disable();

    if (kernel_init(_early_printf) < 0) {
        rv = -1;
        goto cleanup;
    }

    kern.printf("Installing sys_kexec to system call #%d\n", SYS_KEXEC);
    kernel_syscall_install(SYS_KEXEC, sys_kexec, SYS_KEXEC_NARGS);
    kern.printf("kexec_init() successful\n\n");

    if (sys_kexec_ptr)
        *sys_kexec_ptr = sys_kexec;

cleanup:
    write_protect_restore(wp);
    if (kern.sched_unpin && wp & CR0_WP) {
        // If we're returning to a state with WP enabled, assume the caller
        // wants the thread unpinned. Else the caller is expected to
        // call kern.sched_unpin() manually.
        kern.sched_unpin();
    }
    intr_restore(flags);
    return rv;
}
