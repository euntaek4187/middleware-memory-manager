#include "pch.h"
#include "../SMemoryManagerV0.21/MemoryManager.h"
#include "../SMemoryManagerV0.21/Object.h"
#include "./TestObject.h"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define SIZE_MEMORY 1024
#define SIZE_PAGE 64
#define SIZE_SLOT 16
#define SIZE_INSTANCE 64UL

static size_t normalizeSize(size_t size) {
	size_t size1 = size >> 4;
	size_t size2 = size1 << 4;
	size = ((size == size2) ? size1 : ++size1) << 4;
	return size;
}

/*
TEST(MemoryManagerTest, PageSizeEqualsAllocatedMemory1) {
	//memory = page = slot = object
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, SIZE_MEMORY);
	TestObject* object = new(SIZE_MEMORY, "1") TestObject();
	EXPECT_EQ(object->getSizeSlot(), SIZE_MEMORY);
}
TEST(MemoryManagerTest, PageSizeEqualsAllocatedMemory2) {
	//memory = page = slot > object
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, SIZE_MEMORY);
	TestObject* object = new(SIZE_MEMORY - 1, "1") TestObject();
	EXPECT_EQ(object->getSizeSlot(), normalizeSize(SIZE_MEMORY - 1));
}
TEST(MemoryManagerTest, PageSizeEqualsAllocatedMemory3) {
	//memory = page = slot < object
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, SIZE_MEMORY);
	EXPECT_THROW(new(SIZE_MEMORY + 1, "1") TestObject(); , Exception);
}
TEST(MemoryManagerTest, PageSizeSmallerThenAllocatedMemory1) {
	//memory > page = slot = object
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, SIZE_PAGE);
	TestObject* object = new(SIZE_PAGE, "1") TestObject();
	EXPECT_EQ(object->getSizeSlot(), SIZE_PAGE);
}
TEST(MemoryManagerTest, PageSizeSmallerThenAllocatedMemory2) {
	//memory > page = slot > object
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, SIZE_PAGE);
	TestObject* object = new(SIZE_PAGE-1, "1") TestObject();
	EXPECT_EQ(object->getSizeSlot(), normalizeSize(SIZE_PAGE - 1));
}
TEST(MemoryManagerTest, PageSizeSmallerThenAllocatedMemory3) {
	//memory > page = slot < object
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, SIZE_PAGE);
	TestObject* object = new(SIZE_PAGE + 1, "1") TestObject();
	EXPECT_EQ(object->getSizeSlot(), normalizeSize(SIZE_PAGE + 1));
}
TEST(MemoryManagerTest, PageSizeLargerThenAllocatedMemory1) {
	//memory < page = slot = object
	//exception
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, 2*SIZE_MEMORY);
	EXPECT_THROW(new(2 * SIZE_MEMORY, "1") TestObject();, Exception);
}
TEST(MemoryManagerTest, PageSizeLargerThenAllocatedMemory2) {
	//memory < page = slot < object
	//exception
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, 2 * SIZE_MEMORY);
	EXPECT_THROW(new(2 * SIZE_MEMORY + 1, "1") TestObject(); , Exception);
}
TEST(MemoryManagerTest, PageSizeLargerThenAllocatedMemory3) {
	//memory < page = slot > object
	//exception
	char* pBuffer = new char[SIZE_MEMORY];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * SIZE_MEMORY, SIZE_MEMORY);
	EXPECT_THROW(new(2 * SIZE_MEMORY - 1, "1") TestObject(); , Exception);
}
*/

void makeTestCase(string caseName, int memorySize, int pageSize, int objectSize) {
	//cout << caseName << endl;
	//cout << "memorySize: " << memorySize << ", pageSize: " << pageSize << ", objectSize: " << objectSize << endl;

	cout << "[--------" + caseName + "--------]" << endl;
	if(Object::s_pMemoryManager!=nullptr){
		delete Object::s_pMemoryManager;
		Object::s_pMemoryManager = nullptr;
	}
	char* pBuffer = new char[memorySize];
	Object::s_pMemoryManager = new MemoryManager(pBuffer, sizeof(char) * memorySize, pageSize);
	if (memorySize < objectSize || (memorySize < pageSize)) {
		EXPECT_THROW(new(objectSize, "1") TestObject(); , Exception);
	}
	else {
		printf("memorySize = %d\n", memorySize);
		printf("pageSize = %d\n", pageSize);
		printf("ObjectSize = %d\n", normalizeSize(objectSize));

		int numSlots = pageSize / normalizeSize(objectSize);
		int numPages = memorySize / pageSize;
		if (objectSize > pageSize) {
			int numberOfPages = (normalizeSize(objectSize) + pageSize - 1) / pageSize;
			int sizeSlot = numberOfPages * pageSize;
			numSlots = memorySize / sizeSlot;
		}
		else {
			numSlots = numSlots * numPages;
		}
		printf("numSlot = %d\n", numSlots);

		TestObject** testObject = new TestObject * [numSlots];

		for (int i = 0; i < numSlots; i++) {
			testObject[i] = new(objectSize, to_pchar(i)) TestObject();
			EXPECT_EQ(testObject[i]->getSizeSlot(), normalizeSize(objectSize));
		}

		for (int i = 0; i < numSlots; i++) {
			Object::s_pMemoryManager->dellocate(testObject[i]);
		}

		for (int i = 0; i < numSlots; i++) {
			testObject[i] = new(objectSize, to_pchar(i)) TestObject();
			EXPECT_EQ(testObject[i]->getSizeSlot(), normalizeSize(objectSize));
		}
		EXPECT_THROW(new(objectSize, "last") TestObject();, Exception);
	}
}
TEST(MemoryManagerTest, AllCaseTest) {
	string line;
	ifstream file("testCaseSample.txt");
	if (file.is_open()) {
		string caseName;
		string memorySize;
		string pageSize;
		string objectSize;
		while (getline(file, line)) {
			stringstream ss(line);
			string token;
			vector<string> tokens;
			while (ss >> token) {
				tokens.push_back(token);
			}
			size_t found = line.find("//");
			if (found != string::npos) {
				caseName = line;
			}
			else {
				if (tokens.size() >= 3) {
					memorySize = tokens[0];
					pageSize = tokens[1];
					objectSize = tokens[2];
				}
				makeTestCase(caseName, stoi(memorySize), stoi(pageSize), stoi(objectSize));
			}
		}
		file.close();
	}
	else {
		cout << "Unable to open file -> Exception";
	}
}
