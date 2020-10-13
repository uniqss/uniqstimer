#include "simplemessagequeue.h"

void SimpleMessageQueue::AddMessage(uint64_t msgId, const string& msg)
{
	SimpleMessage m;
	m.msgId = msgId;
	m.msg = msg;
	mutex.lock();
	msgQueue.push(m);
	mutex.unlock();
}

void SimpleMessageQueue::AddMessage(uint64_t msgId, const char* msg)
{
	SimpleMessage m;
	m.msgId = msgId;
	m.msg = msg;
	mutex.lock();
	msgQueue.push(m);
	mutex.unlock();
}

bool SimpleMessageQueue::PopMessage(SimpleMessage& msg)
{
	mutex.lock();

	if (msgQueue.empty())
	{
		mutex.unlock();
		return false;
	}

	msg = msgQueue.back();
	msgQueue.pop();

	mutex.unlock();
	return true;
}
