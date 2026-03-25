# ============================================================
# PS4 VRAM Payload Generation Makefile
# Configured for maximum 4GB (4096MB) VRAM allocation
# Generates standard, pro, baikal, and pro-baikal AIO payloads
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

PAYLOADS = $(foreach sz,$(SIZES_MB_NORMAL), \
             aio/payload-$(sz)mb.bin \
             aio/payload-$(sz)mb-pro.bin) \
           $(foreach sz,$(SIZES_MB_BAIKAL), \
             aio/payload-$(sz)mb-baikal.bin \
             aio/payload-$(sz)mb-pro-baikal.bin)

.PHONY: all clean
all: $(PAYLOADS)

clean:
	rm -rf fw*/* aio/*
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

define make-kmain-rule
fw$(1)/kernel_main_$(1)$(2).o: kernel_main.c ps4-kexec-$(1)$(2)/kexec.bin
	@mkdir -p fw$(1)
	$(CC) $(CFLAGS) -c kernel_main.c \
	    -D__$(call FW_VER,$(1))__ -DPS4_$(call FW_VER,$(1)) -DFW_SUFFIX=$(1) \
	    -DKEXEC_PATH=ps4-kexec-$(1)$(2)/kexec.bin \
	    -o $$@
endef

$(foreach fw,$(FIRMWARES), \
  $(eval $(call make-kmain-rule,$(fw),)) \
  $(eval $(call make-kmain-rule,$(fw),-pro)) \
  $(eval $(call make-kmain-rule,$(fw),-baikal)) \
  $(eval $(call make-kmain-rule,$(fw),-pro-baikal)) \
)

define make-aio-rule
aio/payload-$(1)mb$(2).elf: ../lib/lib.a main.c $(foreach fw,$(FIRMWARES),fw$(fw)/kernel_main_$(fw)$(2).o)
	@mkdir -p aio
	$(CC) $(CFLAGS) ../lib/lib.a main.c \
	    $(foreach fw,$(FIRMWARES),fw$(fw)/kernel_main_$(fw)$(2).o) \
	    -DVRAM_MB_DEFAULT=$(1) \
	    -DVRAM_MB_MIN=$(SIZE_LINEAR_MIN_MB) \
	    -DVRAM_MB_MAX=$(3) \
	    $(LDFLAGS) -o $$@
endef

$(foreach sz,$(SIZES_MB_NORMAL), \
  $(eval $(call make-aio-rule,$(sz),,$(SIZE_MAX_NORMAL_MB))) \
  $(eval $(call make-aio-rule,$(sz),-pro,$(SIZE_MAX_NORMAL_MB))) \
)

$(foreach sz,$(SIZES_MB_BAIKAL), \
  $(eval $(call make-aio-rule,$(sz),-baikal,$(SIZE_MAX_BAIKAL_MB))) \
  $(eval $(call make-aio-rule,$(sz),-pro-baikal,$(SIZE_MAX_BAIKAL_MB))) \
)
