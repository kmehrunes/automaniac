#include <boost/algorithm/string.hpp>
#include <regex>
#include <array>
#include <functional>

#include "jobs.h"

using namespace jobparsers;

bool
jobparsers::validateDescriptionLine(const std::string & description)
{
	static const std::string segments[] = {
		"\\s*[a-zA-Z]+(\\s+\\S+)*\\s*\\(.*\\)\\s*:\\s*", // matches with options
		"\\s*[a-zA-Z]+(\\s+\\S+)*\\s*:\\s*" // matches without options
	};
	static const std::string combined = "(" + segments[0] + "|" + segments[1] + ")(\\n|$)";
	static const std::regex lineregex(combined);

	return std::regex_match(description, lineregex);
}

bool
jobparsers::validateOptionsString(const std::string & optionsString)
{
	static const std::string firstOne = "\\s*\\S+\\s*=\\s*\\S+\\s*";
	static const std::string subsequentOnes = "\\s*\\," + firstOne;
	static const std::string combined = firstOne + "(" + subsequentOnes + ")*($|\\n)";
	static const std::regex optionsregex(combined);

	return std::regex_match(optionsString, optionsregex);
}

ExtractionResult
jobparsers::extractScheduler(const std::string & description, int from)
{
	size_t pos = description.find_first_of("(:");

	if (pos == std::string::npos)
		return ExtractionResult { "", std::string::npos, std::string::npos };

	return ExtractionResult { boost::trim_copy(description.substr(0, pos)), 0, pos };
}


ExtractionResult
jobparsers::extractOptions(const std::string & description, int from)
{
	size_t startPos = description.find('(');
	size_t endPos = description.find(')');

	if (startPos == std::string::npos || endPos == std::string::npos)
		return ExtractionResult { "", startPos, endPos };

	return ExtractionResult { 
		boost::trim_copy(description.substr(startPos + 1, endPos - startPos - 1)), 
		startPos, endPos 
	};
}


OptionsMap 
jobparsers::mapOptions(const std::string & optionsText, 
		std::function<std::vector<std::string>(const std::string &)> splitter)
{
	auto singleOptions = splitter(optionsText);
	OptionsMap optionsMap;

	for (const auto & optionText : singleOptions) {
		std::vector<std::string> optionParts;
		boost::split(optionParts, optionText, boost::is_any_of("="), boost::token_compress_on);

		if (optionParts.size() != 2)
			continue;

		boost::trim(optionParts.at(0));
		boost::trim(optionParts.at(1));

		optionsMap[optionParts.at(0)] = optionParts.at(1);
	}

	return optionsMap;
}

std::vector<std::string> 
jobparsers::splitScheduler(const std::string & schedulerText)
{
	size_t firstDelim = schedulerText.find_first_of(" \t\n");
	if (firstDelim == schedulerText.length() || firstDelim == std::string::npos) {
		return { schedulerText, "" };
	}

	std::string schedulerName = schedulerText.substr(0, firstDelim);
	std::string schedulerArgs = schedulerText.substr(firstDelim + 1);

	return { schedulerName , schedulerArgs };
}

std::vector<std::string> 
jobparsers::splitByCommas(const std::string & optionsText)
{
	std::vector<std::string> options;
	boost::split(options, optionsText, boost::is_any_of(","), boost::token_compress_on);
	return options;
}

ExtractionResult
jobparsers::extractCommand(const std::string & statementText)
{
	size_t firstDelim = statementText.find_first_of(" \t\n");
	if (firstDelim == std::string::npos)
		return ExtractionResult { statementText, 0, statementText.size() - 1 };

	return ExtractionResult { statementText.substr(0, firstDelim) , 0, firstDelim - 1 };
}

bool 
jobparsers::validateStatementString(const std::string & statementString)
{
	static const std::regex statementregex("\\S+\\s+\\S+(\\s+\\S+)*\\s*($|\\n)");

	return std::regex_match(statementString, statementregex);
}

Statement
jobparsers::parseStatement(const std::string & statementText)
{
	std::vector<std::string> parts;
	boost::split(parts, statementText, boost::is_any_of(" \t"), boost::token_compress_on);

	if (parts.size() < 1)
		return Statement { "", std::vector<std::string>() };
 
	return Statement { parts.at(0), std::vector<std::string>(parts.begin() + 1, parts.end()) };
}

int
jobparsers::skipToJobDescription(const std::vector<std::string> & lines, int fromIndex)
{
	for (unsigned i = fromIndex; i < lines.size(); i++) {
		const std::string & line = lines.at(i);

		if (line.empty() || line.at(0) == ' ' || line.at(0) == '\t' || line.at(0) == '#')
			continue;

		return i;
	}
	return -1;
}

std::pair<std::vector<std::string>, unsigned>
jobparsers::getNextJob(const std::vector<std::string> & lines, unsigned fromIndex)
{
	std::vector<std::string> jobLines;
	unsigned offset = 0;

	unsigned jobDescriptionIndex = skipToJobDescription(lines, fromIndex);

	jobLines.push_back(lines.at(jobDescriptionIndex));

	for (auto iter = lines.begin() + jobDescriptionIndex + 1; iter != lines.end(); iter++) {
		const std::string & line = *iter;
		offset++;

		if (line.empty())
			continue;

		if (line.at(0) != ' ' && line.at(0) != '\t') 
			break;

		std::string trimmed = boost::trim_copy(line);
		if (trimmed.at(0) == '#')
			continue;

		jobLines.push_back(trimmed);
	}

	return { jobLines, fromIndex + offset };
}

std::vector<std::vector<std::string>>
jobparsers::separateJobsLines(const std::vector<std::string> & allLines) 
{
	std::vector<std::vector<std::string>> jobsLines;

	unsigned nextIndex = 0;

	while (nextIndex < allLines.size()) {
		std::pair<std::vector<std::string>, unsigned> nextJobLines = getNextJob(allLines, nextIndex);
		jobsLines.push_back(nextJobLines.first);

		if (nextJobLines.second == nextIndex || nextJobLines.second >= allLines.size() - 1)
			break;

		nextIndex = nextJobLines.second;
	}

	return jobsLines;
}

ResultOrError<JobDescription> 
jobparsers::parseDescription(const std::string & descriptionLine)
{
	if (!validateDescriptionLine(descriptionLine)) 
		return fail("Invalid description line");

	ExtractionResult scheduler = extractScheduler(descriptionLine);
	if (scheduler.extractedText.empty())
		return fail("Couldn't extract scheduler");

	std::vector<std::string> schedulerParts = splitScheduler(scheduler.extractedText);

	ExtractionResult optionsString = extractOptions(descriptionLine);
	if (!optionsString.extractedText.empty() && !validateOptionsString(optionsString.extractedText))
		return fail("Invalid options text");

	OptionsMap optionsStringMap = mapOptions(optionsString.extractedText, splitByCommas);

	return mapJobOptions(optionsStringMap)
			.mapSuccess<JobDescription>([&] (const JobOptions & options) {
				return succeed(JobDescription { 
					schedulerParts.at(0),
					schedulerParts.at(1), 
					options 
				});
			});
}

ResultOrError<Job> 
jobparsers::parseJob(const std::vector<std::string> & jobLines)
{
	if (jobLines.size() == 0) return fail("Empty job");

	auto descriptionOrError = parseDescription(jobLines.at(0));

	return descriptionOrError.mapSuccess<Job>([&](const auto & description) -> ResultOrError<Job> {
		Job job;

		job.description = description;

		for (auto iter = jobLines.begin() + 1; iter != jobLines.end(); iter++) {
			const std::string & line = *iter;
			if (line.empty())
				continue;

			if (line[0] == ' ' || line[0] == '\t')
				break;

			job.statements.push_back(parseStatement(boost::trim_copy(*iter)));
		}

		return succeed(job);
	});
}

ResultOrError<JobOptions>
jobparsers::mapJobOptions(const OptionsMap & optionsMap)
{
	auto nameIter = optionsMap.find("name");
	auto outputIter = optionsMap.find("output");
	auto exitIter = optionsMap.find("fail_exit");

	std::string name = nameIter != optionsMap.end() ? nameIter->second : "unnamed job";
	std::string output = outputIter != optionsMap.end() ? outputIter->second : "";

	bool exit = true;
	if (exitIter != optionsMap.end()) {
		if (exitIter->second.compare("yes") == 0) {
			exit = true;
		}
		else if (exitIter->second.compare("no") == 0) {
			exit = false;
		}
		else {
			return fail("Invalid value for option 'fail_exit'; only 'yes' and 'no' are accepted");
		}
	}

	return succeed((JobOptions) {
		name,
		output,
		exit
	});
}