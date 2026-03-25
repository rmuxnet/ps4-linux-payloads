# ============================================================
# PS4 VRAM Payload Generation Makefile
# Configured for maximum 4GB (4096MB) VRAM allocation
# Generates standard, pro, baikal, and pro-baikal payloads
# ============================================================

FIRMWARES = 505 672 700 900 903 960 1000 1050 1100 1102 1150 1200 1250 1300 1302

SIZES_SMALL_MB     := 32 64 128
SIZE_STEP_MB       := 256
SIZE_LINEAR_MIN_MB := 256
SIZE_MAX_NORMAL_MB := 4096
SIZE_MAX_BAIKAL_MB := 4096

gen-range = $(shell seq $(SIZE_LINEAR_MIN_MB) $(SIZE_STEP_MB) $(1))

SIZES_MB_NORMAL := $(SIZES_SMALL_MB) $(call gen-range,$(SIZE_MAX_NORMAL_MB))
SIZES_MB_BAIKAL := $(SIZES_SMALL_MB) $(call gen-range,$(SIZE_MAX_BAIKAL_MB))

CC      = gcc
CFLAGS  = -isystem ../freebsd-headers -nostdinc -nostdlib -fno-stack-protector \
           -static -fPIE -ffreestanding
LDFLAGS = -Wl,-gc-sections

define FW_VER
$(shell printf '%s' '$(1)' | sed 's/\(.*\)\(..\)$$/\1_\2/')
endef

define normal-payloads-for-fw
$(foreach sz,$(SIZES_MB_NORMAL), \
  fw$(1)/payload-$(1)-$(sz)mb.bin \
  fw$(1)/payload-$(1)-$(sz)mb-pro.bin)
endef

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

../lib/lib.a:
	cd ../lib && $(MAKE)

ps4-kexec-%/kexec.bin:
	cd ps4-kexec-$* && $(MAKE)

%.bin: %.elf
	objcopy $< --only-section .text --only-section .data --only-section .bss \
	            --only-section .rodata -O binary $@
	file $@ | fgrep -q '$@: DOS executable (COM)'

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

$(foreach fw,$(FIRMWARES), \
  $(foreach sz,$(SIZES_MB_NORMAL), \
    $(eval $(call make-payload-rule,$(fw),$(sz),main.c,,$(SIZE_MAX_NORMAL_MB))) \
    $(eval $(call make-payload-rule,$(fw),$(sz),main.c,-pro,$(SIZE_MAX_NORMAL_MB)))) \
  $(foreach sz,$(SIZES_MB_BAIKAL), \
    $(eval $(call make-payload-rule,$(fw),$(sz),main-baikal.c,-baikal,$(SIZE_MAX_BAIKAL_MB))) \
    $(eval $(call make-payload-rule,$(fw),$(sz),main-baikal.c,-pro-baikal,$(SIZE_MAX_BAIKAL_MB)))))
