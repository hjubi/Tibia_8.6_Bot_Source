#pragma once

namespace mem
{
	uintptr_t FindDMAAddy2(uintptr_t ptr, std::vector<uint32_t> offsets);
	void DetourCall(uintptr_t dwCallAddress, uintptr_t dwNewFunction, uintptr_t* pOldFunction);
	void Patch(BYTE* dst, BYTE* src, uint32_t size);
	void Nop(BYTE* dst, uint32_t size);
}