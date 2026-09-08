#include "game/shared/interfaces/extdll.h"
#include "game/shared/entities/util.h"
#include "game/shared/entities/cbase.h"
#include "game/shared/players/player.h"
#include "game/server/players/client.h"

#include "game/server/match/modes/mods.h"

#include <algorithm>

namespace sv {

void CBasePlayer::AddAccount(int amount, bool bTrackChange)
{
	m_iAccount += amount;
}

CPlayerAccount &CPlayerAccount::operator+=(int delta)
{
	const int iMax = g_pModRunning->MaxMoney();
	m_iAmount = std::min(std::max(0, m_iAmount + delta), iMax);

	return *this;
}

void CPlayerAccount::UpdateHUD(CBasePlayer *player, bool bTrackChange) const
{
	if (!bTrackChange || m_iAmount != m_iLastAmount)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMoney, NULL, player->pev);
		WRITE_LONG(m_iAmount);
		WRITE_BYTE(bTrackChange);
		MESSAGE_END();

		m_iLastAmount = m_iAmount;
	}
}

}
