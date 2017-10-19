/*

Gamemode: Final Deathmatch

This is a remake of the GTO gamemode. It will be suited to the AGS server.
The gamemode is built into a filterscript since it is more flexible that way.

Credits:
KillFrenzy, aka Evan Tran - Wrote this gamemode.
Slick, aka Tim - INII script - Made file processing much easier and decently fast. Also helped me in making vehicle spawns.

Special Thanks:
SAMP Dev Team - Making SAMP - This won't be possible without them.
Dabombber - ALAR script - The gamemode is designed to work with ALAR.
Iain Gilbert - GTO - The orignal maker of GTO.
Others - Anyone I've forgetten, and whoever made some of those miscellaneous functions.

TODO:
	- Anti-AFK for minimodes
	- Boundaries are minimode specific, so they are added to races.
*/

#pragma dynamic 300000

#include <a_samp>

//#undef MAX_PLAYERS
//#define MAX_PLAYERS 101

#define ISOLATE_VIRTUAL_WORLDS true
#define ENABLE_CALLBACKS true
#define ENABLE_INTERIOR_WEAPONS false
#define ENABLE_INTERIOR_WORLD_SYNC true

// For crippled versions
//#define CRIPPLE_PLAYERS 4
#if defined CRIPPLE_PLAYERS
	#define MAX_PLAYERS_EX MAX_PLAYERS
	#undef MAX_PLAYERS
	#define MAX_PLAYERS CRIPPLE_PLAYERS
#endif

// Constants
#define MAX_STRING 256
#define MAX_FILENAME 256
#define MAX_WEAPONNAME 20
#define MAX_INPUT 129
#define MAX_IP 16
#define MAX_HASH 33
#define MAX_NAME MAX_PLAYER_NAME
#define MAX_REAL 12
#define MAX_HEX 9

#define MAX_GANG_NAME 32
#define MAX_LOGIN_MSG 32

#define MAX_WEAPON_ID 47
#define MAX_WEAPON_SLOTS 13

#define WRAPPED_MESSAGE_PREFIX "    "

// Important includes
#include "Functions\alar.inc"
#include "Functions\misc.inc" // These functions are needed for almost every include
#include "Functions\pause.inc" // Pausing functions
#include "alarstuff.inc"

// Some gamemodewide constants
#define COLOUR_TITLE 0xFFEEBBAA
#define COLOUR_ATTENTION 0xDD1111AA
#define COLOUR_HELP 0xBBEE00AA
#define COLOUR_ERROR 0xDD0000AA
#define COLOUR_SUCCESSFUL 0xDDFF00AA
#define COLOUR_LOG 0

#define ADMIN_LEVEL_MISC 4
#define ADMIN_LEVEL_MODERATOR 8
#define ADMIN_LEVEL_MASTER 10

#define SIDECHAT_MAX_LINES 16
#define SIDECHAT_DEFAULT_LINES 4
#define SIDECHAT_DEFAULT_TIME 10000

//#define IP_DEFINITIONS "alar/IP2c/"
#define FILE_OTHER_SETTINGS "FDM/Settings/Other.ini"

// Commands handling
#define CMD_SET_LOGIN 1
#define CMD_SET_GANG 2
#define CMD_SET_MONEY 3
#define CMD_SET_WEAPONS 4
#define CMD_SET_MINIMODES 5
#define CMD_SET_MISC 6
#define CMD_SET_ADMIN 7
#define CMD_SET_SPECIALTY 8

// Anti-cheat
#include "anticheat.inc"

// Functions
#include "skins.inc" // Skin Spawning
//#include "Functions\YSF.inc" // Dynamic gravity per player
//#include "Functions\ip2c.inc" // IP Stuff
#include "Functions\SII.inc" // Slick's INI script
#include "Functions\md5.inc" // MD5 used for password handling
//#include "Functions\MidoStream.inc" // MidoStreamer for gamemode objects

// YSF gravity workaround
/*#define _YSI_included
native SetPlayerGravity(playerid, Float:gravity);
native Float:GetPlayerGravity(playerid);*/

// Incognito's Streamer for gamemode objects
#include "Functions\streamer.inc"

// Function redefines
stock kClearAnimations(playerid) {
	new Float:x, Float:y, Float:z;
	new vehicleid = GetPlayerVehicleID(playerid);
	if (vehicleid) {
		GetVehicleVelocity(vehicleid, x, y, z);
	} else {
		GetPlayerVelocity(playerid, x, y, z);
	}
	ClearAnimations(playerid);
	SetPlayerVelocity(playerid, x, y, z);
}
#define ClearAnimations kClearAnimations

// xStreamer's invalid object id
#if defined INVALID_OBJECT_ID
	#undef INVALID_OBJECT_ID
#endif
#define INVALID_OBJECT_ID -1

// Fs Gamemode defines
#define MAX_GMVEHICLES MAX_VEHICLES
#define MAX_GMVEHICLE_TYPES 50
#define MAX_TIMERS 256
#define DEFAULT_RESPAWN_DELAY 180
#define ENABLE_VEHICLE_WORLD_FIX false // Not needed for SAMP 0.3

// Streamers / anti-cheat / other misc
//#include "fsgamemode.inc" // Gamemode to filterscript
#include "cpstream.inc" // Checkpoint streaming
#include "worldstream.inc" // Virtual worlds streamer
#include "time.inc" // Sync the clock
#include "weather.inc" // Support for proper weather in scripts
#include "customgametext.inc" // Custom gametext

// Gamemode includes/defines
#define GAMEMODE_AUTHOR "KillFrenzy"
#define GAMEMODE_NAME "Final Deathmatch"
#define GAMEMODE_VERSION "0.9.27.5c"

// Gamemode core
#include "players.inc" // Player account core
#include "gangs.inc" // Gangs core
#include "levels.inc" // Experience system core
#include "spawns.inc" // Player spawn locations

// Gamemode additions
#include "sidechat.inc" // Side chat box beside the radar
#include "godspawning.inc" // Gives player temporary godmode when spawning
#include "money.inc" // Banks and money handler
#include "weapons.inc" // Ammunation and spawn weapons
#include "vehicles.inc" // Vehicle spawns
#include "xpzones.inc" // Experience/cash zones
#include "bounty.inc" // Bounties
#include "interiors.inc" // Interior weapon disables
#include "stealth.inc" // Stealth - calculates the sound a player is making

#include "combatdetect.inc" // In-combat detection
#include "nodriveby.inc" // Anti-driveby
#include "nopassengerglitch.inc" // Anti-passenger glitch

#include "minimodes.inc" // Minimode handler - starts/stops minimodes such as DM's and Races

#include "specialties.inc" // Player specialties, like medic, engineer, etc
#include "deathmsg.inc" // Performs death messages

#include "misc_commands.inc" // Miscellaneous useful commands
#include "external.inc" // A list of public functions which can be called externally

// Misc that needs to go last
#include "Functions\oneTimer.inc" // Functions here are needed to compensate for the amount of timers this gamemode has

//static FDMLoadComplete = 0;

main () {}

public OnGameModeInit() {
	// Crippled
	#if defined CRIPPLE_PLAYERS
		for (new i = CRIPPLE_PLAYERS; i < MAX_PLAYERS_EX; i++) {
			if (IsPlayerConnected(i)) {
				Kick(i);
			}
		}
	#endif
	
	// Set gamemode name
	new message[MAX_INPUT];
	format(message, sizeof(message), "Loading %s...", GAMEMODE_NAME);
	SendClientMessageToAll(0xFFFF00AA, message);
	
	#if defined CRIPPLE_PLAYERS
		format(message, sizeof(message), "%s v%s by %s (Crippled)", GAMEMODE_NAME, GAMEMODE_VERSION, GAMEMODE_AUTHOR);
	#else
		format(message, sizeof(message), "%s v%s by %s", GAMEMODE_NAME, GAMEMODE_VERSION, GAMEMODE_AUTHOR);
	#endif
	print("\n--------------------------------------");
	print(message);
	print("--------------------------------------\n");
	
	#if defined CRIPPLE_PLAYERS
		format(message, sizeof(message), "%s v%s (Crippled)", GAMEMODE_NAME, GAMEMODE_VERSION);
	#else
		format(message, sizeof(message), "%s v%s", GAMEMODE_NAME, GAMEMODE_VERSION);
	#endif
	SetGameModeText(message);
	
	// Initialize includes
	print("[FDM] Loading....");
	new time = GetTickCount();
	
	UpdateConnectedPlayers();
	
	//FsGamemodeInit();
	AntiCheatInit();
	CheckpointInit();
	SkinsInit();
	TimeInit();
	WeatherInit();
	GodSpawnInit();
	IsSpawnedInit();
	miscPauseEventsInit();
	misccmdInit();
	alarInit();
	
	AllowInteriorWeapons(1);
	ShowPlayerMarkers(1);
	ShowNameTags(1);
	
	print("[FDM] Loading settings..");
	SpawnsInit();
	WeaponsInit();
	
	PlayersInit();
	GangsInit();
	LevelsInit();
	SidechatInit();
	
	MoneyInit();
	XPZoneInit();
	SpecialtyInit();
	
	// 'Other' settings
	INI_Open(FILE_OTHER_SETTINGS);
	DriveByInit();
	PassengerGlitchInit();
	CombatInit();
	MinimodesSettingsInit();
	AntiCheatSettingsInit();
	INI_Close();
	
	print("[FDM] Loading vehicles..");
	VehiclesInit();
	
	print("[FDM] Loading minimodes..");
	MinimodesInit();
	SpecInit();
	
	#if defined CRIPPLE_PLAYERS
		format(message, sizeof(message), "%s v%s (Crippled) Loaded", GAMEMODE_NAME, GAMEMODE_VERSION);
	#else
		format(message, sizeof(message), "%s v%s Loaded", GAMEMODE_NAME, GAMEMODE_VERSION);
	#endif
	AddAdminLogLine(COLOUR_LOG, message);
	SendClientMessageToAll(0xFFFF00AA, message);
	
	#if defined CRIPPLE_PLAYERS
		format(message, sizeof(message), "Welcome to %s v%s (Crippled) by %s", GAMEMODE_NAME, GAMEMODE_VERSION, GAMEMODE_AUTHOR);
	#else
		format(message, sizeof(message), "Welcome to %s v%s by %s", GAMEMODE_NAME, GAMEMODE_VERSION, GAMEMODE_AUTHOR);
	#endif
	SendClientMessageToAll(0xFFDD00AA, message);
	
	// Perform autologins
	/*print("[FDM] Loading player accounts..");
	for (new i; i < MAX_PLAYERS; i++) {
		if (!IsPlayerConnected(i)) continue;
		StartGodSpawning(i, 12000);
		ResetPlayerWeapons(i);
		SetPlayerHealth(i, 100.0);
		players_OnPlayerConnect(i, 1);
	}*/
	
	InteriorsInit();
	OneTimerInit();
	gametextInit();
	
	printf("[FDM] Loading complete. Time Taken: %i ms.", GetTickCount() - time);
	//FDMLoadComplete = 1;
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnInit", "");
	#endif
	
	return 1;
}

public OnGameModeExit() {
	print("[FDM] Unloading....");
	new time = GetTickCount();
	
	new message[MAX_INPUT];
	format(message, sizeof(message), "Unloading %s...", GAMEMODE_NAME);
	SendClientMessageToAll(0xFFFF00AA, message);
	
	// Uninitialize gamemode
	print("[FDM] Unloading gamemode..");
	SpecExit();
	UnloadXPZones();
	CheckpointExit();
	SkinsExit();
	SpecialtyExit();
	MinimodesUnload();
	SidechatExit();
	miscPauseEventsExit();
	InteriorsExit();
	LevelsExit();
	CombatExit();
	PassengerGlitchExit();
	AntiCheatExit();
	
	// Perform logout's
	print("[FDM] Saving player accounts..");
	for (new i; i < MAX_PLAYERS; i++) {
		if (!IsPlayerConnected(i)) continue;
		/*DisablePlayerRaceCheckpoint(i);
		RemovePlayerMapIcon(i, RACE_ICON_ID);
		SetPlayerVirtualWorld(i, 0);
		TogglePlayerControllable(i, 1);*/
		
		if (!pData[i][pIsLoggedIn]) continue;
		players_OnPlayerDisconnect(i, -1);
	}
	
	print("[FDM] Cleaning up..");
	//FsGamemodeExit();
	//DestroyObjectsCreatedByMe();
	
	#if defined CRIPPLE_PLAYERS
		format(message, sizeof(message), "%s v%s (Crippled) Unloaded", GAMEMODE_NAME, GAMEMODE_VERSION);
	#else
		format(message, sizeof(message), "%s v%s Unloaded", GAMEMODE_NAME, GAMEMODE_VERSION);
	#endif
	AddAdminLogLine(COLOUR_LOG, message);
	SendClientMessageToAll(0xFFFF00AA, message);
	
	printf("[FDM] Unload complete. Time Taken: %i ms.", GetTickCount() - time);
	//FDMLoadComplete = 0;
	
	SetSpawnType(SPAWN_DEFAULT);
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnExit", "");
	#endif
	
	return 1;
}

/*public OnGameModeInit() {
	if (FDMLoadComplete) return 1;
	FDMLoadComplete = 1;
	// Player connects/disconnects are done automatically
	SidechatInit();
	LoadLevelsList();
	LoadXPZones();
	VehiclesInit();
	MinimodesInit();
	SpecialtyInit();
	SpecInit();
	return 1;
}

public OnGameModeExit() {
	if (!FDMLoadComplete) return 1;
	// Player connects/disconnects are done automatically
	FDMLoadComplete = 0;
	SpecExit();
	SidechatExit();
	MinimodesUnload();
	UnloadXPZones();
	VehiclesUnload();
	SpecialtyExit();
	return 1;
}*/

public OnPlayerRequestClass(playerid, classid) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	#if ENABLE_VEHICLE_WORLD_FIX
		fsgm_OnPlayerRequestClass(playerid);
	#endif
	
	if (players_OnPlayerRequestClass(playerid)) {
		OnPlayerLoaded(playerid);
	}
	skin_OnPlayerRequestClass(playerid);
	return 1;
}

OnPlayerLoaded(playerid) {
	weapons_OnPlayerLoaded(playerid);
	return 1;
}

public OnPlayerRequestSpawn(playerid) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	if (fdm_IsPlayerSelectingSkin(playerid)) return 1; // Don't execute spawn code upon the skin selection
	// xSetSpawnInfo(playerid, Float:X, Float:Y, Float:Z, Float:RotZ)
	skin_OnPlayerRequestSpawn(playerid);
	return 1;
}

public OnPlayerConnect(playerid) {
	// Crippled
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) {
			Kick(playerid);
			return 0;
		}
	#endif
	
	UpdateConnectedPlayers();
	
	if (!ac_OnPlayerConnect(playerid)) return 1;
	miscPause_OnPlayerConnect(playerid);
	players_OnPlayerConnect(playerid);
	weapons_OnPlayerConnect(playerid);
	stealth_OnPlayerConnect(playerid);
	//skin_OnPlayerConnect(playerid);
	spec_OnPlayerConnect(playerid);
	specialty_OnPlayerConnect(playerid);
	time_OnPlayerConnect(playerid);
	checkpoint_OnPlayerConnect(playerid);
	weather_OnPlayerConnect(playerid);
	
	#if ENABLE_VEHICLE_WORLD_FIX
		fsgm_OnPlayerConnect(playerid);
	#endif
	
	new tmpstr[MAX_INPUT];
	#if defined CRIPPLE_PLAYERS
		format(tmpstr, sizeof(tmpstr), "Welcome to %s v%s (Crippled) by %s", GAMEMODE_NAME, GAMEMODE_VERSION, GAMEMODE_AUTHOR); 
	#else
		format(tmpstr, sizeof(tmpstr), "Welcome to %s v%s by %s", GAMEMODE_NAME, GAMEMODE_VERSION, GAMEMODE_AUTHOR); 
	#endif
	SendClientMessage(playerid, COLOUR_HELP, tmpstr);
	
	return 1;
}

public OnPlayerDisconnect(playerid, reason) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	stealth_OnPlayerDisonnect(playerid);
	spec_OnPlayerDisconnect(playerid);
	interior_OnPlayerDisconnect(playerid);
	minimode_OnPlayerDisconnect(playerid);
	gangs_OnPlayerDisconnect(playerid);
	players_OnPlayerDisconnect(playerid, reason);
	levels_OnPlayerDisconnect(playerid);
	specialty_OnPlayerDisconnect(playerid);
	heal_OnPlayerDisconnect(playerid);
	repair_OnPlayerDisconnect(playerid);
	stealthing_OnPlayerDisconnect(playerid);
	pg_OnPlayerDisconnect(playerid);
	target_OnPlayerDisconnect(playerid);
	skin_OnPlayerDisconnect(playerid);
	ac_OnPlayerDisconnect(playerid);
	spawns_OnPlayerDisconnect(playerid);
	zones_OnPlayerDisconnect(playerid);
	misccmd_OnPlayerDisconnect(playerid);
	godspawn_OnPlayerDisconnect(playerid);
	sidechat_OnPlayerDisconnect(playerid);
	combat_OnPlayerDisconnect(playerid);
	weather_OnPlayerDisconnect(playerid);
	gametext_OnPlayerDisconnect(playerid);
	alarOnPlayerDisconnect(playerid);
	
	#if ENABLE_MTA_STYLE_CHECKPOINTS
		checkpoint_OnPlayerDisconnect(playerid);
	#endif
	
	UpdateConnectedPlayers(playerid);
	
	return 1;
}

OnPlayerLogin(playerid) {
	sidechat_OnPlayerLogin(playerid);
	levels_OnPlayerLogin(playerid);
	weapons_OnPlayerLogin(playerid);
	gangs_OnPlayerLogin(playerid);
	//bounty_OnPlayerLogin(playerid);
	specialty_OnPlayerLogin(playerid);
	skin_OnPlayerLogin(playerid, pData[playerid][pSkin]);
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnPlayerLogin", "i", playerid);
	#endif
	return 1;
}

OnPlayerRegister(playerid) {
	sidechat_OnPlayerLogin(playerid);
	levels_OnPlayerLogin(playerid);
	money_OnPlayerRegister(playerid);
	weapons_OnPlayerRegister(playerid);
	skin_OnPlayerRegister(playerid);
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnPlayerRegister", "i", playerid);
	#endif
	return 1;
}

OnPlayerLevelChange(playerid, oldlevel, newlevel) {
	weapons_OnPlayerLevelChange(playerid, oldlevel, newlevel);
	if (!IsPlayerInMinimode(playerid)) {
		spawns_OnPlayerLevelChange(playerid);
	}
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnPlayerLevelChange", "iii", playerid, oldlevel, newlevel);
	#endif
	return 1;
}

OnPlayerUnPause(playerid) {
	time_OnPlayerUnPause(playerid);
	return 1;
}

OnPlayerSpectate(playerid, specid) {
	spec_OnPlayerSpectate(playerid, specid);
	mode_OnPlayerSpectate(playerid, specid);
	race_OnPlayerSpectate(playerid, specid);
	dmffa_OnPlayerSpectate(playerid, specid);
	dmteam_OnPlayerSpectate(playerid, specid);
	ctf_OnPlayerSpectate(playerid, specid);
	levels_OnPlayerSpectate(playerid, specid);
	specialty_OnPlayerSpectate(playerid, specid);
	return 1;
}

OnPlayerSpectateChangePlayer(playerid, oldspecid, newspecid) {
	mode_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid);
	race_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid);
	dmffa_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid);
	dmteam_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid);
	ctf_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid);
	levels_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid);
	spl_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid);
	return 1;
}

OnPlayerUnspectate(playerid, specid, unloading = 0) {
	if (!unloading) {
		if (!spec_OnPlayerUnspectate(playerid)) return 0;
	}
	mode_OnPlayerUnspectate(playerid, specid);
	race_OnPlayerUnspectate(playerid, specid);
	dmffa_OnPlayerUnspectate(playerid, specid);
	dmteam_OnPlayerUnspectate(playerid, specid);
	ctf_OnPlayerUnspectate(playerid, specid);
	levels_OnPlayerUnspectate(playerid, specid);
	specialty_OnPlayerUnspectate(playerid, specid);
	return 1;
}

OnPlayerVolumeChange(playerid, Float:newvolume, Float:oldvolume) {
	#pragma unused newvolume, oldvolume
	minimode_OnPlayerVolumeChange(playerid);
	return 1;
}

OnMinimodeStart(modeid, players[]) {
	
	#if ENABLE_CALLBACKS
		new sendPlayers[MAX_PLAYERS + 1];
		for (new i; i < MAX_PLAYERS; i++) {
			if (players[i]) {
				sendPlayers[i] = 1;
			} else {
				sendPlayers[i] = 2;
			}
		}
		CallRemoteFunction("fdmOnMinimodeStart", "is", modeid, sendPlayers);
	#endif
	return 1;
}

OnMinimodeStop(modeid) {
	spec_OnMinimodeStop(modeid);
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnMinimodeStop", "i", modeid);
	#endif
	return 1;
}

OnPlayerJoinMinimode(playerid, modeid) {
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnPlayerJoinMinimode", "ii", playerid, modeid);
	#endif
	return 1;
}

OnPlayerLeaveMinimode(playerid, modeid) {
	spec_OnPlayerLeaveMinimode(playerid, modeid);
	
	// ALAR stuff
	if (GetAdminState(playerid) & ADMIN_STATE_JAILED || IsAdminSpectating(playerid)) {
		ClearAdminSpawnData(playerid);
	}
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnPlayerLeaveMinimode", "ii", playerid, modeid);
	#endif
	return 1;
}

OnPlayerStartSelection(playerid) {
	spawns_OnPlayerStartSelection(playerid);
	return 1;
}

OnPlayerStopSelection(playerid) {
	if (!player_OnPlayerStopSelection(playerid)) return 0;
	spawns_OnPlayerStopSelection(playerid);
	return 1;
}

public OnPlayerSpawn(playerid) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	ac_OnPlayerSpawn(playerid);
	
	misc_OnPlayerSpawn(playerid);
	time_OnPlayerSpawn(playerid);
	weather_OnPlayerSpawn(playerid);
	
	if (!IsPlayerInMinimode(playerid)) {
		gangs_OnPlayerSpawn(playerid);
		stealthing_OnPlayerSpawn(playerid);
	}
	
	if (skin_OnPlayerSpawn(playerid)) return 1;
	
	if (spec_OnPlayerSpawn(playerid)) return 1;
	spawns_OnPlayerSpawn(playerid);
	specialty_OnPlayerSpawn(playerid);
	target_OnPlayerSpawn(playerid);
	
	if (minimode_OnPlayerSpawn(playerid)) return 1;
	
	gangs_OnPlayerSpawn(playerid);
	godspawn_OnPlayerSpawn(playerid);
	levels_OnPlayerSpawn(playerid);
	weapons_OnPlayerSpawn(playerid);
	
	//Tricky way to get hackers who regen health
	if (!pData[playerid][pIsLoggedIn]) SetPlayerHealth(playerid, 99.0);
	
	return 1;
}

public OnPlayerDeath(playerid, killerid, reason) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	new noloss;
	if (!ac_OnPlayerDeath(playerid, killerid, reason) ||
		!combat_OnPlayerDeath(playerid) ||
		!misc_OnPlayerDeath(playerid) ||
		!godspawn_OnPlayerDeath(playerid) ||
		!spec_OnPlayerDeath(playerid, killerid) ||
		!time_OnPlayerDeath(playerid) ||
		!deathmsg_OnPlayerDeath(playerid, killerid, reason, noloss) ||
		
		!players_OnPlayerDeath(playerid, killerid) ||
		!spawns_OnPlayerDeath(playerid) ||
		minimode_OnPlayerDeath(playerid, killerid, reason) ||
		
		!gangs_OnPlayerDeath(playerid, killerid) ||
		
		noloss ||
		
		!bounty_OnPlayerDeath(playerid, killerid) ||
		!levels_OnPlayerDeath(playerid, killerid) ||
		!money_OnPlayerDeath(playerid, killerid)) {
		
		return 1;
	}
	
	return 1;
}

public OnPlayerGiveDamage(playerid, damagedid, Float: amount, weaponid) {
	ac_OnPlayerGiveDamage(playerid, damagedid, amount, weaponid);
	return 1;
}

public OnPlayerTakeDamage(playerid, issuerid, Float: amount, weaponid) {
	ac_OnPlayerTakeDamage(playerid, issuerid, weaponid);
	return 1;
}

public OnVehicleSpawn(vehicleid) {
	ac_OnVehicleSpawn(vehicleid);
	vehicles_OnVehicleSpawn(vehicleid);
	if (minimode_OnVehicleSpawn(vehicleid)) return 1;
	return 1;
}

public OnVehicleDeath(vehicleid, killerid) {
	if (minimode_OnVehicleDeath(vehicleid)) return 1;
	return 1;
}

public OnVehicleMod(playerid, vehicleid, componentid) {
	if (!ac_OnVehicleMod(playerid, vehicleid, componentid)) return 0;
	return 1;
}

public OnVehiclePaintjob(playerid, vehicleid, paintjobid) {
	if (!ac_OnVehiclePaintjob(playerid, paintjobid)) return 0;
	return 1;
}

/*public OnVehicleDamageStatusUpdate(vehicleid, playerid) {
	if (!ac_OnVehicleDamageStatusUpdate(vehicleid, playerid)) return 0;
	return 1;
}*/

public OnPlayerText(playerid, text[]) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	if (!players_OnPlayerText(playerid)) return 0;
	if (!gangs_OnPlayerText(playerid, text)) return 0;
	return 1;
}

/*public OnPlayerPrivmsg(playerid, recieverid, text[]) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	misccmd_OnPlayerPrivmsg(playerid, recieverid);
	return 1;
}*/

public OnPlayerCommandText(playerid, cmdtext[]) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	if (!pData[playerid][pFirstConnect]) return 1;
	
	if (kcmd_OnPlayerCommandText(playerid, cmdtext)) {
		return 1;
	} else {
		// misc stuff
		new cmd[32 - 5], idx;
		splitcpy(cmd, cmdtext, idx, ' ');
		
		if (gangs_OnPlayerCommandText(playerid, cmd) ||
			weapons_OnPlayerCommandText(playerid, cmd)) {
			
			return 1;
		}
	}
	
	return 0;
}

public OnPlayerUpdate(playerid) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	//skin_OnPlayerUpdate(playerid);
	misc_OnPlayerUpdate(playerid);
	heal_OnPlayerUpdate(playerid);
	#if ENABLE_MTA_STYLE_CHECKPOINTS
		checkpoint_OnPlayerUpdate(playerid);
	#endif
	combat_OnPlayerUpdate(playerid);
	
	race_OnPlayerUpdate(playerid);
	derby_OnPlayerUpdate(playerid);
	
	if (!ac_OnPlayerUpdate(playerid) ||
		!alarOnPlayerUpdate(playerid) ||
		!driveby_OnPlayerUpdate(playerid)) return 0;
	
	return 1;
}

public OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	spec_OnPlayerInteriorChange(playerid, newinteriorid);
	interior_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
	return 1;
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	stealth_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
	skin_OnPlayerKeyStateChange(playerid, newkeys);
	heal_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
	spec_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
	return 1;
}

public OnPlayerEnterVehicle(playerid, vehicleid, ispassenger) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	ac_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
	pg_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
	return 1;
}

public OnPlayerStateChange(playerid, newstate, oldstate) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	spawns_OnPlayerStateChange(playerid, newstate);
	if (!ac_OnPlayerStateChange(playerid, newstate, oldstate)) return 1;
	heal_OnPlayerStateChange(playerid, newstate, oldstate);
	repair_OnPlayerStateChange(playerid, newstate, oldstate);
	stealth_OnPlayerStateChange(playerid, newstate, oldstate);
	godspawn_OnPlayerStateChange(playerid, newstate, oldstate);
	driveby_OnPlayerStateChange(playerid, newstate, oldstate);
	spec_OnPlayerStateChange(playerid, newstate, oldstate);
	minimode_OnPlayerStateChange(playerid, newstate, oldstate);
	return 1;
}

public OnPlayerEnterCheckpoint(playerid) {checkpoint_OnPlayerEnterCP(playerid); return 1;}
kOnPlayerEnterCheckpoint(playerid, checkpointid) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	if (!money_OnPlayerEnterCheckpoint(playerid, checkpointid) ||
		!weapons_OnPlayerEnterCheckpoint(playerid, checkpointid) ||
		!mode_OnPlayerEnterCheckpoint(playerid, checkpointid)) {
		
		return 0;
	}
	return 1;
}

public OnPlayerLeaveCheckpoint(playerid) {checkpoint_OnPlayerLeaveCP(playerid); return 1;}
kOnPlayerLeaveCheckpoint(playerid, checkpointid) {
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	if (!mode_OnPlayerLeaveCheckpoint(playerid, checkpointid)) return 0;
	return 1;
}

#if ENABLE_MTA_STYLE_CHECKPOINTS
kOnPlayerEnterRaceCheckpoint(playerid) {
#else
public OnPlayerEnterRaceCheckpoint(playerid) {
#endif
	#if defined CRIPPLE_PLAYERS
		if (playerid >= CRIPPLE_PLAYERS) return 0;
	#endif
	
	if (IsPlayerInMinimode(playerid)) {
		if (minimode_OnPlayerEnterRaceCP(playerid)) return 1;
	}
	return 1;
}

#if ENABLE_MTA_STYLE_CHECKPOINTS
kOnPlayerLeaveRaceCheckpoint(playerid) {
#else
public OnPlayerLeaveRaceCheckpoint(playerid) {
#endif
	#pragma unused playerid
	return 1;
}

public OnPlayerStreamIn(playerid, forplayerid) {
	spec_OnPlayerStream(playerid, forplayerid);
	return 1;
}

public OnPlayerStreamOut(playerid, forplayerid) {
	spec_OnPlayerStream(playerid, forplayerid);
	return 1;
}

public OnVehicleStreamIn(vehicleid, forplayerid) {
	spec_OnVehicleStream(vehicleid, forplayerid);
	return 1;
}

public OnVehicleStreamOut(vehicleid, forplayerid) {
	spec_OnVehicleStream(vehicleid, forplayerid);
	return 1;
}

#endinput

public OnPlayerPickUpPickup(playerid, pickupid) {
	/*if (race_OnPlayerPickUpPickup(playerid, pickupid) ||
		derby_OnPlayerPickUpPickup(playerid, pickupid)) {
		
		return 1;
	}*/
	return 1;
}

public OnPlayerExitVehicle(playerid, vehicleid) {
	return 1;
}

public OnVehicleRespray(playerid, vehicleid, color1, color2) {
	return 1;
}

public OnRconCommand(cmd[]) {
	return 1;
}

public OnObjectMoved(objectid) {
	return 1;
}

public OnPlayerObjectMoved(playerid, objectid) {
	return 1;
}

public OnPlayerSelectedMenuRow(playerid, row) {
	return 1;
}

public OnPlayerExitedMenu(playerid) {
	return 1;
}

