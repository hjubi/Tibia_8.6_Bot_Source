#pragma once
class AutoAttack : public ISingleton<AutoAttack>
{
private:
	int32_t m_TimerAutoAttack;
	int32_t m_TimerAttackSpell;
	int32_t m_TimerHoldTarget;
	int32_t m_lockedTargetId;

	int32_t GetEntityId();
public:
	AutoAttack();
	void UseAOEspell(int32_t numberOfMobs, int32_t relativeDistance, const char* spell, int32_t manaCost, bool safeMode); //area of effect spell
	void HoldTarget();
	void Enable();
};