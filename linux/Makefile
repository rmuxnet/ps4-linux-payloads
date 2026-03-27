FIRMWARES = 505 672 700 750 900 903 960 1000 1050 1100 1102 1150 1200 1250 1300 1302
SIZES_MB  = 32 64 128 256 512 1024 2048 3072 4096

# Generate payload targets automatically
PAYLOADS = $(foreach fw,$(FIRMWARES), \
             $(foreach sz,$(SIZES_MB), \
               fw$(fw)/payload-$(fw)-$(sz)mb.bin \
               fw$(fw)/payload-$(fw)-$(sz)mb-baikal.bin))

# Common compiler flags
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
# AIO (All-In-One) targets — detect firmware at runtime, one binary covers
# every supported firmware version.
# ═══════════════════════════════════════════════════════════════════════════

# List of all kexec blobs the AIO binary depends on.
AIO_KEXEC_NORMAL = $(foreach fw,$(FIRMWARES),ps4-kexec-$(fw)/kexec.bin)
AIO_KEXEC_BAIKAL = $(foreach fw,$(FIRMWARES),ps4-kexec-$(fw)-baikal/kexec.bin)

# Common AIO source files (fw_detect.c / fw_offsets.c live in linux/).
AIO_SRCS = main-aio.c fw_detect.c fw_offsets.c

# ── 256 MB normal ────────────────────────────────────────────────────────
fw_aio/payload-aio-256mb-normal.elf: ../lib/lib.a $(AIO_SRCS) $(AIO_KEXEC_NORMAL)
	@mkdir -p fw_aio
	$(CC) $(CFLAGS) ../lib/lib.a \
		-DVRAM_MB_DEFAULT=256 \
		-DVRAM_MB_MIN=32 \
		-DVRAM_MB_MAX=4096 \
		$(AIO_SRCS) \
		$(LDFLAGS) -o $@

fw_aio/payload-aio-256mb-normal.bin: fw_aio/payload-aio-256mb-normal.elf
	objcopy $< \
		--only-section .text \
		--only-section .data \
		--only-section .bss  \
		--only-section .rodata \
		-O binary $@
	file $@ | fgrep -q '$@: DOS executable (COM)'

# ── 256 MB baikal ────────────────────────────────────────────────────────
fw_aio/payload-aio-256mb-baikal.elf: ../lib/lib.a $(AIO_SRCS) $(AIO_KEXEC_BAIKAL)
	@mkdir -p fw_aio
	$(CC) $(CFLAGS) ../lib/lib.a \
		-DAIO_BAIKAL \
		-DVRAM_MB_DEFAULT=256 \
		-DVRAM_MB_MIN=32 \
		-DVRAM_MB_MAX=4096 \
		$(AIO_SRCS) \
		$(LDFLAGS) -o $@

fw_aio/payload-aio-256mb-baikal.bin: fw_aio/payload-aio-256mb-baikal.elf
	objcopy $< \
		--only-section .text \
		--only-section .data \
		--only-section .bss  \
		--only-section .rodata \
		-O binary $@
	file $@ | fgrep -q '$@: DOS executable (COM)'

# ── Convenience targets ───────────────────────────────────────────────────
AIO_PAYLOADS = \
	fw_aio/payload-aio-256mb-normal.bin \
	fw_aio/payload-aio-256mb-baikal.bin

.PHONY: aio
aio: $(AIO_PAYLOADS)

# To include the AIO builds in the default `make all`, uncomment:
# all: $(PAYLOADS) $(AIO_PAYLOADS)

# ── Debug build (shows fw detection + copyinstr offset via alert()) ───────
fw_aio/payload-aio-256mb-normal-debug.elf: ../lib/lib.a $(AIO_SRCS) $(AIO_KEXEC_NORMAL)
	@mkdir -p fw_aio
	$(CC) $(CFLAGS) ../lib/lib.a \
		-DVRAM_MB_DEFAULT=256 \
		-DVRAM_MB_MIN=32 \
		-DVRAM_MB_MAX=4096 \
		-DAIO_DEBUG \
		$(AIO_SRCS) \
		$(LDFLAGS) -o $@

fw_aio/payload-aio-256mb-normal-debug.bin: fw_aio/payload-aio-256mb-normal-debug.elf
	objcopy $< \
		--only-section .text \
		--only-section .data \
		--only-section .bss  \
		--only-section .rodata \
		-O binary $@
	file $@ | fgrep -q '$@: DOS executable (COM)'

fw_aio/payload-aio-256mb-baikal-debug.elf: ../lib/lib.a $(AIO_SRCS) $(AIO_KEXEC_BAIKAL)
	@mkdir -p fw_aio
	$(CC) $(CFLAGS) ../lib/lib.a \
		-DAIO_BAIKAL \
		-DVRAM_MB_DEFAULT=256 \
		-DVRAM_MB_MIN=32 \
		-DVRAM_MB_MAX=4096 \
		-DAIO_DEBUG \
		$(AIO_SRCS) \
		$(LDFLAGS) -o $@

fw_aio/payload-aio-256mb-baikal-debug.bin: fw_aio/payload-aio-256mb-baikal-debug.elf
	objcopy $< \
		--only-section .text \
		--only-section .data \
		--only-section .bss  \
		--only-section .rodata \
		-O binary $@
	file $@ | fgrep -q '$@: DOS executable (COM)'

.PHONY: aio-debug
aio-debug: \
	fw_aio/payload-aio-256mb-normal-debug.bin \
	fw_aio/payload-aio-256mb-baikal-debug.bin