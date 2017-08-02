// myAllocatorWithMemoryPool.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <iostream>
#include <vector>
#include <list>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "memorypool.h"

using namespace std;
#define MAX 15000

int main()
{

	ifstream in("TestMem.txt");
	typedef vector<int, myAllocator<int> > myVecType;
	typedef vector<int, myAllocator<int> >* myVecTypePtr;
	typedef vector<int> norVecType;
	typedef vector<int> * norVecTypePtr;

	typedef list<int, myAllocator<int> > myListType;
	typedef list<int, myAllocator<int> > *myListTypePtr;
	typedef list<int> norListType;
	typedef list<int> *norListTypePtr;

	double totalMyVecCost = 0, totalNorVecCost = 0;
	double totalMyListCost = 0, totalNorListCost = 0;
	double totalMyCost = 0, totalNorCost = 0;
	for (int k = 0; k < 10; k++) {
		in.seekg(ios::beg);
		char s[10];
		clock_t start;
		clock_t end;
		double costMyVec = 0, costNorVec = 0;
		double costMyList = 0, costNorList = 0;
		int size;
		int vecNum, resizeNum;
		int listNum, resizeListNum;
		int resize, index;
		myVecTypePtr myVecs[MAX];
		myVecTypePtr myVec;
		norVecTypePtr norVecs[MAX];
		norVecTypePtr norVec;

		myListTypePtr myLists[MAX];
		myListTypePtr myList;
		norListTypePtr norLists[MAX];
		norListTypePtr norList;

		// test vector 
		start = clock();
		// my vector with memory pool
		in >> vecNum;
		for (int i = 0; i < vecNum; i++) {
			in >> size;
			in >> s;
			myVec = new myVecType(size);
			myVecs[i] = myVec;
		}
		// resize vector
		in >> resizeNum;
		for (int i = 0; i < resizeNum; i++) {
			in >> index;
			in >> resize;
			myVecs[i]->resize(resize);
		}
		for (int i = 0; i < vecNum; i++) {
			delete myVecs[i];
		}
		end = clock();
		costMyVec = double(end - start);

		// test list 
		start = clock();
		// my list with memory pool
		in >> listNum;
		for (int i = 0; i < listNum; i++) {
			in >> size;
			in >> s;
			myList = new myListType(size);
			myLists[i] = myList;
		}
		// resize list
		in >> resizeNum;
		for (int i = 0; i < resizeNum; i++) {
			in >> index;
			in >> resize;
			myLists[i]->resize(resize);
		}
		for (int i = 0; i < listNum; i++) {
			delete myLists[i];
		}
		end = clock();
		costMyList = double(end - start);

		in.seekg(ios::beg);
		start = clock();
		// vector with default allocator
		in >> vecNum;
		for (int i = 0; i < vecNum; i++) {
			in >> size;
			in >> s;
			norVec = new norVecType(size);
			norVecs[i] = norVec;
		}
		in >> resizeNum;
		for (int i = 0; i < resizeNum; i++) {
			in >> index;
			in >> resize;
			norVecs[i]->resize(resize);
		}
		for (int i = 0; i < vecNum; i++) {
			delete norVecs[i];
		}
		end = clock();
		costNorVec = double(end - start);

		// test normal list 
		start = clock();
		// nor list with default allocator
		in >> listNum;
		for (int i = 0; i < listNum; i++) {
			in >> size;
			in >> s;
			norList = new norListType(size);
			norLists[i] = norList;
		}
		// resize list
		in >> resizeNum;
		for (int i = 0; i < resizeNum; i++) {
			in >> index;
			in >> resize;
			norLists[i]->resize(resize);
		}
		for (int i = 0; i < listNum; i++) {
			delete norLists[i];
		}
		end = clock();
		costNorList = double(end - start);

		totalMyVecCost += costMyVec;
		totalNorVecCost += costNorVec;
		totalMyListCost += costMyList;
		totalNorListCost += costNorList;
	}
	totalMyCost = totalMyListCost + totalMyVecCost;
	totalNorCost = totalNorListCost + totalNorVecCost;
	cout << "mempool vector " << totalMyVecCost << endl;
	cout << "normal vector " << totalNorVecCost << endl;
	cout << (totalNorVecCost - totalMyVecCost) / totalNorVecCost << endl;

	cout << "mempool list " << totalMyListCost << endl;
	cout << "normal list " << totalNorListCost << endl;
	cout << (totalNorListCost - totalMyListCost) / totalNorListCost << endl;

	cout << "mempool " << totalMyCost << endl;
	cout << "normal " << totalNorCost << endl;
	cout << (totalNorCost - totalMyCost) / totalNorCost << endl;

	system("pause");
	return 0;
}

