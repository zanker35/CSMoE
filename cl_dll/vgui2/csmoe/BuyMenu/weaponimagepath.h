#pragma once

#include <algorithm>
#include <cctype>
#include <string>

// Resource names are independent of entity prefixes. Keep all buy-menu
// surfaces on this mapping, including favorites and the mouse-over preview.
inline std::string GetBuyMenuBasketImage(const char *weapon)
{
	std::string name = weapon ? weapon : "";
	for (const char *prefix : {"weapon_", "z4b_", "csgo_", "knife_"})
		if (name.compare(0, std::char_traits<char>::length(prefix), prefix) == 0)
			name.erase(0, std::char_traits<char>::length(prefix));
	std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
	if (name == "mp5navy") name = "mp5";
	if (name == "mp7a1c") name = "mp7a1";
	if (name == "scarl" || name == "scarh") name = "scar";
	if (name == "xm8c" || name == "xm8s") name = "xm8";
	return name.empty() ? std::string() : "gfx/vgui/basket/" + name;
}
