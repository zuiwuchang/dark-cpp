// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <dark/bytes/buffer.hpp>

#include <gtest/gtest.h>
#include <boost/smart_ptr.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool.hpp>

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

boost::pool<> pool_bytes(1024);
void d_bytes_t(char* p)
{
	pool_bytes.free(p);
}
dark::bytes::bytes_spt create_bytes_t(int capacity)
{
	return dark::bytes::bytes_spt((char*)pool_bytes.malloc(),d_bytes_t);
}

boost::object_pool<dark::bytes::fragmentation_t> pool_fragmentation;
void d_fragmentation_spt(dark::bytes::fragmentation_t* p)
{
	pool_fragmentation.destroy(p);
}
dark::bytes::fragmentation_spt create_fragmentation_spt(int capacity)
{
	return boost::shared_ptr<dark::bytes::fragmentation_t>(pool_fragmentation.construct(capacity,create_bytes_t),d_fragmentation_spt);
}


TEST(BytesBufferTest, HandleNoneZeroInput)
{
	
	{
		//實例化 對象
		dark::bytes::buffer_t buf(8,create_fragmentation_spt);
		std::string str = "0123456789";
		//寫入數據
		EXPECT_EQ(buf.write(str.data(),str.size()),str.size());
		//獲取 可讀數據 大小
		EXPECT_EQ(buf.size(),str.size());

		char bytes[11] = {0};
		//copy 可讀 緩衝區
		EXPECT_EQ(buf.copy_to(0,bytes,10),str.size());
		EXPECT_STREQ(bytes,str.c_str());

		memset(bytes,0,sizeof(bytes));
		int offset = 0;
		int len = 10;
		while (true)
		{
			int need = 3;

			//讀取 緩衝區
			int n = buf.read(bytes + offset, need);
			if (!n)
			{
				//無 數據 可讀 
				EXPECT_STREQ(bytes,str.c_str());
				break;
			}
			offset += n;
			len -= n;
		}

		EXPECT_EQ(buf.size(),0);
	}
	{
		
		dark::bytes::buffer_t buf(8);
		std::string str = "0123456789abcdefghijklmnopqrstwxz";
		EXPECT_EQ(buf.write(str.data(),str.size()),str.size());

		std::size_t size = str.size();
		boost::shared_array<char> b(new char[size]);
		std::size_t n = buf.copy_to(b.get(),size);
		EXPECT_EQ(std::string(b.get(),n),str);

		std::size_t pos = 3;
		//跳過字節 copy
		n = buf.copy_to(pos,b.get(),size);
		EXPECT_EQ(std::string(b.get(),n),str.substr(pos));
	}
}