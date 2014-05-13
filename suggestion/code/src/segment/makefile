INCLUDE=-I../../include

wordsplit:WordData.o DATrieDict.o WordSplit.o WordSplitter.o SplitTest.o
	g++ -g *.o -D_FILE_OFFSET_BITS=64 -o wordsplit -Wall -O2 $(INCLUDE)
SplitTest.o: SplitTest.cpp
	g++ -g -c SplitTest.cpp $(INCLUDE)
WordSplitter.o: WordSplitter.cpp
	g++ -g -c WordSplitter.cpp $(INCLUDE)
WordData.o: WordData.cpp 
	g++ -g -c -fPIC WordData.cpp $(INCLUDE)
DATrieDict.o: DATrieDict.cpp 
	g++ -g -c -fPIC DATrieDict.cpp $(INCLUDE)
WordSplit.o: WordSplit.cpp 
	g++ -g -c -fPIC WordSplit.cpp $(INCLUDE)
.PHONY : clean
clean :
	-rm wordsplit *.o
