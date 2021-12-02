/*
 * Simple test program for testing Vinegere and Caesar cipher in user space
 *
 * Author:  Abhijit Shanbhag<abhijit.shanbhag@rutgers.edu>
 * 		    Priyanka Dhingra<pd374@scarletmail.rutgers.edu>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sign;
int i,j,length;
int vinegere_cipher(char* text,char* key,int mode){
	sign = (mode) ? -1 : 1;
	for(i = 0, j = 0, length = strlen(text); i < length; i++, j++)
    {
		if (j >= strlen(key))
        {
            j = 0;
        }
        if (!isalpha(text[i]))
        {
            j = (j - 1);
        } else{
			text[i] = 'A' + (26 + (text[i] - 'A') + sign * (key[j] - 'A'))%26;
		}
	}
	
	return 0;
}



int main(){
	
	int mode;
	char text[100],key[100];
	
	printf("Mode?: ");
	scanf("%d",&mode);
	
	//printf("Key?: ");
	//scanf("%s",&key);
		
	printf("Text?: ");
	scanf("%s",&text);
	
	vinegere_cipher(text,key,mode);
	


	printf("Cipher text= %s\n",text);
	
	return 0;
}
