FIRMWARES = 505 672 700 900 903 960 1000 1050 1100 1102 1150 1200 1250 1300 1302

# ============================================================
# Configurable VRAM size range
# ============================================================
# Sub-256 MB sizes (powers of 2) — included for every variant
SIZES_SMALL_MB     := 32 64 128

# Step size for the linear range, in MB.
#   256 MB = 0.25 GB steps  -> gives 1.0, 1.25, 1.5, 1.75, 2.0, 2.25 ...
#   Change to 512 for 0.5 GB steps, 1024 for whole-GB steps only.
SIZE_STEP_MB       := 256

# Inclusive lower bound of the linear range (must be a multiple of SIZE_STEP_MB)
SIZE_LINEAR_MIN_MB := 256

# Upper bound per hardware variant (in MB)
#   Normal PS4            : AMD Kabini/Jaguar GPU - up to 5 GB  (5120 MB)
#   PS4 Pro / Baikal 
SIZE_MAX_NORMAL_MB := 5120
SIZE_MAX_BAIKAL_MB := 5120
# ============================================================

# Build a list of MB values from SIZE_LINEAR_MIN_MB to $(1) in SIZE_STEP_MB steps
gen-range = $(shell seq $(SIZE_LINEAR_MIN_MB) $(SIZE_STEP_MB) $(1))

# Full size lists per variant
SIZES_MB_NORMAL := $(SIZES_SMALL_MB) $(call gen-range,$(SIZE_MAX_NORMAL_MB))
SIZES_MB_BAIKAL := $(SIZES_SMALL_MB) $(call gen-range,$(SIZE_MAX_BAIKAL_MB))

# Common compiler flags
CC      = gcc
CFLAGS  = -isystem ../freebsd-headers -nostdinc -nostdlib -fno-stack-protector \
           -static -fPIE -ffreestanding
LDFLAGS = -Wl,-gc-sections

# Firmware version formatting helper (505->5_05 / 1000->10_00 / 1302->13_02)
define FW_VER
$(shell printf '%s' '$(1)' | sed 's/\(.*\)\(..\)$$/\1_\2/')
endef

# ============================================================
# Generate PAYLOADS target list
# ============================================================
# Normal variants: sizes up to SIZE_MAX_NORMAL_MB
define normal-payloads-for-fw
$(foreach sz,$(SIZES_MB_NORMAL), \
  fw$(1)/payload-$(1)-$(sz)mb.bin \
  fw$(1)/payload-$(1)-$(sz)mb-pro.bin)
endef

# Baikal variants: sizes up to SIZE_MAX_BAIKAL_MB
define baikal-payloads-for-fw
$(foreach sz,$(SIZES_MB_BAIKAL), \
  fw$(1)/payload-$(1)-$(sz)mb-baikal.bin \
  fw$(1)/payload-$(1)-$(sz)mb-pro-baikal.bin)
endef

PAYLOADS = $(foreach fw,$(FIRMWARES), \
             $(call normal-payloads-for-fw,$(fw)) \
             $(call baikal-payloads-for-fw,$(fw)))

.PHONY: all clean
all: $(PAYLOADS)

clean:
	rm -f fw*/*
	@for fw in $(FIRMWARES); do \
		for var in "" "-pro" "-baikal" "-pro-baikal"; do \
			if [ -d "ps4-kexec-$${fw}$${var}" ]; then \
				cd "ps4-kexec-$${fw}$${var}" && $(MAKE) clean && cd ..; \
			fi; \
		done; \
	done
	cd ../lib/ && $(MAKE) clean

# ============================================================
# Library & kexec binary dependencies
# ============================================================
../lib/lib.a:
	cd ../lib && $(MAKE)

ps4-kexec-%/kexec.bin:
	cd ps4-kexec-$* && $(MAKE)

# ELF -> raw binary
%.bin: %.elf
	objcopy $< --only-section .text --only-section .data --only-section .bss \
	            --only-section .rodata -O binary $@
	file $@ | fgrep -q '$@: DOS executable (COM)'

# ============================================================
# Rule template
#
# $(1) firmware version  (e.g. 1000)
# $(2) size in MB        (e.g. 1280)
# $(3) main source file  (main.c or main-baikal.c)
# $(4) variant suffix    (empty / -pro / -baikal / -pro-baikal)
# $(5) VRAM_MB_MAX value for this variant
# ============================================================
define make-payload-rule
fw$(1)/payload-$(1)-$(2)mb$(4).elf: ../lib/lib.a $(3) ps4-kexec-$(1)$(4)/kexec.bin
	@mkdir -p fw$(1)
	$(CC) $(CFLAGS) ../lib/lib.a \
	    -D__$(call FW_VER,$(1))__ -DPS4_$(call FW_VER,$(1)) \
	    -DVRAM_MB_DEFAULT=$(2) \
	    -DVRAM_MB_MIN=$(SIZE_LINEAR_MIN_MB) \
	    -DVRAM_MB_MAX=$(5) \
	    $(3) $(LDFLAGS) -o $$@
endef

# ============================================================
# Generate all rules
# ============================================================
$(foreach fw,$(FIRMWARES), \
  $(foreach sz,$(SIZES_MB_NORMAL), \
    $(eval $(call make-payload-rule,$(fw),$(sz),main.c,,$(SIZE_MAX_NORMAL_MB))) \
    $(eval $(call make-payload-rule,$(fw),$(sz),main.c,-pro,$(SIZE_MAX_NORMAL_MB)))) \
  $(foreach sz,$(SIZES_MB_BAIKAL), \
    $(eval $(call make-payload-rule,$(fw),$(sz),main-baikal.c,-baikal,$(SIZE_MAX_BAIKAL_MB))) \
    $(eval $(call make-payload-rule,$(fw),$(sz),main-baikal.c,-pro-baikal,$(SIZE_MAX_BAIKAL_MB)))))
