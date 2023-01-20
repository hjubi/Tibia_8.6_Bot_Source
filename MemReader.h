#pragma once

typedef int(__stdcall* GetObjectProperty)(int itemId, int flag);

class MemReader : public ISingleton<MemReader>
{
private:

	
	EquipmentSlots* m_EquipmentSlots;
	const char* m_XRayPatchOpcodes = "\xEB\x13";
	const char* m_XRayOriginalOpcodes = "\x75\x13";
	const char* m_XRay2OriginalOpcodes = "\x75\x40";

public:
	Self* m_LocalPlayer;
	GetObjectProperty ItemFlagInfo;
	int32_t m_SelfCellNr;
	void ReadSelfMapCell();
	MapObject* ReadMapTile(int32_t relativeX, int32_t relativeY);
	uintptr_t m_XrayAddy1;
	uintptr_t m_XrayAddy2;

	MemReader();

	void ReadSelfCharacter(CSelfCharacter* character);
	std::vector<Entity*> ReadVisibleCreatures();
	std::vector<Entity*> ReadVisibleMonsters();
	Item FindFoodInContainers();
	Item ReadContainersForItem2(int32_t itemId);
	std::vector<ContainerStruct*> GetContainers(uintptr_t node, std::vector<uintptr_t> pointers);

	//std::vector<SContainerStruct> GetContainers(uintptr_t node, std::vector<uintptr_t> pointers);
	
	std::vector<VipStruct*> GetVips();
	EquipmentSlots* GetItemOnEquipmentSlots();
	bool IsFlagTrue(uint32_t flag);
	int32_t GetVipCount();
	Entity* GetEntityInEntityList(uint32_t characterId);
	uintptr_t GetContainerListStartAddress();


	std::vector<Entity*> GetEntityById(uint32_t creatureId);



	bool IsOnline();
	bool IsMonster(int32_t creatureId);
	bool IsPlayer(int32_t creatureId);
	bool IsNPC(int32_t creatureId);
	bool IsEntityOnScreen(Entity* ent);
	bool IsShootable(const Position& toPosition, const Position& myPosition);
	void SetAttackedCreature(uint32_t creatureId);
	void EnableXray(bool bXray);
	int32_t GetAttackedCreature();
};
