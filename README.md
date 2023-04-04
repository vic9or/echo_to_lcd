# echo_to_lcd

Check dmesg for major number\n
create character device with command:\n
mknod /dev/"device_name" -c "The assigned major number" 0\n

whatever you echo into the character file will be displayed
