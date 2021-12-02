
#ifndef CIPHERDEV_H
#define CIPHERDEV_H
#include <linux/ioctl.h>

#define CIPHERDEV_MAGIC_NUMBER 'k'
#define DEVICE_NAME "cipher"
#define DEFAULT -1

// Cipher MODES
#define ENCIPHER 0
#define DECIPHER 1

// Cipher MODES
#define IOCTL_SET_MODE _IOR(CIPHERDEV_MAGIC_NUMBER, 1, int)
#define IOCTL_GET_MODE _IOW(CIPHERDEV_MAGIC_NUMBER, 2, int)
// Cipher Key
#define IOCTL_SET_KEY _IOR(CIPHERDEV_MAGIC_NUMBER, 3, char*)
#define IOCTL_GET_KEY _IOR(CIPHERDEV_MAGIC_NUMBER, 4, char*)
// Clear cipher
#define IOCTL_CLEAR_CIPHER _IOWR(CIPHERDEV_MAGIC_NUMBER, 5, int)

#define BUF_LEN 100
#define SUCCESS 0
#define BLOCK 1
#define UNBLOCK 0

#define ERROR -1
#define ERROR_KEY_NOT_SET -2
#define ERROR_MSG_IN_BUF -3
#define ERROR_MSG_NOT_IN_BUF -4
#define ERROR_KEY_NOT_ALPHABET -5

#endif
