
#ifndef WEAPON_MODEL_H
#define WEAPON_MODEL_H

namespace sv {

const char *GetCSModelName(WeaponIdType item_id);
float GetArmorRatioModifier(WeaponIdType iGunType);
WeaponBuyAmmoConfig GetBuyAmmoConfig(WeaponIdType id);

}

#endif
