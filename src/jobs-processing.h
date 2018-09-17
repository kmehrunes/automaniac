#ifndef JOBSPROCESSING_H
#define JOBSPROCESSING_H

#include "commands.h"
#include "jobs.h"

namespace jobs 
{
	void runJobStatements(const std::vector<Statement> & statements, bool stopOnFail = true);
}

#endif