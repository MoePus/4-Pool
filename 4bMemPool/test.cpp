// 4bMemPool.cpp : 定义控制台应用程序的入口点。
//

#include "4bPool.h"
#include "time.h"
using namespace std;

int main()
{
	struct rbnode
	{
		unsigned char red;
		rbnode* left;
		rbnode* right;
		unsigned int value;
	};

	clock_t time = clock();

	malloc(0xffffff * sizeof(rbnode));
	cout << "largeMalloc:\t" << clock() - time << endl;
	time = clock();
	fbPool<rbnode,4096> pool;
	for (int i = 0; i < 0xffffff;i++)
		pool.assign();
	cout << "4bMemPool:\t" << clock() - time << endl;
	time = clock();
	for (int i = 0; i < 0xffffff; i++)
		new rbnode;
	cout << "newDelete:\t" << clock() - time << endl;

    return 0;
}

