#include "timeutil.h"

#include <ctime>

#include "util.hpp"
void timeutil::testTimeDiff()
{
	std::string fullstr = "06-Feb-2019-10:15:23";
	std::string datestr = "06-Feb-2019";
	std::string timestr = "10:15:23";
	std::string fullOther = "06-Sep-2018-08:35:50";

	timeutil::parseFullDateTime(fullstr)
		.onSuccess([&](const std::tm & t) {
			println("Diff from now: ");
			println(timeutil::timeDiffFromNow(t));
		})
		.onFailure([](const Error & err) {
			println(err.message);
		});

	timeutil::parseDatePattern(datestr)
		.onSuccess([&](const std::tm & t) {
			println("Diff from now: ");
			println(timeutil::timeDiffFromNow(t));
		})
		.onFailure([](const Error & err) {
			println(err.message);
		});
}

std::tm
timeutil::now()
{
	std::time_t t = std::time(0);   // get time now
    return *(std::localtime(&t));
}

std::time_t 
timeutil::timeDiff(const std::tm & t1, const std::tm & t2)
{
	std::tm copyT1 = t1, copyT2 = t2;
	std::time_t arithTime1 = std::mktime(&copyT1);
	std::time_t arithTime2 = std::mktime(&copyT2);

	return arithTime2 - arithTime1;
}

std::time_t 
timeutil::timeDiffFromNow(const std::tm & time)
{
	return timeDiff(timeutil::now(), time);
}

ResultOrError<std::tm> 
timeutil::parseTimeString(const std::string & timeString, const std::string & pattern)
{
	std::tm t;
	std::stringstream ss(timeString);
	ss >> std::get_time(&t, pattern.c_str());

	if (ss.fail()) {
		return fail("Failed to parse the given time");
	}
	return succeed(t);
}

ResultOrError<std::tm> 
timeutil::parseFullDateTime(const std::string & timeString)
{
	return parseTimeString(timeString, FULL_DATE_TIME_PATTERN);
}

ResultOrError<std::tm> 
timeutil::parseDatePattern(const std::string & timeString)
{
	return parseTimeString(timeString, FULL_DATE_PATTERN)
			.mapSuccess<std::tm>([] (const auto & parsedTime) {
				std::tm copy = parsedTime;
				copy.tm_sec = 0;
				copy.tm_min = 0;
				copy.tm_hour = 0;
				return copy;
			});
}

ResultOrError<std::tm> 
timeutil::parseDayMonthPattern(const std::string & timeString)
{
	return parseTimeString(timeString, DM_DATE_PATTERN)
			.mapSuccess<std::tm>([] (const auto & parsedTime) {
				std::tm copy = parsedTime;
				copy.tm_sec = 0;
				copy.tm_min = 0;
				copy.tm_hour = 0;
				copy.tm_year = 0;
				return copy;
			});
}

ResultOrError<std::tm> 
timeutil::parseTimePattern(const std::string & timeString)
{
	return parseTimeString(timeString, FULL_TIME_PATTERN);
}
