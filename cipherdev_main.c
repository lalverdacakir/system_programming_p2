/*

 * Supports callbacks for the following system calls: open(),release(),read(),write(),ioctl()
 * Supports a Vigenre, as well as a simple Caesar cipher.
 *
 */
#define pr_fmt(fmt) "["KBUILD_MODNAME "]: " fmt
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "cipherdev.h"
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/cred.h>


static int cipherdev_open(struct inode *inode, struct file *filp);
static int cipherdev_release(struct inode *inode, struct file *filp);
static ssize_t cipherdev_write(struct file *filp,const char* buffer, size_t length, loff_t * offset);
long cipherdev_ioctl(struct file *file,unsigned int ioctl_num,unsigned long ioctl_param);
static ssize_t cipherdev_read(struct file* filp,char* buffer,size_t length,loff_t* offset)

//overwrites to the kernel structure file_operations
static struct file_operations cipherdev_fops = {
	.owner = THIS_MODULE,
	.open = cipherdev_open,
	.release = cipherdev_release,
	.write = cipherdev_write,
	.unlocked_ioctl = cipherdev_ioctl,
	.read = cipherdev_read
};





static struct class *cipherdev_class = NULL;
static struct device *cipherdev_device = NULL;
static struct cdev cipherdev_cdev;
static int cipherdev_major;
static struct file_operations cipherdev_fops;
int ret;
char *tempStr;
int sign = 1;
int i,j,length;
char temp[100];
struct semaphore lock;

struct cipher_device_t{
	char message[BUF_LEN];
	int open_count;
	int open_uid;
	int method; //will be deleted
	int mode;
	char key[BUF_LEN];
	int flag; // 0 - unblocked 1-blocked
} cipher_device;


/***************************************************************************
 * Helper functions
 ***************************************************************************/



int vinegere_cipher(char* text){
	pr_info("cipher device : vinegere_cipher\n");
	if(!(*(cipher_device.key))){//Key is not present
		pr_err("cipher device : Key is not set!\n");
		return ERROR_KEY_NOT_SET;
	}
	sign = (cipher_device.mode) ? -1 : 1;
	for(i = 0, j = 0, length = strlen(text); i < length; i++, j++)
    {
		if (j >= strlen(cipher_device.key))
        {
            j = 0;
        }
        if (!isalpha(text[i]))
        {
            j = (j - 1);
        } else{
			text[i] = 'A' + (26 + (text[i] - 'A') + sign * (cipher_device.key[j] - 'A'))%26;
		}
	}
	return SUCCESS;
}
/*
int caesar_cipher(char* text){
	pr_info("cipher device : caesar_cipher\n");
	sign = (cipher_device.mode) ? 1 : -1;
	for(i = 0, length = strlen(text); i < length; i++)
    {
        if (isalpha(text[i]))
        {
			text[i] = 'A' + (26 + (text[i] - 'A') + sign * 3)%26;
		}
	}
	return SUCCESS;
}
*/
/***************************************************************************
 * Module functions
 ***************************************************************************/

//initilize the driver

static int __init cipherdev_init(void)
{
	dev_t dev = 0;

	pr_info("module loaded\n");

	// Go through the stuff required to register the character device.

	// Create a class for the device (classification)
	cipherdev_class = class_create(THIS_MODULE, "cipherdev");
	if (IS_ERR(cipherdev_class)) {
		pr_err("error in class_create(), cannot load module.\n");
		ret = PTR_ERR(cipherdev_class);
		goto err_class_create;
	}

	// Allocate a single minor for the device
	ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
	if (ret) {
		pr_err("error in alloc_chrdev_region(), cannot load module.\n");
		goto err_alloc_chrdev_region;
	}
	// Extract the major number
	cipherdev_major = MAJOR(dev);
	pr_info("cipher Major number is %d\n",cipherdev_major);
	pr_info("Use: mknod /dev/%s c %d 0\n",DEVICE_NAME,cipherdev_major);
	
	// Set up and add the cdev
	cdev_init(&cipherdev_cdev, &cipherdev_fops);
	cipherdev_cdev.owner = THIS_MODULE;
	ret = cdev_add(&cipherdev_cdev, MKDEV(cipherdev_major, 0), 1);
	if (ret) {
		pr_err("error in cdev_add(), cannot load module.\n");
		goto err_cdev_add;
	}


	// Create a device structure
	// This is what lets the system automatically create the /dev entry.
	cipherdev_device = device_create(cipherdev_class, NULL, dev, NULL, "cipher");
	if (IS_ERR(cipherdev_device)) {
		pr_err("error in device_create(), cannot load module.\n");
		ret = PTR_ERR(cipherdev_device);
		goto err_device_create;
	}
	//Init message and key
	memset(cipher_device.message, '\0', BUF_LEN);
	memset(cipher_device.key, '\0', BUF_LEN);
	
	//Init semaphore
	//sema_init(&cipher_device.sem,1);
	//getuid_call = sys_call_table[__NR_getuid];
	sema_init(&lock,1);
	cipher_device.open_count = 0;
	cipher_device.open_uid = 0;
	cipher_device.flag = 0;
	
	//Init Cipher_device
	cipher_device.mode = ENCIPHER;
	
	// If no errors have occured, return 0.
	return 0;

	// Otherwise, we jump to one of these labels and unwind the setup procedure.
err_device_create:
	// Remove cdev
	cdev_del(&cipherdev_cdev);

err_cdev_add:
	// Unregister the chrdev region
	unregister_chrdev_region(MKDEV(cipherdev_major, 0), 1);

err_alloc_chrdev_region:
	// Clean up class
	if (cipherdev_class)
		class_destroy(cipherdev_class);

err_class_create:
	// Nothing to do.
	return 0;
}

static void __exit cipherdev_exit(void)
{
	dev_t dev = MKDEV(cipherdev_major, 0);

	pr_info("module unloaded\n");

	// Destroy device
	device_destroy(cipherdev_class, dev);
	// Remove cdev
	cdev_del(&cipherdev_cdev);
	// Unregister the chrdev region
	unregister_chrdev_region(MKDEV(cipherdev_major, 0), 1);
	// Clean up class
	if (cipherdev_class)
		class_destroy(cipherdev_class);
}

static int cipherdev_open(struct inode *inode, struct file *filp){
	pr_info("cipherdev_open(%p,%p)\n", inode, filp);
	//allow only 1 user
	ret = SUCCESS;
	down_interruptible(&lock);
	if(cipher_device.open_count == 0){
		cipher_device.open_uid =  current_uid().val;
		//Allow open
		cipher_device.open_count++;
	} else if(cipher_device.open_count > 0){
		if(cipher_device.open_uid ==  current_uid().val){
			//Allow open
			cipher_device.open_count++;
		}else{
			ret = ERROR;
		}
	}else {
		ret = ERROR;
	}
	up(&lock);
	pr_info("cipher: opened device\n");
	return ret;
}

static int cipherdev_release(struct inode *inode, struct file *filp)
{
	pr_info("cipherdev_release(%p,%p)\n", inode, filp);
	//Release the process
	//up(&cipher_device.sem);
	down_interruptible(&lock);
	cipher_device.open_count--;
	if(cipher_device.open_count == 0){
		//Reset the user
		cipher_device.open_uid = 0;
	}
	up(&lock);
	pr_info("cipher: released device\n");
	return SUCCESS;
}

static ssize_t cipherdev_read(struct file* filp,char* buffer,size_t length,loff_t* offset){
	pr_info("cipher: reading from device\n");
	if(!cipher_device.flag){
		//Device blocked
		return ERROR_MSG_NOT_IN_BUF;
	}
	cipher_device.flag = UNBLOCK;
	strcpy(temp,cipher_device.message);
	pr_info("cipher device: message stored %s msg\n",temp);
	pr_info("cipher device: Key stored %s msg\n",cipher_device.key);
	
	ret = vinegere_cipher(temp);
	pr_info("cipher device: Cipher message:%s of length:%d/n",temp,(int)strlen(temp));
	if(ret < 0)
	{
		return ret;
	}
	ret= copy_to_user(buffer,temp,length);
	return ret;
}

static ssize_t cipherdev_write(struct file *filp,const char* buffer, size_t length, loff_t * offset){
	pr_info("cipher: writing to device\n");
	if(cipher_device.flag){
		//Device blocked
		return ERROR_MSG_IN_BUF;
	}
	cipher_device.flag = BLOCK;
	ret =  copy_from_user(cipher_device.message,buffer,length);
	return ret;
}

int check_if_mesg_exists(struct file *file){
	if(*(cipher_device.message)){
		return 1;
	}
	return 0;
}

long cipherdev_ioctl(struct file *file,unsigned int ioctl_num,unsigned long ioctl_param){
	pr_info("cipher ioctl: File:%p IOCTL:%d\n",file,ioctl_num);
	
	switch (ioctl_num) {
		
		case IOCTL_SET_MODE:
			if(cipher_device.flag){
				//Device blocked
				return ERROR_MSG_IN_BUF;
			}
			cipher_device.mode = ioctl_param;
			pr_info("cipher ioctl: Mode set as: %d\n",cipher_device.mode);
			break;
		case IOCTL_GET_MODE:
			pr_info("cipher ioctl: Mode get as: %d\n",cipher_device.mode);
			return cipher_device.mode;
			break;
		case IOCTL_SET_KEY:
			if(cipher_device.flag){
				//Device blocked
				return ERROR_MSG_IN_BUF;
			}
			ret =  copy_from_user(temp,(char *)ioctl_param,BUF_LEN);
			
			
			strcpy(cipher_device.key,temp);
			return SUCCESS;
			break;
		case IOCTL_GET_KEY:
			if(!(*(cipher_device.key))){//Key is not present
				pr_err("cipher device : Key is not set!\n");
				return ERROR_KEY_NOT_SET;
			}
			ret =  copy_to_user((char *)ioctl_param,cipher_device.key,BUF_LEN);
			pr_info("cipher ioctl: Get Key: %s\n",(char *)ioctl_param);
			return ret;
			break;
		case IOCTL_CLEAR_CIPHER:
			cipher_device.flag = UNBLOCK;
			memset(cipher_device.message, '\0', BUF_LEN);
			return SUCCESS;
		default :
			pr_err("cipher device: Incorrect IOCTL_NUMBER: %d\n",ioctl_num);
			return ERROR;
	}

	return SUCCESS;
}


//initilize the module
module_init(cipherdev_init);
//exit the module
module_exit(cipherdev_exit);

//büyük ihtimal sil
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abhijit Shanbhag <abhijit.shanbhag@rutgers.edu>");
MODULE_DESCRIPTION("Pseudo-Character Driver for Cipher Processing");


//make cipherdev to viegeneredev