#include "game/client/hud/hud.h"
#include "game/client/menus/CBaseViewport.h"
#include "game/shared/interfaces/cdll_dll.h"
#include "game/client/runtime/cl_util.h"
#include "game/client/menus/csmoe/BuyMenu/cstrikebuymenu.h"
#include "game/client/menus/csmoe/BuyMenu/cstrikebuysubmenu.h"
#include "game/shared/strings/shared_util.h"
#include "ui/vgui2/interfaces/vgui/ISurface.h"
#include "ui/vgui2/interfaces/vgui/ILocalize.h"
#include "ui/vgui2/vgui_controls/RichText.h"
#include "ui/vgui2/vgui_controls/CheckButton.h"
#include "ui/vgui2/vgui_controls/TextEntry.h"
#include "ui/vgui2/vgui_controls/MessageBox.h"
#include "game/client/menus/csmoe/BuyMenu/buymouseoverpanelbutton.h"
#include "ui/vgui2/cso_controls/ButtonGlow.h"
#include "ui/vgui2/cso_controls/DarkTextEntry.h"

#include "base/json.hpp"

#include <string>

using namespace vgui2;

const Color COL_NONE = { 255, 255, 255, 255 };
const Color COL_CT = { 192, 205, 224, 255 };
const Color COL_TR = { 216, 182, 183, 255 };

static const char *EQUIPMENT_BUYLIST[] = { "vest","vesthelm","flash","hegrenade","sgren","defuser","nvgs" };
static const char *EQUIPMENT_BUYLIST_TEXT[] = { "#Cstrike_Kevlar","#Cstrike_Kevlar_Helmet","#Cstrike_Flashbang","#Cstrike_HE_Grenade","#Cstrike_Smoke_Grenade","#Cstrike_Defuser","#Cstrike_NightVision_Button_CT" };
static const char *EQUIPMENT_BUYLIST_CMD[] = { "vest","vesthelm","flash","VGUI_BuyMenu_BuyWeapon weapon_hegrenade","sgren","defuser","nvgs" };

CCSBuySubMenu::CCSBuySubMenu(vgui2::Panel *parent, const char *name) : CBuySubMenu(parent, name)
{
	m_pTitleLabel = new vgui2::Label(this, "CaptionLabel", "#CSO_WeaponSelectMenu");

	char buffer[64];

	m_pShowCTWeapon = new NewTabButton(this, "ShowCTWeapon", "#CSO_BuyShowCT");
	m_pShowTERWeapon = new NewTabButton(this, "ShowTERWeapon", "#CSO_BuyShowTER");
	m_pShowCTWeapon->SetTextColor(COL_CT);
	m_pShowTERWeapon->SetTextColor(COL_TR);

	for (int i = 0; i < 10; i++)
	{
		sprintf(buffer, "slot%d", i);
		m_pSlotButtons[i] = new CSBuyMouseOverPanelButton(this, buffer, m_pPanel);
		m_pSlotButtons[i]->GetClassPanel()->SetName("ItemInfo");
	}
	m_pPrevBtn = new vgui2::Button(this, "prevBtn", "#CSO_PrevBuy");
	m_pNextBtn = new vgui2::Button(this, "nextBtn", "#CSO_NextBuy");

	m_pRebuyButton = new ButtonGlow(this, "RebuyButton", "#Cstrike_BuyMenuRebuy");
	m_pAutobuyButton = new ButtonGlow(this, "AutobuyButton", "#Cstrike_BuyMenuAutobuy");

	m_pBasketClear = new vgui2::Button(this, "BasketClear", "#CSO_BasketClear");
	m_pBasketBuy = new ButtonGlow(this, "BasketBuy", "#CSO_BasketBuy");
	m_pQuitButton = new vgui2::Button(this, "QuitButton", "#Cstrike_Cancel");

	primaryBG = new vgui2::ImagePanel(this, "primaryBG");
	secondaryBG = new vgui2::ImagePanel(this, "secondaryBG");
	knifeBG = new vgui2::ImagePanel(this, "knifeBG");
	grenadeBG = new vgui2::ImagePanel(this, "grenadeBG");
	equipBG = new vgui2::ImagePanel(this, "equipBG");

	pwpnBG = new WeaponImagePanel(this, "pwpnBG");
	pammoBG = new vgui2::ImagePanel(this, "pammoBG");
	swpnBG = new WeaponImagePanel(this, "swpnBG");
	sammoBG = new vgui2::ImagePanel(this, "sammoBG");
	hgrenBG = new WeaponImagePanel(this, "hgrenBG");
	sgrenBG = new vgui2::ImagePanel(this, "sgrenBG");
	fgrenBG = new vgui2::ImagePanel(this, "fgrenBG");
	fgren2BG = new vgui2::ImagePanel(this, "fgren2BG");
	dfBG = new vgui2::ImagePanel(this, "dfBG");
	nvBG = new vgui2::ImagePanel(this, "nvBG");
	kevBG = new vgui2::ImagePanel(this, "kevBG");
	newknifeBG = new WeaponImagePanel(this, "newknifeBG");

	for (int i = 0; i < 5; i++)
	{
		sprintf(buffer, "fav%d", i);
		m_pFavButtons[i] = new BuyPresetButton(this, buffer);
		sprintf(buffer, "fav_save%d", i);
		m_pFavSaveButtons[i] = new vgui2::Button(this, buffer, "#CSO_FavSave");
	}
	m_pFavDirectBuy = new vgui2::CheckButton(this, "fav_direct_buy_ckbtn", "#CSO_FavDirectBuy");

	account_num = new vgui2::Label(this, "account_num", "");
	buytime_num = new vgui2::Label(this, "buytime_num", "");
	moneyText = new vgui2::Label(this, "moneyText", "");

	account = new DarkTextEntry(this, "account");
	buytime = new DarkTextEntry(this, "buytime");
	moneyBack = new DarkTextEntry(this, "moneyBack");
	freezetime = new DarkTextEntry(this, "freezetime");


	m_pSetSelBg = new vgui2::ImagePanel(this, "SetSelBg");
	m_pSetLabel = new vgui2::Label(this, "SetLabel", "X Set");
	m_pPrevSetBtn = new vgui2::Button(this, "PrevSetBtn", "");
	m_pNextSetBtn = new vgui2::Button(this, "NextSetBtn", "");

	m_pEditDescLabel_DM = new vgui2::Label(this, "EditDescLabel_DM", "#CSO_BuySubMenu_EditDesc");
	m_pEditDescBg = new vgui2::ImagePanel(this, "EditDescBg");
	m_pEquipSample = new vgui2::ImagePanel(this, "EquipSample");

	//m_iniFavorite.OpenFile("quickbuy.ini");
	ReadFavoriteSets();

	SetupItems(BUY_NONE);
}

void CCSBuySubMenu::OnCommand(const char *command)
{
	if (!command)
		return;
	constexpr size_t showLength = sizeof("VGUI_BuyMenu_Show") - 1;
	constexpr size_t teamLength = sizeof("VGUI_BuyMenu_SetTeam") - 1;
	constexpr size_t weaponLength = sizeof("VGUI_BuyMenu_BuyWeapon ") - 1;
	if (!strncmp(command, "VGUI_BuyMenu_Show", showLength) && (command[showLength] == '\0' || command[showLength] == ' '))
	{
		int type = BUY_NONE;
		if (command[showLength] != '\0')
			sscanf(command + showLength, "%d", &type);
		if (type < BUY_NONE || type > BUY_KNIFE)
			type = BUY_NONE;
		SetupItems(static_cast<MoEWeaponBuyType>(type));
		
		GotoNextSubPanel();
		return;
	}
	else if (!strncmp(command, "VGUI_BuyMenu_SetTeam", teamLength) && (command[teamLength] == '\0' || command[teamLength] == ' '))
	{
		int team = UNASSIGNED;
		if (command[teamLength] != '\0')
			sscanf(command + teamLength, "%d", &team);
		SetTeam(team == CT ? CT : team == TERRORIST ? TERRORIST : UNASSIGNED);

		GotoNextSubPanel();
		return;
	}
	else if (strlen(command) == 4 && !strncmp(command, "fav", 3) && command[3] >= '0' && command[3] <= '4')
	{
		int n = command[3] - '0';
		OnSelectFavoriteWeapons(n);
		return;
	}
	else if (strlen(command) == 7 && !strncmp(command, "favSav", 6) && command[6] >= '0' && command[6] <= '4')
	{
		int n = command[6] - '0';
		OnSaveFavoriteWeapons(n);
		return;
	}
	else if (!strncmp(command, "VGUI_BuyMenu_BuyWeapon ", weaponLength))
	{
		OnSelectWeapon(command + weaponLength);
		return;
	}
	else if (!Q_strcmp(command, "showctwpn"))
	{
		SetTeam(CT);
		SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "showterwpn"))
	{
		SetTeam(TERRORIST);
		SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "buy_unavailable"))
	{
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "autobuy_in"))
	{
		if (m_iTeam == CT)
			gEngfuncs.pfnClientCmd("m4a1;usp;hegrenade;moe_buy weapon_knife;secammo;primammo;vesthelm;defuser");
		else
            gEngfuncs.pfnClientCmd("ak47;glock;hegrenade;moe_buy weapon_knife;secammo;primammo;vesthelm");
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "rebuy_in"))
	{
		gHUD.m_Ammo.UserCmd_Rebuy();
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "basketbuy"))
	{
		OnBuySelectedItems();
		BaseClass::OnCommand("vguicancel");
		return;
	}
	else if (!Q_strcmp(command, "prevpage"))
	{
		SetupPage(m_iCurrentPage > 0 ? m_iCurrentPage - 1 : 0);
		return;
	}
	else if (!Q_strcmp(command, "nextpage"))
	{
		SetupPage(m_iCurrentPage + 1);
		return;
	}
	else if (!Q_strcmp(command, "basketclear"))
	{
		OnClearSelectedItems();
		return;
	}

	BaseClass::OnCommand(command);
}

void CCSBuySubMenu::PerformLayout()
{
	// fix these not in .res
	int  w, h;
	GetSize(w, h);
	int w2, h2;
	m_pTitleLabel->GetSize(w2, h2);
	m_pTitleLabel->SetPos(w / 2 - w2 / 2, vgui2::scheme()->GetProportionalScaledValue(12));
	
	m_pShowTERWeapon->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(12), 
		vgui2::scheme()->GetProportionalScaledValue(32), 
		vgui2::scheme()->GetProportionalScaledValue(98), 
		vgui2::scheme()->GetProportionalScaledValue(16)
	);
	m_pShowCTWeapon->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(108), 
		vgui2::scheme()->GetProportionalScaledValue(32), 
		vgui2::scheme()->GetProportionalScaledValue(98), 
		vgui2::scheme()->GetProportionalScaledValue(16)
	);
	newknifeBG->SetBounds(
		vgui2::scheme()->GetProportionalScaledValue(505), 
		vgui2::scheme()->GetProportionalScaledValue(327), 
		vgui2::scheme()->GetProportionalScaledValue(75), 
		vgui2::scheme()->GetProportionalScaledValue(50)
	);

	for (int i = 0; i < 10; ++i)
	{
		m_pSlotButtons[i]->GetClassPanel()->SetBounds(
			vgui2::scheme()->GetProportionalScaledValue(216),
			vgui2::scheme()->GetProportionalScaledValue(60),
			vgui2::scheme()->GetProportionalScaledValue(152),
			vgui2::scheme()->GetProportionalScaledValue(145)
		); 
	}

	BaseClass::PerformLayout();
}

void CCSBuySubMenu::OnThink()
{
	BaseClass::OnThink();

	account_num->SetText(std::to_wstring(gHUD.m_Money.GetMoney()).c_str());
	buytime_num->SetText("-");
}

void CCSBuySubMenu::SetupItems(MoEWeaponBuyType type)
{
	m_BuyItemList.clear();
	if (type == BUY_NONE)
	{
		m_iCurrentPage = 0;
		static const char szTitles[10][32] = { "#Cstrike_Pistols", "#Cstrike_Shotguns", "#Cstrike_SubMachineGuns", "#Cstrike_Rifles",
			"#Cstrike_MachineGuns", "#Cstrike_Prim_Ammo", "#Cstrike_Sec_Ammo", "#Cstrike_Equipment",
			"&9 近身武器", "#Cstrike_Cancel" };
		static const char szCommands[10][32] = { "VGUI_BuyMenu_Show 0", "VGUI_BuyMenu_Show 1", "VGUI_BuyMenu_Show 2", "VGUI_BuyMenu_Show 3",
			"VGUI_BuyMenu_Show 4", "primammo", "secammo", "VGUI_BuyMenu_Show 5",
			"VGUI_BuyMenu_Show 6", "vguicancel" };

		for (int i = 0; i < 10; ++i)
		{
			m_pSlotButtons[i]->SetEnabled(true);
			m_pSlotButtons[i]->SetText(szTitles[i]);
			m_pSlotButtons[i]->SetCommand(szCommands[i]);
			m_pSlotButtons[i]->SetHotkey('0' + i + 1);
			m_pSlotButtons[i]->SetVisible(true);
			m_pSlotButtons[i]->UpdateWeapon("");
		}
		m_pSlotButtons[9]->SetHotkey('0');
		m_pPrevBtn->SetVisible(false);
		m_pNextBtn->SetVisible(false);
		return;
	}
	else
	{
		if (type == BUY_EQUIP)
		{
			for (int i = 0; i < 7; ++i)
			{
				if (i == 5 && (m_iTeam != CT || gHUD.m_FollowIcon.m_iBombTargetsNum == 0))
					continue;
				m_BuyItemList.push_back({ EQUIPMENT_BUYLIST[i], EQUIPMENT_BUYLIST_TEXT[i], EQUIPMENT_BUYLIST_CMD[i] });
			}
		}

		for (const auto &x : GetBuyMenuWeapons())
		{
			if (type != x.iMenu)
				continue;
			if (x.team != UNASSIGNED && m_iTeam != x.team)
				continue;

			const char *name = x.pszClassName;

			if (type == BUY_EQUIP)
			{
				if (!strcmp(name, "weapon_flashbang") || !strcmp(name, "weapon_hegrenade") || !strcmp(name, "weapon_smokegrenade"))
					continue;
			}

			m_BuyItemList.push_back(ItemInfo{ name, x.pszDisplayName, std::string("VGUI_BuyMenu_BuyWeapon ") + name });
		}

		m_pPrevBtn->SetHotkey(L'-');
		m_pNextBtn->SetHotkey(L'=');
		SetupPage(0);
	}
}

void CCSBuySubMenu::SetupPage(size_t iPage)
{
	const size_t totalpages = (m_BuyItemList.size() + 8) / 9;
	iPage = totalpages ? std::min(iPage, totalpages - 1) : 0;

	m_iCurrentPage = iPage;
	
	// page buttons
	m_pPrevBtn->SetVisible(iPage != 0);
	m_pNextBtn->SetVisible(iPage + 1 < totalpages);
	
	for (int i = 0; i < 9; ++i)
	{
		const size_t iElement = m_iCurrentPage * 9 + i;
		if (iElement >= m_BuyItemList.size())
		{
			m_pSlotButtons[i]->SetText("");
			m_pSlotButtons[i]->SetCommand("");
			m_pSlotButtons[i]->SetVisible(false);
			m_pSlotButtons[i]->SetHotkey('\0');
			m_pSlotButtons[i]->UpdateWeapon("");
		}
		else
		{
			m_pSlotButtons[i]->SetEnabled(true);
			

			const char *weapon = m_BuyItemList[iElement].name.c_str();
			const char *showname = m_BuyItemList[iElement].showname.c_str();
			const char *cmd = m_BuyItemList[iElement].command.c_str();
			m_pSlotButtons[i]->SetText(showname);
			m_pSlotButtons[i]->SetCommand(cmd);
			m_pSlotButtons[i]->SetVisible(true);
			m_pSlotButtons[i]->SetHotkey('0' + i + 1);
			m_pSlotButtons[i]->UpdateWeapon(weapon);
		}
	}
	
	m_pSlotButtons[9]->SetEnabled(true);
	m_pSlotButtons[9]->SetText("#CSO_PrevWpnBuy");
	m_pSlotButtons[9]->SetCommand("VGUI_BuyMenu_Show");
	m_pSlotButtons[9]->SetHotkey('0');
	m_pSlotButtons[9]->SetVisible(true);
	m_pSlotButtons[9]->UpdateWeapon("");
	


}

void CCSBuySubMenu::SetTeam(TeamName team)
{
	m_iTeam = team;

	for (int i = 0; i < 10; ++i)
	{
		m_pSlotButtons[i]->SetTeam(team);
	}
	
	m_pShowCTWeapon->SetActive(false);
	m_pShowTERWeapon->SetActive(false);
	if (team== CT)
		m_pShowCTWeapon->SetActive(true);
	if (team == TERRORIST)
		m_pShowTERWeapon->SetActive(true);

	Color col = { 255,255,255,255 };
	Color bg = { 255,255,255,255 };
	if (m_iTeam == TERRORIST)
	{
		col = COL_TR;
	}
	else if (m_iTeam == CT)
	{
		col = COL_CT;
	}

}

void CCSBuySubMenu::OnSelectWeapon(const char *weapon)
{
    const auto *info = FindBuyMenuWeapon(weapon);
	if (!info)
        return;
    
    switch (info->iSlot)
	{
	case PRIMARY_WEAPON_SLOT: m_SelectedItems.Primary = weapon; break;
	case PISTOL_SLOT: m_SelectedItems.Secondary = weapon; break;
	case KNIFE_SLOT: m_SelectedItems.Melee = weapon; break;
	case GRENADE_SLOT: m_SelectedItems.HEGrenade = weapon; break;
	default: return;
	}
	SetupItems(BUY_NONE);
	UpdateFavoriteSetsControls();
	SaveFavoriteSets();
}

void CCSBuySubMenu::ReadFavoriteSets()
{
	for (size_t i = 0; i < std::size(m_FavoriteItems); ++i)
	{
		m_FavoriteItems[i] = {
			g_BuyMenuDefaultFavorites[i][0], g_BuyMenuDefaultFavorites[i][1],
			"weapon_knife", "weapon_hegrenade"
		};
	}
	// Reading never writes the defaults. An existing QuickBuy slot replaces the
	// entire preset below, so deliberately saved empty slots stay empty.
    std::string content;
    FileHandle_t fh = vgui2::filesystem()->Open("moe_buy.json", "rb", "CONFIG");
    if (fh != FILESYSTEM_INVALID_HANDLE)
    {
        content.resize(vgui2::filesystem()->Size(fh));
        if (!content.empty())
            content.resize(std::max(0, vgui2::filesystem()->Read(&content[0], content.size(), fh)));
        vgui2::filesystem()->Close(fh);
    }

    try {
        if (!content.empty())
        {
            const auto j = nlohmann::json::parse(content);
            const auto readSet = [](const nlohmann::json &value) {
                FavoriteSet set;
                const auto readWeapon = [&value](const char *key, InventorySlotType slot) {
                    const auto name = value.value(key, std::string());
                    const auto *weapon = FindBuyMenuWeapon(name.c_str());
                    return weapon && weapon->iSlot == slot ? name : std::string();
                };
                set.Primary = readWeapon("Primary", PRIMARY_WEAPON_SLOT);
                set.Secondary = readWeapon("Secondary", PISTOL_SLOT);
                set.Melee = readWeapon("Knife", KNIFE_SLOT);
                set.HEGrenade = readWeapon("Grenade", GRENADE_SLOT);
                set.nFlashBang = std::clamp(value.value("FlashBang", 0), 0, 2);
                set.nSmokeGrenade = std::clamp(value.value("SmokeGrenade", 0), 0, 1);
                set.bDefuser = value.value("Defuser", 0) != 0;
                set.bNightVision = value.value("NightVision", 0) != 0;
                set.iKelmet = static_cast<ArmorType>(std::clamp(value.value("Armor", 2), 0, 2));
                return set;
            };

            if (j.count("QuickBuy0"))
                m_SelectedItems = readSet(j.at("QuickBuy0"));
            for (int i = 0; i < 5; ++i)
            {
                const auto key = "QuickBuy" + std::to_string(i + 1);
                if (j.count(key))
                    m_FavoriteItems[i] = readSet(j.at(key));
            }
        }
    } catch(const std::exception &e)
    {
        gEngfuncs.Con_DPrintf("Unable to read moe_buy.json: %s\n", e.what());
    }
	UpdateFavoriteSetsControls(); // update images
}

void CCSBuySubMenu::SaveFavoriteSets()
{
    std::string content;
    try {
        nlohmann::json j;
        const auto writeSet = [](const FavoriteSet &set) {
            return nlohmann::json {
                {"Primary", set.Primary}, {"Secondary", set.Secondary},
                {"Knife", set.Melee}, {"Grenade", set.HEGrenade},
                {"FlashBang", set.nFlashBang}, {"SmokeGrenade", set.nSmokeGrenade},
                {"Defuser", set.bDefuser}, {"NightVision", set.bNightVision},
                {"Armor", static_cast<int>(set.iKelmet)}
            };
        };
        for (int i = 0; i < 5; ++i)
        {
            j["QuickBuy" + std::to_string(i + 1)] = writeSet(m_FavoriteItems[i]);
        }
        j["QuickBuy0"] = writeSet(m_SelectedItems);
        content = j.dump();
    } catch(const std::exception &e)
    {
        gEngfuncs.Con_DPrintf("Unable to save moe_buy.json: %s\n", e.what());
        return;
    }

    FileHandle_t fh = vgui2::filesystem()->Open("moe_buy.json", "wb", "CONFIG");
    if (fh != FILESYSTEM_INVALID_HANDLE)
    {
        vgui2::filesystem()->Write(content.c_str(), content.length(), fh);
        vgui2::filesystem()->Close(fh);
    }
	UpdateFavoriteSetsControls(); // update images
}

void CCSBuySubMenu::UpdateFavoriteSetsControls()
{
	for (int i = 0; i < 5; ++i)
	{
		m_pFavButtons[i]->SetPrimaryWeapon(m_FavoriteItems[i].Primary.c_str());
		m_pFavButtons[i]->SetSecondaryWeapon(m_FavoriteItems[i].Secondary.c_str());
		m_pFavButtons[i]->SetKnifeWeapon(m_FavoriteItems[i].Melee.c_str());
	}

	pwpnBG->SetWeapon(m_SelectedItems.Primary.c_str());
	swpnBG->SetWeapon(m_SelectedItems.Secondary.c_str());
	hgrenBG->SetWeapon(m_SelectedItems.HEGrenade.c_str());
	newknifeBG->SetWeapon(m_SelectedItems.Melee.c_str());

	

	fgrenBG->SetImage(m_SelectedItems.nFlashBang ? "gfx/vgui/basket/flash" : "");
	fgren2BG->SetImage(m_SelectedItems.nFlashBang > 1 ? "gfx/vgui/basket/flash" : "");
	sgrenBG->SetImage(m_SelectedItems.nSmokeGrenade ? "gfx/vgui/basket/sgren" : "");
	dfBG->SetImage(m_SelectedItems.bDefuser ? "gfx/vgui/basket/defuser" : "");
	nvBG->SetImage(m_SelectedItems.bNightVision ? "gfx/vgui/basket/nvgs" : "");

	switch (m_SelectedItems.iKelmet)
	{
	case ArmorType::NONE: kevBG->SetImage(""); break;
	case ArmorType::ARMOR: kevBG->SetImage("gfx/vgui/basket/vest"); break;
	case ArmorType::ARMOR_HELMET: kevBG->SetImage("gfx/vgui/basket/vesthelm"); break;
	}
}

void CCSBuySubMenu::OnSelectFavoriteWeapons(int i)
{
	if (i < 0 || i >= 5)
		return;
	m_SelectedItems = m_FavoriteItems[i];
	if (m_pFavDirectBuy->IsSelected())
		OnBuySelectedItems();
	SaveFavoriteSets();
}

void CCSBuySubMenu::OnSaveFavoriteWeapons(int i)
{
	if (i < 0 || i >= 5)
		return;
	m_FavoriteItems[i] = m_SelectedItems;
	SaveFavoriteSets();
}

void CCSBuySubMenu::OnClearSelectedItems()
{
	m_SelectedItems = {
		"" ,
		"" ,
		"" ,
		"" ,
		0,0,0,0, ArmorType::NONE
	};
	UpdateFavoriteSetsControls();
	SaveFavoriteSets();
}

void CCSBuySubMenu::OnBuySelectedItems()
{
	std::string szCommand;
	for(const std::string &wpn : { m_SelectedItems.Primary,m_SelectedItems.Secondary,m_SelectedItems.Melee,m_SelectedItems.HEGrenade })
		if (!wpn.empty())
		{
			const auto command = GetBuyMenuWeaponCommand(wpn.c_str());
			if (!command.empty())
				szCommand += command + ';';
		}
	szCommand += "secammo;primammo";
	if (m_SelectedItems.iKelmet == ArmorType::ARMOR)
		szCommand += ";vest";
	else if (m_SelectedItems.iKelmet == ArmorType::ARMOR_HELMET)
		szCommand += ";vesthelm";
	for (int i = 0; i < m_SelectedItems.nFlashBang; ++i)
		szCommand += ";flash";
	if (m_SelectedItems.nSmokeGrenade)
		szCommand += ";sgren";
	if (m_SelectedItems.bDefuser && m_iTeam == CT && gHUD.m_FollowIcon.m_iBombTargetsNum > 0)
		szCommand += ";defuser";
	if (m_SelectedItems.bNightVision)
		szCommand += ";nvgs";
	gEngfuncs.pfnClientCmd(const_cast<char *>(szCommand.c_str()));

	OnCommand("vguicancel");
}

CSBuyMouseOverPanelButton *CCSBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new CSBuyMouseOverPanelButton(this, NULL, panel);
}

CCSBuySubMenu *CCSBuySubMenu::CreateNewSubMenu(const char *name)
{
	return new CCSBuySubMenu(this, name);
}

void CCSBuySubMenu::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);
	// ItemInfo is only the resource template. Each weapon button owns the
	// actual CSBuyMouseOverPanel, sized explicitly in PerformLayout().
	m_pPanel->SetVisible(false);
	m_pPanel->SetMouseInputEnabled(false);
	m_pPanel->SetKeyBoardInputEnabled(false);
	if (auto* hint = dynamic_cast<Label*>(FindChildByName("fav_edit_desc")))
	{
		hint->SetText(L"本地收藏：选购武器后点击“保存”。\n点击收藏载入组合，或勾选“直接购买”。");
		hint->SetContentAlignment(Label::a_northwest);
	}

	// 0->exit
	m_pSlotButtons[9]->SetText("#Cstrike_Cancel");
	m_pSlotButtons[9]->SetCommand("vguicancel");

	account->SetText("#Cstrike_Current_Money");
	buytime->SetText("#CSO_BuyTime");
	for (vgui2::TextEntry *p : { account , buytime })
	{
		p->SetMouseInputEnabled(false);
	}

	m_pTitleLabel->SetFont(scheme()->GetIScheme(m_pTitleLabel->GetScheme())->GetFont("Title", IsProportional()));
	m_pTitleLabel->SizeToContents();

	const wchar key[5] = { L'S',L'D', L'F', L'G', L'H' };
	for (int i = 0; i < 5; ++i)
	{
		m_pFavButtons[i]->SetHotkey(key[i]);
	}

	float scale = vgui2::scheme()->GetProportionalScaledValue(4096) / 4096.0f;
	for (vgui2::ImagePanel * pPanel : { pwpnBG, swpnBG, hgrenBG, newknifeBG })
	{
		pPanel->SetShouldScaleImage(true);
		pPanel->SetShouldCenterImage(true);
	}
	hgrenBG->SetScaleAmount(0.54f * scale);
	swpnBG->SetScaleAmount(0.375f * scale);
	newknifeBG->SetScaleAmount(0.375f * scale);
	for (vgui2::ImagePanel * pPanel : { sgrenBG, fgrenBG, fgren2BG, dfBG, nvBG, kevBG })
	{
		pPanel->SetShouldScaleImage(true);
		pPanel->SetShouldCenterImage(true);
		pPanel->SetScaleAmount(0.54f * scale);
	}
	

	m_pShowCTWeapon->SetCommand("showctwpn");
	m_pShowTERWeapon->SetCommand("showterwpn");

	UpdateFavoriteSetsControls();
}

void CCSBuySubMenu_DefaultMode::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);
	BaseClass::LoadControlSettings("Resource/UI/cso_buysubmenu_ver2.res", "GAME");
	if (auto* hint = dynamic_cast<Label*>(FindChildByName("fav_edit_desc")))
		hint->SetText(L"本地武器预设\nS / D / F / G / H：购买对应组合。");


	// hide dm set
	m_pSetSelBg->SetVisible(false);
	m_pSetLabel->SetVisible(false);
	m_pPrevSetBtn->SetVisible(false);
	m_pNextSetBtn->SetVisible(false);
	m_pEditDescLabel_DM->SetVisible(false);
	m_pEditDescBg->SetVisible(false);
	m_pEquipSample->SetVisible(false);

	// Lower Right Corner
	m_pBasketClear->SetVisible(false);
	m_pBasketBuy->SetVisible(false);
	m_pQuitButton->SetVisible(false);

	m_pFavDirectBuy->SetVisible(false);
	m_pFavDirectBuy->SetSelected(true);

	for (vgui2::Button * pPanel : m_pFavSaveButtons)
	{
		pPanel->SetVisible(false);
	}

	// hide set
	for (vgui2::ImagePanel * pPanel : { primaryBG, secondaryBG, knifeBG, grenadeBG, equipBG })
	{
		pPanel->SetVisible(false);
	}

	for (vgui2::ImagePanel * pPanel : { pwpnBG, swpnBG, hgrenBG, newknifeBG })
	{
		pPanel->SetVisible(false);
	}
	for (vgui2::ImagePanel * pPanel : { sgrenBG, fgrenBG, fgren2BG, dfBG, nvBG, kevBG })
	{
		pPanel->SetVisible(false);
	}

	m_pShowCTWeapon->SetVisible(false);
	m_pShowTERWeapon->SetVisible(false);

	moneyText->SetVisible(false);
	moneyBack->SetVisible(false);
	freezetime->SetVisible(false);
}

void CCSBuySubMenu_DefaultMode::PerformLayout()
{
	BaseClass::PerformLayout();
	for (int i = 0; i < 10; ++i)
	{
		m_pSlotButtons[i]->SetPos(
			vgui2::scheme()->GetProportionalScaledValue(16),
			vgui2::scheme()->GetProportionalScaledValue(50 + i * 25)
		);
	}
}

void CCSBuySubMenu_ZombieMod::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);
	BaseClass::LoadControlSettings("Resource/UI/cso_buysubmenu_ver5.res", "GAME");


	// hide dm set
	m_pSetSelBg->SetVisible(false);
	m_pSetLabel->SetVisible(false);
	m_pPrevSetBtn->SetVisible(false);
	m_pNextSetBtn->SetVisible(false);
	m_pEditDescLabel_DM->SetVisible(false);
	m_pEditDescBg->SetVisible(false);
	m_pEquipSample->SetVisible(false);

	// hide money
	for(auto pPanel : { account_num ,moneyText })
		pPanel->SetVisible(false);
	for (auto pPanel : { freezetime, account, moneyBack })
		pPanel->SetVisible(false);
}

void CCSBuySubMenu_DeathMatch::LoadControlSettings(const char *dialogResourceName, const char *pathID, KeyValues *pPreloadedKeyValues)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID, pPreloadedKeyValues);
	BaseClass::LoadControlSettings("Resource/UI/cso_buysubmenu_ver5.res", "GAME");


	// Right Fav List Hide
	for (int i = 0; i < 5; i++)
	{
		m_pFavButtons[i]->SetVisible(false);
		m_pFavSaveButtons[i]->SetVisible(false);
	}
	m_pFavDirectBuy->SetVisible(false);

	// hide money & buy time
	for (auto pPanel : { account_num ,buytime_num ,moneyText })
		pPanel->SetVisible(false);
	for (auto pPanel : { freezetime, account, buytime, moneyBack })
		pPanel->SetVisible(false);

	// hide rebuy & autobuy
	m_pRebuyButton->SetVisible(false);
	m_pAutobuyButton->SetVisible(false);
}

void CCSBuySubMenu_DefaultMode::SetupPage(size_t iPage)
{
	BaseClass::SetupPage(iPage);
	m_pSlotButtons[9]->SetText("#CSO_EndWpnBuy");
	m_pSlotButtons[9]->SetCommand("vguicancel");
	m_pSlotButtons[9]->SetHotkey('0');
}

void CCSBuySubMenu_DefaultMode::OnSelectWeapon(const char *weapon)
{
	const auto command = GetBuyMenuWeaponCommand(weapon);
	if (command.empty())
		return;
	gEngfuncs.pfnClientCmd(const_cast<char *>(command.c_str()));
	OnCommand("vguicancel");
}

void CCSBuySubMenu_DefaultMode::OnSelectFavoriteWeapons(int iSet)
{
	BaseClass::OnSelectFavoriteWeapons(iSet);
}

void CCSBuySubMenu_ZombieMod::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "vest"))
	{
		m_SelectedItems.iKelmet = ArmorType::ARMOR;
		SaveFavoriteSets();
		BaseClass::SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "vesthelm"))
	{
		m_SelectedItems.iKelmet = ArmorType::ARMOR_HELMET;
		SaveFavoriteSets();
		BaseClass::SetupItems(BUY_NONE);
		return;
	}
	else if (!Q_strcmp(command, "flash") || !Q_strcmp(command, "sgren") || !Q_strcmp(command, "defuser") || !Q_strcmp(command, "nvgs"))
	{
		auto msgbox = new vgui2::MessageBox("#CSO_MODE_LOCK_H", "#CSO_MODE_LOCK_B", this);
		msgbox->SetOKButtonText("#CSO_OKl_Btn");
		msgbox->SetOKButtonVisible(true);
		msgbox->SetBounds(GetWide() / 2 - 150, GetTall() / 2 - 100, 300, 200);
		msgbox->DoModal();
		msgbox->Activate();
		BaseClass::SetupItems(BUY_NONE);
		return;
	}
	BaseClass::OnCommand(command);
}

void CCSBuySubMenu_ZombieMod::OnSelectWeapon(const char *weapon)
{
	BaseClass::OnSelectWeapon(weapon);
}

void CCSBuySubMenu_ZombieMod::SetupItems(MoEWeaponBuyType type)
{
	BaseClass::SetupItems(type);

	if (type == BUY_NONE)
	{
		for (int i : {5,6})
		{
			m_pSlotButtons[i]->SetText("");
			m_pSlotButtons[i]->SetCommand("VGUI_BuyMenu_Show");
			m_pSlotButtons[i]->SetVisible(true);
			m_pSlotButtons[i]->SetHotkey('0' + i + 1);
		}
		m_pSlotButtons[9]->SetCommand("VGUI_BuyMenu_Show");
	}
}
