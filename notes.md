[youtube](https://www.youtube.com/watch?v=mTJ_vKlMS_4)
[git](https://github.com/ShawnHymel/introduction-to-zephyr)
[how to debug core dump](https://developer.espressif.com/blog/core-dump-a-powerful-tool-for-debugging-programs-in-zephyr-with-esp32-boards/?utm_source=chatgpt.com)

CONFIG_DEBUG_COREDUMP=y 
CONFIG_DEBUG_COREDUMP_BACKEND_LOGGING=y 

    ../zephyr/scripts/coredump/coredump_serial_log_parser.py coredump.log coredump.bin 
1 terminal 
    ../zephyr/scripts/coredump/coredump_gdbserver.py build/zephyr/zephyr.elf coredump.bin -v
2 terminal 
    ~/zephyr-sdk-0.17.0/xtensa-espressif_esp32_zephyr-elf/bin/xtensa-espressif_esp32_zephyr-elf-gdb build/zephyr/zephyr.elf 
    (gdb) target remote localhost:1234 




# how to debug 
```
    openocd --version

```

- First terminal: run openocd (sometimes you have to run it with sudo)
```
sudo openocd -f board/esp32s3-builtin.cfg
```

- Second termina: run espressif gdb
```
~/zephyr-sdk-0.17.0/xtensa-espressif_esp32s3_zephyr-elf/bin/xtensa-espressif_esp32s3_zephyr-elf-gdb build/zephyr/zephyr.elf 
```
```
target remote localhost:3333
```
  
- gdb commands 
```
mon reset halt
layout src
i b
delete
```