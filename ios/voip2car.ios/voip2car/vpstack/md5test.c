#include <stdio.h>
#include <string.h>
#include "md5.h"

/*char *nonce="457c402495f80c46bce94d074bbfdd5bfb462ed5";
char *uri="sip:voip.eutelia.it";
char *realm="voip.eutelia.it";
char *username="0409896429";
char *response="07bac702d8f3151486340906fae3b048";
char *cnonce="f704c96d12cf91d5cecccbc34255fdbb";*/

char *nonce="457c29c5042a94dc6d522a3d717e4708b83d3e0f", *uri="sip:voip.eutelia.it", *realm="voip.eutelia.it",
	*username="0409896429", *response="24003ecafcf1c4197a29fdb3be04bacf", *cnonce="8668b8cf42f2841d5183d199179822bc";
char *password="63y2jeoY";


#define TOHEX(a) ((a) >= 10 ? 'a' + (a) - 10 : '0' + (a))
#define TOBIN(a) ((a) >= 'a' ? (a) - 'a' + 10 : (a) >= 'A' ? (a) - 'A' + 10 : (a) - '0')

void BinaryToHex(unsigned char *binary, int len, char *hex)
{
	int i;

	for(i = 0; i < len; i++)
		sprintf(hex + 2 * i, "%c%c", TOHEX(binary[i] / 16), TOHEX(binary[i] % 16));
	hex[2 * i] = 0;
}

static void MD5(const char *s, char *digest)
{
	MD5_CTX ctx;
	unsigned char bdigest[16];

	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)s, strlen(s));
	MD5Final(bdigest, &ctx);
	BinaryToHex(bdigest, 16, digest);
}

void main()
{
	char s[300], digest1[33], digest2[33];

	strcpy(s, username);
	strcat(s, ":");
	strcat(s, realm);
	strcat(s, ":");
	strcat(s, password);
	MD5(s, digest1);
	printf("%s\n%s\n", s, digest1);
	strcpy(s, "REGISTER:");
	strcat(s, uri);
	MD5(s, digest2);
	printf("%s\n%s\n", s, digest2);
	strcpy(s, digest1);
	strcat(s, ":");
	strcat(s, nonce);
	strcat(s, ":");
	sprintf(s + strlen(s), "00000001:%s:auth:", cnonce);
	strcat(s, digest2);
	MD5(s, digest1);
	printf("%s\n%s\n", s, digest1);
}
