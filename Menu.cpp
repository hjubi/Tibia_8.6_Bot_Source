#include "pch.h"
#include "Menu.h"
#include "Healer.h"
#include "Hooks.h"

void Menu::HandleMsgs()
{
	while (GetMessage(&messages, NULL, 0, 0)) {
		TranslateMessage(&messages);
		DispatchMessage(&messages);
		Sleep(0.1f);
	}
}

HMENU Menu::CreateDLLWindowMenu()
{
	HMENU hMenu;
	hMenu = CreateMenu();
	if (hMenu == NULL)
		return FALSE;
	HMENU hMenuPopup = CreatePopupMenu();
	AppendMenuW(hMenuPopup, MF_STRING, CONFIG_SAVE, TEXT("Save config"));
	AppendMenuW(hMenuPopup, MF_STRING, CONFIG_LOAD, TEXT("Load config"));
	AppendMenuW(hMenuPopup, MF_STRING, AUTO_CONFIG_LOAD, TEXT("Auto load config"));
	AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuPopup, TEXT("Config"));
	return hMenu;
}

void CALLBACK Menu::MainTimerLoop(HWND hwnd, UINT uMsg, int32_t timerId, DWORD dwTime)
{
	if (!MemReader::GetInstance().IsOnline()) return;

	if (bHealer)
	{
		Healer::GetInstance().HealWithSpells(lightSpell, midSpell, heavySpell);
		Healer::GetInstance().HealWithItems(healthPotionHealer, manaPotionHealer);
	}

	if (bHealFriendSio || bHealFriendUH) Healer::GetInstance().HealPlayer(safeHpForSio, healFriendOnHp, friendList, bHealFriendSio, selectedHealFriendMode);

	if (bAdvertising)
	{
		if ((Util::isNotExhausted(clockAdvertising, Cooldowns::GetInstance().SAY_ADVERTISING)))
		{
			PacketSend::GetInstance().SayOnAdvertisting(&advertisingMessage[0]);
		}
	}

	if (bAntiIdle)
	{
		if (Util::isNotExhausted(clockAntiIdle, Cooldowns::GetInstance().IDLE_TIME))
		{
			PacketSend::GetInstance().Turn(TURN_DIRECTION::TURN_NORTH);
			PacketSend::GetInstance().Turn(TURN_DIRECTION::TURN_EAST);
		}
	}

	if (bAutoAttack)
	{
		AutoAttack::GetInstance().Enable();
	}

	if (bHoldPosition)
	{
		MemReader::GetInstance().ReadSelfCharacter(&localPlayer);
		if (selfPlayer.xPos != localPlayer.xPos)
		{
			if (Util::isNotExhausted(clockHoldPosition, Cooldowns::GetInstance().WALK))
			{
				int32_t distance = localPlayer.xPos - selfPlayer.xPos;
				if (distance > 0)
				{
					PacketSend::GetInstance().Step(STEP_DIRECTION::WEST);
				}
				else if (distance < 0)
				{
					PacketSend::GetInstance().Step(STEP_DIRECTION::EAST);
				}
			}
		}
		else if (selfPlayer.yPos != localPlayer.yPos)
		{
			if (Util::isNotExhausted(clockHoldPosition, Cooldowns::GetInstance().WALK))
			{
				int32_t distance = localPlayer.yPos - selfPlayer.yPos;
				if (distance > 0)
				{
					PacketSend::GetInstance().Step(STEP_DIRECTION::NORTH);
				}
				else if (distance < 0)
				{
					PacketSend::GetInstance().Step(STEP_DIRECTION::SOUTH);
				}
			}
		}
	}

	if (bEatFood)
	{
		Item foodItem = MemReader::GetInstance().FindFoodInContainers();
		
		std::string a = "";

		if ((foodItem.id != 0) && (Util::isNotExhausted(clockEatFood, Cooldowns::GetInstance().EAT_FOOD)))
		{
			PacketSend::GetInstance().UseItemInContainer(foodItem.contNr,foodItem.slotNumber, foodItem.id);
		}
	}



	if (bDash)
	{
		if ((GetKeyState(VK_NUMPAD9) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::NORTH_EAST);
		else if ((GetKeyState(VK_NUMPAD3) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::SOUTH_EAST);
		else if ((GetKeyState(VK_NUMPAD1) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::SOUTH_WEST);
		else if ((GetKeyState(VK_NUMPAD7) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::NORTH_WEST);
		// side first ^
		else if ((GetKeyState(VK_NUMPAD8) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::NORTH);
		else if ((GetKeyState(VK_NUMPAD6) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::EAST);
		else if ((GetKeyState(VK_NUMPAD2) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::SOUTH);
		else if ((GetKeyState(VK_NUMPAD4) & 0x80) && (Util::isNotExhausted(clockHoldDash, Cooldowns::GetInstance().BOOST_DELAY)))
			PacketSend::GetInstance().Step(STEP_DIRECTION::WEST);
	}

	if (bAntiParalayse)	Healer::GetInstance().CureParalyze(cureParalayseSpell);

	if (bAutoHaste)		Healer::GetInstance().Haste(hasteSpell);

	if (bHoldTarget)	AutoAttack::GetInstance().HoldTarget();

	if (bEquipModeHotkey || bEquipModeMoveItem)
	{
		Healer::GetInstance().EquipAmuletBalancer(bEquipModeHotkey, bEquipModeMoveItem, holdAmulet, balanceAmulet, dequipAmulet);
	}

	if (bManaShield)
	{
		if (!MemReader::GetInstance().IsFlagTrue(CHARACTER_FLAGS::MANA_SHIELD) && Util::isNotExhausted(clockManaShield, Cooldowns::GetInstance().SUPPORT_SPELL))
		{
			PacketSend::GetInstance().Say("utamo vita");
		}
	}

	if (bKillTarget)
	{
		int32_t targetId = MemReader::GetInstance().GetAttackedCreature();
		CSelfCharacter selfCharacter;
		MemReader::GetInstance().ReadSelfCharacter(&selfCharacter);
		if ((targetId > 0) && (killTarget.hpPercentage > 0))
		{
			Entity* entity = MemReader::GetInstance().GetEntityInEntityList(targetId);
			bool isNotExhausted = Util::isNotExhausted(clockKillTarget, Cooldowns::GetInstance().ATTACK_SPELL);
			if ((entity->hpPercentage <= killTarget.hpPercentage) && (selfCharacter.mana >= killTarget.manaCost) && (isNotExhausted))
			{
				PacketSend::GetInstance().Say(killTarget.spell);
			}
		}
	}

}

void ClickCheckBox(HWND windowHandle)
{
	SendMessage(windowHandle, BM_CLICK, BST_PUSHED, 0);
	SendMessage(windowHandle, BM_CLICK, BST_PUSHED, 0);
}

void Menu::LoadConfig(LPWSTR filePath)
{
	std::ifstream file(filePath);
	std::string content;

	if (file.is_open())
	{
		//Light spell input
		std::getline(file, content);
		lightSpell.spell[0] = '\0';
		strcat_s(lightSpell.spell, content.c_str());

		//Light spell Hp percentage
		std::getline(file, content);
		lightSpell.hpPercentage = NULL;
		lightSpell.hpPercentage = atoi(content.c_str());

		//Light spell Mana cost
		std::getline(file, content);
		lightSpell.manaCost = NULL;
		lightSpell.manaCost = atoi(content.c_str());

		//Mid spell input
		std::getline(file, content);
		midSpell.spell[0] = '\0';
		strcat_s(midSpell.spell, content.c_str());

		//Mid spell hp percentage
		std::getline(file, content);
		midSpell.hpPercentage = NULL;
		midSpell.hpPercentage = atoi(content.c_str());

		//Mid spell mana cost
		std::getline(file, content);
		midSpell.manaCost = NULL;
		midSpell.manaCost = atoi(content.c_str());

		//Heavy spell input
		std::getline(file, content);
		heavySpell.spell[0] = '\0';
		strcat_s(heavySpell.spell, content.c_str());

		//Heavy spell hp percentage
		std::getline(file, content);
		heavySpell.hpPercentage = NULL;
		heavySpell.hpPercentage = atoi(content.c_str());

		//Heavy spell mana cost
		std::getline(file, content);
		heavySpell.manaCost = NULL;
		heavySpell.manaCost = atoi(content.c_str());


		//Health Potion Healer
		std::getline(file, content);
		healthPotionHealer.itemId = atoi(content.c_str());
		std::getline(file, content);
		healthPotionHealer.hpPerc = atoi(content.c_str());


		//Mana Potion Healer
		std::getline(file, content);
		manaPotionHealer.itemId = atoi(content.c_str());
		std::getline(file, content);
		manaPotionHealer.manaPerc = atoi(content.c_str());


		ClickCheckBox(cBoxEnableHealer);
	}
	else
	{
		MessageBox(NULL, L"File you tried to open doesn't exist, is wrong format or opened by another process.", L"Error Opening file.", MB_OK);
	}
	file.close();
}

void Menu::OpenFileExplorer(HWND hwnd)
{
	OPENFILENAME ofn;

	char fileName[100];

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = LPWSTR(fileName);
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fileName);
	ofn.lpstrFilter = L"config files\0*.bot\0";
	ofn.nFilterIndex = 1;

	if (GetOpenFileName(&ofn))
	{
		LoadConfig(ofn.lpstrFile);
	}
}

void Menu::SaveConfig()
{
	if (!MemReader::GetInstance().IsOnline())
	{
		MessageBox(NULL, L"You need to be online to save config with player name.", L"Error saving config.", MB_OK);
		return;
	}
	CSelfCharacter selfCharacter;
	MemReader::GetInstance().ReadSelfCharacter(&selfCharacter);
	char FilePathToBot[100];
	GetModuleFileNameA(inj_hModule, FilePathToBot, sizeof(FilePathToBot));
	if (strrchr(FilePathToBot, '\\') != NULL)
		*strrchr(FilePathToBot, '\\') = 0;
	strcat_s(FilePathToBot, 100, "\\");
	strcat_s(FilePathToBot, 100, selfCharacter.name);
	strcat_s(FilePathToBot, 100, ".bot");

	std::ofstream file(FilePathToBot);

	if (file.is_open())
	{
		// Light spell
		file << &lightSpell.spell[0] << std::endl;
		file << lightSpell.hpPercentage << std::endl;
		file << lightSpell.manaCost << std::endl;

		//Mid spell
		file << &midSpell.spell[0] << std::endl;
		file << midSpell.hpPercentage << std::endl;
		file << midSpell.manaCost << std::endl;

		//Heave spell
		file << &heavySpell.spell[0] << std::endl;
		file << heavySpell.hpPercentage << std::endl;
		file << heavySpell.manaCost << std::endl;

		//Health Potion Healer
		file << healthPotionHealer.itemId << std::endl;
		file << healthPotionHealer.hpPerc << std::endl;

		//Mana Potion Healer
		file << manaPotionHealer.itemId << std::endl;
		file << manaPotionHealer.manaPerc << std::endl;
	}
	else
	{
		MessageBox(NULL, L"Saving config failed. Try again or report issue to developers.", L"Error saving config.", MB_OK);
	}
	file.close();
}

void Menu::AutoLoadConfig()
{
	if (!MemReader::GetInstance().IsOnline())
	{
		MessageBox(NULL, L"You need to be online to load config with player name.", L"Error auto loading config.", MB_OK);
		return;
	}
	CSelfCharacter selfCharacter;
	MemReader::GetInstance().ReadSelfCharacter(&selfCharacter);
	char FilePathToBot[100];
	GetModuleFileNameA(inj_hModule, FilePathToBot, sizeof(FilePathToBot));
	if (strrchr(FilePathToBot, '\\') != NULL)
		*strrchr(FilePathToBot, '\\') = 0;
	strcat_s(FilePathToBot, 100, "\\");
	strcat_s(FilePathToBot, 100, selfCharacter.name);
	strcat_s(FilePathToBot, 100, ".bot");

	wchar_t wtext[100];
	size_t numberCharsConverted;
	mbstowcs_s(&numberCharsConverted, wtext, FilePathToBot, strlen(FilePathToBot) + 1);
	LPWSTR ptr = wtext;

	LoadConfig(ptr);
}

void Menu::ReadSioListTxt(std::array<std::string, 100>& friendList)
{
	char FilePathToBot[100];
	GetModuleFileNameA(inj_hModule, FilePathToBot, sizeof(FilePathToBot));
	if (strrchr(FilePathToBot, '\\') != NULL)
		*strrchr(FilePathToBot, '\\') = 0;
	strcat_s(FilePathToBot, 100, "\\sioList.txt");

	int32_t arrayIndex = 0;
	std::ifstream file(FilePathToBot);
	std::string content;
	if (file.is_open())
	{
		for (int32_t i = 0; i < friendList.size(); i++)
		{
			std::getline(file, content);
			if (!content.empty())
			{
				friendList[arrayIndex] = content;
				arrayIndex++;
			}
		}
	}
	else
	{
		MessageBox(NULL, L"Failed to open sioList.txt. Check if sioList.txt exists in bot directory.", L"Error opening sioList.txt.", MB_OK);
	}
	file.close();
}

void Menu::OpenSioListTxt()
{
	char FilePathToBot[100];
	GetModuleFileNameA(inj_hModule, FilePathToBot, sizeof(FilePathToBot));
	if (strrchr(FilePathToBot, '\\') != NULL)
		*strrchr(FilePathToBot, '\\') = 0;
	strcat_s(FilePathToBot, sizeof(FilePathToBot), "\\sioList.txt");

	std::ifstream file(FilePathToBot);
	if (!file.is_open())
	{
		MessageBox(NULL, L"Failed to open sioList.txt. Check if sioList.txt exists in bot directory.", L"Error opening sioList.txt.", MB_OK);
	}
	else
	{
		file.close();
		ShellExecuteA(NULL, "open", FilePathToBot, "", FilePathToBot, SW_SHOW);
	}
}

LRESULT CALLBACK Menu::HealerMessageHandler(HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_CREATE:
		bHealerWindowOpen = true;
		CreateHealerMenu(hWindow);
		CheckDlgButton(hWindow, CLB_HEALER, bHealer);
		ToggleHealer();
		CheckDlgButton(hWindow, CLB_HEAL_FRIENDS_SIO, bHealFriendSio);
		ToggleSio();
		CheckDlgButton(hWindow, CLB_HEAL_FRIENDS_UH, bHealFriendUH);
		break;
	case WM_CLOSE:
		bHealerWindowOpen = false;
		break;
	case WM_DESTROY:
		bHealerWindowOpen = false;
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CLB_HEALER:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_HEALER, BM_GETCHECK, 0, 0))
				{
					bHealer = !bHealer;
					CheckDlgButton(hWindow, CLB_HEALER, bHealer);
					ToggleHealer();
				}
				else
				{
					bHealer = !bHealer;
					CheckDlgButton(hWindow, CLB_HEALER, bHealer);
					ToggleHealer();
				}
				break;
			}
			break;
		case CLB_OPEN_SIO_LIST:
			OpenSioListTxt();
			break;
		case CLB_HEAL_FRIENDS_SIO:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_HEAL_FRIENDS_SIO, BM_GETCHECK, 0, 0))
				{
					bHealFriendSio = !bHealFriendSio;
					CheckDlgButton(hWindow, CLB_HEAL_FRIENDS_SIO, bHealFriendSio);
					ToggleSio();
				}
				else
				{
					bHealFriendSio = !bHealFriendSio;
					CheckDlgButton(hWindow, CLB_HEAL_FRIENDS_SIO, bHealFriendSio);
					ToggleSio();
				}
				break;
			}
			break;
		case CLB_HEAL_FRIENDS_UH:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_HEAL_FRIENDS_UH, BM_GETCHECK, 0, 0))
				{
					bHealFriendUH = !bHealFriendUH;
					CheckDlgButton(hWindow, CLB_HEAL_FRIENDS_UH, bHealFriendUH);
					ToggleSio();
				}
				else
				{
					bHealFriendUH = !bHealFriendUH;
					CheckDlgButton(hWindow, CLB_HEAL_FRIENDS_UH, bHealFriendUH);
					ToggleSio();
				}
				break;
			}
			break;
		case CLB_LIST_BOX_HEAL_MODES:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				int32_t itemSelected = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				if (selectedHealFriendMode == itemSelected)	break;
				selectedHealFriendMode = itemSelected;
				break;
			}
			break;
		}
		break;
	}
	return DefWindowProc(hWindow, uMessage, wParam, lParam);
}

LRESULT CALLBACK Menu::TimersMessageHandler(HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_CREATE:
		bTimersWindowOpen = true;
		CreateTimersMenu(hWindow);
		break;
	case WM_CLOSE:
		bTimersWindowOpen = false;
		break;
	case WM_DESTROY:
		bTimersWindowOpen = false;
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CLB_TIMERS:
			ToggleTimers();
			break;
		}
		break;
	}
	return DefWindowProc(hWindow, uMessage, wParam, lParam);
}

LRESULT CALLBACK Menu::UtilsMessageHandler(HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_CREATE:
		bUtilsWindowOpen = true;
		CreateUtilsMenu(hWindow);
		CheckDlgButton(hWindow, CLB_ADVERTISING, bAdvertising);
		ToggleAdvertising();
		CheckDlgButton(hWindow, CLB_XRAY, bXray);
		
		//TODO
		//MemReader::GetInstance().EnableXray(bXray);
		CheckDlgButton(hWindow, CLB_ANTI_IDLE, bAntiIdle);
		CheckDlgButton(hWindow, CLB_AUTO_ATTACK, bAutoAttack);
		CheckDlgButton(hWindow, CLB_MOUNT, bMount);
		CheckDlgButton(hWindow, CLB_FOOD_EAT, bEatFood);
		CheckDlgButton(hWindow, CLB_LOOK_IDS, Hooks::bLookIds);
		CheckDlgButton(hWindow, CLB_HOLD_POSITION, bHoldPosition);
		CheckDlgButton(hWindow, CLB_DISABLE_MOUNTS, bDisableMounts);
		CheckDlgButton(hWindow, CLB_COMBO_BOT, Hooks::bComboBot);
		ToggleComboBot();
		break;
	case WM_CLOSE:
		bUtilsWindowOpen = false;
		break;
	case WM_DESTROY:
		//KillTimer(hWindow, 0);
		bUtilsWindowOpen = false;
		//PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CLB_ADVERTISING:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_ADVERTISING, BM_GETCHECK, 0, 0))
				{
					bAdvertising = !bAdvertising;
					CheckDlgButton(hWindow, CLB_ADVERTISING, bAdvertising);
					ToggleAdvertising();
				}
				else
				{
					bAdvertising = !bAdvertising;
					CheckDlgButton(hWindow, CLB_ADVERTISING, bAdvertising);
					ToggleAdvertising();
				}
				break;
			}
			break;
		case CLB_XRAY:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_XRAY, BM_GETCHECK, 0, 0))
				{
					bXray = !bXray;
					CheckDlgButton(hWindow, CLB_XRAY, bXray);
					MemReader::GetInstance().EnableXray(bXray);
				}
				else
				{
					bXray = !bXray;
					CheckDlgButton(hWindow, CLB_XRAY, bXray);
					MemReader::GetInstance().EnableXray(bXray);
				}
				break;
			}
			break;
		case CLB_ANTI_IDLE:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_ANTI_IDLE, BM_GETCHECK, 0, 0))
				{
					bAntiIdle = !bAntiIdle;
					CheckDlgButton(hWindow, CLB_ANTI_IDLE, bAntiIdle);
				}
				else
				{
					bAntiIdle = !bAntiIdle;
					CheckDlgButton(hWindow, CLB_ANTI_IDLE, bAntiIdle);
				}
				break;
			}
			break;
		case CLB_AUTO_ATTACK:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_ANTI_IDLE, BM_GETCHECK, 0, 0))
				{
					bAutoAttack = !bAutoAttack;
					CheckDlgButton(hWindow, CLB_AUTO_ATTACK, bAutoAttack);
				}
				else
				{
					bAutoAttack = !bAutoAttack;
					CheckDlgButton(hWindow, CLB_AUTO_ATTACK, bAutoAttack);
				}
				break;
			}
			break;
		case CLB_MOUNT:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_MOUNT, BM_GETCHECK, 0, 0))
				{
					bMount = !bMount;
					CheckDlgButton(hWindow, CLB_MOUNT, bMount);
				}
				else
				{
					bMount = !bMount;
					CheckDlgButton(hWindow, CLB_MOUNT, bMount);
				}
				break;
			}
			break;
		case CLB_FOOD_EAT:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_FOOD_EAT, BM_GETCHECK, 0, 0))
				{
					bEatFood = !bEatFood;
					CheckDlgButton(hWindow, CLB_FOOD_EAT, bEatFood);
				}
				else
				{
					bEatFood = !bEatFood;
					CheckDlgButton(hWindow, CLB_FOOD_EAT, bEatFood);
				}
				break;
			}
			break;
		case CLB_LOOK_IDS:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_LOOK_IDS, BM_GETCHECK, 0, 0))
				{
					Hooks::bLookIds = !Hooks::bLookIds;
					CheckDlgButton(hWindow, CLB_LOOK_IDS, Hooks::bLookIds);
				}
				else
				{
					Hooks::bLookIds = !Hooks::bLookIds;
					CheckDlgButton(hWindow, CLB_LOOK_IDS, Hooks::bLookIds);
				}
				break;
			}
			break;
		case CLB_HOLD_POSITION:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_HOLD_POSITION, BM_GETCHECK, 0, 0))
				{
					MemReader::GetInstance().ReadSelfCharacter(&localPlayer);
					bHoldPosition = !bHoldPosition;
					CheckDlgButton(hWindow, CLB_HOLD_POSITION, bHoldPosition);
					//TODO
					selfPlayer.xPos = localPlayer.xPos;
					selfPlayer.yPos = localPlayer.yPos;
				}
				else
				{
					MemReader::GetInstance().ReadSelfCharacter(&localPlayer);
					bHoldPosition = !bHoldPosition;
					CheckDlgButton(hWindow, CLB_HOLD_POSITION, bHoldPosition);
					//TODO
					selfPlayer.xPos = localPlayer.xPos;
					selfPlayer.yPos = localPlayer.yPos;
				}
				break;
			}
			break;
		case CLB_DISABLE_MOUNTS:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_DISABLE_MOUNTS, BM_GETCHECK, 0, 0))
				{
					bDisableMounts = !bDisableMounts;
					CheckDlgButton(hWindow, CLB_DISABLE_MOUNTS, bDisableMounts);
				}
				else
				{
					bDisableMounts = !bDisableMounts;
					CheckDlgButton(hWindow, CLB_DISABLE_MOUNTS, bDisableMounts);
				}
				break;
			}
			break;
		case CLB_COMBO_BOT:

			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_COMBO_BOT, BM_GETCHECK, 0, 0))
				{
					Hooks::bComboBot = !Hooks::bComboBot;
					CheckDlgButton(hWindow, CLB_COMBO_BOT, Hooks::bComboBot);
					ToggleComboBot();
				}
				else
				{
					Hooks::bComboBot = !Hooks::bComboBot;
					CheckDlgButton(hWindow, CLB_COMBO_BOT, Hooks::bComboBot);
					ToggleComboBot();
				}
				break;
			}
			break;
		}
		break;
	}
	return DefWindowProc(hWindow, uMessage, wParam, lParam);
}

LRESULT CALLBACK Menu::MessageHandler(HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_CREATE:
		Hooks::InitHooks();
		SetTimer(hWindow, 0, 0, (TIMERPROC)&MainTimerLoop);
		break;
	case WM_CLOSE: break;
	case WM_DESTROY:
		KillTimer(hWindow, 0);
		if (f) fclose(f);
		FreeConsole();
		Hooks::UnHook();
		PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case MYMENU_EXIT:
			KillTimer(hWindow, 0);
			if (f) fclose(f);
			FreeConsole();
			Hooks::UnHook();
			PostQuitMessage(0);
			return 0;
			break;

		case CONFIG_LOAD:
			OpenFileExplorer(hWindow);
			break;

		case AUTO_CONFIG_LOAD:
			AutoLoadConfig();
			break;

		case CONFIG_SAVE:
			SaveConfig();
			break;

		case CLB_OPEN_HEALER_WINDOW:
		{
			if (!bHealerWindowOpen)
			{
				RECT rc;
				RegisterDLLWindowHealerClass();
				healerHWND = CreateWindowExA(0, "HealerWindowClass", "Healer", WS_EX_LAYERED | WS_MINIMIZEBOX, 100, 100, 326, 321, NULL, NULL, inj_hModule, NULL);
				GetWindowRect(healerHWND, &rc);
				int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
				int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
				SetWindowPos(healerHWND, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				ShowWindow(healerHWND, SW_SHOWNORMAL);
				SetForegroundWindow(healerHWND);
			}
			break;
		}

		case CLB_OPEN_TIMERS_WINDOW:
		{
			if (!bTimersWindowOpen)
			{
				RECT rc;
				RegisterDLLWindowTimersClass();
				timersHWND = CreateWindowExA(0, "TimersWindowClass", "Timers", WS_EX_LAYERED | WS_MINIMIZEBOX, 100, 100, 296, 208, NULL, NULL, inj_hModule, NULL);
				GetWindowRect(timersHWND, &rc);
				int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
				int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
				SetWindowPos(timersHWND, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				ShowWindow(timersHWND, SW_SHOWNORMAL);
				SetForegroundWindow(timersHWND);
			}
			break;
		}

		case CLB_OPEN_UTILS_WINDOW:
		{
			if (!bUtilsWindowOpen)
			{
				RECT rc;
				RegisterDLLWindowUtilsClass();
				utilsHWND = CreateWindowExA(0, "UtilsWindowClass", "Utils", WS_EX_LAYERED | WS_MINIMIZEBOX, 100, 100, 256, 333, NULL, NULL, inj_hModule, NULL);
				GetWindowRect(utilsHWND, &rc);
				int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
				int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
				SetWindowPos(utilsHWND, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				ShowWindow(utilsHWND, SW_SHOWNORMAL);
				SetForegroundWindow(utilsHWND);
			}
			break;
		}

		case CLB_OPEN_PVP_WINDOW:
		{
			if (!bPvpWindowOpen)
			{
				RECT rc;
				RegisterDLLWindowPvpClass();
				pvpHWND = CreateWindowExA(0, "PvpWindowClass", "Pvp", WS_EX_LAYERED | WS_MINIMIZEBOX, 100, 100, 425, 325, NULL, NULL, inj_hModule, NULL);
				GetWindowRect(pvpHWND, &rc);
				int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
				int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
				SetWindowPos(pvpHWND, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				ShowWindow(pvpHWND, SW_SHOWNORMAL);
				SetForegroundWindow(pvpHWND);
			}
			break;
		}

		}
		break;
	}
	return DefWindowProc(hWindow, uMessage, wParam, lParam);
}

LRESULT CALLBACK Menu::PvpMessageHandler(HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_CREATE:
		bPvpWindowOpen = true;
		CreatePvpMenu(hWindow);
		CheckDlgButton(hWindow, CLB_HOLD_TARGET, bHoldTarget);
		CheckDlgButton(hWindow, CLB_DASH, bDash);
		CheckDlgButton(hWindow, CLB_MAGIC_SHIELD, bManaShield);
		CheckDlgButton(hWindow, CLB_KILL_ENEMY_ON_LOW_HP, bKillTarget);
		ToggleKillTarget();
		CheckDlgButton(hWindow, CLB_HASTE, bAutoHaste);
		ToggleHaste();
		CheckDlgButton(hWindow, CLB_ANTI_PARALAYSE, bAntiParalayse);
		ToggleAntiParalayse();
		CheckDlgButton(hWindow, CLB_EQUIP_MODE_AS_HOTKEY, bEquipModeHotkey);
		ToggleItemBalancer();
		CheckDlgButton(hWindow, CLB_EQUIP_MODE_AS_MOVE_ITEM, bEquipModeMoveItem);
		ToggleItemBalancer();
		break;
	case WM_CLOSE:
		bPvpWindowOpen = false;
		break;
	case WM_DESTROY:
		//KillTimer(hWindow, 0);
		bPvpWindowOpen = false;
		bLoadedAmulets = false;
		selectedAmuletOption = -1;
		bLoadedRings = false;
		selectedRingsOption = -1;
		//PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CLB_HOLD_TARGET:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_HOLD_TARGET, BM_GETCHECK, 0, 0))
				{
					bHoldTarget = !bHoldTarget;
					CheckDlgButton(hWindow, CLB_HOLD_TARGET, bHoldTarget);
				}
				else
				{
					bHoldTarget = !bHoldTarget;
					CheckDlgButton(hWindow, CLB_HOLD_TARGET, bHoldTarget);
				}
				break;

			}
			break;
		case CLB_DASH:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_DASH, BM_GETCHECK, 0, 0))
				{
					bDash = !bDash;
					CheckDlgButton(hWindow, CLB_DASH, bDash);
					//UpdateInfoConsole("Use numpad keys to dash.");
				}
				else
				{
					bDash = !bDash;
					CheckDlgButton(hWindow, CLB_DASH, bDash);
				}
				break;
			}
			break;
		case CLB_MAGIC_SHIELD:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_MAGIC_SHIELD, BM_GETCHECK, 0, 0))
				{
					bManaShield = !bManaShield;
					CheckDlgButton(hWindow, CLB_MAGIC_SHIELD, bManaShield);
				}
				else
				{
					bManaShield = !bManaShield;
					CheckDlgButton(hWindow, CLB_MAGIC_SHIELD, bManaShield);
				}
				break;
			}
			break;
		case CLB_KILL_ENEMY_ON_LOW_HP:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_KILL_ENEMY_ON_LOW_HP, BM_GETCHECK, 0, 0))
				{
					bKillTarget = !bKillTarget;
					CheckDlgButton(hWindow, CLB_KILL_ENEMY_ON_LOW_HP, bKillTarget);
					ToggleKillTarget();
					//UpdateInfoConsole("Use spell when target is on x hp %");
				}
				else
				{
					bKillTarget = !bKillTarget;
					CheckDlgButton(hWindow, CLB_KILL_ENEMY_ON_LOW_HP, bKillTarget);
					ToggleKillTarget();
					//UpdateInfoConsole("Use spell when target is on x hp %");
				}
				break;
			}
			break;
		case CLB_HASTE:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_HASTE, BM_GETCHECK, 0, 0))
				{
					bAutoHaste = !bAutoHaste;
					CheckDlgButton(hWindow, CLB_HASTE, bAutoHaste);
					ToggleHaste();
					//UpdateInfoConsole("Use spell when target is on x hp %");
				}
				else
				{
					bAutoHaste = !bAutoHaste;
					CheckDlgButton(hWindow, CLB_HASTE, bAutoHaste);
					ToggleHaste();
					//UpdateInfoConsole("Use spell when target is on x hp %");
				}
				break;
			}
			break;
		case CLB_ANTI_PARALAYSE:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_HASTE, BM_GETCHECK, 0, 0))
				{
					bAntiParalayse = !bAntiParalayse;
					CheckDlgButton(hWindow, CLB_ANTI_PARALAYSE, bAntiParalayse);
					ToggleAntiParalayse();
					//UpdateInfoConsole("Use spell when target is on x hp %");
				}
				else
				{
					bAntiParalayse = !bAntiParalayse;
					CheckDlgButton(hWindow, CLB_ANTI_PARALAYSE, bAntiParalayse);
					ToggleAntiParalayse();
					//UpdateInfoConsole("Use spell when target is on x hp %");
				}
				break;
			}
			break;
		case CLB_EQUIP_MODE_AS_HOTKEY:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_EQUIP_MODE_AS_HOTKEY, BM_GETCHECK, 0, 0))
				{
					bEquipModeHotkey = !bEquipModeHotkey;
					CheckDlgButton(hWindow, CLB_EQUIP_MODE_AS_HOTKEY, bEquipModeHotkey);
					ToggleItemBalancer();
					//UpdateInfoConsole("Bot will use equip packet to equip/dequip items as you would press a hotkey.");

				}
				else
				{
					bEquipModeHotkey = !bEquipModeHotkey;
					CheckDlgButton(hWindow, CLB_EQUIP_MODE_AS_HOTKEY, bEquipModeHotkey);
					ToggleItemBalancer();
					//UpdateInfoConsole("Bot will use equip packet to equip/dequip items as you would press a hotkey.");

				}
				break;
			}
			break;
		case CLB_EQUIP_MODE_AS_MOVE_ITEM:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (SendDlgItemMessage(hWindow, CLB_EQUIP_MODE_AS_MOVE_ITEM, BM_GETCHECK, 0, 0))
				{
					bEquipModeMoveItem = !bEquipModeMoveItem;
					CheckDlgButton(hWindow, CLB_EQUIP_MODE_AS_MOVE_ITEM, bEquipModeMoveItem);
					ToggleItemBalancer();
					//UpdateInfoConsole("Bot will use move packet to equip/dequip items as you would drag from amulet/ring slot to main backpack.");

				}
				else
				{
					bEquipModeMoveItem = !bEquipModeMoveItem;
					CheckDlgButton(hWindow, CLB_EQUIP_MODE_AS_MOVE_ITEM, bEquipModeMoveItem);
					ToggleItemBalancer();
					//UpdateInfoConsole("Bot will use move packet to equip/dequip items as you would drag from amulet/ring slot to main backpack.");

				}
				break;
			}
			break;
		case CLB_LIST_BOX_AMULETS:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
			{
				int32_t itemSelected = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				if (selectedAmuletOption == itemSelected)	break;
				selectedAmuletOption = itemSelected;

				if (!bLoadedAmulets) {
					lBoxAmulets = CreateGeneralComboBox("Amulet Options", 10, 245, 160, 70, CLB_LIST_BOX_AMULETS, hWindow);

					for (int32_t i = 0; i < sizeof(amuletOptions) / sizeof(amuletOptions[0]); i++)
					{
						SendMessageA(lBoxAmulets, CB_ADDSTRING, 0, (LPARAM)amuletOptions[i].name);
					}


					CreateGeneralLabel("Equip%:", 180, 245, 52, 20, hWindow);


					inputHoldAmulet.inputHpPerc = CreateGeneralInputBox("90", 240, 245, 30, 20, hWindow);

					CreateGeneralLabel("Item Id:", 280, 245, 45, 20, hWindow);

					inputHoldAmulet.inputItemId = CreateGeneralInputBox("3081", 330, 245, 50, 20, hWindow);

					bLoadedAmulets = true;
				}

				switch (selectedAmuletOption)
				{
				case 0:
					if (inputDequipAmulet.inputItemId)
					{
						DestroyWindow(inputDequipAmulet.inputItemId);
						inputDequipAmulet.inputItemId = NULL;
						if (itemsToDeleteAmulets[0])
						{
							DestroyWindow(itemsToDeleteAmulets[0]);
							DestroyWindow(itemsToDeleteAmulets[1]);
						}
					}
					else if (inputBalanceAmulet.inputItemId)
					{
						DestroyWindow(inputBalanceAmulet.inputItemId);
						inputBalanceAmulet.inputItemId = NULL;
						if (itemsToDeleteAmulets[0])
						{
							DestroyWindow(itemsToDeleteAmulets[0]);
							DestroyWindow(itemsToDeleteAmulets[1]);
						}
					}
					break;
				case 1:
					if (inputBalanceAmulet.inputItemId)
					{
						DestroyWindow(inputBalanceAmulet.inputItemId);
						inputBalanceAmulet.inputItemId = NULL;
						if (itemsToDeleteAmulets[0])
						{
							DestroyWindow(itemsToDeleteAmulets[0]);
							DestroyWindow(itemsToDeleteAmulets[1]);
						}
					}
					itemsToDeleteAmulets[0] = CreateGeneralLabel("Dequip:", 180, 270, 52, 20, hWindow);

					itemsToDeleteAmulets[1] = CreateGeneralLabel("Item Id:", 280, 270, 45, 20, hWindow);

					inputDequipAmulet.inputItemId = CreateGeneralInputBox("3081", 330, 270, 50, 20, hWindow);
					break;
				case 2:
					if (inputDequipAmulet.inputItemId)
					{
						DestroyWindow(inputDequipAmulet.inputItemId);
						inputDequipAmulet.inputItemId = NULL;
						if (itemsToDeleteAmulets[0])
						{
							DestroyWindow(itemsToDeleteAmulets[0]);
							DestroyWindow(itemsToDeleteAmulets[1]);
						}
					}
					itemsToDeleteAmulets[0] = CreateGeneralLabel("Balance:", 180, 270, 52, 20, hWindow);

					itemsToDeleteAmulets[1] = CreateGeneralLabel("Item Id:", 280, 270, 45, 20, hWindow);

					inputBalanceAmulet.inputItemId = CreateGeneralInputBox("7532", 330, 270, 50, 20, hWindow);
					break;
				}
				break;
			}
			}
			break;

			
		}

		break;
	}
	return DefWindowProc(hWindow, uMessage, wParam, lParam);
}

BOOL Menu::RegisterDLLWindowClass(const wchar_t szClassName[])
{
	HBRUSH hb = ::CreateSolidBrush(RGB(30, 144, 255));
	WNDCLASSEX wc;
	wc.hInstance = inj_hModule;
	wc.lpszClassName = (LPCWSTR)szClassName;
	wc.lpfnWndProc = MessageHandler;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.cbSize = sizeof(wc);
	wc.hIcon = LoadIcon(NULL, IDI_ERROR);
	wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = hb;
	if (!RegisterClassEx(&wc))
		return 0;
	return 1;
}

void Menu::RegisterDLLWindowHealerClass()
{

	WNDCLASSEX wcex;


	wcex.hInstance = inj_hModule;
	wcex.lpszClassName = L"HealerWindowClass";
	wcex.lpfnWndProc = HealerMessageHandler;
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.cbSize = sizeof(wcex);
	wcex.hIcon = LoadIcon(NULL, IDI_ERROR);
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(101));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);

	RegisterClassEx(&wcex);
}

void Menu::RegisterDLLWindowTimersClass()
{

	WNDCLASSEX wcex;


	wcex.hInstance = inj_hModule;
	wcex.lpszClassName = L"TimersWindowClass";
	wcex.lpfnWndProc = TimersMessageHandler;
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.cbSize = sizeof(wcex);
	wcex.hIcon = LoadIcon(NULL, IDI_ERROR);
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(101));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);

	RegisterClassEx(&wcex);
}

void Menu::RegisterDLLWindowUtilsClass()
{

	WNDCLASSEX wcex;


	wcex.hInstance = inj_hModule;
	wcex.lpszClassName = L"UtilsWindowClass";
	wcex.lpfnWndProc = UtilsMessageHandler;
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.cbSize = sizeof(wcex);
	wcex.hIcon = LoadIcon(NULL, IDI_ERROR);
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(101));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);

	RegisterClassEx(&wcex);
}

void Menu::RegisterDLLWindowPvpClass()
{

	WNDCLASSEX wcex;


	wcex.hInstance = inj_hModule;
	wcex.lpszClassName = L"PvpWindowClass";
	wcex.lpfnWndProc = PvpMessageHandler;
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.cbSize = sizeof(wcex);
	wcex.hIcon = LoadIcon(NULL, IDI_ERROR);
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(101));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);

	RegisterClassEx(&wcex);
}

HWND Menu::CreateGeneralCheckBox(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, int32_t hMenu, HWND hWnd)
{
	HWND hwnd = CreateWindowExA(0, "button", lpWindowName, WS_CHILD | WS_VISIBLE | BS_CHECKBOX, x, y, iWidth, iHeight, hWnd, (HMENU)hMenu, NULL, 0);
	return hwnd;
}
HWND Menu::CreateGeneralButton(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, int32_t hMenu, HWND hWnd)
{
	HWND hwnd = CreateWindowExA(0, "button", lpWindowName, WS_CHILD | WS_VISIBLE, x, y, iWidth, iHeight, hWnd, (HMENU)hMenu, NULL, 0);
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, true);
	return hwnd;
}
HWND Menu::CreateGeneralLabel(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, HWND hWnd)
{
	HWND hwnd = CreateWindowExA(0, "static", lpWindowName, WS_CHILD | WS_VISIBLE, x, y, iWidth, iHeight, hWnd, 0, NULL, 0);
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, true);
	return hwnd;
}
HWND Menu::CreateGeneralGroupBox(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, HWND hWnd)
{
	HWND hwnd = CreateWindowExA(0, "button", lpWindowName, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, x, y, iWidth, iHeight, hWnd, 0, NULL, 0);
	return hwnd;
}
HWND Menu::CreateGeneralListBox(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, int32_t hMenu, HWND hWnd)
{
	HWND hwnd = CreateWindowExA(WS_EX_STATICEDGE, "listbox", lpWindowName, WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_DISABLENOSCROLL, x, y, iWidth, iHeight, hWnd, (HMENU)hMenu, NULL, 0);
	return hwnd;
}
HWND Menu::CreateGeneralInputBox(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, HWND hWnd)
{
	HWND hwnd = CreateWindowExA(WS_EX_STATICEDGE, "edit", lpWindowName, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER, x, y, iWidth, iHeight, hWnd, NULL, NULL, 0);
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, true);
	return hwnd;
}
HWND Menu::CreateGeneralComboBox(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, int32_t hMenu, HWND hWnd)
{
	HWND hwnd = CreateWindowExA(WS_EX_STATICEDGE, "combobox", lpWindowName, WS_VISIBLE | WS_CHILD | LBS_STANDARD | LBS_DISABLENOSCROLL, x, y, iWidth, iHeight, hWnd, (HMENU)hMenu, NULL, 0);
	return hwnd;
}

void Menu::Init(HMODULE hModule)
{
	inj_hModule = hModule;
	HMENU hMenu = CreateDLLWindowMenu();

	// Main Window
	RegisterDLLWindowClass(L"DLLWindowClass");
	mainHWND = CreateWindowExA(0, "DLLWindowClass", "Tibia v8.6.0.0", WS_EX_LAYERED | WS_MINIMIZEBOX, 0, 0, mainWindowWidth, mainWindowHeight, NULL, hMenu, inj_hModule, NULL);



	RECT rc;
	//retrieves the dimensions of the bounding rectangle
	GetWindowRect(mainHWND, &rc);

	//get center position for main window
	int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

	//set position of main window
	SetWindowPos(mainHWND, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	//show main window
	ShowWindow(mainHWND, SW_SHOWNORMAL);

	//display main window on top
	SetForegroundWindow(mainHWND);



	CreateMainMenu();
}
HWND Menu::CreateButton(const char* lpWindowName, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight, int32_t hMenu)
{
	HWND hwnd = CreateWindowExA(0, "button", lpWindowName, WS_CHILD | WS_VISIBLE, x, y, iWidth, iHeight, mainHWND, (HMENU)hMenu, inj_hModule, 0);
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, true);
	return hwnd;
}

void Menu::CreateMainMenu()
{
	buttonHealer = CreateButton("Healer", 10, 10, 100, 20, CLB_OPEN_HEALER_WINDOW);
	buttonTimers = CreateButton("Timers", 120, 10, 100, 20, CLB_OPEN_TIMERS_WINDOW);
	buttonUtils = CreateButton("Utils", 230, 10, 100, 20, CLB_OPEN_UTILS_WINDOW);
	buttonPvp = CreateButton("Pvp", 340, 10, 100, 20, CLB_OPEN_PVP_WINDOW);
}



void Menu::CreateHealerMenu(HWND hWindow)
{

	// REGION HEALING

	std::string LIGHT_HEAL_SPELL_PERC = std::to_string(lightSpell.hpPercentage);
	char const* LHPERC = LIGHT_HEAL_SPELL_PERC.c_str();
	std::string MID_HEAL_SPELL_PERC = std::to_string(midSpell.hpPercentage);
	char const* MHPERC = MID_HEAL_SPELL_PERC.c_str();
	std::string HEAVY_HEAL_SPELL_PERC = std::to_string(heavySpell.hpPercentage);
	char const* HHPERC = HEAVY_HEAL_SPELL_PERC.c_str();


	std::string LIGHT_HEAL_SPELL_MP = std::to_string(lightSpell.manaCost);
	char const* LHSMP = LIGHT_HEAL_SPELL_MP.c_str();
	std::string MID_HEAL_SPELL_MP = std::to_string(midSpell.manaCost);
	char const* MHSMP = MID_HEAL_SPELL_MP.c_str();
	std::string HEAVY_HEAL_SPELL_MP = std::to_string(heavySpell.manaCost);
	char const* HHSMP = HEAVY_HEAL_SPELL_MP.c_str();


	std::string HEALTH_POTION_ID = std::to_string(healthPotionHealer.itemId);
	char const* HPID = HEALTH_POTION_ID.c_str();
	std::string HEALTH_POTION_HPPERC = std::to_string(healthPotionHealer.hpPerc);
	char const* HPPERC = HEALTH_POTION_HPPERC.c_str();


	std::string MANA_POTION_ID = std::to_string(manaPotionHealer.itemId);
	char const* MPID = MANA_POTION_ID.c_str();
	std::string MANA_POTION_HPPERC = std::to_string(manaPotionHealer.manaPerc);
	char const* MPPERC = MANA_POTION_HPPERC.c_str();

	std::string FRIEND_HP_HPPERC = std::to_string(sioUhPercent.friendHpPercentage);
	char const* FHPPERC = FRIEND_HP_HPPERC.c_str();
	std::string MY_HP_HPPERC = std::to_string(sioUhPercent.myHpPercentage);
	char const* MYHPPERC = MY_HP_HPPERC.c_str();

	CreateGeneralGroupBox("Healing Spells / Potions", 5, 0, 310, 188, hWindow);

	cBoxEnableHealer = CreateGeneralCheckBox("", 172, 0, 15, 20, CLB_HEALER, hWindow);

	CreateGeneralLabel("Light:", 10, 25, 40, 20, hWindow);
	CreateGeneralLabel("Mid:", 10, 55, 25, 20, hWindow);
	CreateGeneralLabel("Heavy:", 10, 85, 40, 20, hWindow);

	inputLightSpell.inputSpell = CreateGeneralInputBox(&lightSpell.spell[0], 60, 25, 100, 20, hWindow);
	inputMidSpell.inputSpell = CreateGeneralInputBox(&midSpell.spell[0], 60, 55, 100, 20, hWindow);
	inputHeavySpell.inputSpell = CreateGeneralInputBox(&heavySpell.spell[0], 60, 85, 100, 20, hWindow);

	CreateGeneralLabel("%", 170, 25, 20, 20, hWindow);
	CreateGeneralLabel("%", 170, 55, 20, 20, hWindow);
	CreateGeneralLabel("%", 170, 85, 20, 20, hWindow);

	inputLightSpell.inputHpPerc = CreateGeneralInputBox(LHPERC, 190, 25, 30, 20, hWindow);
	inputMidSpell.inputHpPerc = CreateGeneralInputBox(MHPERC, 190, 55, 30, 20, hWindow);
	inputHeavySpell.inputHpPerc = CreateGeneralInputBox(HHPERC, 190, 85, 30, 20, hWindow);

	CreateGeneralLabel("MP:", 230, 25, 30, 20, hWindow);
	CreateGeneralLabel("MP:", 230, 55, 30, 20, hWindow);
	CreateGeneralLabel("MP:", 230, 85, 30, 20, hWindow);

	inputLightSpell.inputMana = CreateGeneralInputBox(LHSMP, 260, 25, 45, 20, hWindow);
	inputMidSpell.inputMana = CreateGeneralInputBox(MHSMP, 260, 55, 45, 20, hWindow);
	inputHeavySpell.inputMana = CreateGeneralInputBox(HHSMP, 260, 85, 45, 20, hWindow);

	CreateGeneralLabel("Health Potion:", 10, 120, 100, 20, hWindow);
	inputHealthPotion.inputItemId = CreateGeneralInputBox(HPID, 100, 120, 60, 20, hWindow);
	CreateGeneralLabel("HP", 170, 120, 50, 20, hWindow);
	inputHealthPotion.inputHpPerc = CreateGeneralInputBox(HPPERC, 200, 120, 30, 20, hWindow);

	CreateGeneralLabel("Mana Potion:", 10, 150, 100, 20, hWindow);
	inputManaPotion.inputItemId = CreateGeneralInputBox(MPID, 100, 150, 60, 20, hWindow);
	CreateGeneralLabel("MP", 170, 150, 50, 20, hWindow);
	inputManaPotion.inputMana = CreateGeneralInputBox(MPPERC, 200, 150, 30, 20, hWindow);


	CreateGeneralGroupBox("Heal Friends", 5, 188, 310, 100, hWindow);

	buttonSioList = CreateGeneralButton("Sio List", 10, 208, 100, 20, CLB_OPEN_SIO_LIST, hWindow);

	CreateGeneralLabel("FriendHP:", 10, 233, 80, 20, hWindow);
	inputSioHp.inputHpPerc = CreateGeneralInputBox(FHPPERC, 75, 233, 30, 20, hWindow);
	CreateGeneralLabel("%", 110, 233, 20, 20, hWindow);

	CreateGeneralLabel("Sio:", 140, 233, 80, 20, hWindow);
	cBoxHealFriendsSio = CreateGeneralCheckBox("", 170, 233, 10, 20, CLB_HEAL_FRIENDS_SIO, hWindow);

	CreateGeneralLabel("MyHP:", 10, 258, 80, 20, hWindow);
	inputSioMyHp.inputHpPerc = CreateGeneralInputBox(MYHPPERC, 75, 258, 30, 20, hWindow);
	CreateGeneralLabel("%", 110, 258, 20, 20, hWindow);

	CreateGeneralLabel("UH:", 140, 258, 80, 20, hWindow);
	cBoxHealFriendsUH = CreateGeneralCheckBox("", 170, 258, 10, 20, CLB_HEAL_FRIENDS_UH, hWindow);

	lBoxHealFriendsOptions = CreateGeneralComboBox("", 210, 208, 100, 70, CLB_LIST_BOX_HEAL_MODES, hWindow);

	for (int32_t i = 0; i < sizeof(healFriendOptions) / sizeof(healFriendOptions[0]); i++)
	{
		SendMessageA(lBoxHealFriendsOptions, CB_ADDSTRING, 0, (LPARAM)healFriendOptions[i].name);
	}

	SendMessage(lBoxHealFriendsOptions, CB_SETCURSEL, selectedHealFriendMode, (LPARAM)0);
}

void Menu::CreateTimersMenu(HWND hWindow)
{
	std::string HEAL_SPELL = std::to_string(Cooldowns::GetInstance().HEAL_SPELL);
	const char* HEAL_SPELL_COOLDOWN = HEAL_SPELL.c_str();

	std::string POTIONS = std::to_string(Cooldowns::GetInstance().HEAL_ITEM);
	const char* POTIONS_COOLDOWN = POTIONS.c_str();

	std::string RUNES = std::to_string(Cooldowns::GetInstance().ATTACK_ITEM);
	const char* RUNES_COOLDOWN = RUNES.c_str();

	std::string DASH = std::to_string(Cooldowns::GetInstance().BOOST_DELAY);
	const char* DASH_COOLDOWN = DASH.c_str();

	std::string PUSH = std::to_string(Cooldowns::GetInstance().PUSH_CREATURE);
	const char* PUSH_COOLDOWN = PUSH.c_str();

	std::string EQUIP = std::to_string(Cooldowns::GetInstance().EQUIP_ITEM);
	const char* EQUIP_COOLDOWN = EQUIP.c_str();

	std::string DROP = std::to_string(Cooldowns::GetInstance().DROP_ITEM);
	const char* DROP_COOLDOWN = DROP.c_str();

	std::string ADVERTISING = std::to_string(Cooldowns::GetInstance().SAY_ADVERTISING);
	const char* ADVERTISING_COOLDOWN = ADVERTISING.c_str();

	std::string HOLDTARGET = std::to_string(Cooldowns::GetInstance().ATTACK_CREATURE);
	const char* HOLDTARGET_COOLDOWN = HOLDTARGET.c_str();

	std::string COMBO = std::to_string(Cooldowns::GetInstance().COMBO_BOT);
	const char * COMBO_COOLDOWN = COMBO.c_str();


	CreateGeneralGroupBox("Timers", 5, 0, 280, 175, hWindow); // 437

	CreateGeneralLabel("Spells:", 10, 20, 80, 20, hWindow);
	inputTimerHealingSpells.inputTime = CreateGeneralInputBox(HEAL_SPELL_COOLDOWN, 70, 20, 60, 20, hWindow);

	CreateGeneralLabel("Potions:", 10, 45, 80, 20, hWindow);
	inputTimerHealingItems.inputTime = CreateGeneralInputBox(POTIONS_COOLDOWN, 70, 45, 60, 20, hWindow);

	CreateGeneralLabel("Runes:", 10, 70, 80, 20, hWindow);
	inputTimerRunes.inputTime = CreateGeneralInputBox(RUNES_COOLDOWN, 70, 70, 60, 20, hWindow);

	CreateGeneralLabel("Dash:", 10, 95, 80, 20, hWindow);
	inputTimerDash.inputTime = CreateGeneralInputBox(DASH_COOLDOWN, 70, 95, 60, 20, hWindow);

	CreateGeneralLabel("Push:", 10, 120, 80, 20, hWindow);
	inputTimerPush.inputTime = CreateGeneralInputBox(PUSH_COOLDOWN, 70, 120, 60, 20, hWindow);

	CreateGeneralLabel("Equip:", 10, 145, 80, 20, hWindow);
	inputTimerEquip.inputTime = CreateGeneralInputBox(EQUIP_COOLDOWN, 70, 145, 60, 20, hWindow);

	CreateGeneralLabel("Drop item:", 140, 20, 100, 20, hWindow);
	inputTimerDrop.inputTime = CreateGeneralInputBox(DROP_COOLDOWN, 220, 20, 60, 20, hWindow);

	CreateGeneralLabel("Advertising:", 140, 45, 100, 20, hWindow);
	inputTimerAdvertising.inputTime = CreateGeneralInputBox(ADVERTISING_COOLDOWN, 220, 45, 60, 20, hWindow);

	CreateGeneralLabel("Hold Target:", 140, 70, 100, 20, hWindow);
	inputTimerHoldTarget.inputTime = CreateGeneralInputBox(HOLDTARGET_COOLDOWN, 220, 70, 60, 20, hWindow);

	CreateGeneralLabel("Combo Bot:", 140, 95, 100, 20, hWindow);
	inputTimerComboBot.inputTime = CreateGeneralInputBox(COMBO_COOLDOWN, 220, 95, 60, 20, hWindow);

	buttonCooldowns = CreateGeneralButton("Set new cooldowns", 140, 120, 125, 20, CLB_TIMERS, hWindow);


}

void Menu::CreateUtilsMenu(HWND hWindow)
{
	CreateGeneralGroupBox("Usefull Tools", 5, 0, 240, 300, hWindow);

	cBoxEnableAdvert = CreateGeneralCheckBox("Advertisting", 10, 25, 100, 20, CLB_ADVERTISING, hWindow);

	inputAdvertisingString = CreateGeneralInputBox(&advertisingMessage[0], 120, 25, 120, 20, hWindow);

	cBoxXray = CreateGeneralCheckBox("Xray", 10, 45, 100, 20, CLB_XRAY, hWindow);

	cBoxEnableAntiIdle = CreateGeneralCheckBox("Anti Idle", 10, 65, 100, 20, CLB_ANTI_IDLE, hWindow);

	cBoxEnableOutfitHack = CreateGeneralCheckBox("Auto Attack", 10, 85, 100, 20, CLB_AUTO_ATTACK, hWindow);

	cBoxEnableAutoMount = CreateGeneralCheckBox("Auto Mount", 10, 105, 100, 20, CLB_MOUNT, hWindow);

	cBoxEnableEatFood = CreateGeneralCheckBox("Eat Food", 10, 125, 100, 20, CLB_FOOD_EAT, hWindow);

	cBoxLookIds = CreateGeneralCheckBox("Look Ids", 10, 145, 120, 20, CLB_LOOK_IDS, hWindow);

	cBoxEnableHoldPosition = CreateGeneralCheckBox("Hold Position", 10, 165, 120, 20, CLB_HOLD_POSITION, hWindow);

	cBoxEnableDisableMounts = CreateGeneralCheckBox("Disable Mounts", 10, 185, 140, 20, CLB_DISABLE_MOUNTS, hWindow);

	cBoxComboBot = CreateGeneralCheckBox("Combo Bot", 10, 205, 100, 20, CLB_COMBO_BOT, hWindow);

	HWND leaderName = CreateGeneralLabel("Leader name: ", 10, 225, 100, 20, hWindow);

	SendMessage(leaderName, WM_SETFONT, (WPARAM)hFontNew, true);

	inputComboBot.inputLeaderName = CreateGeneralInputBox(&Hooks::comboBotSpells.leaderName[0], 120, 225, 100, 20, hWindow);

	HWND leaderSpell = CreateGeneralLabel("Leader spell: ", 10, 247, 100, 20, hWindow);

	SendMessage(leaderSpell, WM_SETFONT, (WPARAM)hFontNew, true);

	inputComboBot.inputLeaderSpell = CreateGeneralInputBox(&Hooks::comboBotSpells.leaderSpell[0], 120, 247, 100, 20, hWindow);

	HWND yourSpell = CreateGeneralLabel("Your spell: ", 10, 270, 100, 20, hWindow);

	SendMessage(yourSpell, WM_SETFONT, (WPARAM)hFontNew, true);

	inputComboBot.inputMySpell = CreateGeneralInputBox(&Hooks::comboBotSpells.mySpell[0], 120, 270, 100, 20, hWindow);
}

void Menu::CreatePvpMenu(HWND hWindow)
{
	std::string KILL_TARGET_HP_PERCENTAGE = std::to_string(killTarget.hpPercentage);
	char const* KHPPERC = KILL_TARGET_HP_PERCENTAGE.c_str();

	std::string KILL_TARGET_MANA_COST = std::to_string(killTarget.manaCost);
	char const* KTMCOST = KILL_TARGET_MANA_COST.c_str();

	std::string HASTE_MANA_COST = std::to_string(hasteSpell.manaCost);
	char const* HMCOST = HASTE_MANA_COST.c_str();

	std::string PARALYZE_SPELL_MANA_COST = std::to_string(cureParalayseSpell.manaCost);
	char const* PSMCOST = PARALYZE_SPELL_MANA_COST.c_str();


	CreateGeneralGroupBox("PVP Tools / Helpers", 5, 0, 410, 220, hWindow);

	cBoxEnableHoldTarget = CreateGeneralCheckBox("Hold Target", 10, 25, 100, 20, CLB_HOLD_TARGET, hWindow);

	cBoxEnableHoldDash = CreateGeneralCheckBox("Numpad Dash", 10, 45, 120, 20, CLB_DASH, hWindow);

	cBoxEnableMagicShield = CreateGeneralCheckBox("Magic Shield", 10, 65, 100, 20, CLB_MAGIC_SHIELD, hWindow);

	cBoxKillTarget = CreateGeneralCheckBox("Kill Target", 10, 85, 100, 20, CLB_KILL_ENEMY_ON_LOW_HP, hWindow);

	CreateGeneralLabel("HP %:", 10, 105, 100, 20, hWindow);

	inputTargetSpell.inputHpPerc = CreateGeneralInputBox(KHPPERC, 50, 105, 50, 20, hWindow);

	CreateGeneralLabel("MP:", 105, 105, 80, 20, hWindow);

	inputTargetSpell.inputMana = CreateGeneralInputBox(KTMCOST, 130, 105, 50, 20, hWindow);

	inputTargetSpell.inputSpell = CreateGeneralInputBox(&killTarget.spell[0], 185, 105, 115, 20, hWindow);

	cBoxEnableHaste = CreateGeneralCheckBox("Auto Haste", 10, 125, 100, 20, CLB_HASTE, hWindow);

	inputHasteSpell.inputSpell = CreateGeneralInputBox(&hasteSpell.spell[0], 10, 145, 121, 20, hWindow);

	CreateGeneralLabel("MP:", 135, 145, 30, 20, hWindow);
	inputHasteSpell.inputMana = CreateGeneralInputBox(HMCOST, 160, 145, 40, 20, hWindow);


	cBoxEnableAntiParalayse = CreateGeneralCheckBox("Anti Paralyze", 10, 165, 110, 20, CLB_ANTI_PARALAYSE, hWindow);
	inputCureParalayseSpell.inputSpell = CreateGeneralInputBox(&cureParalayseSpell.spell[0], 10, 185, 120, 20, hWindow);
	CreateGeneralLabel("MP:", 135, 185, 30, 20, hWindow);
	inputCureParalayseSpell.inputMana = CreateGeneralInputBox(PSMCOST, 160, 185, 40, 20, hWindow);







	CreateGeneralGroupBox("Amulet Balancer", 5, 220, 410, 73, hWindow);

	//cBoxEquipByHotkey = CreateGeneralCheckBox("Mode: hotkey", 168, 220, 107, 20, CLB_EQUIP_MODE_AS_HOTKEY, hWindow);
	cBoxEquipByMoveItem = CreateGeneralCheckBox("Mode: move item", 168, 220, 135, 20, CLB_EQUIP_MODE_AS_MOVE_ITEM, hWindow);

	lBoxAmulets = CreateGeneralComboBox("Amulet Options", 10, 245, 160, 70, CLB_LIST_BOX_AMULETS, hWindow);

	for (int32_t i = 0; i < sizeof(amuletOptions) / sizeof(amuletOptions[0]); i++)
	{
		SendMessageA(lBoxAmulets, CB_ADDSTRING, 0, (LPARAM)amuletOptions[i].name);
	}
	//SendMessage(lBoxAmulets, CB_SETCURSEL, selectedAmuletOption, (LPARAM)0);



	/*lBoxRings = CreateGeneralComboBox("Ring Options", 10, 300, 160, 70, CLB_LIST_BOX_RINGS, hWindow);

	for (int32_t i = 0; i < sizeof(ringOptions) / sizeof(ringOptions[0]); i++)
	{
		SendMessageA(lBoxRings, CB_ADDSTRING, 0, (LPARAM)ringOptions[i].name);
	}*/
}

void Menu::ChangeCooldown(const InputTimerLabel& timerLabel, int32_t& cooldown)
{
	GetWindowTextA(timerLabel.inputTime, &buf[0], sizeof(buf));
	cooldown = atoi(buf);
}

void Menu::ToggleSio()
{
	if (bHealFriendSio || bHealFriendUH)
	{
		if (GetWindowTextA(inputSioHp.inputHpPerc, &buf[0], sizeof(buf)))
		{
			healFriendOnHp = atoi(buf);
			sioUhPercent.friendHpPercentage = atoi(buf);
			if (GetWindowTextA(inputSioMyHp.inputHpPerc, &buf[0], sizeof(buf)))
			{
				safeHpForSio = atoi(buf);
				sioUhPercent.myHpPercentage = atoi(buf);
			}
			else
			{
				safeHpForSio = NULL;
			}
		}
		else
		{
			safeHpForSio = NULL;
			healFriendOnHp = NULL;
			sioUhPercent.myHpPercentage = NULL;
			sioUhPercent.friendHpPercentage = NULL;
		}
		if (selectedHealFriendMode == 0)
		{
			ReadSioListTxt(friendList);
		}
	}
	if (bHealFriendSio)
	{
		EnableWindow(inputSioHp.inputHpPerc, !bHealFriendSio);
		EnableWindow(inputSioMyHp.inputHpPerc, !bHealFriendSio);
		EnableWindow(cBoxHealFriendsUH, !bHealFriendSio);
		EnableWindow(buttonSioList, !bHealFriendSio);
		EnableWindow(lBoxHealFriendsOptions, !bHealFriendSio);
	}
	else if (bHealFriendUH)
	{
		EnableWindow(inputSioHp.inputHpPerc, !bHealFriendUH);
		EnableWindow(inputSioMyHp.inputHpPerc, !bHealFriendUH);
		EnableWindow(cBoxHealFriendsSio, !bHealFriendUH);
		EnableWindow(buttonSioList, !bHealFriendUH);
		EnableWindow(lBoxHealFriendsOptions, !bHealFriendUH);
	}
	else
	{
		EnableWindow(inputSioHp.inputHpPerc, true);
		EnableWindow(inputSioMyHp.inputHpPerc, true);
		EnableWindow(cBoxHealFriendsUH, true);
		EnableWindow(cBoxHealFriendsSio, true);
		EnableWindow(buttonSioList, true);
		EnableWindow(lBoxHealFriendsOptions, true);
	}
}

void Menu::ToggleHealer()
{
	if (bHealer)
	{
		lightSpell.spellLenght = GetWindowTextA(inputLightSpell.inputSpell, &lightSpell.spell[0], sizeof(lightSpell.spell));
		if (lightSpell.spellLenght)
		{
			GetWindowTextA(inputLightSpell.inputMana, &buf[0], sizeof(buf));
			lightSpell.manaCost = atoi(buf);
			GetWindowTextA(inputLightSpell.inputHpPerc, &buf[0], sizeof(buf));
			lightSpell.hpPercentage = atoi(buf);
		}
		else
		{
			lightSpell.manaCost = 0;
			lightSpell.hpPercentage = 0;
		}

		midSpell.spellLenght = GetWindowTextA(inputMidSpell.inputSpell, &midSpell.spell[0], sizeof(midSpell.spell));
		if (midSpell.spellLenght)
		{
			GetWindowTextA(inputMidSpell.inputMana, &buf[0], sizeof(buf));
			midSpell.manaCost = atoi(buf);
			GetWindowTextA(inputMidSpell.inputHpPerc, &buf[0], sizeof(buf));
			midSpell.hpPercentage = atoi(buf);
		}
		else
		{
			midSpell.manaCost = 0;
			midSpell.hpPercentage = 0;
		}

		heavySpell.spellLenght = GetWindowTextA(inputHeavySpell.inputSpell, &heavySpell.spell[0], sizeof(heavySpell.spell));
		if (heavySpell.spellLenght)
		{
			GetWindowTextA(inputHeavySpell.inputMana, &buf[0], sizeof(buf));
			heavySpell.manaCost = atoi(buf);
			GetWindowTextA(inputHeavySpell.inputHpPerc, &buf[0], sizeof(buf));
			heavySpell.hpPercentage = atoi(buf);
		}
		else
		{
			heavySpell.manaCost = 0;
			heavySpell.hpPercentage = 0;
		}

		if (GetWindowTextA(inputHealthPotion.inputItemId, &buf[0], sizeof(buf)))
		{
			healthPotionHealer.itemId = atoi(buf);

			if (GetWindowTextA(inputHealthPotion.inputHpPerc, &buf[0], sizeof(buf)))
			{
				healthPotionHealer.hpPerc = atoi(buf);
			}
			else
			{
				healthPotionHealer.hpPerc = NULL;
			}
		}
		else
		{
			healthPotionHealer.itemId = NULL;
		}

		if (GetWindowTextA(inputManaPotion.inputItemId, &buf[0], sizeof(buf)))
		{
			manaPotionHealer.itemId = atoi(buf);
			if (GetWindowTextA(inputManaPotion.inputMana, &buf[0], sizeof(buf)))
			{
				manaPotionHealer.manaPerc = atoi(buf);
			}
			else
			{
				manaPotionHealer.manaPerc = NULL;
			}
		}
		else
		{
			manaPotionHealer.itemId = NULL;
		}
	}
	EnableWindow(inputLightSpell.inputSpell, !bHealer);
	EnableWindow(inputMidSpell.inputSpell, !bHealer);
	EnableWindow(inputHeavySpell.inputSpell, !bHealer);

	EnableWindow(inputLightSpell.inputMana, !bHealer);
	EnableWindow(inputMidSpell.inputMana, !bHealer);
	EnableWindow(inputHeavySpell.inputMana, !bHealer);

	EnableWindow(inputLightSpell.inputHpPerc, !bHealer);
	EnableWindow(inputMidSpell.inputHpPerc, !bHealer);
	EnableWindow(inputHeavySpell.inputHpPerc, !bHealer);

	EnableWindow(inputHealthPotion.inputItemId, !bHealer);
	EnableWindow(inputHealthPotion.inputHpPerc, !bHealer);

	EnableWindow(inputManaPotion.inputItemId, !bHealer);
	EnableWindow(inputManaPotion.inputMana, !bHealer);
}

void Menu::ToggleTimers()
{
	ChangeCooldown(inputTimerHealingSpells, Cooldowns::GetInstance().HEAL_SPELL);
	ChangeCooldown(inputTimerHealingItems, Cooldowns::GetInstance().HEAL_ITEM);
	ChangeCooldown(inputTimerAdvertising, Cooldowns::GetInstance().SAY_ADVERTISING);
	ChangeCooldown(inputTimerComboBot, Cooldowns::GetInstance().COMBO_BOT);
	ChangeCooldown(inputTimerDash, Cooldowns::GetInstance().BOOST_DELAY);
	ChangeCooldown(inputTimerDrop, Cooldowns::GetInstance().DROP_ITEM);
	ChangeCooldown(inputTimerEquip, Cooldowns::GetInstance().EQUIP_ITEM);
	ChangeCooldown(inputTimerHoldTarget, Cooldowns::GetInstance().ATTACK_CREATURE);
	ChangeCooldown(inputTimerRunes, Cooldowns::GetInstance().ATTACK_ITEM);
	ChangeCooldown(inputTimerPush, Cooldowns::GetInstance().PUSH_CREATURE);
}

void Menu::ToggleAdvertising()
{
	// lock input after activating module
	GetWindowTextA(inputAdvertisingString, &advertisingMessage[0], sizeof(advertisingMessage));
	EnableWindow(inputAdvertisingString, !bAdvertising);
}

void Menu::ToggleComboBot()
{
	if (Hooks::bComboBot)
	{
		bool inputLenghtLeaderName = GetWindowTextA(inputComboBot.inputLeaderName, &Hooks::comboBotSpells.leaderName[0], sizeof(Hooks::comboBotSpells.leaderName)) > 0;
		bool inputLenghtLeaderSpell = GetWindowTextA(inputComboBot.inputLeaderSpell, &Hooks::comboBotSpells.leaderSpell[0], sizeof(Hooks::comboBotSpells.leaderSpell)) > 0;
		bool inputLenghtMySpell = GetWindowTextA(inputComboBot.inputMySpell, &Hooks::comboBotSpells.mySpell[0], sizeof(Hooks::comboBotSpells.mySpell)) > 0;
		if (!inputLenghtLeaderName || !inputLenghtLeaderSpell || !inputLenghtMySpell)
		{
			Hooks::comboBotSpells.leaderName[0] = NULL;
			Hooks::comboBotSpells.leaderSpell[0] = NULL;
			Hooks::comboBotSpells.mySpell[0] = NULL;
		}
	}
	EnableWindow(inputComboBot.inputLeaderName, !Hooks::bComboBot);
	EnableWindow(inputComboBot.inputLeaderSpell, !Hooks::bComboBot);
	EnableWindow(inputComboBot.inputMySpell, !Hooks::bComboBot);
}



void Menu::ToggleKillTarget()
{
	if (bKillTarget)
	{
		killTarget.spellLenght = GetWindowTextA(inputTargetSpell.inputSpell, &killTarget.spell[0], sizeof(killTarget.spell));
		if (killTarget.spellLenght)
		{
			GetWindowTextA(inputTargetSpell.inputMana, &buf[0], sizeof(buf));
			killTarget.manaCost = atoi(buf);
			GetWindowTextA(inputTargetSpell.inputHpPerc, &buf[0], sizeof(buf));
			killTarget.hpPercentage = atoi(buf);
		}
		else
		{
			killTarget.manaCost = 0;
			killTarget.hpPercentage = 0;
		}
	}
	EnableWindow(inputTargetSpell.inputSpell, !bKillTarget);
	EnableWindow(inputTargetSpell.inputHpPerc, !bKillTarget);
	EnableWindow(inputTargetSpell.inputMana, !bKillTarget);
}

void Menu::ToggleHaste()
{
	if (bAutoHaste)
	{
		hasteSpell.spellLenght = GetWindowTextA(inputHasteSpell.inputSpell, &hasteSpell.spell[0], sizeof(hasteSpell.spell));
		if (hasteSpell.spellLenght)
		{
			GetWindowTextA(inputHasteSpell.inputMana, &buf[0], sizeof(buf));
			hasteSpell.manaCost = atoi(buf);
		}
		else
		{
			hasteSpell.manaCost = 0;
		}
	}
	EnableWindow(inputHasteSpell.inputSpell, !bAutoHaste);
	EnableWindow(inputHasteSpell.inputMana, !bAutoHaste);
}

void Menu::ToggleAntiParalayse()
{
	if (bAntiParalayse)
	{
		cureParalayseSpell.spellLenght = GetWindowTextA(inputCureParalayseSpell.inputSpell, &cureParalayseSpell.spell[0], sizeof(cureParalayseSpell.spell));
		if (cureParalayseSpell.spellLenght)
		{
			GetWindowTextA(inputCureParalayseSpell.inputMana, &buf[0], sizeof(buf));
			cureParalayseSpell.manaCost = atoi(buf);
		}
		else
		{
			cureParalayseSpell.manaCost = 0;
		}
	}
	EnableWindow(inputCureParalayseSpell.inputSpell, !bAntiParalayse);
	EnableWindow(inputCureParalayseSpell.inputMana, !bAntiParalayse);
}

void Menu::ToggleItemBalancer()
{
	if (bEquipModeHotkey || bEquipModeMoveItem)
	{
		if (bLoadedAmulets)
		{
			if (GetWindowTextA(inputHoldAmulet.inputItemId, &buf[0], sizeof(buf)))
			{
				holdAmulet.Id = atoi(buf);
				if (GetWindowTextA(inputHoldAmulet.inputHpPerc, &buf[0], sizeof(buf)))
				{
					holdAmulet.hpPercentage = atoi(buf);

					///////
					balanceAmulet.hpPercentage = NULL;
					balanceAmulet.Id = NULL;
					dequipAmulet.hpPercentage = NULL;
					dequipAmulet.Id = NULL;
				}
				else
				{
					holdAmulet.hpPercentage = NULL;
				}
			}
			else
			{
				holdAmulet.hpPercentage = NULL;
				holdAmulet.Id = NULL;
			}
			switch (selectedAmuletOption)
			{
			case 1:
				if (GetWindowTextA(inputDequipAmulet.inputItemId, &buf[0], sizeof(buf)))
				{
					dequipAmulet.Id = atoi(buf);
					dequipAmulet.hpPercentage = holdAmulet.hpPercentage;

					///////
					balanceAmulet.hpPercentage = NULL;
					balanceAmulet.Id = NULL;
				}
				else
				{
					dequipAmulet.hpPercentage = NULL;
					dequipAmulet.Id = NULL;
				}
				break;
			case 2:
				if (GetWindowTextA(inputBalanceAmulet.inputItemId, &buf[0], sizeof(buf)))
				{
					balanceAmulet.Id = atoi(buf);
					balanceAmulet.hpPercentage = holdAmulet.hpPercentage;

					///////
					dequipAmulet.hpPercentage = NULL;
					dequipAmulet.Id = NULL;
				}
				else
				{
					balanceAmulet.hpPercentage = NULL;
					balanceAmulet.Id = NULL;
				}
				break;
			}
		}
		if (bLoadedRings)
		{
			if (GetWindowTextA(inputHoldRing.inputItemId, &buf[0], sizeof(buf)))
			{
				holdRing.Id = atoi(buf);
				if (GetWindowTextA(inputHoldRing.inputHpPerc, &buf[0], sizeof(buf)))
				{
					holdRing.hpPercentage = atoi(buf);
					holdRing.manaPerc = 0;
				}
				else
				{
					holdRing.hpPercentage = NULL;
				}
			}
			else
			{
				holdRing.hpPercentage = NULL;
				holdRing.Id = NULL;
			}
			switch (selectedRingsOption)
			{
			case 1:
				if (GetWindowTextA(inputDequipRing.inputItemId, &buf[0], sizeof(buf)))
				{
					dequipRing.Id = atoi(buf);
					dequipRing.hpPercentage = holdRing.hpPercentage;
				}
				else
				{
					dequipRing.hpPercentage = NULL;
					dequipRing.Id = NULL;
				}
				break;
			case 2:
				if (GetWindowTextA(inputBalanceRing.inputItemId, &buf[0], sizeof(buf)))
				{
					balanceRing.Id = atoi(buf);
					balanceRing.hpPercentage = holdRing.hpPercentage;
					balanceRing.manaPerc = 0;
				}
				else
				{
					balanceRing.hpPercentage = NULL;
					balanceRing.Id = NULL;
				}
				break;
			}
		}
		EnableWindow(cBoxEquipByMoveItem, !bEquipModeHotkey);
		EnableWindow(cBoxEquipByHotkey, !bEquipModeMoveItem);
		if (bLoadedAmulets)
		{
			EnableWindow(inputHoldAmulet.inputItemId, !bEquipModeHotkey);
			EnableWindow(inputHoldAmulet.inputHpPerc, !bEquipModeHotkey);
			switch (selectedAmuletOption)
			{
			case 1:
				EnableWindow(inputDequipAmulet.inputItemId, !bEquipModeHotkey);
				break;

			case 2:
				EnableWindow(inputBalanceAmulet.inputItemId, !bEquipModeHotkey);
				break;
			}
		}
		if (bLoadedRings)
		{
			EnableWindow(inputHoldRing.inputItemId, !bEquipModeHotkey);
			EnableWindow(inputHoldRing.inputHpPerc, !bEquipModeHotkey);
			switch (selectedRingsOption)
			{
			case 1:
				EnableWindow(inputDequipRing.inputItemId, !bEquipModeHotkey);
				break;

			case 2:
				EnableWindow(inputBalanceRing.inputItemId, !bEquipModeHotkey);
				break;
			}
		}
	}
	bool state = bEquipModeHotkey || bEquipModeMoveItem;
	EnableWindow(cBoxEquipByMoveItem, !bEquipModeHotkey);
	EnableWindow(cBoxEquipByHotkey, !bEquipModeMoveItem);
	EnableWindow(lBoxAmulets, !state);
	EnableWindow(lBoxRings, !state);
	EnableWindow(inputHoldAmulet.inputHpPerc, !state);
	EnableWindow(inputHoldAmulet.inputItemId, !state);
	EnableWindow(inputDequipAmulet.inputItemId, !state);
	EnableWindow(inputBalanceAmulet.inputItemId, !state);
	EnableWindow(inputHoldRing.inputItemId, !state);
	EnableWindow(inputHoldRing.inputHpPerc, !state);
	EnableWindow(inputDequipRing.inputItemId, !state);
	EnableWindow(inputBalanceRing.inputItemId, !state);
}