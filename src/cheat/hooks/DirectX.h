#include "../render/gui.h"
#include <iostream>
#include <string>

vec2_t MenuPos = { 200.f, 200.f };

JBMenu::JBMenu(void)
{
	this->Visible = true;
}


void JBMenu::Init_Menu(ID3D11DeviceContext* pContext, LPCWSTR Title, int x, int y)
{
	this->pContext = pContext;
	this->Is_Ready = true;
	this->sMenu.Title = Title;
	this->sMenu.x = x;
	this->sMenu.y = y;
}


void JBMenu::AddFolder(LPCWSTR Name, int* Pointer, int limit)
{
	sOptions[this->Items].Name = (LPCWSTR)Name;
	sOptions[this->Items].Function = Pointer;
	sOptions[this->Items].Type = T_FOLDER;
	this->Items++;
}

void JBMenu::AddOption(LPCWSTR Name, int* Pointer, int* Folder, int Limit, int type)
{
	if (*Folder == 0)
		return;
	sOptions[this->Items].Name = Name;
	sOptions[this->Items].Function = Pointer;
	sOptions[this->Items].Type = type;
	sOptions[this->Items].limit = Limit;
	this->Items++;
}
void JBMenu::Navigation()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		this->Visible = !this->Visible;

	if (!this->Visible)
		return;

	int value = 0;

	if (GetAsyncKeyState(VK_DOWN) & 1)
	{
		this->Cur_Pos++;
		if (sOptions[this->Cur_Pos].Name == 0)
			this->Cur_Pos--;
	}

	if (GetAsyncKeyState(VK_UP) & 1)
	{
		this->Cur_Pos--;
		if (this->Cur_Pos == -1)
			this->Cur_Pos++;
	}

	else if (GetAsyncKeyState(VK_RIGHT) & 1)
	{
		if (*sOptions[this->Cur_Pos].Function == 0)
			value++;
	}

	else if ((GetAsyncKeyState(VK_LEFT) & 1) && *sOptions[this->Cur_Pos].Function == 1)
	{
		value--;
	}


	if (value) {
		*sOptions[this->Cur_Pos].Function += value;
		if (sOptions[this->Cur_Pos].Type == T_FOLDER)
		{
			memset(&sOptions, 0, sizeof(sOptions));
			this->Items = 0;
		}
	}


}

void JBMenu::Draw_Menu()
{
	if (!this->Visible)
		return;

	//this->DrawText(this->sMenu.Title, 14, sMenu.x + 10, sMenu.y, this->Color_Font);
	for (int i = 0; i < this->Items; i++)
	{
		if (this->sOptions[i].Type == T_OPTION)
		{
			if (*this->sOptions[i].Function)
			{
				Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"On");
				//this->DrawText(L"On", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_On);
			}
			else
			{
				Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"Off");
				//this->DrawText(L"Off", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Off);
			}
		}
		if (this->sOptions[i].Type == T_FOLDER)
		{
			if (*this->sOptions[i].Function)
			{
				Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"Opened");
				//this->DrawText(L"Opend", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
			else
			{
				Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"Closed");
				//this->DrawText(L"Closed", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
		}
		if (this->sOptions[i].Type == T_BOOL)
		{
			if (*this->sOptions[i].Function)
			{
				Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"True", false, Render::Green());
				//this->DrawText(L"Opend", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
			else
			{
				Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"False", false, Render::Red());
				//this->DrawText(L"Closed", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
		}
		if (this->sOptions[i].Type == T_INT)
		{
			auto sval = std::to_string((*this->sOptions[i].Function));
			std::wstring wide_string = std::wstring(sval.begin(), sval.end());
			const wchar_t* result = wide_string.c_str();
			Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, result);
		}
		Render::Color Color = Render::Color(250, 250, 250);
		if (this->Cur_Pos == i)
			Color = Render::Color(250, 0, 0);
		else if (this->sOptions[i].Type == T_FOLDER)
			Color = Render::Color(0, 250, 250);

		//this->DrawText(this->sOptions[i].Name, 12, sMenu.x + 5, sMenu.y + LineH * (i + 2), Color);
		Render::String(vec2_t{ (float)(sMenu.x + 15), (float)(sMenu.y + LineH * (i + 2)) }, this->sOptions[i].Name, false, Color);

	}
}
bool JBMenu::IsReady()
{
	if (this->Items)
		return true;
	return false;
}

enum ItemID
{
	GOLD_KRABER = 1,
	GOLD_MASTIFF = 2,
	LSTAR = 3,
	HAVOC = 4,
	GOLD_HAVOC = 5,
	DEVOTION = 6,
	TRIPLE_TAKE = 7,
	GOLD_TRIPLE_TAKE = 8,
	FLATLINE = 9,
	GOLD_FLATLINE = 10,
	HEMLOCK = 11,
	G7_SCOUT = 12,
	GOLD_G7_SCOUT = 13,
	ALTERNATOR = 14,
	GOLD_ALTERNATOR = 15,
	R99 = 16,
	PROWLER = 17,
	GOLD_PROWLER = 18,
	LONGBOW = 19,
	GOLD_LONGBOW = 20,
	CHARGE_RIFLE = 21,
	GOLD_CHARGE_RIFLE = 22,
	SPITFIRE = 23,
	R301 = 24,
	GOLD_R301 = 25,
	EVA8_AUTO = 26,
	GOLD_EVA8_AUTO = 27,
	PEACEKEEPER = 28,
	GOLD_PEACEKEEPER = 29,
	MOZAMBIQUE = 30,
	GOLD_MOZAMBIQUE = 31,
	WINGMAN = 32,
	GOLD_WINGMAN = 33,
	P2020 = 34,
	GOLD_P2020 = 35,
	RE45 = 36,
	GOLD_RE45 = 37,
	SENTINEL = 38,

	LIGHT_ROUNDS = 40,
	ENERGY_AMMO,
	SHOTGUN_SHELLS,
	HEAVY_ROUNDS,
	SNIPER_AMMO,

	ULTIMATE_ACCELERANT,
	PHOENIX_KIT,
	MED_KIT,
	SYRINGE,
	SHIELD_BATTERY,
	SHIELD_CELL,

	HELMET_LV1,
	HELMET_LV2,
	HELMET_LV3,
	HELMET_LV4,
	BODY_ARMOR_LV1,
	BODY_ARMOR_LV2,
	BODY_ARMOR_LV3,
	BODY_ARMOR_LV4,
	BODY_ARMOR_LV5,
	UNKNOWN_1,
	UNKNOWN_2,
	UNKNOWN_3,
	KNOCKDOWN_SHIELD_LV1,
	KNOCKDOWN_SHIELD_LV2,
	KNOCKDOWN_SHIELD_LV3,
	KNOCKDOWN_SHIELD_LV4,
	BACKPACK_LV1,
	BACKPACK_LV2,
	BACKPACK_LV3,
	BACKPACK_LV4,

	THERMITE_GRENADE,
	FRAG_GRENADE,
	ARC_STAR,

	HCOG_CLASSIC,
	HCOG_BRUISER,
	HOLO,
	VARIABLE_HOLO,
	DIGITAL_THREAT,
	HCOG_RANGER,
	VARIABLE_AOG,
	SNIPER,
	VARIABLE_SNIPER,
	DIGITAL_SNIPER_THREAT,

	BARREL_STABILIZER_LV1,
	BARREL_STABILIZER_LV2,
	BARREL_STABILIZER_LV3,
	BARREL_STABILIZER_LV4,
	LIGHT_MAGAZINE_LV1,
	LIGHT_MAGAZINE_LV2,
	LIGHT_MAGAZINE_LV3,
	HEAVY_MAGAZINE_LV1,
	HEAVY_MAGAZINE_LV2,
	HEAVY_MAGAZINE_LV3,
	SNIPER_MAGAZINE_LV1,
	SNIPER_MAGAZINE_LV2,
	SNIPER_MAGAZINE_LV3,
	SHOTGUN_BOLT_LV1,
	SHOTGUN_BOLT_LV2,
	SHOTGUN_BOLT_LV3,
	STANDARD_STOCK_LV1,
	STANDARD_STOCK_LV2,
	STANDARD_STOCK_LV3,
	SNIPER_STOCK_LV1,
	SNIPER_STOCK_LV2,
	SNIPER_STOCK_LV3,

	SELECTFIRE_RECEIVER,
	PRECISION_CHOKE,
	HAMMERPOINT_ROUNDS,
	ANVIL_RECEIVER,
	DOUBLE_TAP_TRIGGER,
	VAULT_KEY,
};

enum class ItemRarity
{
	Common,
	Uncommon,
	Rare,
	Legendary,
	Ultra
};

enum class ItemTypes
{
	Guns,
	Armor,
	Medical,
	Ammo,
	Grenade,
	Attachment,
	Misc
};

__forceinline const wchar_t* LootName(int ScriptID, Render::Color& Color, ItemRarity& rarity, ItemTypes& itemtype)
{
	switch (ScriptID)
	{
		//Energy Weapons
		case ItemID::HAVOC:  Color = Render::LightGreen(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Havoc");
		case ItemID::LSTAR:  Color = Render::LightGreen(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"L-Star");
		case ItemID::ENERGY_AMMO: Color = Render::LightGreen(); rarity = ItemRarity::Common; itemtype = ItemTypes::Ammo; return E(L"Ammo - Energy");

		//Golden Weapons
		case ItemID::GOLD_KRABER:  Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Guns; return E(L"Kraber");
		case ItemID::GOLD_MASTIFF:  Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Guns; return E(L"Mastiff");
		case ItemID::DEVOTION:  Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Guns; return E(L"Devotion");

		//Snipers
		case ItemID::TRIPLE_TAKE:  Color = Render::LightPurple(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Triple-Take");
		case ItemID::SENTINEL: Color = Render::LightPurple(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Sentinel");
		case ItemID::LONGBOW: Color = Render::LightPurple(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"LongBow");
		case ItemID::CHARGE_RIFLE: Color = Render::LightPurple(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Charge Rifle");
		case ItemID::SNIPER_AMMO: Color = Render::LightPurple(); rarity = ItemRarity::Common; itemtype = ItemTypes::Ammo; return E(L"Ammo - Sniper");

		//Heavy Weapons
		case ItemID::FLATLINE:  Color = Render::Green(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"FlatLine");
		case ItemID::HEMLOCK: Color = Render::Green(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Hemlok");
		case ItemID::PROWLER: Color = Render::Green(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Prowler");
		case ItemID::SPITFIRE: Color = Render::Green(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"SpitFire");
		case ItemID::WINGMAN: Color = Render::Green(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"WingMan");
		case ItemID::HEAVY_ROUNDS: Color = Render::Green(); rarity = ItemRarity::Common; itemtype = ItemTypes::Ammo; return E(L"Ammo - Heavy");

		//Light Weapons
		case ItemID::G7_SCOUT: Color = Render::Orange(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"G7-Scout");
		case ItemID::ALTERNATOR: Color = Render::Orange(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Alternator");
		case ItemID::R99: Color = Render::Orange(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"R-99");
		case ItemID::R301: Color = Render::Orange(); rarity = ItemRarity::Ultra; itemtype = ItemTypes::Guns; return E(L"R-301");
		case ItemID::P2020: Color = Render::Orange(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"P2020");
		case ItemID::RE45: Color = Render::Orange(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"RE-45");
		case ItemID::LIGHT_ROUNDS: Color = Render::Orange();  rarity = ItemRarity::Common; itemtype = ItemTypes::Ammo; return E(L"Ammo - Light");

		//Shotguns
		case ItemID::EVA8_AUTO: Color = Render::Red(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"EVA-8");
		case ItemID::PEACEKEEPER: Color = Render::Red(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"PeaceKeeper");
		case ItemID::MOZAMBIQUE: Color = Render::Red(); rarity = ItemRarity::Common; itemtype = ItemTypes::Guns; return E(L"Mozambique");
		case ItemID::SHOTGUN_SHELLS: Color = Render::Red(); rarity = ItemRarity::Common; itemtype = ItemTypes::Ammo; return E(L"Ammo - Shotgun");
		
		//Items
		case ItemID::ULTIMATE_ACCELERANT: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Misc; return E(L"Accelerant");
		case ItemID::PHOENIX_KIT: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Medical; return E(L"Phoenix Kit");
		case ItemID::MED_KIT: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Medical; return E(L"MedKit");
		case ItemID::SYRINGE: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Medical; return E(L"Syringe");
		case ItemID::SHIELD_BATTERY: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Medical; return E(L"Shield Battery");
		case ItemID::SHIELD_CELL: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Medical; return E(L"Shield Cell");

		//Helmets
		case ItemID::HELMET_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Armor; return E(L"Helmet-1");
		case ItemID::HELMET_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Armor; return E(L"Helmet-2");
		case ItemID::HELMET_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Armor; return E(L"Helmet-3");
		case ItemID::HELMET_LV4: Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Armor; return E(L"Helmet-4");

		//Armor
		case ItemID::BODY_ARMOR_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Armor; return E(L"Armor-1");
		case ItemID::BODY_ARMOR_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Armor; return E(L"Armor-2");
		case ItemID::BODY_ARMOR_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Armor; return E(L"Armor-3");
		case ItemID::BODY_ARMOR_LV4: Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Armor; return E(L"Armor-4");
		case ItemID::BODY_ARMOR_LV5: Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Armor; return E(L"Armor-5 (EVO SHIELD)");

		//Knockdown Shields
		case ItemID::KNOCKDOWN_SHIELD_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Armor; return E(L"Knock Shield-1");
		case ItemID::KNOCKDOWN_SHIELD_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Armor; return E(L"Knock Shield-2");
		case ItemID::KNOCKDOWN_SHIELD_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Armor; return E(L"Knock Shield-3");
		case ItemID::KNOCKDOWN_SHIELD_LV4: Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Armor; return E(L"Knock Shield-4");

		//Backpacks
		case ItemID::BACKPACK_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Armor; return E(L"Backpack-1");
		case ItemID::BACKPACK_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Armor; return E(L"Backpack-2");
		case ItemID::BACKPACK_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Armor; return E(L"Backpack-3");
		case ItemID::BACKPACK_LV4: Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Armor; return E(L"Backpack-4");

		//Grenades
		case ItemID::THERMITE_GRENADE: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Grenade; return E(L"Thermite Grenade");
		case ItemID::FRAG_GRENADE: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Grenade; return E(L"Frag Grenade");
		case ItemID::ARC_STAR: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Grenade; return E(L"Arc Star");

		//Scopes
		//case ItemID::HCOG_BRUISER: Color = Render::Grey(); return E(L"HCOG BRUISER"); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment;

		//Barrel Stabilizers
		case ItemID::BARREL_STABILIZER_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment; return E(L"Stabilizer-1");
		case ItemID::BARREL_STABILIZER_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Stabilizer-2");
		case ItemID::BARREL_STABILIZER_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Attachment; return E(L"Stabilizer-3");
		case ItemID::BARREL_STABILIZER_LV4: Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Attachment; return E(L"Stabilizer-4");

		//Extended Mags
		case ItemID::LIGHT_MAGAZINE_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment; return E(L"Light Mag-1");
		case ItemID::LIGHT_MAGAZINE_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Light Mag-2");
		case ItemID::LIGHT_MAGAZINE_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Attachment; return E(L"Light Mag-3");

		case ItemID::HEAVY_MAGAZINE_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment; return E(L"Heavy Mag-1");
		case ItemID::HEAVY_MAGAZINE_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Heavy Mag-2");
		case ItemID::HEAVY_MAGAZINE_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Attachment; return E(L"Heavy Mag-3");

		case ItemID::SNIPER_MAGAZINE_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment; return E(L"Sniper Mag-1");
		case ItemID::SNIPER_MAGAZINE_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Sniper Mag-2");
		case ItemID::SNIPER_MAGAZINE_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Attachment; return E(L"Sniper Mag-3");

		//Shotgun Bolts
		case ItemID::SHOTGUN_BOLT_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment; return E(L"Bolt-1");
		case ItemID::SHOTGUN_BOLT_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Bolt-2");
		case ItemID::SHOTGUN_BOLT_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Attachment; return E(L"Bolt-3");

		//Standard Stocks
		case ItemID::STANDARD_STOCK_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment; return E(L"Standard Stock-1");
		case ItemID::STANDARD_STOCK_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Standard Stock-2");
		case ItemID::STANDARD_STOCK_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Attachment; return E(L"Standard Stock-3");

		//Sniper Stocks
		case ItemID::SNIPER_STOCK_LV1: Color = Render::Grey(); rarity = ItemRarity::Common; itemtype = ItemTypes::Attachment; return E(L"Sniper Stock-1");
		case ItemID::SNIPER_STOCK_LV2: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Sniper Stock-2");
		case ItemID::SNIPER_STOCK_LV3: Color = Render::Purple(); rarity = ItemRarity::Rare; itemtype = ItemTypes::Attachment; return E(L"Sniper Stock-3");

		//Misc Attachments
		case ItemID::SELECTFIRE_RECEIVER: Color = Render::Grey(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Selectfire");
		case ItemID::PRECISION_CHOKE: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Percision Choke");
		case ItemID::HAMMERPOINT_ROUNDS: Color = Render::Purple(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Hammerpoint");
		case ItemID::ANVIL_RECEIVER: Color = Render::Gold(); rarity = ItemRarity::Legendary; itemtype = ItemTypes::Attachment; return E(L"Anvil Receiver");
		case ItemID::DOUBLE_TAP_TRIGGER: Color = Render::Blue(); rarity = ItemRarity::Uncommon; itemtype = ItemTypes::Attachment; return E(L"Double Tap");

		//Misc Items
		case ItemID::VAULT_KEY: Color = Render::Red(); rarity = ItemRarity::Ultra; itemtype = ItemTypes::Misc; return E(L"Vault Key");

		default: return nullptr;
	}
}

void Do_Menu()
{
	//ESP Section
	menu.AddFolder(L"ESP", &Visuals::VisualsFolder, 1);
	menu.AddOption(L"Player ESP", &Visuals::ESP, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Glow", &Visuals::GlowESP, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Loot ESP", &Visuals::LootESP, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Draw Crosshair", &Visuals::DrawCrossHair, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Filter Out Gun", &Visuals::FilterOutGuns, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Filter Out Armor", &Visuals::FilterOutArmor, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Filter Out Ammo", &Visuals::FilterOutAmmo, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Filter Out Attachments", &Visuals::FilterOutAttachments, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Filter Out Grenades", &Visuals::FilterOutGrenades, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Filter Out Medical", &Visuals::FilterOutMedical, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Filter Out Misc", &Visuals::FilterOutMisc, &Visuals::VisualsFolder, 1, T_BOOL);
	menu.AddOption(L"Rarity Level", &Visuals::FilterLevel, &Visuals::VisualsFolder, 3, T_INT);

	//Aimbot Section
	menu.AddFolder(L"Aimbot", &AimBot::AimbotFolder, 1);
	menu.AddOption(L"Enabled", &AimBot::Active, &AimBot::AimbotFolder, 1, T_BOOL);
	menu.AddOption(L"Aim Key", &AimBot::AimKey, &AimBot::AimbotFolder, 100, T_INT);
	menu.AddOption(L"Use Camera", &AimBot::UseCamera, &AimBot::AimbotFolder, 1, T_BOOL);
	//menu.AddOption(L"Vis Check", &AimBot::VisCheck, &AimBot::AimbotFolder, 1, T_BOOL);
	menu.AddOption(L"Nearest Bone", &AimBot::Nearest, &AimBot::AimbotFolder, 1, T_BOOL);
	menu.AddOption(L"Ignore Knocked", &AimBot::NoKnocked, &AimBot::AimbotFolder, 1, T_BOOL);
	menu.AddOption(L"Remove Sway/Recoil", &AimBot::WeaponFix, &AimBot::AimbotFolder, 1, T_BOOL);
	menu.AddOption(L"Draw FOV", &AimBot::DrawFOV, &AimBot::AimbotFolder, 1, T_BOOL);
	menu.AddOption(L"FOV", &AimBot::FOV, &AimBot::AimbotFolder, 100, T_INT);
	menu.AddOption(L"Smooth", &AimBot::Smooth, &AimBot::AimbotFolder, 10, T_INT);
	menu.AddOption(L"Distance", &AimBot::Distance, &AimBot::AimbotFolder, 500, T_INT);

	//Misc Section
	menu.AddFolder(L"Misc", &Misc::MiscFolder, 1);
	menu.AddOption(L"BHop", &Misc::Bhop, &Misc::MiscFolder, 1, T_BOOL);
}

//store original function
PVOID PresentOrg = nullptr;

//Present Hook
void __fastcall Present(PVOID a1, PVOID a2)
{
	//VIRTUALIZER_MUTATE_ONLY_START
	Render::String(vec2_t{ (float)(menu.sMenu.x + 5), (float)menu.sMenu.y }, menu.sMenu.Title);
	if (menu.IsReady() == false)
	{
		//menu.Init_Menu(RenderToolKit::D3D_Context, L"Aspect", 0, 0);
		Do_Menu();
	}
	if (Render::NewFrame(I::SwapChain, Global::WndSize))
	{
		//Render::String({ 20, 20 }, E(L"Aspect"));

		//misc visuals
		if (AimBot::Active && AimBot::DrawFOV)
			Render::Circle(Global::WndSize / 2.f, Render::Color(192, 192, 192), AimBot::FOV, 2.f);

		if (Visuals::DrawCrossHair)
			Render::FillCircle(Global::WndSize / 2.f, Render::Color(255, 255, 0), 2.f);

		//dp(EntityCount);
		for (int i = 0; i < EntityCount; i++)
		{
			if (EntityCache[i].EntID == -1) continue;
			CBasePlayer* CurEnt = Ent(EntityCache[i].EntID);
			
			//loot esp
			if (Visuals::LootESP && CurEnt->Type(LootEnt)) 
			{
				vec2_t LootPos2D; Render::Color Color; ItemRarity rarity; ItemTypes itemtype;
				if (World2Screen(CurEnt->AbsOrgin(), LootPos2D))
				{
					const wchar_t* LootStr = LootName(CurEnt->ScriptID(), Color, rarity, itemtype);

					if (Visuals::FilterOutGuns && itemtype == ItemTypes::Guns)
						continue;
					else if (Visuals::FilterOutArmor && itemtype == ItemTypes::Armor)
						continue;
					else if (Visuals::FilterOutAmmo && itemtype == ItemTypes::Ammo)
						continue;
					else if (Visuals::FilterOutAttachments && itemtype == ItemTypes::Attachment)
						continue;
					else if (Visuals::FilterOutMedical && itemtype == ItemTypes::Medical)
						continue;
					else if (Visuals::FilterOutGrenades && itemtype == ItemTypes::Grenade)
						continue;
					else if (Visuals::FilterOutMisc && itemtype == ItemTypes::Misc)
						continue;

					if ((int)rarity < Visuals::FilterLevel)
						continue;

					vec2_t LootTextSize = Render::String(LootPos2D, LootStr, true, Color);
				}
			}

			//player esp
			else if (Visuals::ESP && CurEnt->Type(Player))
			{
				//get screen position
				vec2_t Head2D, Feet2D; vec3_t Head3D = CurEnt->HitBoxPos(0, false);
				if (World2Screen(Head3D, Head2D) && World2Screen(CurEnt->AbsOrgin(), Feet2D))
				{
					//get box size
					float Height = (Head2D.y - Feet2D.y), Width = Height / 2.f;

					//set box color
					Render::Color BoxColor = Render::Color(255, 0, 0);
					if (CurEnt->Knocked()) BoxColor = Render::Color(192, 192, 192);
					else if (EntityCache[i].Visible) BoxColor = Render::Color(0, 255, 0);

					//draw box
					Render::Rectangle({ (Feet2D.x - (Width / 2.f)), Feet2D.y }, { Width, Height }, Render::Color(0, 0, 0), 3.f);
					Render::Rectangle({ (Feet2D.x - (Width / 2.f)), Feet2D.y }, { Width, Height }, BoxColor);

					//glow esp
					if (Visuals::GlowESP) {
						vec3_t Color = { 100.f, 0.f, 10.f };
						if (CurEnt->Knocked()) Color = { 130.f, 130.f, 130.f };
						else if (EntityCache[i].Visible) Color = { 0.f, 100.f, 10.f };
						CurEnt->Glow(Color);
					}

					//draw hp
					float HealthSize = (Height * CurEnt->Health()) / 100.f;
					Render::Line({ (Feet2D.x - (Width / 2.f) + 4.f), Feet2D.y }, { (Feet2D.x - (Width / 2.f) + 4.f), Head2D.y }, Render::Color(0, 0, 0), 5.f);
					Render::Line({ (Feet2D.x - (Width / 2.f) + 4.f), Feet2D.y + HealthSize }, { (Feet2D.x - (Width / 2.f) + 4.f), Head2D.y }, Render::Color(255, 0, 0), 2.f);
					Render::Line({ (Feet2D.x - (Width / 2.f) + 4.f), Feet2D.y }, { (Feet2D.x - (Width / 2.f) + 4.f), Feet2D.y + HealthSize }, Render::Color(0, 255, 0), 2.f);

					//draw name
					const char* Name = CurEnt->PlayerName();
					if (!Name) continue; wchar_t Str[64];
					for (int i = 0;; i++) { if (!(Str[i] = Name[i])) break; }
					Render::String({ Feet2D.x, Head2D.y - 10.f }, Str, true);
				}
			}
		}
		
		
		if (GUI::Visible)
		{
			menu.Draw_Menu();
		}

		Render::EndFrame(I::SwapChain);
	}

	static bool init;
	if (!init) {
		HWND Wnd = FC(user32, GetActiveWindow);
		if (!Wnd)
		{
			goto ret;
		}
		//auto ThreadID = FC(user32, GetWindowThreadProcessId, Wnd, nullptr);
		auto ThID = FC(kernel32, GetCurrentThreadId);
		FC(user32, SetWindowsHookExA, WH_GETMESSAGE, GUI::WndHook, (HINSTANCE)GetModuleBase(nullptr), ThID);
		//GUI::NextWndProc = (WNDPROC)FC(user32, SetWindowLongPtrA, Wnd, GWLP_WNDPROC, (LONG_PTR)GUI::WndProc);
		//ImGui_ImplDX11_Init(Wnd, RenderToolKit::D3D_Device, RenderToolKit::D3D_Context);
		//ImGui_ImplDX11_CreateDeviceObjects();
		init = true;
	}

	ret:
	SpoofCall(PresentOrg, a1, a2);
}

//HRESULT __fastcall Resize(IDXGISwapChain* Swap, UINT a2, UINT a3, UINT a4, DXGI_FORMAT a5, UINT a6) {
//
//	auto state = oResize(Swap, a2, a3, a4, a5, a6);
//	GUI::Render.Release(); 
//	return state;
//}

//void HookD3D()
//{
//	/*DX11PresentFn* pOBS_Present = (DX11PresentFn*)RVA(FindPattern(E("FF 15 ? ? ? ? 48 8B 16 48 8B CE FF 52 10 48"), E(L"graphics-hook64.dll")), 6);
//	oPresent = *pOBS_Present; *pOBS_Present = Present;*/
//
//	DX11PresentFn* pOBS_Present = (DX11PresentFn*)(GetModuleBase(E(L"IGO64")) + 0x163CE8); 
//	oPresent = *pOBS_Present; *pOBS_Present = Present;
//}