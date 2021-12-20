#pragma once
#include "driver.hpp"

int main()
{
	printf("Setup: %d\n", driver::setup());

	cmd_t cmd{};
	cmd.verification_code = SYSCALL_CODE;
	cmd.operation = memory_read;

	printf("Command Sent: %d\n", driver::send_cmd(&cmd));

	getchar();
	return 0;
}