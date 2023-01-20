#pragma once

//types of functions
using FnPacketLPCSTR = void(__cdecl*)(const char* msg);
using FnPacketInt = void(__cdecl*)(int32_t num);
using FnPacketByte = void(__cdecl*)(BYTE num);
using FnPacketIntRetInt = int(__cdecl*)(int32_t num);

class PacketSend : public ISingleton<PacketSend>
{
private:
	uintptr_t moduleBase;
public:
	//Function Pointers
	FnPacketLPCSTR PacketMsg = nullptr;

	FnPacketInt PacketStart = nullptr;
	FnPacketInt PacketItem = nullptr;
	FnPacketInt PacketContainer = nullptr;
	FnPacketInt PacketChannelId = nullptr;
	FnPacketInt PacketPlayerId = nullptr;
	FnPacketLPCSTR PrintWhiteText = nullptr;

	
	FnPacketByte PacketEnd = nullptr;


	PacketSend();

	void Say(const char* msgToSay);
	void UseItemOn(int32_t itemId, uint32_t creatureId);
	void UseItemWithFromContainerToCreature(uint16_t itemId, uint32_t creatureId, BYTE slotNumber, int32_t toXPos, int32_t toYPos, int32_t toZPos);
	void SayOnAdvertisting(const char* msgToSay);
	void Turn(uint8_t direction);
	void Attack(int32_t creatureId);
	void Step(uint8_t direction);
	void UseItemInContainer(int32_t inContainerNumber, int32_t onSlotNumber, int32_t itemId);
	void MoveItemFromContainerToPosition(int32_t itemId, int32_t fromSlotNumber, int32_t toXPos, int32_t toYPos, int32_t toZPos, int32_t contNr);
	void MoveItemFromPositionToContainer(int32_t itemId, int32_t fromSlotNumber, int32_t toXPos, int32_t toYPos, int32_t toZPos);
};
