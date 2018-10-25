#ifndef JOBS_H
#define JOBS_H

#include <string>
#include <vector>
#include <map>
#include <cctype>

#include "failure.hpp"

typedef std::map<std::string, std::string> OptionsMap;

struct Statement
{
	std::string runner;
	std::vector<std::string> arguments;
};

struct JobOptions
{
	std::string name;
	std::string outputFile;
	bool exitOnFail;
};

struct JobDescription
{
	std::string scheduler;
	std::string arguments;
	JobOptions options;
};

struct Job
{
	JobDescription description;
	std::vector<Statement> statements;
};

namespace jobparsers
{
	struct ExtractionResult
	{
		const std::string extractedText;
		const size_t startIndex;
		const size_t endIndex;
	};

	/* Description */
	bool validateDescriptionLine(const std::string & description);
	bool validateOptionsString(const std::string & optionsString);

	ExtractionResult extractScheduler(const std::string & description, int from = 0);
	ExtractionResult extractOptions(const std::string & description, int from = 0);

	OptionsMap mapOptions(const std::string & optionsText, 
		std::function<std::vector<std::string>(const std::string &)> splitter);

	//---- Scheduler splitters ----
	std::vector<std::string> splitScheduler(const std::string & schedulerText);

	//---- Options splitters ----
	std::vector<std::string> splitByCommas(const std::string & optionsText);

	/* ---------- */

	/* Statements */

	bool validateStatementString(const std::string & statementString);
	std::vector<std::string> extractJobStatements(const std::string & body);
	ExtractionResult extractCommand(const std::string & statementText);
	Statement parseStatement(const std::string & statementText);

	/* ---------- */
	
	int skipToJobDescription(const std::vector<std::string> & lines, int fromIndex);
	std::pair<std::vector<std::string>, unsigned> getNextJob(const std::vector<std::string> & lines, unsigned fromIndex);
	std::vector<std::vector<std::string>> separateJobsLines(const std::vector<std::string> & allLines);

	ResultOrError<JobDescription> parseDescription(const std::string & descriptionLine);
	ResultOrError<Job> parseJob(const std::vector<std::string> & jobLines);
	ResultOrError<JobOptions> mapJobOptions(const OptionsMap & optionsMap);
}

#endif