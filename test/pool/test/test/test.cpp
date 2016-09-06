// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <gtest/gtest.h>

#ifdef _DEBUG
#pragma comment(lib,"gtest-mdd.lib")
#else
#pragma comment(lib,"gtest-md.lib")
#endif


#include <dark/pool/pool_chunk.hpp>
#include <dark/pool/pool_object.hpp>

int _tmain(int argc, _TCHAR* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();

	
	std::system("pause");
	return rs;
}

//new delete test
TEST(UnPoolTest, HandleNoneZeroInput)
{
	int count = 10;
	int need = 10000;
	std::list<char*> list;
	for(int i=0;i<count;++i)
	{
		for(int j=0;j<need;++j)
		{
			char* p = new char[1024];
			EXPECT_TRUE(p);
			list.push_back(p);
		}

		BOOST_FOREACH(char* p,list)
		{
			delete p;
		}
		list.clear();
	}
}

//test pool_chunk
TEST(PoolPoolTest, HandleNoneZeroInput)
{
	dark::pool::pool_chunk_t pool(1024,boost::make_shared<boost::mutex>());
	int count = 10;
	int need = 10000;
	std::list<void*> list;
	for(int i=0;i<count;++i)
	{
		for(int j=0;j<need;++j)
		{
			void* p = pool.malloc_chunk();
			EXPECT_TRUE(p);
			list.push_back(p);
		}
		EXPECT_EQ(pool.get_frees(),0);
		EXPECT_EQ(pool.get_mallocs(),need);

		BOOST_FOREACH(void* p,list)
		{
			pool.free_chunk(p);
		}
		list.clear();

		EXPECT_EQ(pool.get_frees(),need);
		EXPECT_EQ(pool.get_mallocs(),0);
	}

	pool.release_memory(5);
	EXPECT_EQ(pool.get_frees(),5);
	pool.release_memory();
	EXPECT_EQ(pool.get_frees(),0);
}

class Animal
{
public:
	std::string name;
	char buf[1024];
	Animal(const std::string& name)
	{
		this->name = name;
	}
	virtual ~Animal()
	{

	}
};
//test pool_object
TEST(PoolPoolObjectTest, HandleNoneZeroInput)
{
	dark::pool::pool_object_t<Animal> pool(boost::make_shared<boost::mutex>());
	int count = 10;
	int need = 10000;
	std::list<Animal*> list;
	for(int i=0;i<count;++i)
	{
		for(int j=0;j<need;++j)
		{
			Animal* p = pool.construct("cat");
			EXPECT_TRUE(p);
			list.push_back(p);
		}
		EXPECT_EQ(pool.get_frees(),0);
		EXPECT_EQ(pool.get_mallocs(),need);

		BOOST_FOREACH(Animal* p,list)
		{
			pool.destory(p);
		}
		list.clear();

		EXPECT_EQ(pool.get_frees(),need);
		EXPECT_EQ(pool.get_mallocs(),0);
	}

	pool.release_memory(5);
	EXPECT_EQ(pool.get_frees(),5);
	pool.release_memory();
	EXPECT_EQ(pool.get_frees(),0);
}