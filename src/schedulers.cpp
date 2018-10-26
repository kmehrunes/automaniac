#include <chrono>
#include <vector>
#include <string>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "schedulers.h"
#include "jobs-processing.h"
#include "util.hpp"
#include "timeutil.h"

using namespace std::chrono;
using namespace schedulers;
using namespace boost;

#define SECONDS 1000
#define MINUTES SECONDS * 60
#define HOURS MINUTES * 60

std::vector<std::string>
schedulers::splitArgsByBlanks(const std::string & argsString)
{
	std::vector<std::string> arguments;
	boost::split(arguments, argsString, boost::is_any_of(" \t"), boost::token_compress_on);

	return arguments;
}

void
schedulers::scheduleJobThread(const Job & job)
{
	const std::string & scheduler = job.description.scheduler;
	const std::vector<std::string> arguments = splitArgsByBlanks(job.description.arguments);

	const SchedulerJobInfo params = SchedulerJobInfo {
		arguments,
		job.description.options,
		job.statements
	};

	if (scheduler.compare("every") == 0) {
		every(params);
	}
	else if (scheduler.compare("after") == 0) {
		after(params);
	}
	else if (scheduler.compare("now") == 0) {
		now(params);
	}
	else if (scheduler.compare("watch") == 0) {
		watch(params);
	}
	else if (scheduler.compare("on") == 0) {
		on(params);
	}
	else if (scheduler.compare("tomorrow") == 0) {
		tomorrow(params);
	}
	else {
		printerr("Unknown scheduler " + scheduler);
	}
}

void
schedulers::runJobThread(timeutil::DurationUnit waitDuration, bool repeat, 
						 JobOptions options, std::vector<Statement> statements)
{
	while (1) {
		std::this_thread::sleep_for(waitDuration);

		jobs::runJobStatements(statements, options.exitOnFail);

		if (!repeat)
			break;
	}
}

void
schedulers::every(const SchedulerJobInfo & jobInfo)
{
	timeutil::parseDurationArgs(jobInfo.arguments)
		.mapSuccess<timeutil::DurationUnit>([&](const timeutil::DurationArgs & args) {
			const std::string & name = jobInfo.options.name;
			println("[" + name + "] will be scheduled to run every " + std::to_string(args.count) 
						+ " " + args.unit + "(s)");
			return parseDuration(args);
		})
		.onSuccess([&](timeutil::DurationUnit duration) {
			const std::string & name = jobInfo.options.name;
			println("[" + name + "] scheduled successfully");
			runJobThread(duration, true, jobInfo.options, jobInfo.statements);
		});
}

void
schedulers::after(const SchedulerJobInfo & jobInfo)
{
	timeutil::parseDurationArgs(jobInfo.arguments)
		.mapSuccess<timeutil::DurationUnit>([&](const timeutil::DurationArgs & args) {
			const std::string & name = jobInfo.options.name;
			println("[" + name + "] will be scheduled to run after " + std::to_string(args.count) 
						+ " " + args.unit + "(s)");
			return parseDuration(args);
		})
		.onSuccess([&](timeutil::DurationUnit duration) {
			const std::string & name = jobInfo.options.name;
			println("[" + name + "] scheduled successfully");
			runJobThread(duration, false, jobInfo.options, jobInfo.statements);
		});
}

void
schedulers::now(const SchedulerJobInfo & jobInfo)
{
	const std::string & name = jobInfo.options.name;
	println("[" + name + "] will run now");
	runJobThread(1ms, false, jobInfo.options, jobInfo.statements);
}

void
schedulers::watch(const SchedulerJobInfo & jobInfo)
{
	if (jobInfo.arguments.size() < 1) {
		println("A file path is required!");
		return;
	}

	const filesystem::path path = jobInfo.arguments[0];
	const std::string & name = jobInfo.options.name;

	bool fileExisted = filesystem::exists(path);
	std::time_t previousLastModified = fileExisted ? last_write_time(path) : 0;

	while (1) {
		std::this_thread::sleep_for(5s);
		bool fileExists = filesystem::exists(path);

		// checking exitence
		if (fileExists) {
			std::time_t currentLastModified = last_write_time(path);

			// the file was created during the sleep interval
			if (!fileExisted) {
				println("[" + name + "] file was created");
				runJobThread(1ms, false, jobInfo.options, jobInfo.statements);
			}
			// the file was modified during the sleep interval
			else if (currentLastModified - previousLastModified != 0) {
				println("[" + name + "] file was modified");
				runJobThread(1ms, false, jobInfo.options, jobInfo.statements);
			}

			previousLastModified = currentLastModified;
		}
	}
}

void
schedulers::on(const SchedulerJobInfo & jobInfo)
{
	int argsSize = jobInfo.arguments.size();

	if (argsSize == 1) {
		const std::string & name = jobInfo.options.name;

		parseTime(timeutil::TimeParsingType::DATE, jobInfo.arguments)
			.mapSuccess<std::time_t>([] (const std::tm & time) {
				return succeed(timeutil::timeDiffFromNow(time));
			})
			.onSuccess([&] (const std::time_t & difference) {
				timeutil::DurationUnit duration = milliseconds(difference * SECONDS);
				println("[" + name + "] will be scheduled to run on " + jobInfo.arguments.at(0) + 
						+ " at 00:00:00 (" + std::to_string(duration.count()) + " ms)");
				runJobThread(duration, false, jobInfo.options, jobInfo.statements);
			})
			.onFailure([] (const Error & err) {
				printerr(err.message);
			});
	} 
	else if (argsSize == 3) {
		onAt(jobInfo);
	}
	else {
		printerr("Invalid number of arguments");
	}
}

void
schedulers::onAt(const SchedulerJobInfo & jobInfo)
{
	if (jobInfo.arguments.size() != 3) {
		printerr("On-at scheduler requires three arguments");
		return;
	}

	if (jobInfo.arguments.at(1).compare("at") != 0) {
		printerr("Unrecognize command " + jobInfo.arguments.at(1));
		return;
	}

	const std::string & name = jobInfo.options.name;

	std::vector<std::string> dateTimeArgs = {
		jobInfo.arguments.at(0) + "-" + jobInfo.arguments.at(2)
	};
	parseTime(timeutil::TimeParsingType::DATE_TIME, dateTimeArgs)
		.mapSuccess<std::time_t>([] (const std::tm & time) {
			return succeed(timeutil::timeDiffFromNow(time));
		})
		.onSuccess([&] (const std::time_t & difference) {
			timeutil::DurationUnit duration = milliseconds(difference * SECONDS);

			println("[" + name + "] will be scheduled to run on " + jobInfo.arguments.at(0) + 
						+ " at " + jobInfo.arguments.at(2) + " (" + std::to_string(duration.count()) + " ms)");
			runJobThread(duration, false, jobInfo.options, jobInfo.statements);
		})
		.onFailure([] (const Error & err) {
			printerr(err.message);
		});
}

void
schedulers::tomorrow(const SchedulerJobInfo & jobInfo)
{
	int argsSize = jobInfo.arguments.size();

	// there's a bug which causes the argsSize to be always at least 1
	if (argsSize == 1) {
		const std::string & name = jobInfo.options.name;
		const std::time_t diff = timeutil::timeDiffFromNow(timeutil::tomorrow());
		timeutil::DurationUnit duration = milliseconds(diff * SECONDS);

		println("[" + name + "] will be scheduled to run tomorrow at 00:00:00 (" + 
			std::to_string(duration.count()) + " ms)");
		runJobThread(duration, false, jobInfo.options, jobInfo.statements);
	} 
	else if (argsSize == 2) {
		tomorrowAt(jobInfo);
	}
	else {
		printerr("Invalid number of arguments");
	}
}

void
schedulers::tomorrowAt(const SchedulerJobInfo & jobInfo)
{
	if (jobInfo.arguments.size() != 2) {
		printerr("Tomorrow-at scheduler requires two arguments");
		return;
	}

	if (jobInfo.arguments.at(0).compare("at") != 0) {
		printerr("Unrecognize command " + jobInfo.arguments.at(0));
		return;
	}

	const std::string & name = jobInfo.options.name;
	std::vector<std::string> timeArgs = { jobInfo.arguments.at(1) };

	parseTime(timeutil::TimeParsingType::TIME, timeArgs)
		.mapSuccess<std::time_t>([] (const std::tm & time) {
			std::tm tomorrowDate = timeutil::tomorrow();
			std::tm tomorrowFull = timeutil::addTime(tomorrowDate, time.tm_hour, time.tm_min, time.tm_sec);
			return succeed(timeutil::timeDiffFromNow(tomorrowFull));
		})
		.onSuccess([&] (const std::time_t & difference) {
			timeutil::DurationUnit duration = milliseconds(difference * SECONDS);

			println("[" + name + "] will be scheduled to run tomorrow at " + jobInfo.arguments.at(1) +
			 " (" + std::to_string(duration.count()) + " ms)");
			runJobThread(duration, false, jobInfo.options, jobInfo.statements);
		})
		.onFailure([] (const Error & err) {
			printerr(err.message);
		});
}