
#include "cipherdev.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>	
#include <sys/ioctl.h>
#include <string.h>

#define DEVICE "/dev/cipher"
//Commands
#define METHOD "method"
#define METHOD_VINEGERE "vigenere"
#define METHOD_CAESAR "caesar"
#define KEY "key"
#define MODE "mode"
#define MODE_ENCIPHER "encipher"
#define MODE_DECIPHER "decipher"
#define CLEAR "clear"
#define WRITE "write"
#define READ "read"

#define ERROR_METHOD_US -20
#define ERROR_DEV_LOCK -21
#define ERROR_MODE_US -22
#define ERROR_CLEAR_US -23
#define ERROR_KEY_US -24
#define ERROR_WRITE_US -25
#define ERROR_READ_US -26
#define ERROR_CIPHERCTL_US -27
/***************************************************************************
 *  Helper function
 ***************************************************************************/
void print_err(int err_num){
	switch(err_num){
		case ERROR:
			printf("ERROR!\n");
			break;
		case ERROR_KEY_NOT_SET:
			printf("Key not set\n");
			break;
		
		case ERROR_DEV_LOCK:
			printf("Either %s does not exist or is locked by another process\n",DEVICE);
			break;
		case ERROR_MODE_US:
			printf("Usage: cipherctl mode [encipher | decipher]\n");
			break;
		case ERROR_MSG_IN_BUF:
			printf("ERROR Message in buffer!\n");
			break;
		case ERROR_MSG_NOT_IN_BUF:
			printf("ERROR Message not in buffer!\n");
			break;
		case ERROR_KEY_NOT_ALPHABET:
			printf("ERROR Key does not contain alphabet!\n");
			break;
		case ERROR_CLEAR_US:
			printf("Usage: cipherctl clear\n");
			break;
		case ERROR_KEY_US:
			printf("Usage: cipherctl key [key]\n");
			break;
		case ERROR_WRITE_US:
			printf("Usage: cipherctl write [message]\n");
			break;
		case ERROR_READ_US:
			printf("Usage: cipherctl read\n");
			break;
		case ERROR_CIPHERCTL_US:
			printf("Invalid usage for cipherctl\n");
			printf("cipherctl key [key] - Set key.\n");
			printf("cipherctl mode [encipher | decipher] - Set operation mode.");
			printf("cipherctl clear - Drop any message pending in the driver.\n");
			printf("cipherctl write [message] - Encipher/decipher a message.\n");
			printf("cipherctl read - Read the result of an encipher/decipher operation.\n");
			break;
		default: printf("Unknown ERROR!\n");
	}
	exit(-1);
}

void ioctl_set_mode(int file_desc, int mode)
{
	int ret_val;
	ret_val = ioctl(file_desc, IOCTL_SET_MODE, mode);
	if (ret_val < 0) {
		print_err(ERROR_MSG_IN_BUF);
	}
	printf("Set Mode success\n");
}

void ioctl_get_mode(int fp)
{
	int ret_val;
	ret_val = ioctl(fp, IOCTL_GET_MODE, 0);
	if (ret_val < 0) {
		print_err(ret_val);
	}
	switch(ret_val){
		case ENCIPHER:
			printf("Get Mode : %s\n",MODE_ENCIPHER);
			break;
		case DECIPHER:
			printf("Get Mode : %s\n",MODE_DECIPHER);
			break;
	}
}
void ioctl_set_key(int fp, const char* key)
{
	int ret_val;
	ret_val = ioctl(fp, IOCTL_SET_KEY, key);
	if (ret_val < 0) {
		print_err(ret_val);
	}
	printf("Set Key success\n");
}
void ioctl_get_key(int fp)
{
	char key[BUF_LEN];
	int ret_val;
	ret_val = ioctl(fp, IOCTL_GET_KEY, key);
	if (ret_val < 0) {
		print_err(ERROR_KEY_NOT_SET);
	}
	printf("Get Key: %s\n",key);
}
void ioctl_write_msg(int fp, const char* mesg){
	int ret_val= write(fp, mesg, strlen(mesg));
	if (ret_val < 0) {
		print_err(ERROR_MSG_IN_BUF);
	}
	printf("Write successfull\n");
}

void ioctl_read_msg(int fp){
	int ret_val;
	char mesg[BUF_LEN];
	ret_val = read(fp, mesg, BUF_LEN);
	if (ret_val < 0) {
		print_err(ret_val);
	}
	printf("Message: %s\n",mesg);
}
void ioctl_clear_msg(int fp){
	int ret_val = ioctl(fp, IOCTL_CLEAR_CIPHER, 0);
	if (ret_val < 0) {
		print_err(ret_val);
	}
	printf("Clear successfull\n");
}
/***************************************************************************
 *  Main Controller
 ***************************************************************************/
int main(int argc, char **argv) {
	int fp;
	fp = open(DEVICE, O_RDWR);
	if(fp<0){
		print_err(ERROR_DEV_LOCK);
	}
	if (argc < 2) {
		print_err(ERROR_CIPHERCTL_US);
	}

	//Set/Get Mode
	else if (strcmp(argv[1], MODE) == 0) {
		if (argc > 3) {
			print_err(ERROR_MODE_US);
		}
		if (argc == 2) {
			//print mode
			ioctl_get_mode(fp);
		}else{
			//Set mode
			if(strcmp(argv[2], MODE_ENCIPHER) == 0){
				ioctl_set_mode(fp,ENCIPHER);
			} else if(strcmp(argv[2], MODE_DECIPHER) == 0){
				ioctl_set_mode(fp,DECIPHER);
			} else {
				print_err(ERROR_MODE_US);
			}
		}
	}
	// Set/Get Key
	else if (strcmp(argv[1], KEY) == 0) {
		if (argc > 3) {
			print_err(ERROR_KEY_US);
		}
		if (argc == 2) {
			//print key
			ioctl_get_key(fp);
		}else{
			//Set key
			ioctl_set_key(fp,argv[2]);			
		}
	}
	//Clear msg
	else if (strcmp(argv[1], CLEAR) == 0) {
		if (argc != 2) {
			print_err(ERROR_CLEAR_US);
		}else{
			ioctl_clear_msg(fp);
		}
	}
	//Write msg
	else if (strcmp(argv[1], WRITE) == 0) {
		if (argc != 3) {
			print_err(ERROR_WRITE_US);
		}else{
			ioctl_write_msg(fp,argv[2]);
		}
	}
	//Read msg
	else if (strcmp(argv[1], READ) == 0) {
		if (argc != 2) {
			print_err(ERROR_READ_US);
		}else{
			ioctl_read_msg(fp);
		}
	}
	else{
		print_err(ERROR_CIPHERCTL_US);
	}

	return 0;
}
