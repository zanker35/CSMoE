#pragma once

#include <array>
#include <map>
#include <string>

namespace combat_report {

// GoldSrc hit groups: generic, head, chest, stomach, left/right arm, left/right leg.
constexpr int kHitGroups = 8;
constexpr int kWeaponNameBytes = 64;
constexpr int kPlayerNameBytes = 64;
enum Message { Begin = 0, Weapon = 1 };

struct Part
{
    int hits = 0;
    int damage = 0;
};
using Parts = std::array<Part, kHitGroups>;
using Weapons = std::map<std::string, Parts>;

inline int Total(const Parts &parts)
{
    int result = 0;
    for (const auto &part : parts)
        result += part.damage;
    return result;
}

struct Trace
{
    std::array<float, kHitGroups> damage{};
    std::array<int, kHitGroups> hits{};

    void Add(int group, float amount)
    {
        if (amount <= 0)
            return;
        if (group < 0 || group >= kHitGroups)
            group = 0;
        damage[group] += amount;
        ++hits[group];
    }

    Parts Resolve(int acceptedDamage) const
    {
        Parts result{};
        if (acceptedDamage <= 0)
            return result;
        double rawTotal = 0;
        for (float amount : damage)
            rawTotal += amount;
        if (rawTotal <= 0)
        {
            // Explosions and other direct damage have no traced body part.
            result[0] = {1, acceptedDamage};
            return result;
        }

        // The engine applies armor/mode modifiers to the combined shot. Apportion
        // that final integer damage across its traced parts without losing points.
        double cumulative = 0;
        int assigned = 0;
        for (int i = 0; i < kHitGroups; ++i)
        {
            cumulative += damage[i];
            const int throughHere = static_cast<int>(acceptedDamage * cumulative / rawTotal + 0.5);
            result[i] = {hits[i], throughHere - assigned};
            assigned = throughHere;
        }
        return result;
    }
};

inline void Accumulate(Parts &total, const Parts &hit)
{
    for (int i = 0; i < kHitGroups; ++i)
    {
        total[i].hits += hit[i].hits;
        total[i].damage += hit[i].damage;
    }
}

} // namespace combat_report
