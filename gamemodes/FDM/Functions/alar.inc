/*
 *	Any Last Administrative Requests - alar.inc
 *		Alar gamemode include
 *
 *	Copyright © 2009 [DRuG]Dabombber
 *
 *	This file is part of Alar.
 *
 * 	Alar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 * 	Alar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Alar.  If not, see <http://www.gnu.org/licenses/>.
 */


#if defined _alar_included
	#endinput
#endif
#define _alar_included

#include <a_samp>

/*
	native GetAlarVersion(version[]="", maxlength=sizeof(version));

	native AddJoinLine(playerid, colour, const string[]);
	native AddAdminLogLine(colour, const string[]);

	native BanPlayer(const playername[]="\01", const playerip[]="\01", const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false);
	native BanPlayerid(playerid, const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false);
	native SuspendPlayer(const playername[]="\01", const playerip[]="\01", Float:hours, const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false);
	native SuspendPlayerid(playerid, Float:hours, const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false);
	native WarnPlayer(playerid, const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false);
	native WarnPlayerid(const playername[], const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false);

	native IsPlayerScriptAdmin(playerid);
	native GetAdminLevel(playerid);

	native bool:GetCountryFromCode(const code[], dest[], maxlength=sizeof(dest));
	native bool:GetIPCountryCode(const IP[], dest[], maxlength=sizeof(dest));
	native bool:GetPlayerCountryCode(playerid, dest[], maxlength=sizeof(dest));
	native bool:GetPlayerCountryName(playerid, dest[], maxlength=sizeof(dest));

	native GetAdminSpectating(playerid);
	native SetAdminSpectating(playerid, targetid, bool:saveplayer);
	native DisableAdminSpectating(playerid);
	native ClearAdminSpawnData(playerid);
	native SetSpawnType(E_SPAWNTYPE:spawntype, E_INFOTYPE:spawnsaves=INFOTYPE_POSITION|INFOTYPE_WORLD|INFOTYPE_WEAPONS|INFOTYPE_CASH|INFOTYPE_HEALTH|INFOTYPE_ARMOUR|INFOTYPE_VEHICLE);

	native SetAlarLoginCheck(const func[]="\01");

	native GetAdminState(playerid);
	native SetAdminState(playerid, newstate, seconds);
	native ClearAdminState(playerid, newstate);

	native HistoryAddMessage(const playername[], colour, const message[], flags=0);
	native HistoryAddLine(colour, const message[], flags=0);
	native HistoryClear();

	native DestroyAdminVehicle(Vehicleid)
*/


// --------------------------------------------------
// Defines
// --------------------------------------------------

#define FREE_SPECTATE_ID	-1
#define ALAR_PROPERTY_ID	41412

#if !defined MAX_COUNTRY_CODE
	#define MAX_COUNTRY_CODE 3
#endif

#if !defined MAX_COUNTRY_NAME
	#define MAX_COUNTRY_NAME 64
#endif


enum (<<=1) {
	ADMIN_STATE_MUTED = 1,
	ADMIN_STATE_FROZEN,
	ADMIN_STATE_JAILED,
	ADMIN_STATE_HIDDEN,
	ADMIN_STATE_DESYNCED
};

enum (<<=1) {
	HISTORY_ADMIN_ONLY = 1,
	HISTORY_PLAYER_ONLY,
	HISTORY_WRAPPED
};

enum E_SPAWNTYPE {
	SPAWN_ALAR_SPAWN = -3,
	SPAWN_ALAR_STATE,
	SPAWN_ALAR_TIMER,
	SPAWN_DEFAULT,
	SPAWN_GAMEMODE_TIMER,
	SPAWN_GAMEMODE_STATE,
	SPAWN_GAMEMODE_SPAWN
};

enum E_INFOTYPE (<<=1) {
	INFOTYPE_POSITION = 1,
	INFOTYPE_WORLD,
	INFOTYPE_WEAPONS,
	INFOTYPE_CASH,
	INFOTYPE_HEALTH,
	INFOTYPE_ARMOUR,
	INFOTYPE_VEHICLE
};

enum E_COUNTRYCODES {
	E_COUNTRY_CODE[MAX_COUNTRY_CODE char],
	E_COUNTRY_NAME[MAX_COUNTRY_NAME char]
}; stock SIZE_E_COUNTRYCODES[E_COUNTRYCODES];


stock static const scCountryCodes[][E_COUNTRYCODES] = {
	{!"A1", !"Anonymous Proxy"},
	{!"A2", !"Satellite Provider"},
	{!"AC", !"Ascension Island"},
	{!"AD", !"Andorra"},
	{!"AE", !"United Arab Emirates"},
	{!"AF", !"Afghanistan"},
	{!"AG", !"Antigua and Barbuda"},
	{!"AI", !"Anguilla"},
	{!"AL", !"Albania"},
	{!"AM", !"Armenia"},
	{!"AN", !"Netherlands Antilles"},
	{!"AO", !"Angola"},
	{!"AQ", !"Antarctica"},
	{!"AR", !"Argentina"},
	{!"AS", !"American Samoa"},
	{!"AT", !"Austria"},
	{!"AU", !"Australia"},
	{!"AW", !"Aruba"},
	{!"AX", !"Aland Islands"},
	{!"AZ", !"Azerbaijan"},
	{!"BA", !"Bosnia and Herzegovina"},
	{!"BB", !"Barbados"},
	{!"BD", !"Bangladesh"},
	{!"BE", !"Belgium"},
	{!"BF", !"Burkina Faso"},
	{!"BG", !"Bulgaria"},
	{!"BH", !"Bahrain"},
	{!"BI", !"Burundi"},
	{!"BJ", !"Benin"},
	{!"BL", !"Saint Barthélemy"},
	{!"BM", !"Bermuda"},
	{!"BN", !"Brunei Darussalam"},
	{!"BO", !"Bolivia, Plurinational State of"},
	{!"BR", !"Brazil"},
	{!"BS", !"Bahamas"},
	{!"BT", !"Bhutan"},
	{!"BV", !"Bouvet Island"},
	{!"BW", !"Botswana"},
	{!"BY", !"Belarus"},
	{!"BZ", !"Belize"},
	{!"CA", !"Canada"},
	{!"CC", !"Cocos (Keeling) Islands"},
	{!"CD", !"Congo, the Democratic Republic of the"},
	{!"CF", !"Central African Republic"},
	{!"CG", !"Congo"},
	{!"CH", !"Switzerland"},
	{!"CI", !"Cote d'Ivoire"},
	{!"CK", !"Cook Islands"},
	{!"CL", !"Chile"},
	{!"CM", !"Cameroon"},
	{!"CN", !"China"},
	{!"CO", !"Colombia"},
	{!"CP", !"Clipperton Island"},
	{!"CR", !"Costa Rica"},
	{!"CU", !"Cuba"},
	{!"CV", !"Cape Verde"},
	{!"CX", !"Christmas Island"},
	{!"CY", !"Cyprus"},
	{!"CZ", !"Czech Republic"},
	{!"DE", !"Germany"},
	{!"DG", !"Diego Garcia"},
	{!"DJ", !"Djibouti"},
	{!"DK", !"Denmark"},
	{!"DM", !"Dominica"},
	{!"DO", !"Dominican Republic"},
	{!"DZ", !"Algeria"},
	{!"EC", !"Ecuador"},
	{!"EE", !"Estonia"},
	{!"EG", !"Egypt"},
	{!"EH", !"Western Sahara"},
	{!"ER", !"Eritrea"},
	{!"ES", !"Spain"},
	{!"ET", !"Ethiopia"},
	{!"EU", !"European Union"},
	{!"FI", !"Finland"},
	{!"FJ", !"Fiji"},
	{!"FK", !"Falkland Islands (Malvinas)"},
	{!"FM", !"Micronesia, Federated States of"},
	{!"FO", !"Faroe Islands"},
	{!"FR", !"France"},
	{!"FX", !"France, Metropolitan"},
	{!"GA", !"Gabon"},
	{!"GB", !"United Kingdom"},
	{!"GD", !"Grenada"},
	{!"GE", !"Georgia"},
	{!"GF", !"French Guiana"},
	{!"GG", !"Guernsey"},
	{!"GH", !"Ghana"},
	{!"GI", !"Gibraltar"},
	{!"GL", !"Greenland"},
	{!"GM", !"Gambia"},
	{!"GN", !"Guinea"},
	{!"GP", !"Guadeloupe"},
	{!"GQ", !"Equatorial Guinea"},
	{!"GR", !"Greece"},
	{!"GS", !"South Georgia and the South Sandwich Islands"},
	{!"GT", !"Guatemala"},
	{!"GU", !"Guam"},
	{!"GW", !"Guinea-Bissau"},
	{!"GY", !"Guyana"},
	{!"HK", !"Hong Kong"},
	{!"HM", !"Heard Island and McDonald Islands"},
	{!"HN", !"Honduras"},
	{!"HR", !"Croatia"},
	{!"HT", !"Haiti"},
	{!"HU", !"Hungary"},
	{!"IC", !"Canary Islands"},
	{!"ID", !"Indonesia"},
	{!"IE", !"Ireland"},
	{!"IL", !"Israel"},
	{!"IM", !"Isle of Man"},
	{!"IN", !"India"},
	{!"IO", !"British Indian Ocean Territory"},
	{!"IQ", !"Iraq"},
	{!"IR", !"Iran, Islamic Republic of"},
	{!"IS", !"Iceland"},
	{!"IT", !"Italy"},
	{!"JE", !"Jersey"},
	{!"JM", !"Jamaica"},
	{!"JO", !"Jordan"},
	{!"JP", !"Japan"},
	{!"KE", !"Kenya"},
	{!"KG", !"Kyrgyzstan"},
	{!"KH", !"Cambodia"},
	{!"KI", !"Kiribati"},
	{!"KM", !"Comoros"},
	{!"KN", !"Saint Kitts and Nevis"},
	{!"KP", !"Korea, Democratic People's Republic of"},
	{!"KR", !"Korea, Republic of"},
	{!"KW", !"Kuwait"},
	{!"KY", !"Cayman Islands"},
	{!"KZ", !"Kazakhstan"},
	{!"LA", !"Lao People's Democratic Republic"},
	{!"LB", !"Lebanon"},
	{!"LC", !"Saint Lucia"},
	{!"LI", !"Liechtenstein"},
	{!"LK", !"Sri Lanka"},
	{!"LR", !"Liberia"},
	{!"LS", !"Lesotho"},
	{!"LT", !"Lithuania"},
	{!"LU", !"Luxembourg"},
	{!"LV", !"Latvia"},
	{!"LY", !"Libyan Arab Jamahiriya"},
	{!"MA", !"Morocco"},
	{!"MC", !"Monaco"},
	{!"MD", !"Moldova, Republic of"},
	{!"ME", !"Montenegro"},
	{!"MF", !"Saint Martin (French part)"},
	{!"MG", !"Madagascar"},
	{!"MH", !"Marshall Islands"},
	{!"MK", !"Macedonia, the former Yugoslav Republic of"},
	{!"ML", !"Mali"},
	{!"MM", !"Myanmar"},
	{!"MN", !"Mongolia"},
	{!"MO", !"Macao"},
	{!"MP", !"Northern Mariana Islands"},
	{!"MQ", !"Martinique"},
	{!"MR", !"Mauritania"},
	{!"MS", !"Montserrat"},
	{!"MT", !"Malta"},
	{!"MU", !"Mauritius"},
	{!"MV", !"Maldives"},
	{!"MW", !"Malawi"},
	{!"MX", !"Mexico"},
	{!"MY", !"Malaysia"},
	{!"MZ", !"Mozambique"},
	{!"NA", !"Namibia"},
	{!"NC", !"New Caledonia"},
	{!"NE", !"Niger"},
	{!"NF", !"Norfolk Island"},
	{!"NG", !"Nigeria"},
	{!"NI", !"Nicaragua"},
	{!"NL", !"Netherlands"},
	{!"NO", !"Norway"},
	{!"NP", !"Nepal"},
	{!"NR", !"Nauru"},
	{!"NU", !"Niue"},
	{!"NZ", !"New Zealand"},
	{!"O1", !"Other"},
	{!"OM", !"Oman"},
	{!"PA", !"Panama"},
	{!"PE", !"Peru"},
	{!"PF", !"French Polynesia"},
	{!"PG", !"Papua New Guinea"},
	{!"PH", !"Philippines"},
	{!"PK", !"Pakistan"},
	{!"PL", !"Poland"},
	{!"PM", !"Saint Pierre and Miquelon"},
	{!"PN", !"Pitcairn"},
	{!"PR", !"Puerto Rico"},
	{!"PS", !"Palestinian Territory, Occupied"},
	{!"PT", !"Portugal"},
	{!"PW", !"Palau"},
	{!"PY", !"Paraguay"},
	{!"QA", !"Qatar"},
	{!"RE", !"Reunion"},
	{!"RO", !"Romania"},
	{!"RS", !"Serbia"},
	{!"RU", !"Russian Federation"},
	{!"RW", !"Rwanda"},
	{!"SA", !"Saudi Arabia"},
	{!"SB", !"Solomon Islands"},
	{!"SC", !"Seychelles"},
	{!"SD", !"Sudan"},
	{!"SE", !"Sweden"},
	{!"SG", !"Singapore"},
	{!"SH", !"Saint Helena"},
	{!"SI", !"Slovenia"},
	{!"SJ", !"Svalbard and Jan Mayen"},
	{!"SK", !"Slovakia"},
	{!"SL", !"Sierra Leone"},
	{!"SM", !"San Marino"},
	{!"SN", !"Senegal"},
	{!"SO", !"Somalia"},
	{!"SR", !"Suriname"},
	{!"ST", !"Sao Tome and Principe"},
	{!"SU", !"USSR"},
	{!"SV", !"El Salvador"},
	{!"SY", !"Syrian Arab Republic"},
	{!"SZ", !"Swaziland"},
	{!"TA", !"Tristan da Cunha"},
	{!"TC", !"Turks and Caicos Islands"},
	{!"TD", !"Chad"},
	{!"TF", !"French Southern Territories"},
	{!"TG", !"Togo"},
	{!"TH", !"Thailand"},
	{!"TJ", !"Tajikistan"},
	{!"TK", !"Tokelau"},
	{!"TL", !"Timor-Leste"},
	{!"TM", !"Turkmenistan"},
	{!"TN", !"Tunisia"},
	{!"TO", !"Tonga"},
	{!"TR", !"Turkey"},
	{!"TT", !"Trinidad and Tobago"},
	{!"TV", !"Tuvalu"},
	{!"TW", !"Taiwan"},
	{!"TZ", !"Tanzania, United Republic of"},
	{!"UA", !"Ukraine"},
	{!"UG", !"Uganda"},
	{!"UK", !"United Kingdom"},
	{!"UM", !"United States Minor Outlying Islands"},
	{!"US", !"United States"},
	{!"UY", !"Uruguay"},
	{!"UZ", !"Uzbekistan"},
	{!"VA", !"Holy See (Vatican City State)"},
	{!"VC", !"Saint Vincent and the Grenadines"},
	{!"VE", !"Venezuela"},
	{!"VG", !"Virgin Islands, British"},
	{!"VI", !"Virgin Islands, U.S."},
	{!"VN", !"Vietnam"},
	{!"VU", !"Vanuatu"},
	{!"WF", !"Wallis and Futuna"},
	{!"WS", !"Samoa"},
	{!"XL", !"Localhost"},
	{!"XN", !"LAN"},
	{!"YE", !"Yemen"},
	{!"YT", !"Mayotte"},
	{!"ZA", !"South Africa"},
	{!"ZM", !"Zambia"},
	{!"ZW", !"Zimbabwe"}
 };


// --------------------------------------------------
// Functions
// --------------------------------------------------

/**
 *  Send a custom join message just before kicking a player
 *  @param	playerid	The playerid of the player to be kicked
 *  @param	colour		The colour of the message
 *  @param	string		The kick reason
 *  @return	1 if the message was sent successfully, otherwise 0
 */
#define AddJoinLine(%1,%2,%3)		(CallRemoteFunction("alar_AddJoinLine", "iis", %1, %2, %3))

/**
 *  Add a message to the admin log
 *  @param	colour		The colour of the message
 *  @param	string		The message
 *  @return	1 if the message was added successfully, otherwise 0
 */
#define AddAdminLogLine(%1,%2)		(CallRemoteFunction("alar_AddAdminLogLine", "is", %1, %2))

/**
 *  Get a player's admin level
 *  @param	playerid	The playerid of the player
 *  @return	The level of the player, or 0 if no player is found
 */
#define GetAdminLevel(%1)			GetPVarInt(%1, "AlarLevel")

/**
 *  Remove an admin from admin spec
 *  @param	playerid	The playerid of the admin
 *  @return	1 if the player was spawned, otherwise 0
 */
#define DisableAdminSpectating(%1)	(CallRemoteFunction("alar_DisableAdminSpectating", "i", %1))

/**
 *  Clear the spawn data of an admin is admin spec, forcing them to use the gamemode spawn info
 *  @param	playerid	The playerid of the admin
 *  @return	1 if the data was cleared, otherwise 0
 */
#define ClearAdminSpawnData(%1)		(CallRemoteFunction("alar_ClearAdminSpawnData", "i", %1))

/**
 *  Get the state of a player (muted/frozen etc)
 *  @param	playerid	The playerid of the player
 *  @return	A mask of the state bits (ADMIN_STATE_MUTED/ADMIN_STATE_FROZEN etc)
 */
#define GetAdminState(%1)			GetPVarInt(%1, "AlarStatus")

/**
 *  Clear the state of a player (ie unmute/unjail etc)
 *  @param	playerid	The playerid of the player
 *  @param	newstate	A mask of the states to clear
 *  @return	1 if alar is loaded and the player is connected, otherwise 0
 */
#define ClearAdminState(%1,%2)		(CallRemoteFunction("alar_ClearAdminState", "ii", %1, %2))

/**
 *  Check if a player is either and RCON or alar admin
 *  @param	playerid	The playerid of the player
 *  @return	true if the player is an admin, otherwise false
 */
#define IsPlayerScriptAdmin(%1)		(IsPlayerAdmin(%1) || GetPVarInt(%1, "AlarLevel") > 0)

/**
 *  Check if a player is either and RCON or alar admin
 *  @return	1 if the history was cleared, otherwise 0
 */
#define HistoryClear(%1)			(CallRemoteFunction("alar_ClearHistory", ""))

/**
 *  Destroy an admin created vehicle
 *  @param	vehicleid	The vehicle to destroy, or INVALID_VEHICLE_ID to destroy all admin vehicles
 *  @return	The number of vehicles destroyed
 */
#define DestroyAdminVehicle(%1)		(CallRemoteFunction("alar_DestroyVehicle", "%i", %1))

/**
 *  Get the version number of alar
 *  @param	version		A string to store the version number in
 *  @param	maxlength	The size of the version string
 *  @return	true if alar is loaded, otherwise false
 */
stock bool:GetAlarVersion(version[]="", maxlength=sizeof(version))
{
	if(existproperty(.value = ALAR_PROPERTY_ID)) {
		if(maxlength > 1) {
			new tmp[128];
			getproperty(.value = ALAR_PROPERTY_ID, .string = tmp);
			strunpack(version, tmp, maxlength);
		}
		return true;
	}
	return false;
}

/**
 *  Gets a country name from an ISO 3166-1 alpha-2 country code
 *  @param	code		The ISO 3166-1 alpha-2 country code
 *  @param	dest		The destination string
 *  @param	maxlength	The size of the destination string
 *  @return	true if the player's country was found, false if it is unknown
 */
stock bool:GetCountryFromCode(const code[], dest[], maxlength=sizeof(dest))
{
	if(code[0] != '\0') {
		for(new i; i < sizeof(scCountryCodes); i++) {
			if(strcmp(code, scCountryCodes[i][E_COUNTRY_CODE], true) == 0) {
				strunpack(dest, scCountryCodes[i][E_COUNTRY_NAME], maxlength);
				return true;
			}
		}
	}
	strmid(dest, "Unknown", 0, cellmax, maxlength);
	return false;
}

/**
 *  Get an IP's ISO 3166-1 alpha-2 country code
 *  @param	IP			The IP
 *  @param	dest		The destination string
 *  @param	maxlength	The size of the destination string
 *  @return	true if the player's country was found, false if it is unknown
 */
stock bool:GetIPCountryCode(const IP[], dest[], maxlength=sizeof(dest))
{
	if(IP[0] != '\0') {
		new string[MAX_COUNTRY_CODE char];
		if((string[0] = CallRemoteFunction("alar_GetCountryCode", "s", IP))) {
			strunpack(dest, string, maxlength);
			return true;
		}
	}
	dest[0] = '\0';
	return false;
}

/**
 *  Get the player's ISO 3166-1 alpha-2 country code
 *  @param	playerid	The playerid of the player
 *  @param	dest		The destination string
 *  @param	maxlength	The size of the destination string
 *  @return	true if the player's country was found, false if it is unknown
 */
stock bool:GetPlayerCountryCode(playerid, dest[], maxlength=sizeof(dest))
{
	if(GetPVarType(playerid, "CountryCode") == PLAYER_VARTYPE_STRING) {
		GetPVarString(playerid, "CountryCode", dest, maxlength);
		return true;
	}
	return false;
}

/**
 *  Get the player's country name
 *  @param	playerid	The playerid of the player
 *  @param	dest		The destination string
 *  @param	maxlength	The size of the destination string
 *  @return	true if the player's country was found, false if it is unknown
 */
stock bool:GetPlayerCountryName(playerid, dest[], maxlength=sizeof(dest))
{
	if(GetPVarType(playerid, "CountryName") == PLAYER_VARTYPE_STRING) {
		GetPVarString(playerid, "CountryName", dest, maxlength);
		return true;
	}
	return false;
}

/**
 *  Get the playerid of the person an admin is spectating
 *  @param	playerid	The playerid of the admin
 *  @return	The playerid of the player being spectated, FREE_SPECTATE_ID if the admin is in free spec,
 *			or INVALID_PLAYER_ID if they are not spectating anyone
 */
stock GetAdminSpectating(playerid)
{
	if(GetPVarType(playerid, "AlarSpectating") == PLAYER_VARTYPE_INT) {
		new specID = GetPVarInt(playerid, "AlarSpectating");
		if(specID == INVALID_PLAYER_ID) return FREE_SPECTATE_ID;
		return specID;
	}
	return INVALID_PLAYER_ID;
}

/**
 *  Set an admin spectating another player
 *  @param	playerid	The playerid of the admin
 *  @param	targetid	The playerid of player to be spectated
 *  @param	saveplayer	Whether the playerid should be saved as the last person the admin spectated
 *  @return	1 if the admin was put into spectate mode, otherwise 0
 */
stock SetAdminSpectating(playerid, targetid=INVALID_PLAYER_ID, bool:saveplayer=false)
{
	return CallRemoteFunction("alar_SetAdminSpectating", "iii", playerid, targetid, saveplayer);
}

/**
 *  Set which data should be saved when an admin goes into spectate mode, and how it should be restored
 *  @param	spawnytype	How the data should be restored (SPAWN_ALAR_SPAWN/SPAWN_ALAR_STATE etc)
 *  @param	spawnsaves	A mask of which data should be stored (INFOTYPE_POSITION/INFOTYPE_WORLD etc)
 *  @return	1 if alar is loaded, otherwise 0
 */
stock SetSpawnType(E_SPAWNTYPE:spawntype, E_INFOTYPE:spawnsaves=INFOTYPE_POSITION|INFOTYPE_WORLD|INFOTYPE_WEAPONS|INFOTYPE_CASH|INFOTYPE_HEALTH|INFOTYPE_ARMOUR|INFOTYPE_VEHICLE)
{
	return CallRemoteFunction("alar_SetSpawnType", "ii", _:spawntype, _:spawnsaves);
}

/**
 *  Set which public function should be called to check if a player is logged in (for banning/suspending names)
 *  @param	func	The public function
 *  @return	1 if alar is loaded, otherwise 0
 */
stock SetAlarLoginCheck(const func[]="\01")
{
	return CallRemoteFunction("alar_LoginFunc", "s", func);
}

/**
 *  Set the state of a player (muted/jailed etc)
 *  @param	newstate	A mask of the states to be set (ADMIN_STATE_MUTED/ADMIN_STATE_JAILED etc)
 *  @param	seconds		The number of seconds to set the state for (0 or less for it to be permanent)
 *  @return	1 if the player is connected, otherwise 0
 */
stock SetAdminState(playerid, newstate, seconds=0)
{
	return CallRemoteFunction("alar_SetAdminState", "iii", playerid, newstate, seconds);
}

/**
 *  Ban a player
 *  @param	playername	The name to be banned
 *  @param	playerip	The IP to be banned
 *  @param	reason		The reason for the ban
 *  @param	adminname	The name of the admin responsible the ban
 *  @param	adminip		The IP of the admin responsible the ban
 *  @param	hidename	Hide the name of the admin responsible
 *  @return	1 if alar is loaded
 */
stock BanPlayer(const playername[]="\01", const playerip[]="\01", const reason[]="\01", const adminname[]="\01", const adminip[]="\01", bool:hidename=false)
{
	return CallRemoteFunction("alar_Ban", "sssssi", playername, playerip, reason, adminname, adminip, hidename);
}

/**
 *  Ban a player
 *  @param	playerid	The playerid of the player to be banned
 *  @param	reason		The reason for the ban
 *  @param	adminname	The name of the admin responsible the ban
 *  @param	adminip		The IP of the admin responsible the ban
 *  @param	hidename	Hide the name of the admin responsible
 *  @return	1 if alar is loaded
 */
stock BanPlayerid(playerid, const reason[]="\01", const adminname[]="\01", const adminip[]="\01", bool:hidename=false)
{
	return CallRemoteFunction("alar_BanPlayer", "isssi", playerid, reason, adminname, adminip, hidename);
}

/**
 *  Suspend a player
 *  @param	playername	The name to be suspended
 *  @param	playerip	The IP to be suspended
 *  @param	hours		The number of hours to suspend for
 *  @param	reason		The reason for the suspension
 *  @param	adminname	The name of the admin responsible the suspension
 *  @param	adminip		The IP of the admin responsible the suspension
 *  @param	hidename	Hide the name of the admin responsible
 *  @return	1 if alar is loaded
 */
stock SuspendPlayer(const playername[]="\01", const playerip[]="\01", Float:hours, const reason[]="\01", const adminname[]="\01", const adminip[]="\01", bool:hidename=false)
{
	return CallRemoteFunction("alar_Suspend", "ssfsssi", playername, playerip, hours, reason, adminname, adminip, hidename);
}

/**
 *  Suspend a player
 *  @param	playerid	The playerid for the player to be suspended
 *  @param	hours		The number of hours to suspend for
 *  @param	reason		The reason for the suspension
 *  @param	adminname	The name of the admin responsible the suspension
 *  @param	adminip		The IP of the admin responsible the suspension
 *  @param	hidename	Hide the name of the admin responsible
 *  @return	1 if alar is loaded
 */
stock SuspendPlayerid(playerid, Float:hours, const reason[]="\01", const adminname[]="\01", const adminip[]="\01", bool:hidename=false)
{
	return CallRemoteFunction("alar_SuspendPlayer", "ifsssi", playerid, hours, reason, adminname, adminip, hidename);
}

/**
 *  Warn a player
 *  @param	playername	The name to be warned
 *  @param	reason		The reason for the warning
 *  @param	adminname	The name of the admin responsible the warning
 *  @param	adminip		The IP of the admin responsible the warning
 *  @param	hidename	Hide the name of the admin responsible
 *  @return	1 if alar is loaded
 */
stock WarnPlayer(const playername[], const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false)
{
	return CallRemoteFunction("alar_Warn", "ssssi", playername, reason, adminname, adminip, hidename);
}

/**
 *  Warn a player
 *  @param	playerid	The playerid for the player to be warned
 *  @param	reason		The reason for the warning
 *  @param	adminname	The name of the admin responsible the warning
 *  @param	adminip		The IP of the admin responsible the warning
 *  @param	hidename	Hide the name of the admin responsible
 *  @return	1 if alar is loaded
 */
stock WarnPlayerid(playerid, const reason[]="No reason", const adminname[]="\01", const adminip[]="\01", bool:hidename=false)
{
	return CallRemoteFunction("alar_WarnPlayer", "isssi", playerid, reason, adminname, adminip, hidename);
}

/**
 *  Add a message to the chat history
 *  @param	playername	The name of the player who sent the message
 *  @param	colour		The colour of the message
 *  @param	message		The message
 *  @param	flags		A mask of the message flags (HISTORY_ADMIN_ONLY, HISTORY_PLAYER_ONLY and HISTORY_WRAPPED)
 *  @return	1 if the message was added to the chat history, otherwise 0
 */
stock HistoryAddMessage(const playername[], colour, const message[], flags=0)
{
	return CallRemoteFunction("alar_AddMessageToHistory", "sisi", playername, colour, message, flags);
}

/**
 *  Add a line to the chat history
 *  @param	colour		The colour of the line
 *  @param	message		The message
 *  @param	flags		A mask of the message flags (HISTORY_ADMIN_ONLY, HISTORY_PLAYER_ONLY and HISTORY_WRAPPED)
 *  @return	1 if the message was added to the chat history, otherwise 0
 */
stock HistoryAddLine(colour, const message[], flags=0)
{
	return CallRemoteFunction("alar_AddLineToHistory", "isi", colour, message, flags);
}

// --------------------------------------------------
// Redefined functions
// --------------------------------------------------

stock alarSetPlayerVirtualWorld(playerid, worldid)
{
	if(SetPlayerVirtualWorld(playerid, worldid)) {
		CallRemoteFunction("alar_OnPlayerWorldChange", "ii", playerid, worldid);
		return 1;
	}
	return 0;
}
#if defined _ALS_SetPlayerVirtualWorld
    #undef SetPlayerVirtualWorld
#else
    #define _ALS_SetPlayerVirtualWorld
#endif
#define SetPlayerVirtualWorld	alarSetPlayerVirtualWorld

stock alarPutPlayerInVehicle(playerid, vehicleid, seatid)
{
	if(IsPlayerInAnyVehicle(playerid)) {
		new vid = GetPlayerVehicleID(playerid),
			ret = PutPlayerInVehicle(playerid, vehicleid, seatid);
		if(vid && ret && vid != vehicleid) {
			CallRemoteFunction("alar_OnPlayerVehicleChange", "ii", playerid, vehicleid);
		}
		return ret;
	}
	return PutPlayerInVehicle(playerid, vehicleid, seatid);
}
#if defined _ALS_PutPlayerInVehicle
    #undef PutPlayerInVehicle
#else
    #define _ALS_PutPlayerInVehicle
#endif
#define PutPlayerInVehicle		alarPutPlayerInVehicle

stock alarPlayerSpectatePlayer(playerid, targetplayerid, mode = SPECTATE_MODE_NORMAL)
{
	if(PlayerSpectatePlayer(playerid, targetplayerid, mode)) {
		CallRemoteFunction("alar_OnPlayerSpectate", "ii", playerid, targetplayerid);
		return 1;
	}
	return 0;
}
#if defined _ALS_PlayerSpectatePlayer
    #undef PlayerSpectatePlayer
#else
    #define _ALS_PlayerSpectatePlayer
#endif
#define PlayerSpectatePlayer	alarPlayerSpectatePlayer

stock alarPlayerSpectateVehicle(playerid, targetvehicleid, mode = SPECTATE_MODE_NORMAL)
{
	if(PlayerSpectateVehicle(playerid, targetvehicleid, mode)) {
		CallRemoteFunction("alar_OnPlayerSpectateVehicle", "ii", playerid, targetvehicleid);
		return 1;
	}
	return 0;
}
#if defined _ALS_PlayerSpectateVehicle
    #undef PlayerSpectateVehicle
#else
    #define _ALS_PlayerSpectateVehicle
#endif
#define PlayerSpectateVehicle	alarPlayerSpectateVehicle

stock alarDestroyVehicle(vehicleid)
{
	if(DestroyVehicle(vehicleid)) {
		CallRemoteFunction("alar_OnVehicleDestroyed", "i", vehicleid);
		return 1;
	}
	return 0;
}
#if defined _ALS_DestroyVehicle
    #undef DestroyVehicle
#else
    #define _ALS_DestroyVehicle
#endif
#define DestroyVehicle	alarDestroyVehicle

// --------------------------------------------------
// Forwards (Callback declarations)
// --------------------------------------------------

forward OnAlarInit();
forward OnAlarExit();

forward OnAdminLogin(playerid, level);
forward OnAdminLogout(playerid);

forward OnAdminStateChange(playerid, newstate, oldstate);
forward OnAdminNameChange(playerid, newname[], oldname[]);
forward OnAdminSpectate(playerid, newid, oldid);

