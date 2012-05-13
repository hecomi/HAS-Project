#include <iostream>
#include "home_automation_system.hpp"

int main(int argc, char const* argv[])
{
	HomeAutomationSystem has;
	has.exec_script("./hoge.js");
	has.exec_script("./test.js");

	return 0;
}
