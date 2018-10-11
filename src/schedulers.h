#ifndef SCHEDULERS_H
#define SCHEDULERS_H

#include <vector>
#include <string>
#include <chrono>

#include "failure.hpp"
#include "jobs.h"

namespace schedulers
{
	typedef std::chrono::milliseconds DurationUnit;
	typedef std::vector<std::string> StringVector;

	struct DurationArgs
	{
		unsigned long count;
		std::string unit;
	};

	struct SchedulerJobInfo
	{
		const std::vector<std::string> & arguments;
		const JobOptions & options;
		const std::vector<Statement> & statements;
	};

	enum TimeParsingType
	{
		DATE,
		TIME,
		DATE_TIME
	};

	ResultOrError<DurationArgs> parseDurationArgs(const std::vector<std::string> & args);
	ResultOrError<DurationUnit> parseDuration(const DurationArgs & args);
	ResultOrError<DurationUnit> parseDuration(unsigned long count, const std::string & unit);
	ResultOrError<std::tm> parseTime(TimeParsingType type, const std::vector<std::string> & args);

	std::vector<std::string> splitArgsByBlanks(const std::string & argsString);

	void scheduleJobThread(const Job & job);
	void runJobThread(DurationUnit waitDuration, bool repeat, 
		   			  JobOptions options, std::vector<Statement> statements);

	void every(const SchedulerJobInfo & params);
	void after(const SchedulerJobInfo & params);
	void now(const SchedulerJobInfo & params);
	void watch(const SchedulerJobInfo & params);
	void on(const SchedulerJobInfo & params);
	void onAt(const SchedulerJobInfo & params);
}

#endif