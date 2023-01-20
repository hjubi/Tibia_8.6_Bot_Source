#pragma once

class Healer : public ISingleton<Healer>
{
private:

public:
	Healer();
	int32_t m_TimerHealerSpells = 0;
	int32_t m_TimerHealerItems = 0;
	int32_t m_TimerHealFriend = 0;
	int32_t m_TimerAutoHaste = 0;
	int32_t m_TimerAntiParal = 0;
	int32_t m_TimerEquipItem = 0;

	void HealWithSpells(const LightSpell& lightSpell, const MidSpell& midSpell, const HeavySpell& heavySpell);
	void HealWithItems(const HealthPotionItemHealer& healthPotion, const ManaPotionItemHealer& manaPotion);
	void HealPlayer(int32_t safeHp, int32_t healFriendOnHp, std::array<std::string, 100>& friendList, bool sioWithSpell, int32_t selectedMode);
	void Haste(const HasteSpell& hasteSpell);
	void CureParalyze(const CureParalayseSpell& paralSpell);

	void EquipAmuletBalancer(bool bEquipModeHotkey, bool bEquipModeMoveItem, const ExItem& toEquipIfLow, const ExItem& toSwitchWhenSafe, const ExItem& toDequipWhenSafe);
};
