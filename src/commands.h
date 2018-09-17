#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>

#include "failure.hpp"

namespace commands
{

ResultOrError<int> exec(const std::string & command, const std::vector<std::string> & args);

ResultOrError<int> exec(const std::string & command, const std::string & file, 
						const std::vector<std::string> & args);

ResultOrError<int> exec(const std::vector<std::string> & allArgs);

ResultOrError<int> run(const std::string & script, const std::vector<std::string> & args);

ResultOrError<int> run(const std::vector<std::string> & allArgs);

ResultOrError<int> spawn(const std::string & command, const std::vector<std::string> & args);

ResultOrError<int> spawn(const std::string & command, const std::string & file, 
						const std::vector<std::string> & args);

ResultOrError<int> spawn(const std::vector<std::string> & allArgs);

} // namespace

#endif