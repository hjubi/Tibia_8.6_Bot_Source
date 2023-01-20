#include "pch.h"
#include "MemReader.h"
#include "Menu.h"

MemReader::MemReader()
{
	m_ModuleBase = (uintptr_t)GetModuleHandle(NULL);
	m_LocalPlayer = (Self*)(m_ModuleBase + Offsets::selfPlayer);
	m_EquipmentSlots = (EquipmentSlots*)(m_ModuleBase + Offsets::equipmentStructAddy);
	ItemFlagInfo = (GetObjectProperty)(m_ModuleBase + 0x100EC0); //0x100EC0
	m_XrayAddy1 = (uintptr_t)(m_ModuleBase + Offsets::XRayAddy1);
	m_XrayAddy2 = (uintptr_t)(m_ModuleBase + Offsets::XRayAddy2);
}

void MemReader::ReadSelfCharacter(CSelfCharacter* character)
{
	character->health = *(int*)(m_ModuleBase + Offsets::m_Health);
	character->maxHealth = *(int*)(m_ModuleBase + Offsets::m_MaxHealth);

	character->mana = *(int*)(m_ModuleBase + Offsets::m_Mana);
	character->maxMana = *(int*)(m_ModuleBase + Offsets::m_MaxMana);
	character->manaPercentage = (int)((float(character->mana) / float(character->maxMana)) * 100);

	character->flags = *(int*)(m_ModuleBase + Offsets::m_Flags);
	character->id = *(int*)(m_ModuleBase + Offsets::m_Id);

	character->level = *(int*)(m_ModuleBase + Offsets::m_Level);
	Entity* selfEnt = nullptr;
	selfEnt = GetEntityInEntityList(character->id);

	if (selfEnt)
	{
		character->hpPercentage = selfEnt->hpPercentage;

		character->xPos = selfEnt->xPos;
		character->yPos = selfEnt->yPos;
		character->zPos = selfEnt->zPos;
		strcpy_s(character->name, selfEnt->name);

		character->outfitId = selfEnt->outfitId;
		character->addonNumber = selfEnt->outfitId;
		character->bodyColor = selfEnt->primaryColor;
		character->bootsColor = selfEnt->detailColor;
		character->headColor = selfEnt->headColor;
		character->direction = selfEnt->direction;
		character->skull = selfEnt->skull;
		character->partyShield = selfEnt->partyShield;
		character->guildShield = selfEnt->guildShield;
	}
}

EquipmentSlots* MemReader::GetItemOnEquipmentSlots()
{
	return m_EquipmentSlots;
}

bool MemReader::IsFlagTrue(uint32_t flag)
{
	CSelfCharacter selfCharacter;
	ReadSelfCharacter(&selfCharacter);
	return (selfCharacter.flags & flag);
}

Entity* MemReader::GetEntityInEntityList(uint32_t characterId)
{
	for (uint32_t i = 0; i < 1300; i++)
	{
		Entity* entity = (Entity*)(m_ModuleBase + Offsets::entityListStartAddy + (Offsets::offsetToAnotherEntity * i));
		if (!entity) continue;

		if ((entity->id == characterId))
		{
			return entity;
		}
	}
	return nullptr;
}

bool MemReader::IsMonster(int32_t creatureId)
{
	return creatureId > 1000000000;
}

bool MemReader::IsPlayer(int32_t creatureId)
{
	return creatureId < 1000000000 && creatureId > 270000000;
}

bool MemReader::IsNPC(int32_t creatureId)
{
	return creatureId < 0;
}


std::vector<Entity*> MemReader::ReadVisibleCreatures()
{
	std::vector<Entity*> entities;
	CSelfCharacter selfCharacter;
	ReadSelfCharacter(&selfCharacter);

	for (uint32_t i = 0; i < 1300; i++)
	{
		Entity* entity = (Entity*)(m_ModuleBase + Offsets::entityListStartAddy + (Offsets::offsetToAnotherEntity * i));
		if (!entity) continue;

		if ((IsEntityOnScreen(entity)) && (entity->id != selfCharacter.id))
		{
			entities.push_back(entity);
		}
	}
	return entities;
}



bool MemReader::IsEntityOnScreen(Entity* ent)
{
	CSelfCharacter selfCharacter;
	ReadSelfCharacter(&selfCharacter);

	if ((ent->zPos == selfCharacter.zPos) && (abs(ent->xPos - selfCharacter.xPos) <= 7) && (abs(ent->yPos - selfCharacter.yPos) <= 5) && (ent->hpPercentage > 0))
	{
		return true;
	}
	return false;
}

int32_t MemReader::GetAttackedCreature()
{
	return *(int32_t*)(m_ModuleBase + Offsets::attackedCreature);
}



std::vector<Entity*> MemReader::GetEntityById(uint32_t creatureId)
{
	std::vector<Entity*> entities;
	CSelfCharacter selfCharacter;
	ReadSelfCharacter(&selfCharacter);

	for (uint32_t i = 0; i < 1300; i++)
	{
		Entity* entity = (Entity*)(m_ModuleBase + Offsets::entityListStartAddy + (Offsets::offsetToAnotherEntity * i));
		if (!entity) continue;

		if ((IsEntityOnScreen(entity)) && (entity->id != selfCharacter.id) && entity->id == creatureId)
		{
			entities.push_back(entity);
		}
	}
	return entities;
}



std::vector<VipStruct*> MemReader::GetVips()
{
	std::vector<VipStruct*> vips;
	std::vector<uintptr_t> pointers;
	uintptr_t newNode = mem::FindDMAAddy2(m_ModuleBase + Offsets::vipEntryPoint, {});
	int32_t vipsCount = GetVipCount();

	for (uint32_t i = 0; i < vipsCount; i++)
	{
		VipStruct* vipEnt = (VipStruct*)(mem::FindDMAAddy2(newNode + (i * 0x2C), {}));
		vips.push_back(vipEnt);
	}


	return vips;
}

int32_t MemReader::GetVipCount()
{
	int32_t vipCount = *(int32_t*)(m_ModuleBase + Offsets::vipCount);
	return vipCount;
}



bool MemReader::IsOnline()
{
	return (*(BYTE*)(m_ModuleBase + Offsets::m_ConnectionState) == CONNECTION_STATE::ONLINE ? true : false);
}


void MemReader::ReadSelfMapCell()
{
	CSelfCharacter selfCharacter;
	MemReader::GetInstance().ReadSelfCharacter(&selfCharacter);
	//std::vector<MapObject*> mapObjects;
	MapObject* mapObject;

	uintptr_t mapList = *(uintptr_t*)(m_ModuleBase + Offsets::mapStartAddy);

	int32_t floorSize = 252;
	int32_t tileNrLowest = max(0, min(1768, (selfCharacter.zPos <= 7) ? floorSize * (7 - selfCharacter.zPos) : (floorSize * 2)));
	int32_t tileNrHighest = tileNrLowest + floorSize;


	if (m_SelfCellNr)
	{
		mapObject = (MapObject*)(mapList + (m_SelfCellNr * Offsets::offsetToAnotherTile));

		std::vector<MapItem*> mapItems = mapObject->readTile();

		for (auto mapItem : mapItems)
		{
			if (mapItem)
			{
				if (mapItem->itemCount == selfCharacter.id)
				{
					return;
				}
			}
		}
	}

	for (tileNrLowest; tileNrLowest < tileNrHighest; tileNrLowest++)
	{
		mapObject = (MapObject*)(mapList + (tileNrLowest * Offsets::offsetToAnotherTile));

		if (!mapObject) continue;

		std::vector<MapItem*> mapItems = mapObject->readTile();

		for (auto mapItem : mapItems)
		{
			if (mapItem)
			{
				if (mapItem->itemCount == selfCharacter.id)
				{
					/*std::string a = std::to_string(mapItem->itemCount);
					MessageBoxA(NULL, a.c_str(), "TILE", NULL);*/
					m_SelfCellNr = tileNrLowest;
				}
			}
		}
	}
}

MapObject* MemReader::ReadMapTile(int32_t relativeX, int32_t relativeY)
{
	MapObject* mapObject = nullptr;

	ReadSelfMapCell();

	uintptr_t mapList = *(uintptr_t*)(m_ModuleBase + Offsets::mapStartAddy);

	int32_t floorSize = 252;
	int32_t tileNrLowest = max(0, min(1768, (m_LocalPlayer->zPos <= 7) ? floorSize * (7 - m_LocalPlayer->zPos) : (floorSize * 2)));
	int32_t tileNrHighest = tileNrLowest + floorSize;

	int32_t myZ = m_SelfCellNr / (14 * 18);
	int32_t myY = (m_SelfCellNr - myZ * 14 * 18) / 18;
	int32_t myX = (m_SelfCellNr - myZ * 14 * 18 - myY * 18);

	int32_t readX = (18 + ((myX + relativeX) % 18)) % 18;
	int32_t readY = (14 + ((myY + relativeY) % 14)) % 14;

	for (tileNrLowest; tileNrLowest < tileNrHighest; tileNrLowest++)
	{
		mapObject = (MapObject*)(mapList + (tileNrLowest * Offsets::offsetToAnotherTile));

		if (!mapObject) continue;

		int32_t testZ = tileNrLowest / (14 * 18);
		int32_t testY = (tileNrLowest - testZ * 14 * 18) / 18;
		int32_t testX = (tileNrLowest - testZ * 14 * 18 - testY * 18);

		if ((testZ == myZ) && (testY == readY) && (testX == readX))
		{
			return mapObject;
		}
	}
	return mapObject;
}


std::vector<Entity*> MemReader::ReadVisibleMonsters()
{
	std::vector<Entity*> entities;
	CSelfCharacter selfCharacter;
	ReadSelfCharacter(&selfCharacter);


	for (uint32_t i = 0; i < 1300; i++)
	{
		Entity* entity = (Entity*)(m_ModuleBase + Offsets::entityListStartAddy + (Offsets::offsetToAnotherEntity * i));
		if (!entity) continue;

		if ((IsEntityOnScreen(entity)) && (entity->id != selfCharacter.id) && !MemReader::GetInstance().MemReader::IsPlayer(entity->id) && !MemReader::GetInstance().MemReader::IsNPC(entity->id))
		{
			entities.push_back(entity);
		}
	}

	return entities;
}


void MemReader::SetAttackedCreature(uint32_t creatureId)
{
	*(int32_t*)(m_ModuleBase + Offsets::attackedCreature) = creatureId;
}

//void MemReader::ReadContainersForItem(int32_t itemId, Item& item)
//{
//	std::vector<InfoContainer*> containers = GetContainers(GetContainerListStartAddress(), {});
//	for (InfoContainer*& container : containers)
//	{
//		if ((!container)) continue;
//		//contNr (0), I open my backpack first so it's number is 0, and we want to loot everything that's not in main backpack into it
//		if ((container->ptrToContStruct->itemsCount <= 0)) continue;
//
//		Item itemInBp = container->ptrToContStruct->ptrToCont->findItem(itemId, container->ptrToContStruct->itemsCount, /*container->containerNumber*/ 1);
//
//		if (itemInBp.count > 0)
//		{
//			item.contNr = itemInBp.contNr;
//			item.count = itemInBp.count;
//			item.id = itemInBp.id;
//			item.slotNumber = itemInBp.slotNumber;
//			return;
//		}
//	}
//}

void MemReader::EnableXray(bool bXray)
{
	if (bXray)
	{
		mem::Patch((BYTE*)(m_XrayAddy1), (BYTE*)m_XRayPatchOpcodes, strlen(m_XRayPatchOpcodes));
		mem::Nop((BYTE*)(m_XrayAddy2), strlen(m_XRay2OriginalOpcodes));
		return;
	}
	mem::Patch((BYTE*)(m_XrayAddy1), (BYTE*)m_XRayPatchOpcodes, strlen(m_XRayPatchOpcodes));
	mem::Patch((BYTE*)(m_XrayAddy2), (BYTE*)m_XRay2OriginalOpcodes, strlen(m_XRay2OriginalOpcodes));
}

std::vector<ContainerStruct*> MemReader::GetContainers(uintptr_t node, std::vector<uintptr_t> pointers)
{
	std::vector<ContainerStruct*> containers;
	for (uint32_t i = 0; i < 16; i += 1)
	{
		ContainerStruct* container = (ContainerStruct*)(mem::FindDMAAddy2(node+(0x1EC*i), {}));
		std::string b = std::to_string(i);
		strcpy_s(container->name, b.c_str());
		if (!container) continue;
		if ((container->itemsCount <= 0)) continue;
		if (container->isOpened == 0) continue;
		
		containers.push_back(container);
	}
	return containers;
}

uintptr_t MemReader::GetContainerListStartAddress()
{
	uintptr_t newNode = mem::FindDMAAddy2(m_ModuleBase + Offsets::containerStartAddy, {});
	return newNode;
}

Item MemReader::FindFoodInContainers()
{
	std::vector<ContainerStruct*> containers = GetContainers(GetContainerListStartAddress(), {});
	std::array<int32_t, 83> m_FoodItems = { 17457, 17821, 17820, 14085, 8020, 3587, 6574, 8017, 3588, 3600, 3602, 3725, 8197, 6277, 6569, 3599, 3595, 3607, 3590, 8019, 3589,
		6543, 6544, 6545, 6542, 6541, 3598, 3597, 904, 8014, 3728, 6278, 3583, 3606, 3731, 3578, 6500, 3592, 3732, 7159, 3582, 12310, 7375, 7372, 7373, 8013, 6276,
		8018, 3604, 5096, 901, 3577, 3593, 3580, 8015, 3586, 3726, 841, 3584, 8011, 8010, 3594, 7158, 8012, 3585, 3724, 10329, 3601, 3579, 3581, 3730, 3729, 3591,
		3596, 5678, 6125, 6392, 836, 3723, 8177, 21143, 21146, 21144 };

	int32_t i = 0;
	for (ContainerStruct*& container : containers)
	{

		if (container->isOpened == 0) continue;
		if ((container->itemsCount <= 0)) continue;
		std::vector<Item> itemsInContainer = container->getAllItemsInContainer(container->itemsCount, i);
		for (Item item : itemsInContainer)
		{
			for (int32_t foodItem : m_FoodItems)
			{
				if (item.id == foodItem)
				{
					return item;
				}
			}
		}
		i++;
	}
	return Item{ 0,0,0,0 };
}

Item MemReader::ReadContainersForItem2(int32_t itemId)
{
	std::vector<ContainerStruct*> containers = GetContainers(GetContainerListStartAddress(), {});
	int32_t i = 0;
	for (ContainerStruct*& container : containers)
	{

		if (container->isOpened == 0) continue;
		if ((container->itemsCount <= 0)) continue;
		
		sscanf_s(container->name, "%ld", &i);
		std::vector<Item> itemsInContainer = container->getAllItemsInContainer(container->itemsCount, i);
		for (Item item : itemsInContainer)
		{
			if (item.id == itemId)
			{
				return item;
			}
		}
	}
	return Item{ 0,0,0,0 };

}

