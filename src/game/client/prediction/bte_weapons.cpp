
#include "game/shared/interfaces/extdll.h"
#include "game/shared/entities/util.h"
#include "game/shared/entities/cbase.h"
#include "game/shared/players/player.h"

#include "game/client/prediction/com_weapons.h"

#include "game/client/runtime/parsemsg.h"

#include "game/client/prediction/bte_weapons.h"
#include "game/shared/weapons/weapons_msg.h"

#include "game/client/prediction/wpn_shared.h"

#include <string>
#include <map>

#include "game/shared/entities/cbase_typelist.h"

/*
	Weapon Registers
*/
namespace cl {

using WeaponEntityFindList_t = std::map<std::string, EntityMetaData>;
	
namespace detail
{
	template<class...MdTypes>
	WeaponEntityFindList_t WeaponEntityFindList_CreateImpl(const MdTypes &...mds)
	{
		return { { mds.ClassName, mds }... };
	}
	
	template<class...Types>
	WeaponEntityFindList_t WeaponEntityFindList_Create(TypeList<Types...>)
	{
		return WeaponEntityFindList_CreateImpl(GetEntityMetaDataFor(type_identity<Types>())...);
	}
}

static const WeaponEntityFindList_t &WeaponEntityFindList()
{
	static const auto singleton = detail::WeaponEntityFindList_Create(AllEntityTypeList());
	return singleton;
}

/*
	Entity Pools
*/
extern CBasePlayerWeapon *g_pWpns[MAX_WEAPONS]; // cs_weapons.cpp

/*
	CBTEClientWeapons impls
*/

void CBTEClientWeapons::PrepEntity(CBasePlayer *pWeaponOwner)
{
	for (auto &kv : WeaponEntityFindList())
	{
		CBasePlayerWeapon *pEntity = kv.second.PlaceHolderEntity;

		if (pWeaponOwner)
		{
			((CBasePlayerWeapon *)pEntity)->m_pPlayer = pWeaponOwner;
		}
	}
	
}

void CBTEClientWeapons::ActiveWeapon(const char *name)
{
	m_pActiveWeapon = nullptr;

	auto iter = WeaponEntityFindList().find({ name });
	if (iter != WeaponEntityFindList().end())
	{
		m_pActiveWeapon = iter->second.PlaceHolderEntity;

		ItemInfo info;
		memset(&info, 0, sizeof(ItemInfo));
		m_pActiveWeapon->GetItemInfo(&info);
		g_pWpns[info.iId] = m_pActiveWeapon;
	}
	else
	{
		gEngfuncs.Con_Printf("BTE Weapon Client Predict: Unknown Weapon %s is active.\n", name);
	}
	
}

/*
	Message Handlers
*/

typedef void MsgDispatchFunction_t(BufferReader &reader);

void MsgDispatch_Active(BufferReader &reader)
{
	const char *name = reader.ReadString();
	BTEClientWeapons().ActiveWeapon(name);
}

MsgDispatchFunction_t *gMsgDispatchFunctions[BTE_Weapon_MaxMsgs] = {
	MsgDispatch_Active
};

int __MsgFunc_BTEWeapon(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader(pszName, pbuf, iSize);

	BTEWeaponMsgType type = static_cast<BTEWeaponMsgType>(reader.ReadByte());
	gMsgDispatchFunctions[type](reader);
	return 1;
}


void CBTEClientWeapons::Init()
{
	gEngfuncs.pfnHookUserMsg("BTEWeapon", __MsgFunc_BTEWeapon);
}

CBTEClientWeapons::CBTEClientWeapons() : m_pActiveWeapon(nullptr)
{
	
}

CBTEClientWeapons &BTEClientWeapons()
{
	static CBTEClientWeapons x;
	return x;
}

void InitializeWeaponEntity(CBasePlayerWeapon *pEntity, entvars_t *pev)
{
	pEntity->pev = pev;
	pEntity->Precache();
	pEntity->Spawn();
}

}