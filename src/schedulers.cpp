#include <chrono>
#include <vector>
#include <string>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "schedulers.h"
#include "jobs-processing.h"
#include "util.hpp"

using namespace std::chrono;
using namespace schedulers;
using namespace boost;

#define SECONDS 1000
#define MINUTES SECONDS * 60
#define HOURS MINUTES * 60

ResultOrError<DurationUnit> 
schedulers::parseDuration(const DurationArgs & args)
{
	return parseDuration(args.count, args.unit);
}

ResultOrError<DurationUnit> 
schedulers::parseDuration(unsigned long count, const std::string & unit)
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

ResultOrError<DurationArgs> 
schedulers::parseDurationArgs(const std::vector<std::string> & args)
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
	else {
		printerr("Unknown scheduler " + scheduler);
	}
}

void
schedulers::runJobThread(DurationUnit waitDuration, bool repeat, 
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
	parseDurationArgs(jobInfo.arguments)
		.mapSuccess<DurationUnit>([&](const DurationArgs & args) {
			const std::string & name = jobInfo.options.name;
			println("[" + name + "] will be scheduled to run every " + std::to_string(args.count) 
						+ " " + args.unit + "(s)");
			return parseDuration(args);
		})
		.onSuccess([&](DurationUnit duration) {
			const std::string & name = jobInfo.options.name;
			println("[" + name + "] scheduled successfully");
			runJobThread(duration, true, jobInfo.options, jobInfo.statements);
		});
}

void
schedulers::after(const SchedulerJobInfo & jobInfo)
{
	parseDurationArgs(jobInfo.arguments)
		.mapSuccess<DurationUnit>([&](const DurationArgs & args) {
			const std::string & name = jobInfo.options.name;
			println("[" + name + "] will be scheduled to run after " + std::to_string(args.count) 
						+ " " + args.unit + "(s)");
			return parseDuration(args);
		})
		.onSuccess([&](DurationUnit duration) {
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

	filesystem::path path = jobInfo.arguments[0];

	// checking exitence
	if (filesystem::exists(path)) {
		println("File exists");
	}
	else {
		println("File not found");
	}

	// if the file didn't exist and then it was created, run the job

	// checking last write time
	// std::time_t lastModified = last_write_time(path);

	// time difference less than checking interval? if so, run task
}