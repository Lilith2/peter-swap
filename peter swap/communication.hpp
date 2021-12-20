#pragma once

#define SYSCALL_CODE 0xDEADBEEF

enum operation : int
{
	memory_read = 0,
	memory_write,
	module_base,
};

struct cmd_t
{
	bool success = false;
	unsigned int verification_code = 0;
	operation operation;
};