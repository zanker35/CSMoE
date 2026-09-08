/*
WeaponTemplate.hpp - part of CSMoE template weapon framework,
                    include all headers,
                    provide with basic meta-functions
					based on template base class, CRTP and SFINAE
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#include "game/shared/interfaces/extdll.h"
#include "game/shared/entities/util.h"
#include "game/shared/entities/cbase.h"
#include "game/shared/players/player.h"
#include "game/shared/weapons/weapons.h"
#ifndef CLIENT_DLL
#include "game/server/match/modes/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

struct CBaseTemplateWeapon : CBasePlayerWeapon
{
	using Base = CBasePlayerWeapon;
	// ...
};

#include "game/shared/weapons/templates/WeaponTemplateDetails.hpp"
#include "game/shared/weapons/templates/WeaponTemplateDataFields.hpp"
#include "game/shared/weapons/templates/WeaponDataVaribles.hpp"
#include "game/shared/weapons/templates/CheckAccuracyBoundary.hpp"
#include "game/shared/weapons/templates/GeneralData.hpp"
#include "game/shared/weapons/templates/PrecacheEvent.hpp"
#include "game/shared/weapons/templates/ItemInfo.hpp"
#include "game/shared/weapons/templates/DeployDefault.hpp"
#include "game/shared/weapons/templates/ReloadDefault.hpp"
#include "game/shared/weapons/templates/PrimaryAttackRifle.hpp"
#include "game/shared/weapons/templates/SecondaryAttackZoom.hpp"
#include "game/shared/weapons/templates/SecondaryAttackSniperZoom1.hpp"
#include "game/shared/weapons/templates/SecondaryAttackSniperZoom2.hpp"
#include "game/shared/weapons/templates/WeaponIdleDefault.hpp"
#include "game/shared/weapons/templates/FireRifle.hpp"
#include "game/shared/weapons/templates/FirePistol.hpp"
#include "game/shared/weapons/templates/RecoilKickBack.hpp"
#include "game/shared/weapons/templates/RecoilPunch.hpp"
#include "game/shared/weapons/templates/GetDamageDefault.hpp"
#include "game/shared/weapons/templates/DoubleModeType.hpp"

/*
 * template<class CFinal, class CBase>
 * concept TWeaponNode;
 */

template<class CFinal, template<class, class> class First, template<class, class> class...Args>
struct LinkWeaponTemplate : First<CFinal, LinkWeaponTemplate<CFinal, Args...>>
{
	using Base = LinkWeaponTemplate;
	using Final = CFinal;
	// ...
};

template<class CFinal, template<class, class> class First>
struct LinkWeaponTemplate<CFinal, First> : First<CFinal, CBaseTemplateWeapon>
{
	using Base = LinkWeaponTemplate;
	using Final = CFinal;
	// ...
};

// CFinal--A--B--C--D--E--...
// typename FindDownSideDerivedClass<typename B::Final, B>::type => A
// typename B::Base => C
template<class CFinal, class CCurrent, class Last=void>
struct FindDownSideDerivedClass
{
	using type = typename FindDownSideDerivedClass<typename CFinal::Base, CCurrent, CFinal>::type;
};
template<class CCurrent, class CLast>
struct FindDownSideDerivedClass<CCurrent, CCurrent, CLast>
{
	using type = CLast;
};

}