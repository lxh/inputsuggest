//gbk code(must gbk)
#ifndef SPELL_PINYIN_H_INCLUDED 
#define SPELL_PINYIN_H_INCLUDED
#include <string>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <assert.h>
//#include "util/StringUtil.h"


#include <ext/hash_map>
#include <ext/hash_set>
using namespace __gnu_cxx;
using namespace std;

struct StrHash
{
	unsigned int operator()(const std::string& str) const
	{   
		unsigned int b    = 378551;
		unsigned int a    = 63689;
		unsigned int hash = 0;

		for(size_t i = 0; i < str.length(); i++)
		{   
			hash = hash * a + str[i];
			a    = a * b;
		}   

		return hash;
	}   
};

/*韵母表*/
const char ymtable[][5]={\
	"a", "an", "", "ang", "ai", "", "", "",\
		"e", "en", "er", "eng", "ei", "ao", "", "",\
		"i", "in", "", "ing", "", "ia", "ian", "ie",\
		"iang", "iu", "", "", "o", "", "", "ong",\
		"iong", "", "iao", "", "", "ou", "", "",\
		"u", "un", "v", "", "ui", "ua", "uan", "ue",\
		"uang", "uai", "", "", "", "uo"};
/**/
const unsigned char ymps[]={\
	5, 0, 0, 0, 7, 0, 2,\
		0, 4, 0, 0, 0, 0, 1,\
		13, 0, 0, 2, 0, 0,\
		9, 0, 0, 0, 0, 0};
/*拼音表*/
const char pytable[][7]={"t", "p",\
	"a","ai","an","ang","ao",\
	"ba","bai","ban","bang","bao","bei","ben","beng","bi","bian","biao","bie","bin","bing","bo","bu",\
	"ca","cai","can","cang","cao","ce","cen","ceng","cha","chai","chan","chang","chao","che","chen","cheng","chi","chong","chou","chu","chuai","chuan","chuang","chui","chun","chuo","ci","cong","cou","cu","cuan","cui","cun","cuo",\
	"da","dai","dan","dang","dao","de","dei","den","deng","di","dia","dian","diao","die","ding","diu","dong","dou","du","duan","dui","dun","duo",\
	"e","ei","en","er",\
	"fa","fan","fang","fei","fen","feng","fo","fou","fu",\
	"ga","gai","gan","gang","gao","ge","gei","gen","geng","gong","gou","gu","gua","guai","guan","guang","gui","gun","guo",\
	"ha","hai","han","hang","hao","he","hei","hen","heng","hong","hou","hu","hua","huai","huan","huang","hui","hun","huo",\
	"ji","jia","jian","jiang","jiao","jie","jin","jing","jiong","jiu","ju","juan","jue","jun",\
	"ka","kai","kan","kang","kao","ke","kei","ken","keng","kong","kou","ku","kua","kuai","kuan","kuang","kui","kun","kuo",\
	"la","lai","lan","lang","lao","le","lei","leng","li","lia","lian","liang","liao","lie","lin","ling","liu","lo","long","lou","lu","luan","lue","lun","luo","lv",\
	"ma","mai","man","mang","mao","me","mei","men","meng","mi","mian","miao","mie","min","ming","miu","mo","mou","mu",\
	"na","nai","nan","nang","nao","ne","nei","nen","neng","ni","nian","niang","niao","nie","nin","ning","niu","nong","nou","nu","nuan","nue","nuo","nv",\
	"o","ou",\
	"pa","pai","pan","pang","pao","pei","pen","peng","pi","pian","piao","pie","pin","ping","po","pou","pu",\
	"qi","qia","qian","qiang","qiao","qie","qin","qing","qiong","qiu","qu","quan","que","qun",\
	"ran","rang","rao","re","ren","reng","ri","rong","rou","ru","rua","ruan","rui","run","ruo",\
	"sa","sai","san","sang","sao","se","sen","seng","sha","shai","shan","shang","shao","she","shei","shen","sheng","shi","shou","shu","shua","shuai","shuan","shuang","shui","shun","shuo","si","song","sou","su","suan","sui","sun","suo",\
	"ta","tai","tan","tang","tao","te","tei","teng","ti","tian","tiao","tie","ting","tong","tou","tu","tuan","tui","tun","tuo",\
	"wa","wai","wan","wang","wei","wen","weng","wo","wu",\
	"xi","xia","xian","xiang","xiao","xie","xin","xing","xiong","xiu","xu","xuan","xue","xun",\
	"ya","yan","yang","yao","ye","yi","yin","ying","yo","yong","you","yu","yuan","yue","yun",\
	"za","zai","zan","zang","zao","ze","zei","zen","zeng","zha","zhai","zhan","zhang","zhao","zhe","zhei","zhen","zheng","zhi","zhong","zhou","zhu","zhua","zhuai","zhuan","zhuang","zhui","zhun","zhuo","zi","zong","zou","zu","zuan","zui","zun","zuo"};

/*多音字*/
const char dyz[] = "\
阿 a e\n扒 ba pa\n膀 bang pang\n磅 bang pang\n\
蚌 bang beng\n薄 bao bo\n暴 bao pu\n堡 bao pu bu\
辟 bi pi\n臂 bei bi\n便 bian pian\n\
泊 bo po\n卜 bo bu\n藏 cang zang\n差 cha chai\n\
颤 chan zhan\n长 chang zhang\n朝 chao zhao\n\
澄 cheng deng\n匙 chi shi\n臭 chou xiu\n\
单 chan dan shan\n弹 dan tan\n得 de dei\n地 de di\n调 diao tiao\n都 dou du\n\
度 du duo\n恶 e wu\n佛 fo fu\n\
脯 fu pu\n蛤 ge ha\n给 gei ji\n\
还 hai huan\n会 hui kuai\n稽 ji qi\n\
缉 ji qi\n降 jiang xiang\n嚼 jiao jue\n\
角 jiao jue\n缴 jiao zhuo\n剿 chao jiao\n解 jie xie\n藉 ji jie\n\
芥 gai jie\n咀 ju zui\n觉 jiao jue\n\
菌 jun xun\n咖 ga ka\n卡 ka qia\n\
壳 ke qia\n咳 hai ke\n吭 hang keng\n\
括 gua kuo\n烙 lao luo pao\n乐 le yue\n肋 le lei\n\
了 le liao\n靓 liang jing\n露 lou lu\n\
率 lv shuai\n纶 guan lun\n落 la lao luo\n络 lao luo\n\
脉 mai mo\n蔓 man wan\n氓 mang meng\n没 mei mo\n\
秘 bi mi\n泌 bi mi\n模 mo mu\n抹 ma mo\n\
那 na nei\n弄 long nong\n\
胖 pang pan\n刨 bao pao\n炮 bao pao\n屏 bing ping\n\
迫 pai po\n朴 piao po pu\n曝 bao pu\n瀑 bao pu\n栖 qi xi\n\
奇 ji qi\n强 jiang qiang\n茄 jia qie\n亲 qin qing\n区 ou qu\n\
圈 juan quan\n雀 qiao que\n塞 sai se\n色 se shai\n刹 cha sha\n\
裳 chang shang\n省 sheng xing\n盛 cheng sheng\n拾 she shi\n\
什 shen shi\n识 shi zhi\n熟 shou shu\n属 shu zhu\n术 shu zhu\n\
数 shu shuo\n谁 shei shui\n说 shui shuo\n伺 ci si\n似 shi si\n\
宿 su xiu\n提 di ti\n褪 tui tun\n驮 duo tuo\n\
拓 ta tuo\n委 qu wei\n尾 wei yi\n尉 wei yu\n系 ji xi\n\
虾 ha xia\n厦 sha xia\n吓 he xia\n纤 qian xian\n巷 hang xiang\n\
削 xiao xue\n校 jiao xiao\n行 hang xing\n畜 chu xu\n血 xie xue\n\
咽 yan ye\n叶 xie ye\n遗 wei yi\n殷 yan yin\n吁 xu yu\n\
约 yao yue\n钥 yao yue\n攒 cuan zan\n择 ze zhai\n曾 ceng zeng\n\
扎 za zha\n轧 ya zha\n咋 za ze zha\n粘 nian zhan\n殖 shi zhi\n\
重 chong zhong\n爪 zhao zhua\n椎 chui zhui\n着 zhao zhuo\n\
仔 zai zi\n柞 zha zuo\n亟 ji qi\n\
伧 cang chen\n偈 ji jie\n偻 lou lv\n诘 ji jie\n圩 wei xu\n\
莞 guan wan\n葚 ren shen\n拗 ao niv\n捋 luo lv\n\
叨 dao tao\n呱 gu gua\n噌 ceng cheng\n忪 song zhong\n遛 iiu liu\n\
孱 can chan\n骠 biao piao\n缪 miao miu mou\n枞 cong zong\n枸 gou ju\n\
桧 gui hui\n熨 yu yun\n禅 chan shan\n\
扁 bian pian\n剥 bo bao\n嘲 chao zhao\n车 che ju\n撮 cuo zuo\n\
大 da dai\n的 de di\n囤 dun tun\n革 ge ji\n谷 gu yu\n\
和 he hu huo\n貉 he hao\n勒 le lei\n侥 jiao yao\n脚 jiao jue\n\
颈 jing geng\n龟 gui jun qiu\n俩 lia liang\n潦 liao lao\n碌 liu lu\n\
埋 mai man\n摩 mo ma\n娜 nuo na\n疟 nue yao\n荨 qian xun\n\
折 zhe she\n幢 zhuang chuang\n\
沓 da ta\n钿 dian tian\n铛 cheng dang\n鹄 gu hu\n矜 jin qin\n\
趄 ju qie\n蹊 qi xi\n\
喳 zha cha\n\
";

class Pinyin
{
	unsigned short pys[65536];                                   //汉字编码
	hash_map<unsigned short, vector<unsigned short> > MultiPY;   //多音字  <汉字编码 拼音列表>
	hash_map<unsigned short, vector<unsigned short> > pymmap;    //<拼音编码  汉字编码>
#ifndef WIN32
	hash_set<string, StrHash> pyset;
	hash_set<string, StrHash> pyset_p;
#else
	hash_set<string> pyset;
	hash_set<string> pyset_p; //
#endif
	vector<string>pinyinSet;//add by lihf.To store all the 音节.
	//static string pinyinPath;
	Pinyin(const string pinyinPath, const string mpinyinPath)
	{
		std::fill(pys, pys+65536, 0);
		ifstream in;

		// string pinyinPath = PropertyFile::getSelf().pinyinPath;
		cout << "pinyin path is:" << pinyinPath <<endl;
		cout << "muti pinyin path is:" << mpinyinPath <<endl;
		in.open(pinyinPath.c_str());
		string ch, py;
		/*可以直接写入表中*/
		while (in>>ch>>py)
		{
			pys[getHZCode(ch[0], ch[1])] = pinyin2code(py);
		}
		in.close();
		ifstream in2;
		in2.open(mpinyinPath.c_str());
		string line;
		//处理多音字
		vector<unsigned short> pycs;
		while(getline(in2, line))
		{
			pycs.clear();
			size_t pos1 =3, pos2;
			do
			{
				pos2 = line.find(' ', pos1);
				if (pos2 == (size_t)-1) pos2 = line.size();
				unsigned short code = pinyin2code(line.substr(pos1, pos2-pos1));
				assert(code > 0);
				pycs.push_back(code);
				pos1 = pos2+1;
			}while(pos1<line.size());
			MultiPY[getHZCode(line[0], line[1])] = pycs;
			for (size_t i=0; i<pycs.size(); i++)
			{
				if (pymmap.count(pycs[i]))
				{
					vector<unsigned short>& rv = pymmap[pycs[i]];
					for (size_t j=0; j<pycs.size(); j++)
					{
						rv.push_back(pycs[j]);
					}
					sort(rv.begin(), rv.end());
					//排重,同时新插入的放在首位
					rv.erase(unique(rv.begin(), rv.end()), rv.end());
					for (size_t j=0; j<rv.size(); j++)
					{
						if (rv[j] == pycs[i])
						{
							swap(rv[0], rv[j]);
							break;
						}
					}
				}
				else
				{
					pymmap[pycs[i]] = pycs;
					swap(pymmap[pycs[i]][0], pymmap[pycs[i]][i]);
				}
			}
		}
		in2.close();
		for (size_t i=0; i<sizeof(pytable)/7; i++)
		{
			pyset.insert(pytable[i]);
		}
		//add by lxh
		//拼音进行切分的时候，可能存在一些简拼的情况，但是尾可能会省略
		//e.g:xisanhuanzho 在树中可能存在这样的词，西三环中的部分，为了让zho在一起
		for(size_t i=0; i<sizeof(pytable)/7; i++) {
			string strTmp = pytable[i];
			for(int j=strTmp.size() - 1; j>1; j--) {
				string strPart = strTmp.substr(0, j);
				if(!pyset.count(strPart)) {
					if(!pyset_p.count(strPart)) {
						pyset_p.insert(strPart);
						//printf("%s\n", strPart.c_str());
					}
				} else {
					break;
				}
			}
		}
	}
	inline unsigned short getHZCode(unsigned char c1, unsigned char c2)
	{
		return (c1<<8)+c2;
	}
	/*处理声母和韵母位置*/
	inline void splitCharPinyin(const string& str, size_t& sm, size_t& ym)
	{
		sm = 0;
		ym = str.size();
		size_t i;
		for (i=0; i<str.size(); i++)
		{
			if (str[i] == 'a' || str[i] == 'e' || str[i] == 'i' || str[i] == 'o' || str[i] == 'u' || str[i] == 'v')
			{
				sm = i;
				break;
			}
		}
		for (;i<str.size(); i++)
		{
			if (isdigit(str[i]))
			{
				ym = i;
				break;
			}
		}
	}

	inline size_t ymhash(const string& str, size_t start, size_t end)
	{
		int hv=0;
		hv = (str[start]-'a')<<1;
		for (size_t i=start+1; i<end; i++)
		{
			hv+=ymps[str[i]-'a'];
		}
		return hv;
	}
	inline char getAscii(char i)
	{
		if ((i <= '9' && i>='0') || (i<='z' && i>='a'))
		{
			return i;
		}
		else if (i<='Z' && i>='A')
		{
			return i+'a'-'A';
		}
		return 0;
	}
	inline void addPinyinCode(vector<unsigned short> &codes, const string& str, size_t start, size_t end)
	{
		for (size_t j=start; j<end; j++)
		{
			codes.push_back((unsigned short)str[j]);
		}
	}
	inline void addPinyinCode(vector<vector<unsigned short> >& codes, const string& str, size_t start, size_t end)
	{
		for (size_t j=start; j<end; j++)
		{
			codes.push_back(vector<unsigned short>(1, (unsigned short)str[j]));
		}
	}
	public:
	inline static Pinyin& getInstance(const string pinPath, const string mpinPath)
	{
		static Pinyin py(pinPath, mpinPath);
		return py;
	}
	inline static Pinyin& getInstance()
	{
		return getInstance("", "");
		//static Pinyin py(pinPath);
		//Init(pinPath);
		//return getInstance(pinPath);
		//return py;
	}

	/*
	   static void Init(string path)
	   {
	   pinyinPath = path; 
	   }
	 */

	inline bool checkPinyin(const string &str)
	{
		vector<string> r;
		return splitPinyin(r, str.c_str(), str.size());
	}
	//全拼, 不管多音字
	inline string getPinyin(const string& str, bool bFirstCharUpper= false)
	{
		string py;
		vector<unsigned short> codes;
		getCode(codes, str);
		return codes2string(codes, false, bFirstCharUpper);
	}
	//add by lxh (for input search)
	inline string getSimplePinyinAll(const string& str)
	{
		string py;
		vector<unsigned short> codes;
		getCode(codes, str);
		return codes2simplestringAll(codes);
	}

	//add by lxh (for input search)
	inline void getMultiSimplePinyinAll(vector<string>& result, const string& str)
	{
		result.clear();
		vector<vector<unsigned short> > t, q;
		getCharMultiCode(t, str);
		combine(q, t);
		for (size_t i=0; i<q.size(); i++)
		{
			result.push_back(codes2simplestringAll(q[i]));
		}
	}

	/*获取简拼*/
	inline string getSimplePinyin(const string& str)
	{
		string py;
		vector<unsigned short> codes;
		getCode(codes, str);
		return codes2simplestring(codes);
	}

	inline void getMultiSimplePinyin(vector<string>& result, const string& str)
	{
		result.clear();
		vector<vector<unsigned short> > t, q;
		getCharMultiCode(t, str);
		combine(q, t);
		for (size_t i=0; i<q.size(); i++)
		{
			result.push_back(codes2simplestring(q[i]));
		}
	}

	inline void getMultiPinyin(vector<string>& result, const string& str)
	{
		result.clear();
		vector<vector<unsigned short> > t, q;
		getCharMultiCode(t, str);
		combine(q, t);
		for (size_t i=0; i<q.size(); i++)
		{
			result.push_back(codes2string(q[i]));
		}
	}
	//per word's head charater is upper
	inline void getMultiPinyinHeadUpper(vector<string>& result, const string& str)
	{
		result.clear();
		vector<vector<unsigned short> > t, q;
		getCharMultiCode(t, str);
		combine(q, t);
		for (size_t i=0; i<q.size(); i++)
		{
			result.push_back(codes2string(q[i], false, true));
		}
	}
	//声母
	inline string getInitial(const string& str, bool fuzzy = false, bool need_ym = true)
	{
		string py;
		vector<unsigned short> codes;
		getCode(codes, str);
		return codes2initial(codes, fuzzy, need_ym);
	}
	inline void getMultiInitial(vector<string>& result, const string& str, bool fuzzy = false, bool need_ym = true)
	{
		string py;
		vector<vector<unsigned short> > t, q;
		getCharMultiCode(t, str);
		combine(q, t);
		for (size_t i=0; i<q.size(); i++)
		{
			result.push_back(codes2initial(q[i], fuzzy, need_ym));
		}
	}
	//根据汉字串获取拼音
	void getCharMultiCode(vector<vector<unsigned short> >& codes, const string& str)
	{
		codes.clear();
		char c;
		bool ascflag = false;
		size_t ascp = 0;
		for (size_t i=0; i<str.size(); i++)
		{
			if (str[i] < 0)
			{
				if (ascflag)
				{
					addPinyinCode(codes, str, ascp, i);
					ascflag = false;
				}

				unsigned short hzcode = getHZCode(str[i], str[i+1]);
				hash_map<unsigned short, vector<unsigned short> >::iterator mit = MultiPY.find(hzcode);
				//单音字和多音字
				if (mit == MultiPY.end())
				{
					unsigned short code = pys[hzcode];
					if (code > 0)
					{
						codes.push_back(vector<unsigned short>(1, code));
					}
				}
				else
				{
					codes.push_back(mit->second);
				}
				i++;
			}
			else
			{
				c=getAscii(str[i]);
				if (c != 0)
				{
					if (!ascflag)
					{
						ascp = i;
						ascflag = true;
					}
				}
				else if (ascflag)
				{
					addPinyinCode(codes, str, ascp, i);
					ascflag = false;
				}
			}
		}
		if (ascflag)
		{
			addPinyinCode(codes, str, ascp, str.size());
			ascflag = false;
		}
	}
	//获取不同发音，根据地区不同发音规则
	void getCharFuzzyCode(vector<vector<unsigned short> >& result)
	{
		for (size_t i=0; i<result.size(); i++)
		{
			size_t size = result[i].size();
			for (size_t j=0; j<size; j++)
			{
				unsigned short s = result[i][j];
				int sm = s>>11;
				int nsm = -1;
				int ym = s&63;
				int nym = -1;
				switch(sm)
				{
					case 2://c
						nsm = 4;
					case 4://ch
						nsm = 2;
						break;
					case 14://sh
						nsm = 18;
						break;
					case 18://s
						nsm = 14;
						break;
					case 21://zh
						nsm = 25;
						break;
					case 25://z
						nsm = 21;
					default:
						break;
				}
				switch(ym)
				{
					case 1://an
						nym = 3;
						break;
					case 3://ang
						nym = 1;
						break;
					case 9://en
						nym = 11;
						break;
					case 11://eng
						nym = 9;
						break;
					case 17://in
						nym = 19;
						break;
					case 19://ing
						nym = 17;
						break;
					case 46://uan
						nym = 48;
						break;
					case 48://uang
						nym = 46;
						break;
					default:
						break;
				}
				if (nsm != -1)
				{
					s = nsm<<11;
					s += ym;
					s += 192;
					result[i].push_back(s);
				}
				if (nym != -1)
				{
					s = sm<<11;
					s += nym;
					s += 192;
					result[i].push_back(s);
				}
				if (nsm != -1 && nym != -1)
				{
					s = nsm<<11;
					s += nym;
					s += 192;
					result[i].push_back(s);
				}
			}
		}
	}

	//组合结果
	void combine(vector<vector<unsigned short> >& result, const vector<vector<unsigned short> >& charpy)
	{
		result.clear();
		if (charpy.empty()) return;
		for (size_t i=0; i<charpy[0].size(); i++)
		{
			result.push_back(vector<unsigned short>(1, charpy[0][i]));
		}
		for (size_t i=1; i<charpy.size(); i++)
		{
			//防止结果数过多，限定在不超过100个
			if (charpy[i].size() == 1 || result.size() > 100)
			{
				for (size_t j=0; j<result.size(); j++)
				{
					result[j].push_back(charpy[i][0]);
				}
			}
			else
			{
				size_t result_size = result.size();
				for (size_t j=1; j<charpy[i].size(); j++)
				{
					for (size_t k=0; k<result_size; k++)
					{
						result.push_back(result[k]);
						result.back().push_back(charpy[i][j]);
					}
				}
				for (size_t j=0; j<result_size; j++)
				{
					result[j].push_back(charpy[i][0]);
				}
			}
		}
	}
	//
	inline void getCode(vector<unsigned short> &codes, const string& str)
	{
		codes.clear();
		char c;
		bool ascflag = false;
		size_t ascp = 0;
		for (size_t i=0; i<str.size(); i++)
		{
			if (str[i] < 0)
			{
				if (ascflag)
				{
					addPinyinCode(codes, str, ascp, i);
					ascflag = false;
				}
				unsigned short code = pys[getHZCode(str[i], str[i+1])];
				if (code > 0)
				{
					codes.push_back(code);
				}
				i++;
			}
			else
			{
				c=getAscii(str[i]);
				if (c != 0)
				{
					if (!ascflag)
					{
						ascp = i;
						ascflag = true;
					}
				}
				else if (ascflag)
				{
					addPinyinCode(codes, str, ascp, i);
					ascflag = false;
				}
			}
		}
		if (ascflag)
		{
			addPinyinCode(codes, str, ascp, str.size());
			ascflag = false;
		}
	}
	inline string codes2string(const vector<unsigned short>& codes, bool need_tone = false, bool with_head_upper = false)
	{
		string py;
		for (size_t i=0; i<codes.size(); i++)
		{
			py+=code2pinyin(codes[i], need_tone, with_head_upper);
		}
		return py;
	}


	//add by lxh (here if a charater in word , the charater is also can save)
	inline string codes2simplestringAll(const vector<unsigned short>& codes, bool need_tone = false)
	{
		string py;
		string temp;
		for (size_t i=0; i<codes.size(); i++)
		{
			temp = code2pinyin(codes[i], need_tone);
			py+=temp[0];
		}
		return py;
	}
	inline string codes2simplestring(const vector<unsigned short>& codes, bool need_tone = false)
	{
		string py;
		string temp;
		for (size_t i=0; i<codes.size(); i++)
		{
			temp = code2pinyin(codes[i], need_tone);
			if(temp.size()>=1)
				py+=temp[0];
		}
		return py;
	}

	inline string codes2initial(const vector<unsigned short>& codes, bool fuzzy = false, bool need_ym = false)
	{
		string py;
		for (size_t i=0; i<codes.size(); i++)
		{
			py+=code2initial(codes[i], fuzzy, need_ym);
		}
		return py;
	}
	inline string code2initial(unsigned short s, bool fuzzy = false, bool need_ym = false)
	{
		string py;
		if (s < 128)
		{
			py.push_back((char)s);
			return py;
		}
		size_t sm = s>>11;
		char buf[3];
		buf[0]=buf[1]=buf[2]=0;
		if (sm == 4)
		{
			buf[0] = 'c';
			if (!fuzzy)
			{
				buf[1] = 'h';
			}
		}
		else if (sm == 14)
		{
			buf[0] = 's';
			if (!fuzzy)
			{
				buf[1] = 'h';
			}
		}
		else if (sm == 21)
		{
			buf[0] = 'z';
			if (!fuzzy)
			{
				buf[1] = 'h';
			}
		}
		else if (sm > 0)
		{
			buf[0] = char(sm+'a');
		}
		else if (need_ym)
		{
			const char *cp = ymtable[s&63];
			if (*cp != 0)
			{
				buf[0] = *cp;
			}
		}
		py = buf;
		return py;
	}
	string code2pinyin(unsigned short s, bool need_tone = false, bool with_head_upper = false)
	{
		if (s < 128)
		{
			string py;
			py.push_back((char)s);
			return py;
		}
		string py = code2initial(s);
		size_t ym = s&63;
		py+=ymtable[ym];
		if(with_head_upper && py[0] >= 'a' && py[0] <= 'z') py[0] += 'A' - 'a'; //put here , some spell don't have shengmu
		if (need_tone)
		{
			py.push_back(char(((s>>8)&7)+'0'));
		}
		return py;
	}
	/*根据汉字拼音获取编码*/
	unsigned short pinyin2code(const string& str)
	{
		unsigned short code = 0;
		size_t sm, ym;
		splitCharPinyin(str, sm, ym);
		if (sm == 1)
		{
			code=(str[0]-'a')<<11;
		}
		else if (sm == 2)
		{
			if (str[0] == 'c')
			{
				code = 4<<11;
			}
			else if (str[0] == 's')
			{
				code = 14<<11;
			}
			else
			{
				code = 21<<11;
			}
		}
		code += ymhash(str, sm, ym);
		if (ym < str.size())
		{
			code += atoi(str.c_str()+ym)<<8;
		}
		code += 192;
		return code;
	}
	/* */
	bool splitPinyin(vector<string>& result, const char *in, size_t inlen)
	{
		if (inlen  == 0) return true;
		if (isalpha(in[0]))
		{
			int minsize = 7u;
			if(inlen+1 < 7u)
				minsize = inlen + 1;

			for (size_t i=minsize; i>0; i--)
			{
				string s(in, i);
				if (pyset.count(s))
				{
					if (i == inlen)
					{
						result.push_back(s);
						return true;
					}
					vector<string> tr;
					if (splitPinyin(tr, in+i, inlen-i))
					{
						for (size_t j=0; j<tr.size(); j++)
						{
							result.push_back(string(in, i)+" "+tr[j]);
						}
						break;
					}
				}
			}
		}
		else if (isdigit(in[0]))
		{
			size_t i;
			for (i=1; i<inlen && isdigit(in[i]); i++);
			if (i == inlen)
			{
				result.push_back(string(in, i));
				return true;
			}
			vector<string> tr;
			if (splitPinyin(tr, in+i, inlen-i))
			{
				for (size_t j=0; j<tr.size(); j++)
				{
					result.push_back(string(in, i)+" "+tr[j]);
				}
			}
		}
		else if (in[0] == '\'')
		{
			return splitPinyin(result, in+1, inlen-1);
		}
		if (result.empty()) return false;
		return true;
	}
	bool IsGoodPartPinyin(const string & strIn) {
		if(pyset_p.count(strIn)) {
			return true;
		}
		return false;
	};
	bool splitPinyinSpellPart(vector<string>& result, const char *in, size_t inlen)
	{
		vector<string> vecTmp;
		string sss(in, 0, inlen);
		if(pyset_p.count(sss)) {
			result.push_back(sss);
			return true;
		}
		if(!splitPinyin(vecTmp, in, strlen(in))) {
			return false;
		}
		int iLoop, iSize = vecTmp.size();
		int jLoop, jLen;
		int iStep = 0;
		int iInLenNoDot = (int)inlen;
		for(iLoop = 0; iLoop < (int)inlen; iLoop++) {
			if(in[iLoop] == '\'') {
				iInLenNoDot--;
			}
		}
		for(iLoop = 0; iLoop < iSize; iLoop++) {
			jLen = vecTmp[iLoop].size();
			string strTmp = "";
			for(jLoop = 0; jLoop < jLen; jLoop++) {
				char c = vecTmp[iLoop][jLoop];
				strTmp += c;
				if(c != ' ') {
					iStep++;
					if(iStep == iInLenNoDot) {
						break;
					}
				}
			}
			result.push_back(strTmp);
		}
		/*
		   static char s_cShengmuFirst[] = 
		//"bpmfdtnlgkhjqxrzcsyw";
		//abcdefghijklmnopqrstuvwzyx";
		"01110111011111011111001111"; 
		if (inlen  == 0) return true;
		if (isalpha(in[0]))
		{
		int minsize = 7u;
		if(inlen+1 < 7u)
		minsize = inlen + 1;

		for (size_t i=minsize; i>0; i--)
		{
		string s(in, i);
		if (pyset.count(s) || (i==inlen&&(pyset_p.count(s) || (s_cShengmuFirst[s[0] - 'a'] == '1' && s.length() == 1))))
		{
		if (i == inlen)
		{
		result.push_back(s);
		return true;
		}
		vector<string> tr;
		if (splitPinyinSpellPart(tr, in+i, inlen-i))
		{
		for (size_t j=0; j<tr.size(); j++)
		{
		result.push_back(string(in, i)+" "+tr[j]);
		}
		break;
		}
		}
		}
		}
		else if (isdigit(in[0]))
		{
		size_t i;
		for (i=1; i<inlen && isdigit(in[i]); i++);
		if (i == inlen)
		{
		result.push_back(string(in, i));
		return true;
		}
		vector<string> tr;
		if (splitPinyinSpellPart(tr, in+i, inlen-i))
		{
		for (size_t j=0; j<tr.size(); j++)
		{
		result.push_back(string(in, i)+" "+tr[j]);
		}
		}
		}
		else if (in[0] == '\'')
		{
		return splitPinyinSpellPart(result, in+1, inlen-1);
		}
		if (result.empty()) return false;
		 */
		return true;
	}
	//切分带有简拼[声母首字母]的拼音字符串
	//add by lxh
	//拼音可能不全，最后一个拼音采用不全策略
	bool splitPinyinWithSimple(vector<string>& result, const char *in, size_t inlen)
	{
		static char s_cShengmuFirst[] = 
			//"bpmfdtnlgkhjqxrzcsyw";
			//abcdefghijklmnopqrstuvwzyx";
			"01110111011111011111001111"; 
		if (inlen  == 0) return true;
		if (isalpha(in[0]))
		{
			int minsize = 7u;
			if(inlen+1 < 7u)
				minsize = inlen + 1;

			for (size_t i=minsize; i>0; i--)
			{
				string s(in, i);
				//printf("--->>%s:i:%d; minsize:%d; inlen:%d\n", s.c_str(), i, minsize, inlen);
				if (pyset.count(s) || (i==1 && s_cShengmuFirst[s[0] - 'a'] == '1') || (i==inlen&&pyset_p.count(s)))
				{
					if (i == inlen)
					{
						result.push_back(s);
						return true;
					}
					vector<string> tr;
					if (splitPinyinWithSimple(tr, in+i, inlen-i))
					{
						for (size_t j=0; j<tr.size(); j++)
						{
							result.push_back(string(in, i)+" "+tr[j]);
						}
						break;
					} else {
						return false;
					}
				}
			}
		}
		else if (isdigit(in[0]))
		{
			size_t i;
			for (i=1; i<inlen && isdigit(in[i]); i++);
			if (i == inlen)
			{
				result.push_back(string(in, i));
				return true;
			}
			vector<string> tr;
			if (splitPinyinWithSimple(tr, in+i, inlen-i))
			{
				for (size_t j=0; j<tr.size(); j++)
				{
					result.push_back(string(in, i)+" "+tr[j]);
				}
			} else {
				return false;
			}
		}
		else if (in[0] == '\'')
		{
			return splitPinyinWithSimple(result, in+1, inlen-1);
		}
		if (result.empty()) return false;
		return true;
	}
void StrTokenize(vector<string> &v, const string& src, string tok, bool trim = false, string null_subst = "")
{
	//vector<string> v;
	if( src.empty())
	{
		return;
	}
	if(tok.empty() )
	{
		v.push_back(src);
		return;
	}

	int pre_index = 0, index = 0, len = 0;
	while( (index = src.find_first_of(tok, pre_index)) != string::npos )
	{
		if( (len = index-pre_index)!=0 )
			v.push_back(src.substr(pre_index, len));
		else if(trim==false)
			v.push_back(null_subst);
		pre_index = index+1;
	}
	string endstr = src.substr(pre_index);
	if( trim==false ) v.push_back( endstr.empty()?null_subst:endstr );
	else if( !endstr.empty() ) v.push_back(endstr);
	
	//return v;

};
	bool collectAvailPinyin(vector<string> & resultSet,const char * inPinyin)
	{
		string tmp;
		vector<string> splitList;
		vector<string> pinyinList;
		bool ret=splitPinyin(splitList,inPinyin,strlen(inPinyin));
		if(ret == false)
			return false;
		for(size_t ii=0;ii<splitList.size();ii++)
		{
			StrTokenize(pinyinList,splitList[ii]," ");
			//altenation
			for(size_t i=0;i<pinyinList.size();i++)
			{
				if(pinyinList.size()<3)
					break;// words len must great 2
				for(size_t num=0;num<pinyinSet.size();num++)
				{
					if(pinyinSet[num]==pinyinList[i])//remove the same ones
						continue;
					tmp="";
					for(size_t j=0;j<i;j++)
					{
						tmp= tmp + pinyinList[j] ;
					}
					tmp = tmp + pinyinSet[num] ;
					size_t kkk=pinyinList.size();
					for(size_t k=i+1;k<kkk;k++)
					{
						tmp = tmp + pinyinList[k] ;
					}
					resultSet.push_back(tmp);
				}
			}
			//insertion
			for(size_t i=0;i<=pinyinList.size();i++)
			{
				for(size_t num=0;num<pinyinSet.size();num++)
				{
					tmp="";
					for(size_t j=0;j<i;j++)
					{
						tmp= tmp + pinyinList[j] ;
					}
					tmp = tmp + pinyinSet[num] ;
					size_t kkk=pinyinList.size();
					for(size_t k=i;k<kkk;k++)
					{
						tmp = tmp + pinyinList[k] ;
					}
					resultSet.push_back(tmp);
				}
			}
			//delete one
			for(size_t i=0;i<pinyinList.size() && pinyinList.size()>2;i++)
			{
				tmp="";
				for(size_t j=0;j<i;j++)
				{
					tmp= tmp + pinyinList[j] ;
				}
				for(size_t k=i+1;k<pinyinList.size();k++)
				{
					tmp = tmp + pinyinList[k] ;
				}
				resultSet.push_back(tmp);
			}
			/* not handle this kind
			//transpostion
			for(int i=0;i<pinyinList.size();i++)
			{
			tmp="";
			for(int j=0;j<i;j++)
			{
			tmp= tmp + pinyinList[j];
			}
			if (i==(pinyinList.size()-1))
			continue;
			tmp =tmp + pinyinList[i+1] ;
			tmp =tmp + pinyinList[i] ;
			int kkk=pinyinList.size();
			for(int k=i+2;k<kkk;k++)
			{
			tmp = tmp + pinyinList[k] ;
			}
			resultSet.push_back(tmp);
			}
			 */
		}
		return true;
	}
	unsigned short getCCharCode(char c1, char c2)
	{
		return pys[getHZCode(c1, c2)];
	}
	void getMultiCCharCode(vector<unsigned short>& codes, char c1, char c2)
	{
		codes.clear();
		unsigned short hzcode = getHZCode(c1, c2);
		hash_map<unsigned short, vector<unsigned short> >::iterator mit = MultiPY.find(hzcode);
		if (mit == MultiPY.end())
		{
			unsigned short code = pys[hzcode];
			if (code > 0)
			{
				codes.push_back(code);
			}
		}
		else
		{
			codes = mit->second;
		}
	}
};

#if 0
int main(int argc, char ** argv)
{
	string strPinyin = "/home/huazi/pro/su/config/pinyin";
	string strMPinyin = "/home/huazi/pro/su/config/mpinyin";
    Pinyin pyInfo(Pinyin::getInstance(strPinyin, strMPinyin));
	vector<string> vecResult;
	const char *psz = "xianrenminshanghai";
	pyInfo.splitPinyin(vecResult, psz, strlen(psz));
	int iLoop;
	for(iLoop = 0; iLoop < vecResult.size(); iLoop++) {
		printf("-->%s\n", vecResult[iLoop].c_str());
	}
	return 0;
}

#endif

#endif // PINYIN_H_INCLUDED
