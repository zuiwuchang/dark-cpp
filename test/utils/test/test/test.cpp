// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "../../../../include/dark-cpp/utils/slice.hpp"
#include "../../../../include/dark-cpp/utils/array.hpp"

#include <gtest/gtest.h>
#include <boost/smart_ptr.hpp>

#ifdef _DEBUG
#pragma comment(lib,"gtest-mdd.lib")
#else
#pragma comment(lib,"gtest-md.lib")
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();

	
	std::system("pause");
	return rs;
}

TEST(UtilsSliceTest, HandleNoneZeroInput)
{
	int x[] = {1,2,3};
	dark::utils::slice_t<int> slice(x,3);
	EXPECT_EQ(slice.size(),3);

	for(int i=0;i<3;++i)
	{
		EXPECT_EQ(slice[i],x[i]);
	}
	slice[0] = 666;
	EXPECT_EQ(666,x[0]);	
}


void DArraySaptInt(int * p)
{
	puts("DArraySaptInt is call");
	delete p;
}
TEST(UtilsArrayTest, HandleNoneZeroInput)
{
	int x[] = {1,2,3};
	int size = 3;
	dark::utils::array_sapt<int> ars(boost::shared_array<int>(new int[size]),size);
	ars = dark::utils::array_sapt<int>(new int[size],DArraySaptInt,size);
	ars = dark::utils::array_sapt<int>(new int[size],size);
	
	EXPECT_EQ(ars.size(),size);

	EXPECT_EQ(ars.size(),3);

	for(int i=0;i<3;++i)
	{
		ars[i] = x[i];
	}
	for(int i=0;i<3;++i)
	{
		EXPECT_EQ(ars[i],x[i]);
	}
	ars[0] = 666;
	EXPECT_EQ(666,ars[0]);

	EXPECT_TRUE(ars);
	ars.reset();
	EXPECT_FALSE(ars);
}