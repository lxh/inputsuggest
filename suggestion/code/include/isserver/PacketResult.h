#ifndef _PACKET_RESULT_H_
#define _PACKET_RESULT_H_
//ר�����ڷ�װ�������
#include "isserver/TreeArray.h"
#include "isserver/TopResult.h"

class CPacketResult {
public:
	CPacketResult();
	string Packet(TopResult & topResult);

};


#endif
