# ![PS4](https://img.shields.io/badge/-PS4-003791?style=flat&logo=PlayStation) Linux Payloads for FW 5.05 - 13.02
(Southbridge: Aeolia & Belize(2) & Baikal)

**Linux-Payloads** kexec for PlayStation 4.

The host with precompiled Linux payloads only works with GoldHEN v2.4b18.5/v2.4b18.6 BinLoader. Just open the web browser and cache the host—it will also work offline too.
[PSFree-Enhanced](https://arabpixel.github.io/PSFree-Enhanced) [▶️ click](https://www.youtube.com/watch?v=T3rXMWw6nIM)

you'll find Linux payloads for your firmware, along with some extra payloads. The rest are already included in GoldHEN.

 
# Supported Firmwares

*   FW 5.05 ✅
*   FW 6.72 ✅
*   FW 7.00 / 7.02(?) ✅
*   FW 9.00 ✅ 
*   FW 9.03 / 9.04 ✅ 
*   FW 9.50 / 9.51 / 9.60 ✅ 
*   FW 10.00 / 10.01 ✅ 
*   FW 10.50 / 10.70 / 10.71 ✅ 
*   FW 11.00 ✅ 
*   FW 11.02 ✅ 
*   FW 11.50 / 11.52 ✅
*   FW 12.00 / 12.02 ✅ 
*   FW 12.50 / 12.52 ✅
*   FW 13.00 ✅
*   13.02(?) ✅


## New
- Automatic boot files placement – The kernel (bzImage) and initramfs.cpio.gz are now automatically copied to /data/linux/boot on the internal drive from the external fat32 partition. Why? No external drive is needed to boot into the rescue shell, only first time.

- RTC time passed to initramfs – The current time from OrbisOS is added to the kernel command line (time=CURRENTTIME), ensuring the correct time is set at boot instead of defaulting to 1970, even if the RTC hardware cannot be read directly. Why? why not. but you need a prepared initramfs that reads the Time from the cmdline and set the time.

- Sub-1GB VRAM payloads – Added 256mb and 512mb payload sizes for PS4 used as a headless server where GPU memory is largely unused. Set `vram.txt` to `256` or `512` (MB) to use them.


## Info 
the internal path by default is at: ``/data/linux/boot``  
the rest is coming from the initramfs.cpio.gz init configuration
so you can go into the rescue shell without a usb stick just upload the ``bzImage`` and ``initramfs.cpio.gz`` over ftp to your PS4 Drive
``/data/linux/boot/[bzimage,initramfs.cpio.gz]`` 
and of course it will work too with a USB / HDD Drive.  USB have highest prio so if a USB is connected he will  use this bzImage and initramfs.cpio.gz from there 

bootargs.txt you can also add a textfile for changing the cmdline.

### vram.txt
Control VRAM size via a plain text file containing a number in **MB** (not GB).

| vram.txt value | VRAM allocated | Payload suffix |
|:-:|:-:|:-:|
| `256` | 256 MB | `-256mb` |
| `512` | 512 MB | `-512mb` |
| `1024` | 1 GB | `-1gb` |
| `2048` | 2 GB | `-2gb` |
| `3072` | 3 GB | `-3gb` |
| `4096` | 4 GB | `-4gb` |

Default is 1024 MB (1 GB) if vram.txt is missing or invalid. Minimum is 256 MB.

> **Note:** 128 MB is not supported — it causes an immediate crash.

## Server Use (256MB / 512MB VRAM)

- **Why 256MB and 512MB?** The idea behind these low VRAM payloads is for users who are repurposing the PS4 as a server for RAM and CPU intensive tasks. If you're just running server stuff headless, you don't need 1GB+ of memory wasted on the GPU. Using these payloads frees up that shared unified memory to be used as regular system RAM instead. 

- **What happened to 128MB?** we tested 128mb but it just crashes the console on boot. 256mb is the absolute lowest safe amount you can give the GPU.

- **How to use:** Just run the 256mb or 512mb payloads. You can also just add `256` or `512` into your `vram.txt` file to set it manually. Don't use these if you plan on gaming or using heavy desktop graphics!

## Note 
* With new GoldHEN Version v2.4b18.5/v2.4b18.6 use .elf`s Files instead of .bin it works better 100% Success. 

* Don`t use PRO Payloads for Phat or Slim. 

* if you need UART just add this to the cmdline i have disabled .... just for now on newer Kernel it doesnt work.

Aeolia/Belize: ``console=uart8250,mmio32,0xd0340000``

Baikal: ``console=uart8250,mmio32,0xC890E000``


## Compile
    git clone https://github.com/ps4boot/ps4-linux-payloads
    cd ps4-linux-payloads/linux
    make


## Credits
* marcan, shuffle2, eeply, rancido, valeryy, ethylamine, Joonie86 (Linux)
* sleirsgoevy (for the script and better exploit FW 672) 
* AlAzif / KiwiDog / Specter / Celesteblue / ChendoChap / zecoxao / SocraticBliss / ctn123 (Exploit and Fun Stuff for the Console)
* bestpig / EchoStretch / EinTim23 / tihmstar / ArabPixel (Offsets)
* others ...
