#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>


namespace Offsets
{
	
	
	static constexpr uintptr_t mapStartAddy = 0x254118;
	static constexpr uintptr_t offsetToAnotherTile = 0xA8;

	static constexpr uintptr_t entityListStartAddy = 0x23FEF8;
	static constexpr uintptr_t offsetToAnotherEntity = 0xA8;


	static constexpr uintptr_t vipEntryPoint = 0x23DBBC;
	static constexpr uintptr_t vipCount = 0x23FE1C;

	static constexpr uintptr_t attackedCreature = 0x23FE64;
	static constexpr uintptr_t followedCreature = 0x23FE60;
	static constexpr uintptr_t selfPlayer = 0x23FE98;

	static constexpr uintptr_t containerStartAddy = 0x24CD10;

	
	static constexpr uintptr_t m_Health = 0x23FE94;
	static constexpr uintptr_t m_MaxHealth = 0x23FE90;
	static constexpr uintptr_t m_Mana = 0x23FE78;
	static constexpr uintptr_t m_MaxMana = 0x23FE74;
	static constexpr uintptr_t m_Flags = 0x23FE20;
	static constexpr uintptr_t m_Id = 0x23FE98;
	static constexpr uintptr_t m_Level = 0x23FE88;
	static constexpr uintptr_t m_ConnectionState = 0x39CF28;

	static constexpr uintptr_t equipmentStructAddy = 0x24CC98;

	static constexpr uintptr_t XRayAddy1 = 0xF27F6;
	static constexpr uintptr_t XRayAddy2 = 0xF2813;

};

class Self
{
public:
	int32_t Id; //0x0000
	char pad_0004[56]; //0x0004
	int32_t zPos; //0x003C
	int32_t yPos; //0x0040
	int32_t goToX; //0x0044
	int32_t walkDir; //0x0048
}; //Size: 0x004C

class Self2
{
public:
	int32_t Id;
	int32_t xPos;
	int32_t yPos;
	int32_t zPos;
};


enum Skulls : uint8_t
{
	NoSkull = 0,
	YellowSkull,
	GreenSkull,
	WhiteSkull,
	RedSkull,
	BlackSkull,
	RevengeSkull
};

enum PartyShields : uint8_t
{
	NoParty = 0,
	InvitedBySomeone,
	InvitedByYou,
	Member = 3,
	Leader = 4,
	MemberSharedExpWorking = 5,
	LeaderSharedExpWorking = 6,
	MemberSharedExpNotWorking = 7,
	LeaderSharedExpNotWorking = 8,
	MemberSharedExpNotWorking2 = 9,
	LeaderSharedExpNotWorking2 = 10
};

enum GuildShield : uint8_t
{
	NoGuild = 0,
	InSameGuild,
	InDifferentGuild,
	SeenForEveryone,
};

struct Item
{
	Item(int32_t containerNumber, int32_t itemsCount, int32_t itemId, int32_t slotNr)
		: contNr(containerNumber), count(itemsCount), id(itemId), slotNumber(slotNr)
	{

	}

	Item()
	{

	}
	int32_t contNr = 0;
	int32_t count = 0;
	int32_t id = 0;
	int32_t slotNumber = 0;
};

class ContainerStruct
{
public:
	int32_t isOpened; //0x0000
	int32_t id; //0x0004
	char pad_0008[8]; //0x0008
	char name[16]; //0x0010
	char pad_0020[16]; //0x0020
	int32_t maxItems; //0x0030
	int32_t isChildBpOpen; //0x0034
	int32_t itemsCount; //0x0038
	int32_t itemId; //0x003C
	int32_t itemCount; //0x0040
	char pad_0044[424]; //0x0044

	//parametars are itemId, maxItems in container cause we don't want to read more than that since it can cause crash
	//returns point32_ter to array which contains [itemsCount, itemId, spotInContainer]
	Item findItem(int32_t itemIdToFind, int32_t maxItemsInContainer, int32_t contNr)
	{
		Item item;

		for (int32_t i = 0; i < maxItemsInContainer; i++)
		{
			int32_t itemIdLocal = *(int32_t*)((uintptr_t)&itemId + (0xC * i)); // 0xC is offset from one itemId to another
			if ((itemIdLocal == itemIdToFind) || (itemIdToFind == 0))
			{
				int32_t itemsCount = *(int32_t*)((uintptr_t)&itemCount + (0xC * i));
				item.count = itemsCount == 0 ? 1 : itemsCount;
				item.id = itemIdLocal;
				item.slotNumber = i;
				item.contNr = contNr;
				break;
			}
		}
		return item;
	}

	std::vector<Item> getAllItemsInContainer(int32_t itemsInContainer, int32_t contNr)
	{
		std::vector<Item> items;
		items.reserve(itemsInContainer);

		for (int32_t i = 0; i < itemsInContainer; i++)
		{
			int32_t itemIdLocal = *(int32_t*)((uintptr_t)&itemId + (0xC * i)); // 0xC is offset from one itemId to another
			int32_t itemsCount = *(int32_t*)((uintptr_t)&itemCount + (0xC * i));
			items.emplace_back(Item(contNr, itemsCount, itemIdLocal, i));
		}
		return items;
	}
}; //Size: 0x0050


class MapItem
{
public:
	uint32_t itemId; //0x0034
	uint32_t itemCount; //0x0030
};

class MapObject
{
public:
	int32_t itemsCount; //0x0000
	char pad_0004[12]; //0x0004 (orig: 44)
	MapItem* tileItem;

	std::vector<MapItem*> readTile()
	{
		std::vector<MapItem*> mapItems;
		mapItems.reserve(itemsCount);

		for (int32_t i = 0; i < itemsCount-1; i++)
		{
			MapItem* item = (MapItem*)((uintptr_t)&tileItem + (i * 0xC));

			mapItems.emplace_back(item);
		}
		/*std::string a = std::to_string(mapItems[0]->itemCount);
		MessageBoxA(NULL, a.c_str(), "asd", NULL);*/
		return mapItems;
	}
}; //Size: 0x003C

class Entity
{
public:
	int32_t id; //0x0000
	char name[32]; //0x0004
	int32_t xPos; //0x0024
	int32_t yPos; //0x0028
	int32_t zPos; //0x002C
	char pad_0030[24]; //0x0030
	int32_t speed; //0x0048
	int32_t isWalking; //0x004C
	int32_t direction; //0x0050
	int32_t walkDir; //0x0054
	char pad_0058[8]; //0x0058
	int32_t outfitId; //0x0060
	int32_t headColor; //0x0064
	int32_t primaryColor; //0x0068
	int32_t secondaryColor; //0x006C
	int32_t detailColor; //0x0070
	int32_t addon; //0x0074
	char pad_0078[16]; //0x0078
	int32_t hpPercentage; //0x0088
	char pad_008C[8]; //0x008C
	int32_t skull; //0x0094
	int32_t partyShield; //0x0098
	char pad_009C[4]; //0x009C
	int32_t guildShield; //0x00A0
	char pad_00A4[4]; //0x00A4

	bool isInParty()
	{
		if ((this->partyShield >= PartyShields::Member) && (this->partyShield < PartyShields::LeaderSharedExpNotWorking2))
		{
			return true;
		}
		return false;
	}

	bool isInGuild()
	{
		if ((this->guildShield == GuildShield::InSameGuild))
		{
			return true;
		}
		return false;
	}

	bool isWarEnemy()
	{
		return this->guildShield == GuildShield::InDifferentGuild;
	}

	bool isSkulled()
	{
		if ((this->skull >= Skulls::WhiteSkull) && (this->skull <= Skulls::BlackSkull))
		{
			return true;
		}
		return false;
	}
}; //Size: 0x00A8


class VipStruct
{
public:
	char vipName[16]; //0x0000
	char pad_0010[20]; //0x0010
	int32_t typeFlag; //0x0024
	char pad_0028[4]; //0x0028

	char* getVipName()
	{
		return vipName;
	}
}; //Size: 0x002C


enum VipFlags : uint8_t
{
	NoFlag,
	Heart,
	Skull,
	Thunder,
	AimTarget,
	Star,
	YingYang,
	GreenTriangle,
	Cross,
	MoneySign,
	GreenTarget
};


class EquipmentSlots
{
public:
	int32_t helmetId; //0x0000
	char pad_0004[8]; //0x0004
	int32_t amuletId; //0x000C
	char pad_0010[8]; //0x0010
	int32_t containerId; //0x0018
	char pad_001C[8]; //0x001C
	int32_t armorId; //0x0024
	char pad_0028[8]; //0x0028
	int32_t rightHandId; //0x0030
	int32_t rightHandCount; //0x0034
	char pad_0038[4]; //0x0038
	int32_t leftHandId; //0x003C
	int32_t leftHandCount; //0x0040
	char pad_0044[4]; //0x0044
	int32_t legsId; //0x0048
	char pad_004C[8]; //0x004C
	int32_t bootsId; //0x0054
	char pad_0058[8]; //0x0058
	int32_t ringId; //0x0060
	char pad_0064[8]; //0x0064
	int32_t arrowId; //0x006C
	int32_t arrowsCount; //0x0070
}; //Size: 0x0074



struct Spell
{
	char spell[150];
	int32_t spellLenght = 0;
	uint32_t hpPercentage = 0;
	uint32_t manaCost = 0;
};

struct LightSpell
{
	char spell[150];
	int32_t spellLenght = 0;
	uint32_t hpPercentage = 0;
	uint32_t manaCost = 0;

	LightSpell() : spell("exura"), hpPercentage(90), manaCost(25)
	{
	}
};

struct MidSpell
{
	char spell[150];
	int32_t spellLenght = 0;
	uint32_t hpPercentage = 0;
	uint32_t manaCost = 0;

	MidSpell() : spell("exura gran"), hpPercentage(80), manaCost(70)
	{
	}
};

struct HeavySpell
{
	char spell[150];
	int32_t spellLenght = 0;
	uint32_t hpPercentage = 0;
	uint32_t manaCost = 0;

	HeavySpell() : spell("exura vita"), hpPercentage(50), manaCost(160)
	{
	}
};

struct ItemHealer
{
	int32_t itemId = 0;
	int32_t manaPerc = 0;
	int32_t hpPerc = 0;
};

struct HealthPotionItemHealer
{
	int32_t itemId = 0;
	int32_t manaPerc = 0;
	int32_t hpPerc = 0;

	HealthPotionItemHealer() : itemId(266), manaPerc(0), hpPerc(50)
	{
	}
};

struct ManaPotionItemHealer
{
	int32_t itemId = 0;
	int32_t manaPerc = 0;
	int32_t hpPerc = 0;

	ManaPotionItemHealer() : itemId(268), manaPerc(50), hpPerc(0)
	{
	}
};

struct ExItem // to equip function
{
	uint32_t hpPercentage = 0;
	uint32_t manaPerc = 80;
	uint32_t Id = 0;
};

struct BalanceItemOptions
{
	const char* name;
	int32_t mode;
};

struct ComboBotSpells
{
	char leaderName[100];
	char leaderSpell[100];
	char mySpell[100];

	ComboBotSpells() : leaderName("Leader Name"), leaderSpell("Leader Spell"), mySpell("Your Spell")
	{
	}
};

struct SioUhPercent
{
	int32_t friendHpPercentage = 0;
	int32_t myHpPercentage = 0;
	SioUhPercent() : friendHpPercentage(80), myHpPercentage(90)
	{
	}
};



struct HasteSpell
{
	char spell[150];
	uint32_t manaCost = 0;
	int32_t spellLenght = 0;

	HasteSpell() : manaCost(100), spell("utani gran hur")
	{
	}
};

struct CureParalayseSpell
{
	char spell[150];
	uint32_t manaCost = 0;
	int32_t spellLenght = 0;

	CureParalayseSpell() : manaCost(100), spell("utani gran hur")
	{
	}
};



enum CHANNEL_ID : uint8_t
{
	SAY = 1,
	WHISPER = 2,
	YELL = 3,
	PRIVATE_MSG = 5,
	NPC_CHANNEL = 12,
	ADVERTISING = 7
};



class Cooldowns : public ISingleton<Cooldowns>
{
private:
	friend ISingleton;
public:
	Cooldowns() {};

	int32_t SUPPORT_SPELL = 200;
	int32_t ATTACK_SPELL = 2000; // original: 200
	int32_t HEAL_SPELL = 1000; //original: HEALSPELL = 100

	int32_t HEAL_ITEM = 500;
	int32_t ATTACK_ITEM = 500;
	int32_t DROP_ITEM = 250;
	int32_t EQUIP_ITEM = 250;

	int32_t SAY_ADVERTISING = 60500;
	int32_t COMBO_BOT = 200;

	int32_t ATTACK_CREATURE = 1000;
	int32_t PUSH_CREATURE = 900;

	int32_t BOOST_DELAY = 50; // to dashing.
	int32_t HOTKEYS_DELAY = 250;

	int32_t IDLE_TIME = 5000;
	int32_t ALARM_TIME = 500;

	int32_t BETWEEN_PACKETS = 50;
	int32_t MOUNT = 300;
	int32_t EAT_FOOD = 30000;
	int32_t WALK = 250;

	int32_t SHOOTABLE = 50;
};


enum CHARACTER_FLAGS : uint32_t
{
	IN_PROTECTION_ZONE = 16384,
	IN_COMBAT = 128,
	HASTED = 64,
	PROTECTION_ZONE_BLOCK = 8320,
	POISONED = 1,
	MANA_SHIELD = 16,
	ELECTRIFIED = 4,
	BURNING = 2,
	PARALYSED = 32,
	CURSED = 2048,
	STRENGTHEDED = 4096,
	DRUNK = 8,
	BLEEDING = 32768
};

class CSelfCharacter
{
public:
	int32_t health;
	int32_t maxHealth;
	int32_t hpPercentage;
	int32_t mana;
	int32_t maxMana;
	int32_t manaPercentage;
	int32_t flags;
	int32_t level;
	char name[32];
	int32_t xPos;
	int32_t yPos;
	int32_t zPos;
	int32_t id;
	int32_t outfitId;
	int32_t headColor;
	int32_t bodyColor;
	int32_t legsColor;
	int32_t bootsColor;
	int32_t addonNumber;
	int32_t direction;
	int32_t mountId;
	int32_t skull;
	int32_t partyShield;
	int32_t guildShield;

	bool isInCombat()
	{
		if ((flags & CHARACTER_FLAGS::IN_COMBAT) || (flags & CHARACTER_FLAGS::PROTECTION_ZONE_BLOCK))
		{
			return true;
		}
		return false;
	}

	bool isLeader()
	{
		if ((partyShield == PartyShields::Leader) || (partyShield == PartyShields::LeaderSharedExpWorking || (partyShield == PartyShields::LeaderSharedExpNotWorking) || (partyShield == PartyShields::LeaderSharedExpNotWorking2)))
		{
			return true;
		}
		return false;
	}
};

struct Position
{
	int32_t xPos;
	int32_t yPos;
	int32_t zPos;
};


enum CONNECTION_STATE : uint8_t
{
	OFFLINE = 0,
	ONLINE = 8
};

enum TURN_DIRECTION : uint32_t
{
	TURN_NORTH = 0x6F,
	TURN_EAST = 0x70,
	TURN_SOUTH = 0x71,
	TURN_WEST = 0x72
};

enum STEP_DIRECTION : uint8_t
{
	NORTH = 0x65,
	EAST = 0x66,
	SOUTH = 0x67,
	WEST = 0x68,
	NORTH_EAST = 0x6A,
	NORTH_WEST = 0x6D,
	SOUTH_WEST = 0x6C,
	SOUTH_EAST = 0x6B
};

struct KillTarget
{
	char spell[150];
	uint32_t hpPercentage = 0;
	uint32_t manaCost = 0;
	int32_t spellLenght = 0;

	KillTarget() : hpPercentage(80), manaCost(1100), spell("exevo gran mas flam")
	{
	}
};

enum EquipmentItems : uint8_t
{
	Helmet = 1,
	Amulet = 2,
	Armor = 4,
	Left_hand = 6,
	Right_hand = 5,
	Backpack = 3,
	Ring = 9,
	Arrow = 10,
	Legs = 7,
	Boots = 8
};
