/*
encoding:gbk
	简记: 输入提示的最核心的树节点的结构

该结构默认做了几个限制:
	1. 数据的属性为两个字节
	2. 字符串的长度最长为255(包含前缀部分)
	3. 数据的权重范围为10bit(0-1023)
	4. 数据量最好不要超过一亿(最好千万以内)->要处理千万或亿甚至更多则需要分割
	5. 数据的扩展部分最好在总大小1G以内(一般会远远小于该值的)

qq: 315753857
*/

#ifndef _XH_TREE_STRUCT_H_
#define _XH_TREE_STRUCT_H_
#include <cstring>

//是否压缩节点
#define _NOT_WITH_COMPACK_ST_ 0

//可以压缩的结点的个数
#define CAN_COMPACK_NOOD_NUMBER 5

//节点权重的最大值[权重范围0-1023]
#define MAX_WEIGHT_CHINESE     0x3ff

//XHRbNode->usHua使用
//XH_TREE_HUA_INFO_WIFE  -> 是否具有wife
//XH_TREE_HUA_INFO_MID   -> 是否具有中间孩子
//XH_TREE_HUA_INFO_LEFT  -> 是否具有左孩子
//XH_TREE_HUA_INFO_RIGHT -> 是否具有右孩子
//XH_TREE_HUA_INFO_SSELF -> 是否具有spellself->指向自己本身的结构信息
#define XH_TREE_HUA_INFO_WIFE  0x01
#define XH_TREE_HUA_INFO_MID   0x02
#define XH_TREE_HUA_INFO_LEFT  0x04
#define XH_TREE_HUA_INFO_RIGHT 0x08
#define XH_TREE_HUA_INFO_SSELF 0x10

//获取偏移位置                                 // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
const static unsigned char g_sucCompackPos[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

//社会病态，只关心孩子不关系父母
//用于查询使用的红黑树(简化版)
//该结构有几点需要注意
//1. left,mid,right指向的是地址，不是数组下标
//2. 最终索引里面，可能data,mid,left,right,ss(spellself)并不全存在，为了节省空间
//3. 存储树节点时，不要从0地址开始，因为0地址是有意义的，代表没有内容
typedef struct _XHRbNode {
    char cByteDif;             //不同字节的内容
    unsigned char cPosDif;     //不同字节的位置

	unsigned short usWPeer:1;     //是否有成对的信息节点存在(跟当前节点字母忽略大小写,字母一样的节点)-->>只有全拼会使用
	unsigned short usHua:5;       //具有多种信息->具体见XH_TREE_HUA_INFO_XXXX
	unsigned short usWeight:10;   //节点的权重

	//下面几个节点都可能会被压缩
    unsigned int  uiWife;      //节点的数据部分
    unsigned int  uiMid;       //中孩子  ----中孩子放到左孩子前面，因为中孩子需要去数据访问多
    unsigned int  uiLeft;      //左孩子
    unsigned int  uiRight;     //右孩子
    unsigned int  uiSpellSelf; //全拼位置 ->为了加速简拼全拼混合查询速度而生的,指向自身全拼所对应的位置 最高位为标志位，最高位为1则代表该节点为含有多个结果的压缩节点(只针对全拼)

public: //接口
    _XHRbNode () {
        memset(this, 0x00, sizeof(struct _XHRbNode));
    };
	//压缩的准备，预处理
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
    //下面几个函数是查询时使用，因为查询的时候，为了减少内存的消耗，对数据结构进行了压缩
    //把左中右孩子、数据部分在没有的情况进行了去除。因此这四个字段,只有在有的情况下才表示
    unsigned int UncomposeWife() const { //因为数据部分用到的多，因此放到了前面
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
    unsigned int GetComposeSize() { //获取压缩后的大小
        unsigned int uiSize = sizeof(_XHRbNode);
#if _NOT_WITH_COMPACK_ST_ 
        return uiSize;
#endif
        unsigned int uiT = g_sucCompackPos[usHua & (XH_TREE_HUA_INFO_SSELF - 1)] + ((usHua & XH_TREE_HUA_INFO_SSELF) ? 1 : 0);
        return uiSize - (CAN_COMPACK_NOOD_NUMBER - uiT) * sizeof(unsigned int);
    };
}XHRbNode;

//ucAttr的意义
#define FLAG_CHINESE_UCATTR_WITHEXTRAINFO  1  //含有扩展信息
#define FLAG_CHINESE_UCATTR_WITHORIGWORD   2  //含有原词信息
#define FLAG_CHINESE_UCATTR_DELETE         4  //该条数据已经删除
//_XHChineseData 与 _XHSpellData的大小需要一致(为了更方便)
typedef struct _XHChineseData {
    unsigned short usWeight:10;  //汉字的权重

    //ucAttr: 1 1 1 1 1 1 //目前还有4位扩展
    //              | | |
    //              | |\|/
    //              | | 是否含有扩展信息 0x01
	//              |\|/
	//              |是否原词跟格式化后的词语不一样 0x02
	//             \|/
	//             是否已经打了删除标记
    unsigned short ucAttr:6;      //字节属性   -->自身具备(是否具有dot属性)
    unsigned short usAttr;        //双字节属性 -->外部设置
	//数据部分的位置->汉字的字符串部分是区分与spelldata的字符串部分的，因为汉字的字符串部分还受FLAG_CHINESE_UCATTR_WITHEXTRAINFO的影响
	//当FLAG_CHINESE_UCATTR_WITHEXTRAINFO为1的时候，uiStringPos指向的位置往前4个字节为一个数字。这个数字指向扩展信息的位置
	//扩展信息有两部分组成，分别用ucAttr定义，前面部分为：原词（如果设定ucAttr&0x02为真），后面部分为真正的扩展信息
    unsigned int uiStringPos;
}XHChineseData;

typedef struct _XHSpellData {
    //是否含有多个结果,这个主要是用于中间汉字,因为中间拼音存储了上级中间汉字的编号,
    //要通过中间汉字去汉字里面查找结果,因此中间拼音需要知道中间汉字是否对应多个结果
    unsigned int bMD:1;
	//两层含义: 
	//1. 当bMD为0 代表该节点指向的上级数据只有一个，因此直接就是uiUpperPos
	//2. 当bMD为1 代表该节点指向的上级数据有多个，那么uiUpperPos则为这多个数据里面的第一个数据的位置，往前4个字节则是上级数据总共的个数
    unsigned int uiUpperPos:31;
    unsigned int uiStringPos; //指向字符串的内容
}XHSpellData;

#endif
