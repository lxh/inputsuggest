INC_IS=-I./include
#格式化
BIN_IS_FORMAT=../bin/isformat
SRC_IS_FORMAT=src/tool/FormatMain.cpp src/isformat/FormatFile.cpp src/util/XHStrUtils.cpp

#创建词典
SPLITWORD_CPP=src/segment/DATrieDict.cpp src/segment/WordData.cpp src/segment/WordSplit.cpp src/segment/WordSplitter.cpp
BIN_IS_DICT=../bin/isdict
SRC_IS_DICT=src/tool/CreateDictMain.cpp src/isdict/CreateDict.cpp src/util/XHStrUtils.cpp 

#创建索引
BIN_IS_INDEX=../bin/isindex
UTIL_SRC=src/util/UnitMemoryManager.cpp src/util/BaseMemoryManager.cpp src/util/FileCommon.cpp src/util/XHStrUtils.cpp src/util/DumpPickIndex.cpp
SRC_IS_INDEX=src/tool/IndexMain.cpp src/isindex/CreateIndex.cpp src/isindex/SysStructInfo.cpp src/isindex/IndexDataInfo.cpp src/isindex/IndexTreeInfo.cpp src/isindex/IndexDeleteItem.cpp
SRC_INDEX_OTHER=src/isindex/WordStringInfo.cpp src/isindex/LeafNode.cpp src/isindex/TreeNode.cpp src/isindex/TSTPat.cpp src/util/WriteLog.cpp src/util/CompressPinyin.cpp

#检索服务
BIN_IS_SERVER=../bin/isserver
IMPORTANT_SRC=src/isserver/TreeArrayMng.cpp src/isserver/TreeArray.cpp src/isserver/XHTree.cpp src/isserver/PacketResult.cpp src/isserver/XHParaCategory.cpp
SRC_IS_SERVER=src/isserver/ServerPlat.cpp src/isserver/TipManage.cpp src/isserver/InputTip.cpp 
SRC_SERVER_OTHER=src/util/XHStrUtils.cpp src/tool/ServerMain.cpp src/util/DumpPickIndex.cpp src/util/WriteLog.cpp src/util/CompressPinyin.cpp
LB_SERVER=-lpthread

CGI_OUT_FILE=../bin/is.cgi
SRC_FILE=src/tool/CgiServer.cpp

all:
#	g++ -g $(SRC_FILE) -o $(CGI_OUT_FILE) -I./include  src/util/XHStrUtils.cpp -I/usr/include/ -liconv -L/usr/local/lib -DUSE_ICONV -L./iconv/
#	g++ -g $(SRC_IS_FORMAT) $(INC_IS) -o $(BIN_IS_FORMAT)
#	g++ -g $(SRC_IS_DICT) $(SPLITWORD_CPP) $(INC_IS) -o $(BIN_IS_DICT) 
	g++ -g $(SRC_IS_INDEX) $(SRC_INDEX_OTHER) $(INC_IS) -o $(BIN_IS_INDEX) $(UTIL_SRC)
	g++ -g $(SRC_IS_SERVER) $(SRC_SERVER_OTHER) $(INC_IS) -o $(BIN_IS_SERVER) $(LB_SERVER) $(IMPORTANT_SRC)

clean:
	rm -rf $(BIN_IS_FORMAT)
	rm -rf $(BIN_IS_DICT)
	rm -rf $(BIN_IS_INDEX)
	rm -rf $(BIN_IS_SERVER)
	rm -rf $(CGI_OUT_FILE)
