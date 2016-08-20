// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../../../../include/dark-cpp/io/msg/message.hpp"
#include "../../../../../include/dark-cpp/io/msg/writer.hpp"
#include "../../../../../include/dark-cpp/io/msg/reader.hpp"

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



TEST(IoMsgMessageWriterTest, HandleNoneZeroInput)
{
	dark::io::msg::writer_t writer(10);
	{
		std::size_t id = 1;
		std::string str1 = "this is msg test";
		std::string str2 = "Cerberus it's an idea";
		EXPECT_EQ(str1.size(),writer.write(str1));
		EXPECT_EQ(str2.size(),writer.write(str2));
	
		dark::io::msg::message_t msg = writer.create_message(id);
		EXPECT_FALSE(writer.create_message(id).body());

		dark::io::msg::header_t* header = msg.header();
		EXPECT_EQ(header->flag,DARK_IO_MSG_HEADER_FLAG);
		EXPECT_EQ(header->id,id);
		EXPECT_EQ(header->size,str1.size() + str2.size());

		dark::io::msg::body_t body = msg.body();
		EXPECT_TRUE(body);
		EXPECT_EQ(body.size(),str1.size() + str2.size());
		EXPECT_EQ(std::string(body.get(),body.size()),str1 + str2);
	}
	
	{
		std::size_t id = 2;
		std::string str1 = "i love kate beckinsale";
		EXPECT_EQ(str1.size(),writer.write(str1));
	
		dark::io::msg::message_t msg = writer.create_message(id);
		EXPECT_FALSE(writer.create_message(id).body());

		dark::io::msg::header_t* header = msg.header();
		EXPECT_EQ(header->flag,DARK_IO_MSG_HEADER_FLAG);
		EXPECT_EQ(header->id,id);
		EXPECT_EQ(header->size,str1.size());

		dark::io::msg::body_t body = msg.body();
		EXPECT_TRUE(body);
		EXPECT_EQ(body.size(),str1.size());
		EXPECT_EQ(std::string(body.get(),body.size()),str1);
	}

}
TEST(IoMsgMessageReaderTest, HandleNoneZeroInput)
{
	dark::io::msg::writer_t writer;
	std::string str1 = "kate beckinsale is so sexy\n";
	std::string str2 = "i love her\n";
	writer.write(str1);
	writer.write(str2);
	std::size_t id = 1;
	dark::io::msg::message_t msg = writer.create_message(id);
	dark::io::msg::data_t data = msg.data();
	EXPECT_EQ(data.size(),str1.size() + str2.size() + sizeof(dark::io::msg::header_t));

	dark::io::msg::reader_t reader;
	reader.write(data,data.size());
	dark::io::msg::message_t msg0 = reader.get_message();
	EXPECT_TRUE(msg0);
	dark::io::msg::message_t msg1 = reader.get_message();
	EXPECT_FALSE(msg1);

	dark::io::msg::header_t* header = msg0.header();
	EXPECT_EQ(header->flag,DARK_IO_MSG_HEADER_FLAG);
	EXPECT_EQ(header->id,id);
	EXPECT_EQ(header->size,str1.size() + str2.size());

	dark::io::msg::body_t body = msg0.body();
	EXPECT_TRUE(body);
	EXPECT_EQ(body.size(),str1.size() + str2.size());
	EXPECT_EQ(std::string(body.get(),body.size()),str1 + str2);
}