#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/process/search_path.hpp>
#include <boost/algorithm/string/join.hpp>

#include <unordered_map>
#include <string>

#include "commands.h"

using namespace boost;

namespace process_wrappers
{
	int system(const std::string & fullCommand)
	{
		return process::system(fullCommand);
	}

	int spawn(const std::string & fullCommand)
	{
		process::spawn(fullCommand);
		return 0;
	}
}

/*
 * TODO: move this map to a file so that it could 
 *       changed without having to recompile the 
 *       the whole thing.
 */
const std::unordered_map<std::string, std::string> 
		scriptRunners = {
			{"py", "python"},
			{"sh", "bash"}
		};

ResultOrError<std::string> getFileExtension(const std::string & filename) {
	size_t index = filename.rfind('.');
	if (index == std::string::npos || index == filename.length() - 1)
		return fail("Failed to detect the type of file " + filename);

	return succeed(filename.substr(index + 1));
}

ResultOrError<boost::filesystem::path> getCommandPath(const std::string & command)
{
	filesystem::path path = process::search_path(command);
	if (path.native().empty())
		return fail("Couldn't locate command '" + command + "'");

	return succeed(path);
}

ResultOrError<int> executeCommand(const std::string & command, const std::string & commandWithArgs, 
									std::function<int(const std::string &)> executor)
{
	return getCommandPath(command)
			.mapSuccess<int>([&](const auto &) {
				return succeed(executor(commandWithArgs));
			});
}

ResultOrError<int> 
commands::exec(const std::string & command, const std::vector<std::string> & args)
{
	return executeCommand(command, command + ' ' + algorithm::join(args, " "), process_wrappers::system);
}

ResultOrError<int> 
commands::exec(const std::string & command, const std::string & file, 
					const std::vector<std::string> & args)
{
	return executeCommand(command, command + ' ' + file + ' ' + algorithm::join(args, " "), 
							process_wrappers::system);
}

ResultOrError<int> 
commands::exec(const std::vector<std::string> & allArgs)
{
	if (allArgs.size() < 1)
		return fail("Needs at least one argument");

	return executeCommand(allArgs.at(0), algorithm::join(allArgs, " "), process_wrappers::system);
}

ResultOrError<int>
commands::run(const std::string & script, const std::vector<std::string> & args)
{
	return getFileExtension(script)
			.mapSuccess<int>([&](const auto & ext) {
				try {
					const std::string & runner = scriptRunners.at(ext);
					return exec(runner, script, args);
				} catch (const std::out_of_range &) {
					return ResultOrError<int>(fail("Couldn't run script with extention " + ext));
				}
			});
}

ResultOrError<int>
commands::run(const std::vector<std::string> & allArgs)
{
	if (allArgs.size() < 1)
		return fail("Needs at least one argument");

	return getFileExtension(allArgs.at(0))
			.mapSuccess<int>([&](const auto & ext) {
				try {
					const std::string & runner = scriptRunners.at(ext);
					return exec(runner, allArgs);
				} catch (const std::out_of_range &) {
					return ResultOrError<int>(fail("Couldn't run script with extention " + ext));
				}
			});
}


ResultOrError<int> 
commands::spawn(const std::string & command, const std::vector<std::string> & args)
{
	return executeCommand(command, command + ' ' + algorithm::join(args, " "), process_wrappers::spawn);
}

ResultOrError<int> 
commands::spawn(const std::string & command, const std::string & file, 
					const std::vector<std::string> & args)
{
	return executeCommand(command, command + ' ' + file + ' ' + algorithm::join(args, " "), 
							process_wrappers::spawn);
}

ResultOrError<int> 
commands::spawn(const std::vector<std::string> & allArgs)
{
	if (allArgs.size() < 1)
		return fail("Needs at least one argument");

	return executeCommand(allArgs.at(0), algorithm::join(allArgs, " "), process_wrappers::spawn);
}
