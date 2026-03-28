/*
 * fw_offsets.h — unified, runtime-selectable kernel offset table
 *
 * Merges data from freebsd-headers/ps4-offsets/*.h (fields needed by
 * kernel_main) and linux/magic.h (full kern_off_* set) into a single
 * struct so an AIO payload can call find_offsets_by_fw() at runtime
 * instead of using compile-time #defines.
 *
 * Supported firmware: 505 672 700 750 900 903 960
 *                     1000 1050 1100 1102 1150 1200 1250 1300 1302
 */

#ifndef FW_OFFSETS_H
#define FW_OFFSETS_H

#include "ps4-kexec-700/types.h"

typedef struct {
    u16 fw;  /* firmware version as returned by get_firmware(), e.g. 700, 1100 */

    /* ── From ps4-offsets/<fw>.h — used directly by kernel_main() ──────── */

    u64 xfast_syscall;      /* kernel_offset_xfast_syscall  (always 0x1c0) */
    u64 printf_off;         /* kernel_offset_printf                         */
    u64 kmem_alloc;         /* kernel_offset_kmem_alloc  (regular alloc)   */
    u64 kernel_map;         /* kernel_offset_kernel_map                     */
    u64 patch_kmem_alloc_1; /* kernel_patch_kmem_alloc_1                   */
    u64 patch_kmem_alloc_2; /* kernel_patch_kmem_alloc_2                   */

    /* ── From magic.h — kern_off_* set (used by kexec blob + utilities) ── */

    u64 pstate_before_shutdown; /* kern_off_pstate_before_shutdown          */
    u64 snprintf;               /* kern_off_snprintf                        */
    u64 copyin;                 /* kern_off_copyin                          */
    u64 copyout;                /* kern_off_copyout                         */
    u64 copyinstr;              /* kern_off_copyinstr                       */
    u64 kmem_alloc_contig;      /* kern_off_kmem_alloc_contig               */
    u64 kmem_free;              /* kern_off_kmem_free                       */
    u64 pmap_extract;           /* kern_off_pmap_extract                    */
    u64 pmap_protect;           /* kern_off_pmap_protect                    */
    u64 sched_pin;              /* kern_off_sched_pin                       */
    u64 sched_unpin;            /* kern_off_sched_unpin                     */
    u64 smp_rendezvous;         /* kern_off_smp_rendezvous                  */
    u64 smp_no_rendevous_barrier; /* kern_off_smp_no_rendevous_barrier      */
    u64 icc_query_nowait;       /* kern_off_icc_query_nowait                */
    u64 sysent;                 /* kern_off_sysent                          */
    u64 kernel_pmap_store;      /* kern_off_kernel_pmap_store               */
    u64 Starsha_UcodeInfo;      /* kern_off_Starsha_UcodeInfo               */
    u64 gpu_devid_is_9924;      /* kern_off_gpu_devid_is_9924               */
    u64 gc_get_fw_info;         /* kern_off_gc_get_fw_info                  */
    u64 pml4pml4i;              /* kern_off_pml4pml4i                       */
    u64 dmpml4i;                /* kern_off_dmpml4i                         */
    u64 dmpdpi;                 /* kern_off_dmpdpi                          */
    u64 eap_hdd_key;            /* kern_off_eap_hdd_key                     */
    u64 edid;                   /* kern_off_edid                            */
    u64 wlanbt;                 /* kern_off_wlanbt                          */
    u64 kern_reboot;            /* kern_off_kern_reboot                     */
    u64 set_gpu_freq;           /* kern_off_set_gpu_freq                    */
    u64 set_pstate;             /* kern_off_set_pstate                      */
    u64 update_vddnp;           /* kern_off_update_vddnp                    */
    u64 set_cu_power_gate;      /* kern_off_set_cu_power_gate               */
    u64 set_nclk_mem_spd;       /* kern_off_set_nclk_mem_spd (0 if unused)  */
} FwOffsets;

/*
 * Returns a pointer into the static offset table for the given firmware
 * version, or NULL if the firmware is not recognised.
 *
 * fw_ver is the integer returned by get_firmware(), e.g. 700 for 7.00,
 * 1100 for 11.00, etc.
 *
 * Result is a pointer to a const entry in BSS — never free() it.
 * Safe to call multiple times; no side effects.
 */
const FwOffsets *find_offsets_by_fw(u16 fw_ver);

#endif /* FW_OFFSETS_H */
