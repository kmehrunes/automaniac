#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <memory>
#include <thread>

#include <boost/algorithm/string.hpp>

#include "util.hpp"
#include "failure.hpp"

#include "jobs.h"
#include "commands.h"
#include "jobs-processing.h"
#include "schedulers.h"
#include "timeutil.h"

using namespace std;

ResultOrError<shared_ptr<ifstream>> openFile(const string & fname) 
{
	shared_ptr<ifstream> file(new ifstream(fname));
	if (file->fail())
		return fail(strerror(errno));
	else
		return succeed(file);
}

ResultOrError<vector<string>> readFile(const string & fname)
{
	auto inputOrError = openFile(fname);

	return inputOrError.mapSuccess<vector<string>>([](const auto & input) {
		vector<string> lines;
		string line;

		while (getline(*input, line)) {
			lines.push_back(line);
		}

		input->close();

		return succeed(lines);
	});
}

int skipToJobDescription(const vector<string> & lines, int fromIndex)
{
	for (unsigned i = fromIndex; i < lines.size(); i++) {
		const std::string & line = lines.at(i);

		if (line.empty() || line.at(0) == ' ' || line.at(0) == '\t')
			continue;

		return i;
	}
	return -1;
}

pair<vector<string>, unsigned> getNextJob(const vector<string> & lines, unsigned fromIndex)
{
	vector<string> jobLines;
	unsigned offset = 0;

	unsigned jobDescriptionIndex = skipToJobDescription(lines, fromIndex);

	offset += jobDescriptionIndex;
	jobLines.push_back(lines.at(jobDescriptionIndex));

	for (auto iter = lines.begin() + jobDescriptionIndex + 1; iter != lines.end(); iter++) {
		const std::string & line = *iter;
		offset++;

		if (line.empty())
			continue;

		if (line.at(0) != ' ' && line.at(0) != '\t') 
			break;

		jobLines.push_back(boost::trim_copy(line));
	}

	return { jobLines, fromIndex + offset };
}

vector<vector<string>> separateJobsLines(const vector<string> & allLines) 
{
	vector<vector<string>> jobsLines;

	unsigned nextIndex = 0;

	while (nextIndex < allLines.size()) {
		pair<vector<string>, unsigned> nextJobLines = getNextJob(allLines, nextIndex);
		jobsLines.push_back(nextJobLines.first);

		if (nextJobLines.second == nextIndex)
			break;

		nextIndex = nextJobLines.second;
	}

	return jobsLines;
}

#include "timeutil.h"
int main(int argc, char const *argv[])
{
	if (argc < 2) {
		printerr("Needs at least one file");
		return 1;
	}

	vector<std::thread> threads;
	vector<Job> jobs;

	for (int i = 1; i < argc; ++i) {
		readFile(argv[i])
			.onSuccess([&](const vector<string> & lines) {
				auto jobsLines = separateJobsLines(lines);

				for (const auto & jobLines : jobsLines) {
					jobparsers::parseJob(jobLines)
						.onSuccess([&jobs] (const auto & job) {
							jobs.push_back(job);
						})
						.onFailure([] (const auto & err) {
							printerr(err.message);
							std::terminate();
						});
				}

				threads.reserve(jobs.size());

				for (const auto & job : jobs) {
					threads.emplace_back(std::thread(schedulers::scheduleJobThread, job));
				}

				for (auto & td : threads) {
					td.join();
				}
			})
			.onFailure([](const Error & err) {
				printerr("Error: " + err.message);
			});
	}

	return 0;
}