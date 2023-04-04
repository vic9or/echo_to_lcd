#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "lcd_driver"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Victor");
MODULE_DESCRIPTION("Simply prints what is written into the device file");

static int major_num;
static int device_open = 0; //ensures only one instance if the device is open
unsigned int gpio_nums[] = {
	6, //RS pin
	0, //E pin
	11, //D0 pin
	9, //D1
	10, //D2
	22, //D3
	27, //D4
	4, //D5
	3, //D6
	2, //D7
};

static char lcd_buffer[18];


#define PIN_RS gpio_nums[0]
#define PIN_E gpio_nums[1]

void tap_PIN_E(void){
	msleep(100);
	gpio_set_value(PIN_E,1);
	msleep(100);
	gpio_set_value(PIN_E,0);
	msleep(100);
}

void send_byte_to_pins(char data){
	int i;
	for (i=0; i<8; i++){
		gpio_set_value(gpio_nums[i+2],((data & (1<<i)) >> i));
	}
	tap_PIN_E();

	for (i=0; i<8; i++){
		gpio_set_value(gpio_nums[i+2], 0);
	}
}

void print_char_to_lcd(char data){
	gpio_set_value(PIN_RS, 1);
	msleep(100);
	send_byte_to_pins(data);
}

void send_instruction_to_lcd(uint8_t data){
	gpio_set_value(PIN_RS, 0);
	msleep(100);
	send_byte_to_pins(data);
}

static int lcd_open(struct inode *inode, struct file *file){
	if (device_open){
		return EBUSY;
	}
	device_open++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int lcd_close(struct inode *device_file, struct file *instance){
	device_open--;
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t lcd_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs){
	int copy_size = min(count, sizeof(lcd_buffer));
	int unread_amount = copy_from_user(lcd_buffer, user_buffer, copy_size);
	send_instruction_to_lcd(0x1);
	int i;
	for(i=0; i<copy_size-1;i++){
		print_char_to_lcd(lcd_buffer[i]);
	}
	return copy_size - unread_amount;

}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = lcd_open,
	.release = lcd_close,
	.write = lcd_write
};


static int __init lcd_init(void){
	major_num = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_num < 0){
		printk(KERN_ALERT "Could not register device\n");
		return major_num;
	}
	printk(KERN_INFO "Device registered with major %d\n", major_num);

	char *gpio_labels[] = {"RS", "E", "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};
	int i;
	//Here we do gpio_request on all the pins
	for(i=0; i<10; i++){
		if(gpio_request(gpio_nums[i], gpio_labels[i])){
			printk(KERN_ALERT "Error in gpio_request\n");
			goto gpio_request_error;
		}
	}
	//Here we set pin directions to output
	for(i=0; i<10; i++){
		if(gpio_direction_output(gpio_nums[i], 0)){
			printk(KERN_ALERT "Error in setting gpio pin directions\n");
			goto gpio_direction_error;
		}
	}
	
	send_instruction_to_lcd(0x30);
	send_instruction_to_lcd(0xf);
	send_instruction_to_lcd(0x1);

	char greetings[] = "LCD ready...";
	for(i=0; i<sizeof(greetings)-1;i++){
		print_char_to_lcd(greetings[i]);
	}
	return 0;

gpio_direction_error:
	i = 9;
gpio_request_error:
	for(;i>=0; i--){
		gpio_free(gpio_nums[i]);
	}
	return -1;
}
static void __exit lcd_release(void){
	printk(KERN_INFO "releasing device\n");
	send_instruction_to_lcd(0x1);
	int i;
	for(i=0; i<10; i++){
		gpio_free(gpio_nums[i]);
	}
	unregister_chrdev(major_num, DEVICE_NAME);
}
module_init(lcd_init);
module_exit(lcd_release);
