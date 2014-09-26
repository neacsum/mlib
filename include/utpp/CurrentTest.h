#pragma once

namespace UnitTest {

class TestResults;
class TestDetails;

namespace CurrentTest
{
	TestResults*& Results();
	const TestDetails*& Details();
}

}
