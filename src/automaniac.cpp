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

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		printerr("Needs one file");
		return 1;
	}

	vector<std::thread> threads;
	vector<Job> jobs;

	for (int i = 1; i < argc; ++i) {
		readFile(argv[i])
			.onSuccess([&](const vector<string> & lines) {
				auto jobsLines = jobparsers::separateJobsLines(lines);

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