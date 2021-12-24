#include "utils.hpp"

/*	
*	READ ME --
*	do not expect this bypass to work for
*	anything better than battleye without
*	pointer chaining.
* 
*	this specific function i used works
*	however you blue screen when resizing
*	a window, find a different Nt function
*	that calls a qword ptr which gets args
*	passed by the Nt func to it and you'll 
*	be fine.
* 
*	credits ---
*	 - krispy
*	 - sariaki (for minor help lol)
*/

#define QWORD_SIGNATURE _("\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x74\x1A\x44\x8B\x54\x24\x00\x44\x89\x54\x24\x00\x44\x8B\x54\x24\x00\x44\x89\x54\x24\x00\xFF\x15\x00\x00\x00\x00\x48\x83\xC4\x48\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x48\x83\xEC\x38")
#define QWORD_MASK		_("xxx????xxxxxxxxx?xxxx?xxxx?xxxx?xx????xxxxxxxxxxxxxxxxxx")

__int64(__fastcall* original_NtUserMoveWindow)(void* arg01, void* arg02);

__int64 __fastcall NtUserMoveWindow(void* arg01, void* arg02)
{
	/* 
	*	make sure the process calling the functions
	*	is ours by comparing the verifcation code
	*/
	if (reinterpret_cast<cmd_t*>(arg01)->verification_code != SYSCALL_CODE)
		return original_NtUserMoveWindow(arg01, arg02);
	
	DbgPrintEx(0, 0, _("[+] Custom NtUserMoveWindow Called\n"));

	cmd_t* cmd = reinterpret_cast<cmd_t*>(arg01);

	switch (cmd->operation)
	{
		case memory_read:
		{
			DbgPrintEx(0, 0, _("[+] Operation: Memory Read"));
			cmd->success = true;
			break;
		}

		case memory_write:
		{
			DbgPrintEx(0, 0, _("[+] Operation: Memory Write"));
			cmd->success = true;
			break;
		}

		case module_base:
		{
			DbgPrintEx(0, 0, _("[+] Operation: Module Base"));
			cmd->success = true;
			break;
		}

		default:
		{
			DbgPrintEx(0, 0, _("[-] No operation found"));
			cmd->success = false;
			break;
		}
	}

	return STATUS_UNSUCCESSFUL;
}

extern "C" NTSTATUS DriverEntry()
{
	const uintptr_t base = utils::get_kernel_module(_("win32k.sys"));
	uintptr_t qword{};

	if (base)
	{
		qword = utils::pattern_scan(base, QWORD_SIGNATURE, QWORD_MASK);
	}
	else
	{
		DbgPrintEx(0, 0, _("[-] win32k.sys not found"));
		return STATUS_UNSUCCESSFUL;
	}

	DbgPrintEx(0, 0, _("[+] win32k.sys @ 0x%p\n"), base);
	DbgPrintEx(0, 0, _("[+] qword @ 0x%p\n"), qword);

	PEPROCESS process_target{};

	if (utils::find_process(_("explorer.exe"), &process_target) == STATUS_SUCCESS
		&& process_target)
	{
		const uintptr_t derefrenced_qword = (uintptr_t)qword + *(int*)((BYTE*)qword + 3) + 7;

		DbgPrintEx(0, 0, _("[+] *qword @ 0x%p"), derefrenced_qword);

		KeAttachProcess(process_target);
		*(void**)&original_NtUserMoveWindow = _InterlockedExchangePointer((void**)derefrenced_qword, (void*)NtUserMoveWindow);
		KeDetachProcess();
	}
	else
	{
		DbgPrintEx(0, 0, _("[-] Can't find explorer.exe"));
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}
