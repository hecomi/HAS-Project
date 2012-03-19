#include <iostream>
#include "home_automation_system.hpp"

int main(int argc, char const* argv[])
{
	iRemocon iremocon("192.168.0.3", 51013);
	Julius julius("julius/hmm_mono.jconf", "julius/gram/kaden");
	OpenJTalk open_jtalk("openjtalk/mei_normal");

	HomeAutomationSystem has(iremocon, julius, open_jtalk);
	has.learn_commands_from_xml("julius/gram/commands.xml");
	has.start();

	return 0;
}
