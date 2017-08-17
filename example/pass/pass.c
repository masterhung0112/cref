#include <stdio.h>
#include <string.h>

#define PASSWORD_SIZE 100
#define PASSWORD "helloWORLD"

int main()
{
	int count = 0; 
	char buf[PASSWORD_SIZE];
	for (;;) {
		printf("Enter password: ");
		fgets(&buf[0], PASSWORD_SIZE, stdin);
		if (strcmp(&buf[0], PASSWORD)) {
			printf("Wrong password\n");
		} else {
			printf("Correct password\n");
			break;
		}
		if (++count>3) return -1;
	}
	return 0;
}
	
	
