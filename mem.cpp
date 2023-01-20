#include "pch.h"

uintptr_t mem::FindDMAAddy2(uintptr_t ptr, std::vector<uint32_t> offsets)
{
	uintptr_t addr = ptr;
	uintptr_t* pointer;
	for (uint32_t i = 0; i < offsets.size(); ++i)
	{
		pointer = (uintptr_t*)addr;
		if (!IsBadReadPtr(pointer, sizeof(pointer)))
		{
			addr = *(uintptr_t*)addr;
			addr += offsets[i];
		}
	}
	if (IsBadReadPtr((uintptr_t*)addr, sizeof(addr))) return 0;
	return addr;
}

void mem::DetourCall(uintptr_t dwCallAddress, uintptr_t dwNewFunction, uintptr_t* pOldFunction)
{
	DWORD dwProtect;
	VirtualProtect((LPVOID)(dwCallAddress), 5, PAGE_READWRITE, &dwProtect);
	if (pOldFunction != 0)
		*pOldFunction = dwCallAddress + *(uintptr_t*)(dwCallAddress + 1) + 5;
	*(uintptr_t*)(dwCallAddress + 1) = dwNewFunction - dwCallAddress - 5;
	VirtualProtect((LPVOID)(dwCallAddress), 5, dwProtect, &dwProtect);
}

void mem::Patch(BYTE* dst, BYTE* src, uint32_t size)
{
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);

	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

void mem::Nop(BYTE* dst, uint32_t size)
{
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memset(dst, 0x90, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}
