/*
encoding:gbk
	���: ������ʾ������ĵ����ڵ�Ľṹ

�ýṹĬ�����˼�������:
	1. ���ݵ�����Ϊ�����ֽ�
	2. �ַ����ĳ����Ϊ255(����ǰ׺����)
	3. ���ݵ�Ȩ�ط�ΧΪ10bit(0-1023)
	4. ��������ò�Ҫ����һ��(���ǧ������)->Ҫ����ǧ�����������������Ҫ�ָ�
	5. ���ݵ���չ����������ܴ�С1G����(һ���ԶԶС�ڸ�ֵ��)

qq: 315753857
*/

#ifndef _XH_TREE_STRUCT_H_
#define _XH_TREE_STRUCT_H_
#include <cstring>

//�Ƿ�ѹ���ڵ�
#define _NOT_WITH_COMPACK_ST_ 0

//����ѹ���Ľ��ĸ���
#define CAN_COMPACK_NOOD_NUMBER 5

//�ڵ�Ȩ�ص����ֵ[Ȩ�ط�Χ0-1023]
#define MAX_WEIGHT_CHINESE     0x3ff

//XHRbNode->usHuaʹ��
//XH_TREE_HUA_INFO_WIFE  -> �Ƿ����wife
//XH_TREE_HUA_INFO_MID   -> �Ƿ�����м亢��
//XH_TREE_HUA_INFO_LEFT  -> �Ƿ��������
//XH_TREE_HUA_INFO_RIGHT -> �Ƿ�����Һ���
//XH_TREE_HUA_INFO_SSELF -> �Ƿ����spellself->ָ���Լ�����Ľṹ��Ϣ
#define XH_TREE_HUA_INFO_WIFE  0x01
#define XH_TREE_HUA_INFO_MID   0x02
#define XH_TREE_HUA_INFO_LEFT  0x04
#define XH_TREE_HUA_INFO_RIGHT 0x08
#define XH_TREE_HUA_INFO_SSELF 0x10

//��ȡƫ��λ��                                 // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
const static unsigned char g_sucCompackPos[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

//��Ს̬��ֻ���ĺ��Ӳ���ϵ��ĸ
//���ڲ�ѯʹ�õĺ����(�򻯰�)
//�ýṹ�м�����Ҫע��
//1. left,mid,rightָ����ǵ�ַ�����������±�
//2. �����������棬����data,mid,left,right,ss(spellself)����ȫ���ڣ�Ϊ�˽�ʡ�ռ�
//3. �洢���ڵ�ʱ����Ҫ��0��ַ��ʼ����Ϊ0��ַ��������ģ�����û������
typedef struct _XHRbNode {
    char cByteDif;             //��ͬ�ֽڵ�����
    unsigned char cPosDif;     //��ͬ�ֽڵ�λ��

	unsigned short usWPeer:1;     //�Ƿ��гɶԵ���Ϣ�ڵ����(����ǰ�ڵ���ĸ���Դ�Сд,��ĸһ���Ľڵ�)-->>ֻ��ȫƴ��ʹ��
	unsigned short usHua:5;       //���ж�����Ϣ->�����XH_TREE_HUA_INFO_XXXX
	unsigned short usWeight:10;   //�ڵ��Ȩ��

	//���漸���ڵ㶼���ܻᱻѹ��
    unsigned int  uiWife;      //�ڵ�����ݲ���
    unsigned int  uiMid;       //�к���  ----�к��ӷŵ�����ǰ�棬��Ϊ�к�����Ҫȥ���ݷ��ʶ�
    unsigned int  uiLeft;      //����
    unsigned int  uiRight;     //�Һ���
    unsigned int  uiSpellSelf; //ȫƴλ�� ->Ϊ�˼��ټ�ƴȫƴ��ϲ�ѯ�ٶȶ�����,ָ������ȫƴ����Ӧ��λ�� ���λΪ��־λ�����λΪ1�����ýڵ�Ϊ���ж�������ѹ���ڵ�(ֻ���ȫƴ)

public: //�ӿ�
    _XHRbNode () {
        memset(this, 0x00, sizeof(struct _XHRbNode));
    };
	//ѹ����׼����Ԥ����
    void ComposeCut() {
#if _NOT_WITH_COMPACK_ST_ 
        return ;
#endif
        int iPos = 0;
        if(usHua & XH_TREE_HUA_INFO_WIFE)   iPos++;
        if(usHua & XH_TREE_HUA_INFO_MID)    (&uiWife)[iPos++] = uiMid;
        if(usHua & XH_TREE_HUA_INFO_LEFT)   (&uiWife)[iPos++] = uiLeft;
        if(usHua & XH_TREE_HUA_INFO_RIGHT)  (&uiWife)[iPos++] = uiRight;
        if(usHua & XH_TREE_HUA_INFO_SSELF)  (&uiWife)[iPos++] = uiSpellSelf;
    };
    //���漸�������ǲ�ѯʱʹ�ã���Ϊ��ѯ��ʱ��Ϊ�˼����ڴ�����ģ������ݽṹ������ѹ��
    //�������Һ��ӡ����ݲ�����û�е����������ȥ����������ĸ��ֶ�,ֻ�����е�����²ű�ʾ
    unsigned int UncomposeWife() const { //��Ϊ���ݲ����õ��Ķ࣬��˷ŵ���ǰ��
        if(!(usHua & XH_TREE_HUA_INFO_WIFE)) return 0;
        return uiWife;
    };
    unsigned int UncomposeMid() const {
        if(!(usHua & XH_TREE_HUA_INFO_MID)) return 0;
#if _NOT_WITH_COMPACK_ST_ 
        return uiMid;
#endif
        return (&uiWife)[usHua & XH_TREE_HUA_INFO_WIFE];
    };
    unsigned int UncomposeLeft() const {
		int iTmp = usHua & XH_TREE_HUA_INFO_LEFT;
        if(!iTmp) return 0;
#if _NOT_WITH_COMPACK_ST_ 
        return uiLeft;
#endif
		return (&uiWife)[g_sucCompackPos[(iTmp - 1) & usHua]];
    };
    unsigned int UncomposeRight() const {
		int iTmp = usHua & XH_TREE_HUA_INFO_RIGHT;
        if(!iTmp) return 0;
#if _NOT_WITH_COMPACK_ST_ 
        return uiRight;
#endif
		return (&uiWife)[g_sucCompackPos[(iTmp - 1) & usHua]];
    };
    unsigned int UncomposeSpellSelf() const {
		int iTmp = usHua & XH_TREE_HUA_INFO_SSELF;
        if(!iTmp) return 0;
#if _NOT_WITH_COMPACK_ST_ 
        return uiSpellSelf;
#endif
		return (&uiWife)[g_sucCompackPos[(iTmp - 1) & usHua]];
    };
    unsigned int GetComposeSize() { //��ȡѹ����Ĵ�С
        unsigned int uiSize = sizeof(_XHRbNode);
#if _NOT_WITH_COMPACK_ST_ 
        return uiSize;
#endif
        unsigned int uiT = g_sucCompackPos[usHua & (XH_TREE_HUA_INFO_SSELF - 1)] + ((usHua & XH_TREE_HUA_INFO_SSELF) ? 1 : 0);
        return uiSize - (CAN_COMPACK_NOOD_NUMBER - uiT) * sizeof(unsigned int);
    };
}XHRbNode;

//ucAttr������
#define FLAG_CHINESE_UCATTR_WITHEXTRAINFO  1  //������չ��Ϣ
#define FLAG_CHINESE_UCATTR_WITHORIGWORD   2  //����ԭ����Ϣ
//_XHChineseData �� _XHSpellData�Ĵ�С��Ҫһ��(Ϊ�˸�����)
typedef struct _XHChineseData {
    unsigned short usWeight:10;  //���ֵ�Ȩ��

    //ucAttr: 1 1 1 1 1 1 //Ŀǰ����4λ��չ
    //                | |
    //                |\|/
    //                | �Ƿ�����չ��Ϣ 0x01
	//               \|/
	//               �Ƿ�ԭ�ʸ���ʽ����Ĵ��ﲻһ�� 0x02
    unsigned short ucAttr:6;      //�ֽ�����   -->����߱�(�Ƿ����dot����)
    unsigned short usAttr;        //˫�ֽ����� -->�ⲿ����
	//���ݲ��ֵ�λ��->���ֵ��ַ���������������spelldata���ַ������ֵģ���Ϊ���ֵ��ַ������ֻ���FLAG_CHINESE_UCATTR_WITHEXTRAINFO��Ӱ��
	//��FLAG_CHINESE_UCATTR_WITHEXTRAINFOΪ1��ʱ��uiStringPosָ���λ����ǰ4���ֽ�Ϊһ�����֡��������ָ����չ��Ϣ��λ��
	//��չ��Ϣ����������ɣ��ֱ���ucAttr���壬ǰ�沿��Ϊ��ԭ�ʣ�����趨ucAttr&0x02Ϊ�棩�����沿��Ϊ��������չ��Ϣ
    unsigned int uiStringPos;
}XHChineseData;

typedef struct _XHSpellData {
    //�Ƿ��ж�����,�����Ҫ�������м人��,��Ϊ�м�ƴ���洢���ϼ��м人�ֵı��,
    //Ҫͨ���м人��ȥ����������ҽ��,����м�ƴ����Ҫ֪���м人���Ƿ��Ӧ������
    unsigned int bMD:1;
	//���㺬��: 
	//1. ��bMDΪ0 ����ýڵ�ָ����ϼ�����ֻ��һ�������ֱ�Ӿ���uiUpperPos
	//2. ��bMDΪ1 ����ýڵ�ָ����ϼ������ж������ôuiUpperPos��Ϊ������������ĵ�һ�����ݵ�λ�ã���ǰ4���ֽ������ϼ������ܹ��ĸ���
    unsigned int uiUpperPos:31;
    unsigned int uiStringPos; //ָ���ַ���������
}XHSpellData;

#endif
