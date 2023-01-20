#include "pch.h"
#include "Hooks.h"

namespace Hooks
{
	int32_t timeVarComboBot = 0;
	uintptr_t moduleBase;
	ComboBotSpells comboBotSpells;
	bool bLookIds;
	bool bComboBot;
	tLookId hLookId;
	
	tPrintText ohkPrintText;
	tSayHook hSayHookAssembly;

	sayFunc say;

	uintptr_t sayHook;
	uintptr_t sayHookOriginal;


	uintptr_t lookIdHook;
	uintptr_t lookIdOriginal;

	uintptr_t printText1;
	uintptr_t printTextOriginal;
}




__declspec(naked) void inLookIdHookFunc()
{


	if (Hooks::bLookIds)
	{
		__asm {
			push ebp
			mov ebp, esp
			sub   esp, 0x7C
		}
		int32_t itemId = 0;
		char buf[10];
		_asm {
			mov itemId, eax
		}
		sprintf_s(buf, sizeof(buf), "%i", itemId);
		PacketSend::GetInstance().PrintWhiteText(buf);
		__asm {
			leave
			ret
		}
	}
	else
	{
		__asm
		{
			push ebp
			mov ebp, esp
			leave
			ret
		}
	}
}

__declspec(noinline) int32_t OUTmyPrintText(int32_t v1, int32_t v2, int32_t v3, int32_t v4, int32_t v5, int32_t v6, int32_t v7, const char* v8, int32_t v9)
{

	__asm {
		push v9 //[ebp + 0x20]
		push v8 //[ebp + 0x1C]
		push v7 //[ebp + 0x18]
		push v6 //[ebp + 0x14]
		push v5 //[ebp + 0x10]
		push v4 //[ebp + 0x0C]
		push v3 //[ebp + 0x08]
		push v2
		push v1
		call Hooks::ohkPrintText
	}
}





int32_t myPrintText(int32_t nSurface, int32_t nX, int32_t nY, int32_t nFont, int32_t nRed, int32_t nGreen, int32_t nBlue, const char* lpText, int32_t nAlign)
{
	//time_t now = time(0);

	//// convert now to string form
	//char* dt = ctime(&now);


	//// convert now to tm struct for UTC
	//tm* gmtm = gmtime(&now);
	//dt = asctime(gmtm);

	OUTmyPrintText(1, 5, 40, 2, 12, 56, 100, R"(__________)", 0);
	OUTmyPrintText(1, 5, 50, 2, 12, 56, 100, R"(    \_____        \)", 0);
	OUTmyPrintText(1, 5, 60, 2, 12, 56, 100, R"(      |             |     _/)", 0);
	OUTmyPrintText(1, 5, 70, 2, 12, 56, 100, R"(      |             |        \)", 0);
	OUTmyPrintText(1, 5, 80, 2, 12, 56, 100, R"(      | _____      /)", 0);
	OUTmyPrintText(1, 5, 90, 2, 12, 56, 100, R"(                        \ /)", 0);





	return OUTmyPrintText(nSurface, nX, nY, nFont, nRed, nGreen, nBlue, lpText, nAlign);
}

__declspec(naked) void INmyPrintText()
{
	__asm {
		push ebp
		mov ebp, esp
		sub esp, 0x10
		push[ebp + 0x28]
		push[ebp + 0x24]
		push[ebp + 0x20]
		push[ebp + 0x1C]
		push[ebp + 0x18]
		push[ebp + 0x14]
		push[ebp + 0x10]
		push[ebp + 0x0C]
		push[ebp + 0x08]
		call myPrintText
		leave
		ret
	}
}

__declspec(naked) void OUTsayHook(int32_t a, const char* message)
{
	__asm {
		push ebp
		mov ebp, esp
		push message
		push a
		call Hooks::hSayHookAssembly
		leave
		ret
	}
}

__declspec(naked) void sayHookFunction(int32_t a, const char* message)
{

	if (Hooks::bComboBot)
	{
		char* sender = nullptr;
		__asm
		{
			sub esp, 0x10
			mov sender, edi
		}


		if (sender)
		{
			if ((strcmp(sender, Hooks::comboBotSpells.leaderName) == 0) && (strcmp(message, Hooks::comboBotSpells.leaderSpell) == 0))
			{
				if (Util::isNotExhausted(Hooks::timeVarComboBot, Cooldowns::GetInstance().COMBO_BOT))
				{
					Hooks::say(1, Hooks::comboBotSpells.mySpell);
				}
			}
		}
		__asm {
			push message
			push a
			call OUTsayHook
			leave
			ret
		}
	}
	else
	{
		__asm
		{
			push ebp
			mov ebp, esp
			push message
			push a
			call OUTsayHook
			leave
			ret
		}
	}
}

__declspec(naked) void INmySayHook()
{
	__asm {
		push ebp
		mov ebp, esp
		push[ebp + 0x0C]
		push[ebp + 0x08]
		call sayHookFunction
		leave
		ret
	}
}


void Hooks::InitHooks()
{
	bComboBot = false;
	bLookIds = false;
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	sayHook = (moduleBase + 0x14AAD0);
	sayHookOriginal = (moduleBase + 0x144220);

	say = (sayFunc)(moduleBase + 0x73F0);

	lookIdHook = (uintptr_t)(moduleBase + 0x7264);
	lookIdOriginal = (uintptr_t)(moduleBase + 0xF8700);


	printText1 = (uintptr_t)(moduleBase + 0xF5823);
	printTextOriginal = (uintptr_t)(moduleBase + 0xB4DD0);

	mem::DetourCall(lookIdHook, (uintptr_t)&inLookIdHookFunc, (uintptr_t*)&hLookId);
	mem::DetourCall(printText1, (uintptr_t)&INmyPrintText, (uintptr_t*)&ohkPrintText);
	mem::DetourCall(sayHook, (uintptr_t)&INmySayHook, (uintptr_t*)&hSayHookAssembly);
}

void Hooks::UnHook()
{
	mem::DetourCall(printText1, printTextOriginal, 0);


	mem::DetourCall(lookIdHook, lookIdOriginal, 0);
	mem::DetourCall(sayHook, sayHookOriginal, 0);
}

