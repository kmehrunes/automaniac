#include "timeutil.h"

#include <ctime>

#define SECONDS 1000
#define MINUTES SECONDS * 60
#define HOURS MINUTES * 60

using namespace std::chrono;

std::tm
timeutil::now()
{
	std::time_t t = std::time(0);   // get time now
	return *(std::localtime(&t));
}

std::tm
timeutil::tomorrow()
{
	std::time_t t = std::time(0);
	t += 24 * 60 * 60; // 24 hours * 60 minutes * 60 seconds
	std::tm tomorrow = *(std::localtime(&t));
	tomorrow.tm_sec = 0;
	tomorrow.tm_min = 0;
	tomorrow.tm_hour = 0;

	return tomorrow;
}

std::tm
timeutil::addTime(const std::tm & t, unsigned hour, unsigned minutes, unsigned seconds)
{
	std::tm copy = t;
	copy.tm_hour = hour;
	copy.tm_min = minutes;
	copy.tm_sec = seconds;

	return copy;
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

ResultOrError<timeutil::DurationUnit> 
timeutil::parseDuration(const DurationArgs & args)
{
	return parseDuration(args.count, args.unit);
}

ResultOrError<timeutil::DurationUnit> 
timeutil::parseDuration(unsigned long count, const std::string & unit)
{
	if (unit.compare("milliseconds") == 0) {
		return succeed(milliseconds(count));
	}
	else if (unit.compare("seconds") == 0) {
		return succeed(milliseconds(count * SECONDS));
	}
	else if (unit.compare("minutes") == 0) {
		return succeed(milliseconds(count * MINUTES));
	}
	else if (unit.compare("hours") == 0) {
		return succeed(milliseconds(count * HOURS));
	}

	return fail("Unrecognized unit " + unit);
}

ResultOrError<timeutil::DurationArgs> 
timeutil::parseDurationArgs(const std::vector<std::string> & args)
{
	if (args.size() != 2)
		return fail("Requires exactly two arguments");

	try {
		unsigned long count = std::stoul(args.at(0));
		return succeed(DurationArgs {
			count,
			args.at(1)
		});
	}
	catch (const std::invalid_argument &) {
		return fail(args.at(0) + " isn't a valid number");
	}
	catch (const std::out_of_range &) {
		return fail(args.at(0) + " is beyond the limits");
	}

	return fail("Failed to process " + args.at(0) + " " + args.at(1));
}

ResultOrError<std::tm> 
timeutil::parseTime(TimeParsingType type, const std::vector<std::string> & args)
{
	if (args.size() == 0)
		return fail("Requires at least one argument");

	if (type == TimeParsingType::DATE) {
		return timeutil::parseDatePattern(args.at(0));
	}
	else if (type == TimeParsingType::TIME) {
		return timeutil::parseTimePattern(args.at(0));
	}
	else if (type == TimeParsingType::DATE_TIME) {
		return timeutil::parseFullDateTime(args.at(0));
	}

	return fail("Unrecognize date/time pattern");
}