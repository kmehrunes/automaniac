#include <string>

#include "catch.hpp"

#include "../jobs.h"

using namespace jobparsers;

TEST_CASE( "Description extraction", "[Description]" ) {

	//-------- Line matching --------
	SECTION( "matching lines" ) {
		REQUIRE( validateDescriptionLine("scheduler():") );
		REQUIRE( validateDescriptionLine("scheduler ():") );
		REQUIRE( validateDescriptionLine("scheduler () : ") );
		REQUIRE( validateDescriptionLine("scheduler () :\n") );
		REQUIRE( validateDescriptionLine("scheduler:") );
		REQUIRE( validateDescriptionLine("scheduler: ") );
		REQUIRE( validateDescriptionLine("scheduler :\n") );
		REQUIRE( validateDescriptionLine(" scheduler :\n") );

		REQUIRE( validateDescriptionLine("scheduler 12 :\n") );
		REQUIRE( validateDescriptionLine("scheduler arg 12:\n") );
		REQUIRE( validateDescriptionLine("scheduler arg 12 :\n") );

		REQUIRE( validateDescriptionLine("scheduler(opt = arg1):") );
		REQUIRE( validateDescriptionLine("scheduler (opt=arg1):") );
		REQUIRE( validateDescriptionLine("scheduler arg (opt = arg1):") );

		REQUIRE( !validateDescriptionLine("scheduler (:") );
		REQUIRE( !validateDescriptionLine("scheduler )(") );
		REQUIRE( !validateDescriptionLine("scheduler ()") );
		REQUIRE( !validateDescriptionLine("scheduler") );
		REQUIRE( !validateDescriptionLine("scheduler()") );
		REQUIRE( !validateDescriptionLine("scheduler):\n") );
	}

	//----------- Scheduler ---------
	SECTION( "full scheduler extraction" ) {
		ExtractionResult result = extractScheduler("scheduler ():");
		REQUIRE( result.startIndex != std::string::npos );
		REQUIRE( result.endIndex != std::string::npos );
		REQUIRE( result.extractedText.compare("scheduler") == 0 );
	}

	SECTION( "no options scheduler extraction" ) {
		ExtractionResult result = extractScheduler("scheduler:");
		REQUIRE( result.startIndex != std::string::npos );
		REQUIRE( result.endIndex != std::string::npos );
		REQUIRE( result.extractedText.compare("scheduler") == 0 );
	}

	SECTION( "malformed no colons scheduler extraction" ) {
		ExtractionResult result = extractScheduler("scheduler ()");
		REQUIRE( result.startIndex != std::string::npos );
		REQUIRE( result.endIndex != std::string::npos );
		REQUIRE( result.extractedText.compare("scheduler") == 0 );
	}

	SECTION( "malformed nothing scheduler extraction" ) {
		ExtractionResult result = extractScheduler("scheduler");
		REQUIRE( result.startIndex == std::string::npos );
		REQUIRE( result.endIndex == std::string::npos );
		REQUIRE( result.extractedText.empty() );
	}

	SECTION( "scheduler and arguments split" ) {
		std::vector<std::string> schedulerParts = splitScheduler("scheduler arg1 arg2");
		REQUIRE( schedulerParts.size() == 2 );
		REQUIRE( schedulerParts.at(0).compare("scheduler") == 0 );
		REQUIRE( schedulerParts.at(1).compare("arg1 arg2") == 0 );
	}

	SECTION( "scheduler and no arguments split" ) {
		std::vector<std::string> schedulerParts = splitScheduler("scheduler");
		REQUIRE( schedulerParts.size() == 2 );
		REQUIRE( schedulerParts.at(0).compare("scheduler") == 0 );
		REQUIRE( schedulerParts.at(1).length() == 0 );
	}

	//----------- Options -----------
	SECTION( "full options extraction" ) {
		ExtractionResult result = extractOptions("scheduler (opt=1, opt=2):");
		REQUIRE( result.startIndex != std::string::npos );
		REQUIRE( result.endIndex != std::string::npos );
		REQUIRE( result.extractedText.compare("opt=1, opt=2") == 0 );
	}

	SECTION( "empty options extraction" ) {
		ExtractionResult result = extractOptions("scheduler ():");
		REQUIRE( result.startIndex != std::string::npos );
		REQUIRE( result.endIndex != std::string::npos );
		REQUIRE( result.extractedText.empty() );
	}

	SECTION( "no options extraction" ) {
		ExtractionResult result = extractOptions("scheduler:");
		REQUIRE( result.startIndex == std::string::npos );
		REQUIRE( result.endIndex == std::string::npos );
		REQUIRE( result.extractedText.empty() );
	}

	SECTION( "malformed left options extraction" ) {
		ExtractionResult result = extractOptions("scheduler (:");
		REQUIRE( result.startIndex != std::string::npos );
		REQUIRE( result.endIndex == std::string::npos );
		REQUIRE( result.extractedText.empty() );
	}

	SECTION( "malformed right options extraction" ) {
		ExtractionResult result = extractOptions("scheduler ):");
		REQUIRE( result.startIndex == std::string::npos );
		REQUIRE( result.endIndex != std::string::npos );
		REQUIRE( result.extractedText.empty() );
	}

	//--------- Options mapping -------
	SECTION( "verifying options" ) {
		REQUIRE( validateOptionsString("opt = 1") );
		REQUIRE( validateOptionsString(" opt= 1 ") );
		REQUIRE( validateOptionsString("opt=1") );
		REQUIRE( validateOptionsString("opt = 1, opt = 2") );
		REQUIRE( validateOptionsString("opt = 1,opt=2 ") );
		REQUIRE( validateOptionsString("opt=1,opt=2") );
		REQUIRE( validateOptionsString("opt=1,") ); // we don't care if it ends with a comma

		REQUIRE( !validateOptionsString("opt=") );
		REQUIRE( !validateOptionsString("opt = ") );
		REQUIRE( !validateOptionsString("opt=1, opt") );
		REQUIRE( !validateOptionsString("opt=1, opt=") );
	}

	SECTION( "two options" ) {
		OptionsMap options = mapOptions("opt1=1, opt2=2", splitByCommas);
		REQUIRE( options.size() == 2 );
		REQUIRE( options["opt1"].compare("1") == 0 );
		REQUIRE( options["opt2"].compare("2") == 0 );
	}

	SECTION( "one option" ) {
		OptionsMap options = mapOptions("opt1=1", splitByCommas);
		REQUIRE( options.size() == 1 );
		REQUIRE( options["opt1"].compare("1") == 0 );
	}

	SECTION( "no options" ) {
		OptionsMap options = mapOptions("", splitByCommas);
		REQUIRE( options.size() == 0 );
	}

	SECTION( "malformed option" ) {
		OptionsMap options = mapOptions("opt1", splitByCommas);
		REQUIRE( options.size() == 0 );
	}
}

TEST_CASE( "Statements extraction", "[Statements]" ) {
	SECTION( "command extraction with arguments" ) {
		ExtractionResult result = extractCommand("command arg");
		REQUIRE( result.startIndex == 0 );
		REQUIRE( result.endIndex == 6 );
		REQUIRE( result.extractedText.compare("command") == 0 );
	}

	SECTION( "command alone" ) {
		ExtractionResult result = extractCommand("command");
		REQUIRE( result.startIndex == 0 );
		REQUIRE( result.endIndex == 6 );
		REQUIRE( result.extractedText.compare("command") == 0 );
	}

	SECTION( "command with arguments" ) {
		Statement result = parseStatement("command arg");
		REQUIRE( result.runner.compare("command") == 0 );
		REQUIRE( result.arguments.size() == 1 );
		REQUIRE( result.arguments.at(0).compare("arg") == 0 );
	}

	SECTION( "command with no arguments" ) {
		Statement result = parseStatement("command");
		REQUIRE( result.runner.compare("command") == 0 );
		REQUIRE( result.arguments.size() == 0 );
	}

	SECTION( "command with arguments-multiple spaces" ) {
		Statement result = parseStatement("command  \t arg");
		REQUIRE( result.runner.compare("command") == 0 );
		REQUIRE( result.arguments.size() == 1 );
		REQUIRE( result.arguments.at(0).compare("arg") == 0 );
	}
}

TEST_CASE( "Parsing" ) {

	SECTION( "parsing description, no options" ) {
		ResultOrError<JobDescription> result = parseDescription("scheduler:");

		REQUIRE( result.succeeded() );
		REQUIRE( result.getResult().scheduler.compare("scheduler") == 0 );
		// REQUIRE( result.getResult().options.size() == 0 );
	}

	SECTION( "parsing description, one option" ) {
		ResultOrError<JobDescription> result = parseDescription("scheduler (arg1 = val1):");

		REQUIRE( result.succeeded() );
		REQUIRE( result.getResult().scheduler.compare("scheduler") == 0 );
		// REQUIRE( result.getResult().options.size() == 1 );
		// REQUIRE( result.getResult().options.at("arg1").compare("val1") == 0 );
	}

	SECTION( "parsing description, multiple options" ) {
		ResultOrError<JobDescription> result = parseDescription("scheduler (arg1 = val1, arg2 = val2):");

		REQUIRE( result.succeeded() );
		REQUIRE( result.getResult().scheduler.compare("scheduler") == 0 );
		// REQUIRE( result.getResult().options.size() == 2 );
		// REQUIRE( result.getResult().options.at("arg1").compare("val1") == 0 );
		// REQUIRE( result.getResult().options.at("arg2").compare("val2") == 0 );
	}
}