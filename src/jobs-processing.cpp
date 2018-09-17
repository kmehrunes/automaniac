
#include "jobs-processing.h"

bool commandFailed(ResultOrError<int> commandResult)
{
	return commandResult.failed() || (commandResult.succeeded() && commandResult.getResult() != 0);
}

#include <iostream>
void
jobs::runJobStatements(const std::vector<Statement> & statements, bool stopOnFail)
{
	for (const auto & statement : statements) {
		if (statement.runner.compare("exec") == 0) {
			if (commandFailed(commands::exec(statement.arguments)) && stopOnFail)
				break;
		}
		else if (statement.runner.compare("run") == 0) {
			if (commandFailed(commands::run(statement.arguments)) && stopOnFail)
				break;
		}
		else if (statement.runner.compare("spawn") == 0) {
			if (commandFailed(commands::spawn(statement.arguments)) && stopOnFail)
				break;
		}
		else {
			break;
		}
	}
}