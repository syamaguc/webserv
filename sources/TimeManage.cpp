#include "TimeManage.hpp"

TimeManage::TimeManage() { }

TimeManage::~TimeManage() { }

/**
 * @brief 計測の開始
 */
void TimeManage::start()
{
	_time = time(NULL);
}

/**
 * @brief 規定時間経過したか確認
 * @return 経過していた場合はtrue
 */
bool TimeManage::check() const
{
	time_t now = time(NULL);
	return _time < now && now - _time >= TIME_LIMIT;
}
