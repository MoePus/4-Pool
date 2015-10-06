#pragma once
#include <iostream>
template <class userType,int size=128,int limitationPercent=37>
class fbPool
{
public:
	fbPool();
	~fbPool();
	unsigned long*  assign();
	int 			recede(unsigned long* pointer);

private:
	typedef typename userType USORD;

	const static unsigned int memBlockSize = sizeof(USORD) * size * 1024;
	const static unsigned int pointerBlockSize = memBlockSize / sizeof(USORD) / 8;
	const static unsigned int memCount = memBlockSize / sizeof(USORD);
	const static unsigned int limitCount = memCount*(limitationPercent/100.0);

	struct poolChunk
	{
		unsigned int	freeCount;
		unsigned int	shojoCount;
		poolChunk*	    nextChunk;
		unsigned int	pointer[pointerBlockSize / sizeof(unsigned int)];
		userType		mem[memCount];

	};

	const unsigned int pad[4] = { 0xffffffff,0xffffffff ,0xffffffff ,0xffffffff };

	typename fbPool<userType, size, limitationPercent>::poolChunk* createNewChunk();
	void RemoveChunks();
	inline unsigned int findFreePointer(unsigned int * flag);


	inline void fNeg(unsigned int * begin, unsigned int order);
	inline void f0(unsigned int * begin, unsigned int order);
	inline void f1(unsigned int * begin, unsigned int order);


	template <int S>
	static inline int findbit0InDWORD(unsigned int flag)
	{
		if ((flag & (1 << S)) == 0)
		{
			return S;
		}
		else
		{
			return findbit0InDWORD<S - 1>(flag);
		}
	
	}
	template <>
	static inline int findbit0InDWORD<0>(unsigned int flag)
	{
		return 0;
	}



	poolChunk* beginChunk;
};


template<class userType, int size, int limitationPercent>
inline unsigned int fbPool<userType, size, limitationPercent>::findFreePointer(unsigned int * flag)
{
	unsigned int* tmpLp = flag;
	while (!memcmp(flag, pad, 16))
	{
		flag = (unsigned int*)((char*)flag + 16);
	}
	while (!memcmp(flag, pad, 4))
	{
		flag = (unsigned int*)((char*)flag + 4);
	}
	int order = findbit0InDWORD<31>(*flag);
	order += ((char*)flag - (char*)tmpLp)*sizeof(unsigned int);

	return order;
}



template<class userType, int size, int limitationPercent>
inline void fbPool<userType, size, limitationPercent>::fNeg(unsigned int * flag, unsigned int order)
{
	unsigned int m = order / 32;
	unsigned int n = 1 << (order % 32);
	flag[m] = (flag[m] & (~n)) | ((flag[m] & n) ^ n);
}

template<class userType, int size, int limitationPercent>
inline void fbPool<userType, size, limitationPercent>::f0(unsigned int * flag, unsigned int order)
{
	unsigned int m = order / 32;
	unsigned int n = 1 << (order % 32);
	flag[m] = flag[m] & (~n);
}

template<class userType, int size, int limitationPercent>
inline void fbPool<userType, size, limitationPercent>::f1(unsigned int * flag, unsigned int order)
{
	unsigned int m = order / 32;
	unsigned int n = 1 << (order % 32);
	flag[m] = flag[m] | n;
}



template<class userType, int size, int limitationPercent>
fbPool<userType, size, limitationPercent>::fbPool()
{
	beginChunk = createNewChunk();
}

template<class userType, int size, int limitationPercent>
fbPool<userType, size, limitationPercent>::~fbPool()
{
	RemoveChunks();
}

template<class userType, int size, int limitationPercent>
unsigned long * fbPool<userType,size, limitationPercent>::assign()
{
	auto currentChunk = beginChunk;
	while (!currentChunk->freeCount || (!(currentChunk->shojoCount) && currentChunk->freeCount <= limitCount))
	{
		if (!currentChunk->nextChunk)
			currentChunk->nextChunk = createNewChunk();
		currentChunk = currentChunk->nextChunk;
	}

	void * ret;
	if (currentChunk->shojoCount)
	{
		currentChunk->freeCount--;
		currentChunk->shojoCount--;
		f1(currentChunk->pointer, currentChunk->shojoCount);
		ret = (char*)currentChunk->mem + sizeof(USORD)*currentChunk->shojoCount;
	}
	else
	{
		currentChunk->freeCount--;
		unsigned int order = findFreePointer(currentChunk->pointer);
		f1(currentChunk->pointer, order);
		ret = (char*)currentChunk->mem + sizeof(USORD)*order;
	}



	return (unsigned long*)ret;
}

template<class userType, int size, int limitationPercent>
int fbPool<userType,size, limitationPercent>::recede(unsigned long * pointer)
{
	poolChunk* currentChunk = beginChunk;
	while (pointer - (void*)currentChunk > sizeof(poolChunk) || pointer - (void*)currentChunk < 0)
	{
		currentChunk = currentChunk->nextChunk;
		if (!currentChunk)
			return -1;
	}
	unsigned int order = ((char*)pointer - (char*)currentChunk->mem) / sizeof(USORD);
	f0(currentChunk->pointer, order);
	currentChunk->freeCount++;
	return 0;
}





template<class userType, int size, int limitationPercent>
typename fbPool<userType, size, limitationPercent>::poolChunk* fbPool<userType,size, limitationPercent>::createNewChunk()
{
	poolChunk* ret = new poolChunk;
	ret->freeCount = memCount;
	ret->shojoCount = memCount;
	memset(&(ret->nextChunk), 0, sizeof(void*) + pointerBlockSize);
	return ret;
}

template<class userType, int size, int limitationPercent>
void fbPool<userType, size, limitationPercent>::RemoveChunks()
{
	poolChunk* tmpPointer = beginChunk->nextChunk;
	delete beginChunk;
	while (tmpPointer)
	{
		poolChunk* tmp2nd = tmpPointer;
		tmpPointer = tmpPointer->nextChunk;
		delete tmp2nd;
	}
}
