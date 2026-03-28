# ─────────────────────────────────────────────────────────────────────────────
# linux/Makefile — AIO-only PS4 Linux payload build system
#
# All legacy per-firmware targets (FIRMWARES / fwXXX/) have been removed.
# The sole build product is the AIO payload family in the aio/ directory.
#
# Supported VRAM sizes (MB):
#   SIZES_MB = 32 64 128 256 512 1024 2048 3072 4096
#
# Each size produces 4 variants:
#   aio/payload-aio-NNmb.bin            — standard PS4
#   aio/payload-aio-NNmb-baikal.bin     — PS4 Pro (Baikal)
#   aio/payload-aio-NNmb-debug.bin      — standard PS4 + debug alerts
#   aio/payload-aio-NNmb-baikal-debug.bin — PS4 Pro + debug alerts
# ─────────────────────────────────────────────────────────────────────────────

SIZES_MB = 32 64 128 256 512 1024 2048 3072 4096

FIRMWARES = 505 672 700 750 800 850 900 903 960 \
            1000 1050 1100 1102 1150 1200 1250 1300 1302

CC      = gcc
CFLAGS  = -isystem ../freebsd-headers \
          -nostdinc -nostdlib \
          -fno-stack-protector \
          -static -fPIE -ffreestanding
LDFLAGS = -Wl,-gc-sections

# ── kexec blob lists ──────────────────────────────────────────────────────────

NORMAL_KEXEC_BINS = $(foreach fw,$(FIRMWARES),ps4-kexec-$(fw)/kexec.bin)
BAIKAL_KEXEC_BINS = $(foreach fw,$(FIRMWARES),ps4-kexec-$(fw)-baikal/kexec.bin)

# ── AIO target list ───────────────────────────────────────────────────────────

AIO_TARGETS = $(foreach sz,$(SIZES_MB),\
    aio/payload-aio-$(sz)mb.bin \
    aio/payload-aio-$(sz)mb-baikal.bin \
    aio/payload-aio-$(sz)mb-debug.bin \
    aio/payload-aio-$(sz)mb-baikal-debug.bin)

# ── Top-level phony targets ───────────────────────────────────────────────────

.PHONY: all aio aio-debug clean

all: aio

aio: $(AIO_TARGETS)

aio-debug: $(filter %-debug.bin,$(AIO_TARGETS))

# ── Library ───────────────────────────────────────────────────────────────────

../lib/lib.a:
	cd ../lib && $(MAKE)

# ── kexec sub-project blobs ───────────────────────────────────────────────────

ps4-kexec-%/kexec.bin:
	cd ps4-kexec-$* && $(MAKE)

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
# Four variants per size:
#   normal        → main-aio.c, no extra flags
#   baikal        → main-aio.c + -DAIO_BAIKAL=1
#   debug         → main-aio.c + -DAIO_DEBUG=1
#   baikal-debug  → main-aio.c + -DAIO_BAIKAL=1 -DAIO_DEBUG=1
#
# Each normal/debug variant depends on NORMAL_KEXEC_BINS.
# Each baikal/baikal-debug variant depends on BAIKAL_KEXEC_BINS.

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

aio/payload-aio-$(1)mb-debug.elf: ../lib/lib.a main-aio.c fw_detect.c $(NORMAL_KEXEC_BINS)
	@mkdir -p aio
	$(CC) $(CFLAGS) ../lib/lib.a \
	    -DVRAM_MB_DEFAULT=$(1) \
	    -DVRAM_MB_MIN=32 \
	    -DVRAM_MB_MAX=4096 \
	    -DAIO_DEBUG=1 \
	    main-aio.c fw_detect.c \
	    $(LDFLAGS) -o $$@

aio/payload-aio-$(1)mb-baikal-debug.elf: ../lib/lib.a main-aio.c fw_detect.c $(BAIKAL_KEXEC_BINS)
	@mkdir -p aio
	$(CC) $(CFLAGS) ../lib/lib.a \
	    -DVRAM_MB_DEFAULT=$(1) \
	    -DVRAM_MB_MIN=32 \
	    -DVRAM_MB_MAX=4096 \
	    -DAIO_BAIKAL=1 \
	    -DAIO_DEBUG=1 \
	    main-aio.c fw_detect.c \
	    $(LDFLAGS) -o $$@
endef

$(foreach sz,$(SIZES_MB),$(eval $(call make-aio-rules,$(sz))))

# ── Clean ─────────────────────────────────────────────────────────────────────

clean:
	rm -rf aio/
	@for fw in $(FIRMWARES); do \
	    for var in "" "-baikal"; do \
	        dir="ps4-kexec-$${fw}$${var}"; \
	        if [ -d "$$dir" ]; then \
	            cd "$$dir" && $(MAKE) clean && cd ..; \
	        fi; \
	    done; \
	done
	cd ../lib/ && $(MAKE) clean
