#ifndef _PDU_H_ICLUDED_
#define _PDU_H_INCLUDED_

typedef struct {
	char snum[20], text[161];
	time_t time, deliverytime;
	unsigned char deliveryref;
	char report, submit;
} SMS;

#ifdef __cplusplus
extern "C" {
#endif

int decodepdu(const char *pdu, SMS *sms);
int encodepdu(const char *number, const char *text, int report, char *pdu);

#ifdef __cplusplus
}
#endif

#endif
