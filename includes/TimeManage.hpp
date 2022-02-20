#ifndef TIMEMANAGE_HPP
#define TIMEMANAGE_HPP

#include <time.h>

class TimeManage
{
private:
	// 秒単位
	static const int TIME_LIMIT = 10;
	time_t _time;

public:
	TimeManage();
	~TimeManage();

	void start();
	bool check() const;

private:
	TimeManage(TimeManage const& other);
	TimeManage& operator=(TimeManage const& other);
};

#endif
