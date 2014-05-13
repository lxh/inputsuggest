#ifndef _PACKET_RESULT_H_
#define _PACKET_RESULT_H_
//专门用于封装结果的类
#include "isserver/TreeArray.h"
#include "isserver/TopResult.h"

class CPacketResult {
public:
	CPacketResult();
	string Packet(TopResult & topResult);

};


#endif
