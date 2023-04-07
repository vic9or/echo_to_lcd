# echo_to_lcd

Check dmesg for major number  
create character device with command:  
mknod /dev/"device_name" c "The assigned major number" 0  
make sure you give it the appropriate permissions  
whatever you echo into the character file will be displayed  

Demo: https://www.youtube.com/watch?v=tsvJ9Zf6ec0
