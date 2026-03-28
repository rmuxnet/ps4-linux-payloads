FIRMWARES = 505 672 700 750 900 903 960 1000 1050 1100 1102 1150 1200 1250 1300 1302
SIZES_MB  = 32 64 128 256 512 1024 2048 3072 4096

# Generate payload targets automatically
PAYLOADS = $(foreach fw,$(FIRMWARES), \
             $(foreach sz,$(SIZES_MB), \
               fw$(fw)/payload-$(fw)-$(sz)mb.bin \
               fw$(fw)/payload-$(fw)-$(sz)mb-baikal.bin))

# Common compiler flags - wot
CC = gcc
CFLAGS = -isystem ../freebsd-headers -nostdinc -nostdlib -fno-stack-protector -static -fPIE -ffreestanding
LDFLAGS = -Wl,-gc-sections

# Firmware version formatting helper (handles both 3-digit and 4-digit versions)
# 505 -> 5_05, 1000 -> 10_00, 1302 -> 13_02
define FW_VERSION
$(shell printf '%s' '$(1)' | sed 's/\(.*\)\(..\)/\1_\2/')
endef

.PHONY: all clean

all: $(PAYLOADS)

clean:
	rm -f fw*/*
	@for fw in $(FIRMWARES); do \
		for var in "" "-baikal"; do \
			if [ -d "ps4-kexec-$${fw}$${var}" ]; then \
				cd "ps4-kexec-$${fw}$${var}" && $(MAKE) clean && cd ..; \
			fi; \
		done; \
	done
	cd ../lib/ && $(MAKE) clean

# Library dependency
../lib/lib.a:
	cd ../lib && $(MAKE)

# Pattern rules for kexec binaries
ps4-kexec-%/kexec.bin:
	cd ps4-kexec-$* && $(MAKE)

# Convert ELF to BIN
%.bin: %.elf
	objcopy $< --only-section .text --only-section .data --only-section .bss --only-section .rodata -O binary $@
	file $@ | fgrep -q '$@: DOS executable (COM)'

# Template function to generate rules
# Usage: $(call make-payload-rule,firmware,size_in_mb,main-source,variant-suffix)
define make-payload-rule
fw$(1)/payload-$(1)-$(2)mb$(4).elf: ../lib/lib.a $(3) ps4-kexec-$(1)$(4)/kexec.bin
	@mkdir -p fw$(1)
	$(CC) $(CFLAGS) ../lib/lib.a \
		-D__$(call FW_VERSION,$(1))__ -DPS4_$(call FW_VERSION,$(1)) \
		-DVRAM_MB_DEFAULT=$(2) \
		-DVRAM_MB_MIN=32 \
		-DVRAM_MB_MAX=4096 \
		$(3) $(LDFLAGS) -o $$@
endef

# Generate all rules for each firmware
$(foreach fw,$(FIRMWARES), \
  $(foreach sz,$(SIZES_MB), \
    $(eval $(call make-payload-rule,$(fw),$(sz),main.c,)) \
    $(eval $(call make-payload-rule,$(fw),$(sz),main-baikal.c,-baikal)) \
  ) \
)

# ═══════════════════════════════════════════════════════════════════════════
# AIO (All-In-One) targets
# Detects firmware at runtime — one binary covers every supported version.
# Size variants mirror normal payloads: 32 64 128 256 512 1024 2048 3072 4096 MB.
#
# Run: make aio          → all sizes, normal + baikal
#      make aio-debug    → all sizes, normal + baikal, with alert() debug output
# ═══════════════════════════════════════════════════════════════════════════

AIO_SRCS    = main-aio.c fw_detect.c fw_offsets.c
AIO_KEXEC_N = $(foreach fw,$(FIRMWARES),ps4-kexec-$(fw)/kexec.bin)
AIO_KEXEC_B = $(foreach fw,$(FIRMWARES),ps4-kexec-$(fw)-baikal/kexec.bin)

# Template mirrors make-payload-rule exactly.
# $(1) = size_mb  $(2) = variant-suffix (e.g. -normal, -baikal)
# $(3) = kexec dep list  $(4) = extra CFLAGS (e.g. -DAIO_BAIKAL -DAIO_DEBUG)
define make-aio-payload-rule
fw_aio/payload-aio-$(1)mb$(2).elf: ../lib/lib.a $(AIO_SRCS) $(3)
	@mkdir -p fw_aio
	$(CC) $(CFLAGS) ../lib/lib.a \
		$(4) \
		-DVRAM_MB_DEFAULT=$(1) \
		-DVRAM_MB_MIN=32 \
		-DVRAM_MB_MAX=4096 \
		$(AIO_SRCS) $(LDFLAGS) -o $$@
endef

$(foreach sz,$(SIZES_MB),\
  $(eval $(call make-aio-payload-rule,$(sz),-normal,$(AIO_KEXEC_N),))\
  $(eval $(call make-aio-payload-rule,$(sz),-baikal,$(AIO_KEXEC_B),-DAIO_BAIKAL))\
  $(eval $(call make-aio-payload-rule,$(sz),-normal-debug,$(AIO_KEXEC_N),-DAIO_DEBUG))\
  $(eval $(call make-aio-payload-rule,$(sz),-baikal-debug,$(AIO_KEXEC_B),-DAIO_BAIKAL -DAIO_DEBUG))\
)

AIO_BINS = $(foreach sz,$(SIZES_MB),\
             fw_aio/payload-aio-$(sz)mb-normal.bin\
             fw_aio/payload-aio-$(sz)mb-baikal.bin)

AIO_DEBUG_BINS = $(foreach sz,$(SIZES_MB),\
                   fw_aio/payload-aio-$(sz)mb-normal-debug.bin\
                   fw_aio/payload-aio-$(sz)mb-baikal-debug.bin)

.PHONY: aio aio-debug
aio:       $(AIO_BINS)
aio-debug: $(AIO_DEBUG_BINS)
