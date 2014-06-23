#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>

char *gen_key();

int main(int argc, char *argv[]) {

	char* key = gen_key();
	struct utsname un;

	uname(&un); // Might check return value here (non-0 = failure)

	printf("QR Code url:\n https://chart.googleapis.com/chart?chs=200x200&chld=M|0&cht=qr&chl=otpauth://totp/%s?secret=%s\n\n",un.nodename,key);
	printf("SSH authorized_keys prefix:\n command=/path/to/ssh-otp %s ssh-dsa AAA...zzz me@example.com\n\n",key);
	return 0;
}

char *gen_key() {
	const char charset[] = "abcdefghijklmnopqrstuvwxyz234567";
	int max = (sizeof charset - 1);
	int n;
	char* str = malloc(sizeof(char) * (17));
	srand(time(0));    
	for (n = 0; n < 16; n++) {
		str[n] = charset[rand()%max];
	}
	str[16] = '\0';
	return str;
}