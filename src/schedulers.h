#ifndef SCHEDULERS_H
#define SCHEDULERS_H

#include <vector>
#include <string>
#include <chrono>

#include "failure.hpp"
#include "timeutil.h"
#include "jobs.h"

namespace schedulers
{
	struct SchedulerJobInfo
	{
		const std::vector<std::string> & arguments;
		const JobOptions & options;
		const std::vector<Statement> & statements;
	};

	std::vector<std::string> splitArgsByBlanks(const std::string & argsString);

	void scheduleJobThread(const Job & job);
	void runJobThread(timeutil::DurationUnit waitDuration, bool repeat, 
		   			  JobOptions options, std::vector<Statement> statements);

	void every(const SchedulerJobInfo & params);
	void after(const SchedulerJobInfo & params);
	void now(const SchedulerJobInfo & params);
	void watch(const SchedulerJobInfo & params);
	void on(const SchedulerJobInfo & params);
	void onAt(const SchedulerJobInfo & params);
	void tomorrow(const SchedulerJobInfo & params);
	void tomorrowAt(const SchedulerJobInfo & params);
}

#endif