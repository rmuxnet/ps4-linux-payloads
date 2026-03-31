# ─────────────────────────────────────────────────────────────────────────────
# linux/Makefile — AIO-only PS4 Linux payload build system
#
# All legacy per-firmware targets (FIRMWARES / fwXXX/) have been removed.
# The sole shipped build product is the AIO payload family in the aio/
# directory. Shared kexec sources are built into kexec-build/.
#
# Supported VRAM sizes (MB):
#   SIZES_MB = 32 64 128 256 512 1024 2048 3072 4096
#
# Each size produces 2 variants:
#   aio/payload-aio-NNmb.bin            — standard PS4
#   aio/payload-aio-NNmb-baikal.bin     — PS4 Pro (Baikal)
# ─────────────────────────────────────────────────────────────────────────────

SIZES_MB = 32 64 128 256 512 1024 2048 3072 4096

FIRMWARES = 505 672 700 750 800 850 900 903 960 \
            1000 1050 1100 1102 1150 1200 1250 1300 1302

FW_DEFINE_505  = PS4_5_05
FW_DEFINE_672  = PS4_6_72
FW_DEFINE_700  = PS4_7_00
FW_DEFINE_750  = PS4_7_50
FW_DEFINE_800  = PS4_8_00
FW_DEFINE_850  = PS4_8_50
FW_DEFINE_900  = PS4_9_00
FW_DEFINE_903  = PS4_9_03
FW_DEFINE_960  = PS4_9_60
FW_DEFINE_1000 = PS4_10_00
FW_DEFINE_1050 = PS4_10_50
FW_DEFINE_1100 = PS4_11_00
FW_DEFINE_1102 = PS4_11_02
FW_DEFINE_1150 = PS4_11_50
FW_DEFINE_1200 = PS4_12_00
FW_DEFINE_1250 = PS4_12_50
FW_DEFINE_1300 = PS4_13_00
FW_DEFINE_1302 = PS4_13_02

KEXEC_BUILD_DIR = kexec-build
KEXEC_COMMON_DIR = ps4-kexec-common
KEXEC_BAIKAL_DIR = ps4-kexec-common-baikal
KEXEC_HASH_MANIFEST = kexec-bin.sha256

fw_define = $(FW_DEFINE_$(1))

CC      = gcc
CFLAGS  = -isystem ../freebsd-headers \
          -nostdinc -nostdlib \
          -fno-stack-protector \
          -static -fPIE -ffreestanding
LDFLAGS = -Wl,-gc-sections

# ── kexec blob lists ──────────────────────────────────────────────────────────

NORMAL_KEXEC_BINS = $(foreach fw,$(FIRMWARES),$(KEXEC_BUILD_DIR)/normal/$(fw)/kexec.bin)
BAIKAL_KEXEC_BINS = $(foreach fw,$(FIRMWARES),$(KEXEC_BUILD_DIR)/baikal/$(fw)/kexec.bin)

# ── AIO target list ───────────────────────────────────────────────────────────

AIO_TARGETS = $(foreach sz,$(SIZES_MB),\
    aio/payload-aio-$(sz)mb.bin \
    aio/payload-aio-$(sz)mb-baikal.bin)

# ── Top-level phony targets ───────────────────────────────────────────────────

.PHONY: all aio kexec-bins kexec-diff-check clean

all: aio

aio: $(AIO_TARGETS)

kexec-bins: $(NORMAL_KEXEC_BINS) $(BAIKAL_KEXEC_BINS)

# ── Library ───────────────────────────────────────────────────────────────────

../lib/lib.a:
	cd ../lib && $(MAKE)

# ── Shared kexec builds ───────────────────────────────────────────────────────

$(KEXEC_BUILD_DIR)/normal/%/kexec.bin:
	@mkdir -p $(dir $@)
	$(MAKE) -C $(KEXEC_COMMON_DIR) \
	    FW_DEFINE=$(call fw_define,$*) \
	    BUILD_DIR=$(abspath $(dir $@))

$(KEXEC_BUILD_DIR)/baikal/%/kexec.bin:
	@mkdir -p $(dir $@)
	$(MAKE) -C $(KEXEC_BAIKAL_DIR) \
	    FW_DEFINE=$(call fw_define,$*) \
	    BUILD_DIR=$(abspath $(dir $@))

# ── ELF → BIN conversion ─────────────────────────────────────────────────────

%.bin: %.elf
	objcopy $< \
	    --only-section .text \
	    --only-section .data \
	    --only-section .bss \
	    --only-section .rodata \
	    -O binary $@

# ── AIO build rules (generated via foreach) ───────────────────────────────────
#
# Two variants per size:
#   normal        → main-aio.c, no extra flags
#   baikal        → main-aio.c + -DAIO_BAIKAL=1
#
# Standard variants depend on NORMAL_KEXEC_BINS.
# Baikal variants depend on BAIKAL_KEXEC_BINS.

define make-aio-rules
aio/payload-aio-$(1)mb.elf: ../lib/lib.a main-aio.c fw_detect.c $(NORMAL_KEXEC_BINS)
	@mkdir -p aio
	$(CC) $(CFLAGS) ../lib/lib.a \
	    -DVRAM_MB_DEFAULT=$(1) \
	    -DVRAM_MB_MIN=32 \
	    -DVRAM_MB_MAX=4096 \
	    main-aio.c fw_detect.c \
	    $(LDFLAGS) -o $$@

aio/payload-aio-$(1)mb-baikal.elf: ../lib/lib.a main-aio.c fw_detect.c $(BAIKAL_KEXEC_BINS)
	@mkdir -p aio
	$(CC) $(CFLAGS) ../lib/lib.a \
	    -DVRAM_MB_DEFAULT=$(1) \
	    -DVRAM_MB_MIN=32 \
	    -DVRAM_MB_MAX=4096 \
	    -DAIO_BAIKAL=1 \
	    main-aio.c fw_detect.c \
	    $(LDFLAGS) -o $$@

endef

$(foreach sz,$(SIZES_MB),$(eval $(call make-aio-rules,$(sz))))

kexec-diff-check: kexec-bins
	@sha256sum -c $(KEXEC_HASH_MANIFEST)

# ── Clean ─────────────────────────────────────────────────────────────────────

clean:
	rm -rf aio/ $(KEXEC_BUILD_DIR)/
	cd ../lib/ && $(MAKE) clean
