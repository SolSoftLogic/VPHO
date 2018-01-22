#ifdef __cplusplus
extern "C" {
#endif

extern int g711_usemulaw;
void G711EncodeBlock(short *input, unsigned char *output, int blocklen);
void G711EncodeBlockNoReverse(short *input, unsigned char *output, int blocklen);
void G711DecodeBlock(unsigned char *input, short *output, int blocklen);
void G711DecodeBlockNoReverse(unsigned char *input, short *output, int blocklen);
void G711Mulaw2Alaw(unsigned char *input, unsigned char *output, int blocklen);
void G711ReverseBlock(unsigned char *input, unsigned char *output, int blocklen);
void G711DecodeMuLaw(unsigned char *input, short *output, int blocklen);
void G711DecodeALaw(unsigned char *input, short *output, int blocklen);

#ifdef __cplusplus
}
#endif
