#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <iostream>
#include <sstream>
#include <string>
#include <locale>
#include <ctime>
#include <iomanip>

#include "failure.hpp"

namespace timeutil
{
	const std::string FULL_DATE_TIME_PATTERN = "%d-%b-%Y-%H:%M:%S";
	const std::string FULL_DATE_PATTERN = "%d-%b-%Y";
	const std::string DM_DATE_PATTERN = "%d-%b";
	const std::string FULL_TIME_PATTERN = "%H:%M:%S";

	void testTimeDiff();

	std::time_t timeDiff(const std::tm & t1, const std::tm & t2);
	std::time_t timeDiffFromNow(const std::tm & time);
	std::tm now();
	std::tm addToday(const std::tm & t);

	ResultOrError<std::tm> parseTimeString(const std::string & timeString, const std::string & pattern);

	ResultOrError<std::tm> parseFullDateTime(const std::string & timeString);
	ResultOrError<std::tm> parseDatePattern(const std::string & timeString);
	ResultOrError<std::tm> parseDayMonthPattern(const std::string & timeString);
	ResultOrError<std::tm> parseTimePattern(const std::string & timeString);
}

#endif