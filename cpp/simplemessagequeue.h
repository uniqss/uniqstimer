#pragma once

#include<mutex>
#include<queue>
#include<stdint.h>
#include<string>
using std::string;

class SimpleMessage
{
public:
	uint64_t msgId;
	string msg;
};

class SimpleMessageQueue
{
private:
	std::mutex mutex;
	std::queue<SimpleMessage> msgQueue;
public:
	void AddMessage(uint64_t msgId, const string& msg);
	void AddMessage(uint64_t msgId, const char* msg);
	bool PopMessage(SimpleMessage& msg);
};
