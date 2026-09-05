#include "../game_shared/combat_report.h"
#include <cassert>
#include <iostream>

int main()
{
    using namespace combat_report;
    // A lethal headshot keeps overkill damage, rather than capping at 100 HP.
    Trace head;
    head.Add(1, 2352);
    auto lethal = head.Resolve(2352);
    assert(Total(lethal) == 2352 && lethal[1].hits == 1 && lethal[1].damage == 2352);

    // A shotgun hits head, chest and leg; armor changes the combined total.
    Trace shotgun;
    shotgun.Add(1, 40);
    shotgun.Add(2, 10);
    shotgun.Add(2, 10);
    shotgun.Add(6, 7.5f);
    auto armored = shotgun.Resolve(33);
    assert(Total(armored) == 33);
    assert(armored[1].hits == 1 && armored[2].hits == 2 && armored[6].hits == 1);
    assert(armored[1].damage == 20 && armored[2].damage == 9 && armored[6].damage == 4);
    // Rounding must conserve all accepted damage, including very small hits.
    for (int damage = 1; damage < 10000; ++damage)
        assert(Total(shotgun.Resolve(damage)) == damage);

    // Shield blocks and rejected damage must not manufacture hits or damage.
    Trace shield;
    shield.Add(1, 0);
    assert(shield.hits[1] == 0);
    for (const auto &part : shotgun.Resolve(0))
        assert(part.hits == 0 && part.damage == 0);

    // A subsequent direct explosion is generic, not the previous headshot.
    auto explosion = Trace{}.Resolve(57);
    assert(explosion[0].hits == 1 && explosion[0].damage == 57);
    assert(explosion[1].hits == 0);

    // Switching weapons retains separate totals and consistent body totals.
    Weapons weapons;
    Accumulate(weapons["weapon_m3"], armored);
    Accumulate(weapons["weapon_deagle"], lethal);
    Accumulate(weapons["weapon_m3"], armored);
    assert(Total(weapons["weapon_m3"]) == 66);
    assert(Total(weapons["weapon_deagle"]) == 2352);
    Parts total{};
    for (const auto &weapon : weapons) Accumulate(total, weapon.second);
    assert(Total(total) == 2418 && total[1].hits == 3 && total[2].hits == 4);

    // Each weapon row stays below the engine's legacy 192-byte message limit.
    static_assert(2 + kWeaponNameBytes + kHitGroups * 8 <= 192, "report message too large");
    std::cout << "combat report: overkill, mixed pellets, armor rounding, blocked hits, direct damage, weapon totals passed\n";
}
