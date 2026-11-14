// Built-in CSSO2 defaults, kept identical to the shipped loose configuration.
#ifndef CSTRIKEBUYDEFAULTS_H
#define CSTRIKEBUYDEFAULTS_H
static const char kBuyMenuDefaultItems[] = R"buyconfig(
// Default player loadout config file for Counter-Strike: Source Offensive.
// It is used to register loadout options and buy menu items for players.
//
// Example usage:
//
//	"CategoryName"																					// category entry name, must be unique; max 32 characters
//	{
//		"name"					"#CStrike_BuyMenu_CategoryName"										// category localized name in buy menu; max 64 characters
//		"numitemsrequired"		"6"																	// determines the amount of items a player has to put in their loadout menu; if above 0, allows players to re-arrange items in this category in their loadout menu
//
//		"ItemName"																					// item entry name; max 32 characters
//		{
//			"name"				"#CStrike_WPNHUD_ItemName"											// item localized name; max 64 characters
//			"description"		"#CStrike_BuyMenu_Description_ItemName"								// item localized description for buy menu; max 64 characters
//			"teamnumber"		"2"																	// what team is this item for (2 - T, 3 - CT)
//			"defaultitem"		"1"																	// whether this item is the default starting item or not; each team must have at least ONE default weapon!
//
//			"maplocalization"																		// map-specific localization
//			{
//				"cs_"																				// map name prefix which will override base name and description; max 32 characters
//				{
//					"name"			"#CStrike_WPNHUD_ItemName_OnHostageMaps"						// item localized name; max 64 characters
//					"description"	"#CStrike_BuyMenu_Description_ItemName_OnHostageMaps"			// item localized description for buy menu; max 64 characters
//				}
//			}
//		}
//	}
//
// NOTE: if you only put one category, the game will automatically open that category when opening buy menu, as if it didn't exist.
//
// DO NOT MODIFY THIS FILE!
// Instead, copy it to playerloadout_server.txt and modify that file. If no custom playerloadout_server.txt file is found,
// this file will be used as the default. This is crucial for the game to tell players if the server is using
// custom configuration or not!
// NOTE: custom playerloadout_server.txt file will make ALL items available to all players, player loadout will be ignored! It's only intended for server-side buy menu usage!

"PlayerLoadout"
{
	"Pistols"
	{
		"name"								"#CStrike_BuyMenu_Pistols"
		"numitemsrequired"					"5"

		"glock18"
		{
			"name"							"#CStrike_WPNHUD_Glock18"
			"description"					"#CStrike_BuyMenu_Description_Glock18"
			"teamnumber"					"2"
			"defaultitem"					"1"
		}
		"hkp2000"
		{
			"name"							"#CStrike_WPNHUD_HKP2000"
			"description"					"#CStrike_BuyMenu_Description_HKP2000"
			"teamnumber"					"3"
			"defaultitem"					"1"
		}
		"usp_silencer"
		{
			"name"							"#CStrike_WPNHUD_USP_Silencer"
			"description"					"#CStrike_BuyMenu_Description_USP_Silencer"
			"teamnumber"					"3"
			"defaultitem"					"1"
		}
		"elite"
		{
			"name"							"#CStrike_WPNHUD_Elite"
			"description"					"#CStrike_BuyMenu_Description_Elite"
		}
		"p250"
		{
			"name"							"#CStrike_WPNHUD_P250"
			"description"					"#CStrike_BuyMenu_Description_P250"
		}
		"tec9"
		{
			"name"							"#CStrike_WPNHUD_Tec9"
			"description"					"#CStrike_BuyMenu_Description_Tec9"
			"teamnumber"					"2"
		}
		"fiveseven"
		{
			"name"							"#CStrike_WPNHUD_FiveSeven"
			"description"					"#CStrike_BuyMenu_Description_FiveSeven"
			"teamnumber"					"3"
		}
		"cz75a"
		{
			"name"							"#CStrike_WPNHUD_CZ75A"
			"description"					"#CStrike_BuyMenu_Description_CZ75A"
		}
		"deagle"
		{
			"name"							"#CStrike_WPNHUD_DesertEagle"
			"description"					"#CStrike_BuyMenu_Description_DesertEagle"
		}
		"revolver"
		{
			"name"							"#CStrike_WPNHUD_Revolver"
			"description"					"#CStrike_BuyMenu_Description_Revolver"
		}
	}
	"Heavy"
	{
		"name"								"#CStrike_BuyMenu_Heavy"
		"numitemsrequired"					"5"

		"nova"
		{
			"name"							"#CStrike_WPNHUD_Nova"
			"description"					"#CStrike_BuyMenu_Description_Nova"
		}
		"xm1014"
		{
			"name"							"#CStrike_WPNHUD_XM1014"
			"description"					"#CStrike_BuyMenu_Description_XM1014"
		}
		"sawedoff"
		{
			"name"							"#CStrike_WPNHUD_SawedOff"
			"description"					"#CStrike_BuyMenu_Description_SawedOff"
			"teamnumber"					"2"
		}
		"mag7"
		{
			"name"							"#CStrike_WPNHUD_MAG7"
			"description"					"#CStrike_BuyMenu_Description_MAG7"
			"teamnumber"					"3"
		}
		"m249"
		{
			"name"							"#CStrike_WPNHUD_M249"
			"description"					"#CStrike_BuyMenu_Description_M249"
		}
		"negev"
		{
			"name"							"#CStrike_WPNHUD_Negev"
			"description"					"#CStrike_BuyMenu_Description_Negev"
		}
//		"shield"
//		{
//			"name"							"#CStrike_WPNHUD_Shield"
//			"description"					"#CStrike_BuyMenu_Description_Shield"
//			"teamnumber"					"3"
//		}
	}
	"SMGs"
	{
		"name"								"#CStrike_BuyMenu_SMGs"
		"numitemsrequired"					"5"

		"mac10"
		{
			"name"							"#CStrike_WPNHUD_MAC10"
			"description"					"#CStrike_BuyMenu_Description_MAC10"
			"teamnumber"					"2"
		}
		"mp9"
		{
			"name"							"#CStrike_WPNHUD_MP9"
			"description"					"#CStrike_BuyMenu_Description_MP9"
			"teamnumber"					"3"
		}
		"mp7"
		{
			"name"							"#CStrike_WPNHUD_MP7"
			"description"					"#CStrike_BuyMenu_Description_MP7"
		}
		"mp5sd"
		{
			"name"							"#CStrike_WPNHUD_MP5SD"
			"description"					"#CStrike_BuyMenu_Description_MP5SD"
		}
		"ump45"
		{
			"name"							"#CStrike_WPNHUD_UMP45"
			"description"					"#CStrike_BuyMenu_Description_UMP45"
		}
		"p90"
		{
			"name"							"#CStrike_WPNHUD_P90"
			"description"					"#CStrike_BuyMenu_Description_P90"
		}
		"bizon"
		{
			"name"							"#CStrike_WPNHUD_Bizon"
			"description"					"#CStrike_BuyMenu_Description_Bizon"
		}
	}
	"Rifles"
	{
		"name"								"#CStrike_BuyMenu_Rifles"
		"numitemsrequired"					"6"

		"galilar"
		{
			"name"							"#CStrike_WPNHUD_GalilAR"
			"description"					"#CStrike_BuyMenu_Description_GalilAR"
			"teamnumber"					"2"
		}
		"famas"
		{
			"name"							"#CStrike_WPNHUD_FAMAS"
			"description"					"#CStrike_BuyMenu_Description_FAMAS"
			"teamnumber"					"3"
		}
		"ak47"
		{
			"name"							"#CStrike_WPNHUD_AK47"
			"description"					"#CStrike_BuyMenu_Description_AK47"
			"teamnumber"					"2"
		}
		"m4a4"
		{
			"name"							"#CStrike_WPNHUD_M4A4"
			"description"					"#CStrike_BuyMenu_Description_M4A4"
			"teamnumber"					"3"
		}
		"m4a1_silencer"
		{
			"name"							"#CStrike_WPNHUD_M4A1_Silencer"
			"description"					"#CStrike_BuyMenu_Description_M4A1_Silencer"
			"teamnumber"					"3"
		}
		"ssg08"
		{
			"name"							"#CStrike_WPNHUD_SSG08"
			"description"					"#CStrike_BuyMenu_Description_SSG08"
		}
		"sg553"
		{
			"name"							"#CStrike_WPNHUD_SG553"
			"description"					"#CStrike_BuyMenu_Description_SG553"
			"teamnumber"					"2"
		}
		"aug"
		{
			"name"							"#CStrike_WPNHUD_AUG"
			"description"					"#CStrike_BuyMenu_Description_AUG"
			"teamnumber"					"3"
		}
		"awp"
		{
			"name"							"#CStrike_WPNHUD_AWP"
			"description"					"#CStrike_BuyMenu_Description_AWP"
		}
		"g3sg1"
		{
			"name"							"#CStrike_WPNHUD_G3SG1"
			"description"					"#CStrike_BuyMenu_Description_G3SG1"
			"teamnumber"					"2"
		}
		"scar20"
		{
			"name"							"#CStrike_WPNHUD_SCAR20"
			"description"					"#CStrike_BuyMenu_Description_SCAR20"
			"teamnumber"					"3"
		}
	}
	"Equipment"
	{
		"name"								"#CStrike_BuyMenu_Equipment"
		"numitemsrequired"					"0" // can't edit

		"kevlar"
		{
			"name"							"#CStrike_WPNHUD_Kevlar"
			"description"					"#CStrike_BuyMenu_Description_Kevlar"
		}
		"assaultsuit"
		{
			"name"							"#CStrike_WPNHUD_AssaultSuit"
			"description"					"#CStrike_BuyMenu_Description_AssaultSuit"
		}
		"taser"
		{
			"name"							"#CStrike_WPNHUD_Taser"
			"description"					"#CStrike_BuyMenu_Description_Taser"
		}
		"defuser"
		{
			"name"							"#CStrike_WPNHUD_Defuser"
			"description"					"#CStrike_BuyMenu_Description_Defuser"
			"teamnumber"					"3"

			"maplocalization"
			{
				"cs_"
				{
					"name"					"#CStrike_WPNHUD_Cutters"
					"description"			"#CStrike_BuyMenu_Description_Cutters"
				}
			}
		}
		// need to hide it, otherwise buy menu looks bad... and I just found
		// a nice svg icon for it too =(
//		"nvgs"
//		{
//			"name"							"#CStrike_WPNHUD_NightVision"
//			"description"					"#CStrike_BuyMenu_Description_NightVision"
//		}
	}
	"Grenades"
	{
		"name"								"#CStrike_BuyMenu_Grenades"
		"numitemsrequired"					"0" // can't edit

		"molotov"
		{
			"name"							"#CStrike_WPNHUD_Molotov"
			"description"					"#CStrike_BuyMenu_Description_Molotov"
			"teamnumber"					"2"
		}
		"incendiarygrenade"
		{
			"name"							"#CStrike_WPNHUD_IncendiaryGrenade"
			"description"					"#CStrike_BuyMenu_Description_IncendiaryGrenade"
			"teamnumber"					"3"
		}
		"decoy"
		{
			"name"							"#CStrike_WPNHUD_Decoy"
			"description"					"#CStrike_BuyMenu_Description_Decoy"
		}
		"flashbang"
		{
			"name"							"#CStrike_WPNHUD_Flashbang"
			"description"					"#CStrike_BuyMenu_Description_Flashbang"
		}
		"hegrenade"
		{
			"name"							"#CStrike_WPNHUD_HEGrenade"
			"description"					"#CStrike_BuyMenu_Description_HEGrenade"
		}
		"smokegrenade"
		{
			"name"							"#CStrike_WPNHUD_SmokeGrenade"
			"description"					"#CStrike_BuyMenu_Description_SmokeGrenade"
		}
	}
}
)buyconfig";
static const char kBuyMenuDefaultLoadout[] = R"buyconfig(
"LoadoutConfig"
{
	"loadout_t"
	{
		"Heavy"
		{
			"nova"		"1"
			"xm1014"		"1"
			"sawedoff"		"1"
			"m249"		"1"
			"negev"		"1"
		}
		"Rifles"
		{
			"galilar"		"1"
			"ak47"		"1"
			"ssg08"		"1"
			"sg553"		"1"
			"awp"		"1"
			"g3sg1"		"1"
		}
		"Pistols"
		{
			"glock18"		"1"
			"elite"		"1"
			"p250"		"1"
			"tec9"		"1"
			"deagle"		"1"
		}
		"SMGs"
		{
			"mac10"		"1"
			"mp7"		"1"
			"ump45"		"1"
			"p90"		"1"
			"bizon"		"1"
		}
	}
	"loadout_ct"
	{
		"Heavy"
		{
			"nova"		"1"
			"xm1014"		"1"
			"mag7"		"1"
			"m249"		"1"
			"negev"		"1"
		}
		"SMGs"
		{
			"mp9"		"1"
			"mp7"		"1"
			"ump45"		"1"
			"p90"		"1"
			"bizon"		"1"
		}
		"Rifles"
		{
			"famas"		"1"
			"m4a4"		"1"
			"ssg08"		"1"
			"aug"		"1"
			"awp"		"1"
			"scar20"		"1"
		}
		"Pistols"
		{
			"hkp2000"		"1"
			"elite"		"1"
			"p250"		"1"
			"fiveseven"		"1"
			"deagle"		"1"
		}
	}
}

)buyconfig";
#endif
