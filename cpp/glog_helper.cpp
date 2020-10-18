#include "glog_helper.h"

void init_glog(const char* pname, const std::string& logDir)
{
	google::InitGoogleLogging(pname);
	FLAGS_colorlogtostderr = true;
	FLAGS_logbufsecs = 0;
	FLAGS_max_log_size = 1024 * 1024;
	FLAGS_stop_logging_if_full_disk = true;

#ifdef GLOG_SYSTEM_TYPE_WINDOWS

#else
	bool result = dirExists(logDir);
	if (!result)
	{
		::mkdir(logDir.c_str(), 0755);
	}
#endif

	char buffer[256] = { 0 };

	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%s/info.log", logDir.c_str());
	google::SetLogDestination(google::GLOG_INFO, buffer);

	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%s/warning.log", logDir.c_str());
	google::SetLogDestination(google::GLOG_WARNING, buffer);

	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%s/error.log", logDir.c_str());
	google::SetLogDestination(google::GLOG_ERROR, buffer);

	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer), "%s/fatal.log", logDir.c_str());
	google::SetLogDestination(google::GLOG_FATAL, buffer);
}

int64_t GetTimeMS()
{
	auto time_now = chrono::system_clock::now();
	auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());
	return duration_in_ms.count();
}


#if 0
int main(int argc, const char** argv)
{
	init_glog(argv[0], "./logs");

#if 0
	char str[20] = "hello glog!";
	LOG(INFO) << str;
	std::string cStr = "hello google!";
	LOG(INFO) << cStr;
	LOG(INFO) << "info test" << "hello log!";  //输出一个Info日志
	LOG(WARNING) << "warning test";  //输出一个Warning日志
	LOG(ERROR) << "error test";  //输出一个Error日志
#else
	srand(GetTimeMS());
	vector<int> vi1;
	vector<int> vi2;
	vector<string> vs1;
	vector<string> vs2;
	for (int nIdx = 0; nIdx < 10; ++nIdx)
	{
		vi1.push_back(rand());
		vi2.push_back(rand());
		vs1.push_back(" " + std::to_string(rand()) + " ");
		vs2.push_back(" " + std::to_string(rand()) + " ");
	}

	int nTestCount = 10000000;

	auto tbegin = GetTimeMS();
	cout << "begin" << endl;

	for (int nIdx = 0; nIdx < nTestCount; ++nIdx)
	{
		LOG(INFO) << " SPEED TEST " << vi1[nIdx % 10] << vs1[nIdx % 10] << vi2[nIdx % 10] << vs2[nIdx % 10];
	}
	auto tend = GetTimeMS();
	auto diff = tend - tbegin;
	auto spd = nTestCount / diff;
	cout << "end " << " diff:" << diff << " speed:" << spd << " msgs/MS " << endl;
#endif

#if 0
	std::string strInput;
	std::cin >> strInput;
	while (strInput != "E" && strInput != "e")
	{
		LOG(ERROR) << strInput;
		std::cin >> strInput;
	}
#endif

	google::ShutdownGoogleLogging();

	return 0;
}

#endif
