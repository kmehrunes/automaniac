#include <string>

#include "catch.hpp"

#include "../commands.h"

using namespace commands;

TEST_CASE( "Commands test", "" ) {
	SECTION( "exec fails" ) {
		REQUIRE( exec("ThisCommandWillMakeItFail", { "--meaningless" }).failed() );
		REQUIRE( exec("ThisCommandWillMakeItFail", "somefile.ext", { "--meaningless" }).failed() );
		REQUIRE( exec({ "ThisCommandWillMakeItFail", "--meaningless" }).failed() );
	}

	SECTION( "run fails" ) {
		REQUIRE( run("noextension", { "--meaningless" }).failed() );
		REQUIRE( run("file.ExtentionToFail", { "--meaningless" }).failed() );
		REQUIRE( run({ "noextension", "--meaningless" }).failed() );
		REQUIRE( run({ "file.ExtentionToFail", "--meaningless" }).failed() );
	}
}