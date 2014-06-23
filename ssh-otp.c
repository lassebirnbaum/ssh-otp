#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include <openssl/hmac.h>

#define MAX_SKEW 1

int b32decode(const char *s, unsigned char *b)
{
  int i;

  memset(b, 0, 10);
  for (i = 0; i < 16; i++) {
    unsigned char x;
    if (isalpha(s[i])) {
      x = toupper(s[i]) - 'A';
    } else if (s[i] >= '2' && s[i] <= '7') {
      x = s[i] - '2' + 26;
    } else {
      return 0;
    }
    b[5*i / 8] |= (x << 3) >> (5*i % 8);
    if (5*i % 8 >= 4) {
      b[5*i / 8 + 1] |= x << (3 + 8 - (5*i % 8));
    }
  }
  return 1;
}

void hotp(const unsigned char *sbytes, time_t movingFactor, char *code)
{
  unsigned char data[8];
  int i, offset, bin_code, otp;

  for (i = 0; i < 8; i++) {
    data[i] = i < 4 ? 0 : movingFactor >> (56 - 8*i);
  }
  unsigned char *r = HMAC(EVP_sha1(), sbytes, 10, data, sizeof(data), NULL, NULL);
  offset = r[19] & 0xf;
  bin_code = ((r[offset] << 24) | (r[offset+1] << 16) | (r[offset+2] << 8) | r[offset+3]) & 0x7fffffff;
  otp = bin_code % 1000000;
  sprintf(code, "%06d", otp);
}

void proceed()
{
  syslog(LOG_INFO, "Login accepted for %s",getenv("LOGNAME"));
  if (getenv("SSH_ORIGINAL_COMMAND") != NULL) {
    execl(getenv("SSH_ORIGINAL_COMMAND"), "-", NULL);
  } else {
    execl(getenv("SHELL"), "-", NULL);
  }
}

int main(int argc, char *argv[])
{
  int i;
  unsigned char sbytes[10];
  char code[7];
  char* input;
  time_t now;

  syslog(LOG_INFO, "Login attempt for %s",getenv("LOGNAME"));
  if (argc < 2) {
   	syslog(LOG_ERR, "No secret given for %s",getenv("LOGNAME"));
    exit(1);
  }

  if (!b32decode(argv[1], sbytes)) {
    syslog(LOG_ERR, "Secret not base32 format for %s",getenv("LOGNAME"));
    exit(1);
  }

  input = getpass("Enter validation code: ");
  if (input == NULL)  {
    syslog(LOG_ERR, "No validation code give given for %s",getenv("LOGNAME"));
    exit(1);
  }
  
  now = time(NULL);
  for (i = 0; i <= MAX_SKEW; i++) {
    hotp(sbytes, now / 30 + i, code);
    if (strncmp(input, code, 6) == 0) {
      proceed();
    }
    hotp(sbytes, now / 30 - i, code);
    if (strncmp(input, code, 6) == 0) {
      proceed();
    }
  }
  syslog(LOG_ALERT, "Invalid attempt for %s",getenv("LOGNAME"));
  fprintf(stderr, "Invalid validation key\n");

  return 1;
}
