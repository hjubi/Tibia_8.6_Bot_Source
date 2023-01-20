#pragma once
typedef void(__cdecl* tLookId)(int32_t a);
typedef int(__cdecl* tPrintText)(int32_t nSurface, int32_t nX, int32_t nY, int32_t nFont, int32_t nRed, int32_t nGreen, int32_t nBlue, const char* lpText, int32_t nAlign);
typedef void(__cdecl* tSayHook)(int32_t a, const char* message);
typedef void(__cdecl* sayFunc)(int32_t a, const char* message);
namespace Hooks
{
	extern bool bComboBot;

	extern uintptr_t moduleBase;
	extern ComboBotSpells comboBotSpells;
	extern bool bLookIds;
	extern bool bComboBot;

	extern tLookId hLookId;
	extern tPrintText ohkPrintText;
	extern tSayHook hSayHookAssembly;
	extern sayFunc say;

	extern uintptr_t lookIdHook;
	extern uintptr_t lookIdOriginal;

	extern uintptr_t printText1;
	extern uintptr_t printTextOriginal;

	extern int32_t timeVarComboBot;


	void InitHooks();
	void UnHook();
};