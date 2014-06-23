all:	ssh-otp gen-key

ssh-otp: ssh-otp.c
	$(CC) -o $@ $< -lcrypto
gen-key: generate-key.c
	$(CC) -o $@ $<
