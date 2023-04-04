# echo_to_lcd

Check dmesg for major number  
create character device with command:  
mknod /dev/"device_name" -c "The assigned major number" 0  
  
whatever you echo into the character file will be displayed  
