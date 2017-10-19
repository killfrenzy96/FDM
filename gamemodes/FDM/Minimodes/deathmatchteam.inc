#include <a_samp>

#define FILE_DEATHMATCH_TEAM "FDM/Minimodes/DeathMatchTeamList.ini"
#define FILE_DEATHMATCH_TEAM_MAPS "FDM/Minimodes/Deathmatch_Team/"

#define MAX_DM_TEAMS 4
#if defined CRIPPLE_PLAYERS
	#define MAX_DM_TEAM_MAPS 6
#else
	#define MAX_DM_TEAM_MAPS 32 // Maximum team deathmatches which can be loaded
#endif
#define MAX_DM_TEAM_SPAWNS 64
#define MAX_DM_TEAM_BOUNDS_TIME 5
#define MAX_DM_TEAM_BOUND_POLYS 32

#define MAX_TEAM_NAME 12

#define COLOUR_TEAMCHAT 0x6600CCAA

#if !defined MAX_WEAPON_ID
	#define MAX_WEAPON_ID 47
#endif

enum dmteamSpawnInfo {
	dmtTeamID,
	Float:dmtX,
	Float:dmtY,
	Float:dmtZ,
	Float:dmtRot
}
enum dmteamBoundsRadiusInfo {
	dmtrUsed,
	Float:dmtCentreX,
	Float:dmtCentreY,
	Float:dmtCentreZ,
	Float:dmtRadius,
}
enum dmteamBoundsRectangleInfo {
	dmteUsed,
	Float:dmteMinX,
	Float:dmteMinY,
	Float:dmteMinZ,
	Float:dmteMaxX,
	Float:dmteMaxY,
	Float:dmteMaxZ
}
enum dmteamInfo {
	dmtIsLoaded,
	dmtModeID,
	dmtStartTime,
	dmtRunTime,
	dmtWinXP,
	dmtWinCash,
	dmtKillXP,
	dmtTeamCount,
	dmtBoundsRadius[dmteamBoundsRadiusInfo],
	dmtBoundsRectangle[dmteamBoundsRectangleInfo],
	dmtBoundPoly[E_POLYGON],
	dmtWeapons[MAX_WEAPON_ID],
	//dmtPlayers[MAX_PLAYERS],
	dmtCountDown,
	Float:dmtPlayerHealth,
	Float:dmtPlayerArmour,
	dmtEnemyBlips,
	dmtBlockJoinMidGame
}
enum dmteamPlayerStatsInfo {
	dmtpMapID,
	dmtpTeamID,
	dmtpKills,
	dmtpDeaths,
	dmtpBoundsTime,
	dmtIsTextDrawOn,
	Text:dmtpTime,
	//Text:dmtpTeam,
	Text:dmtText,
	dmtTextString[MAX_INPUT]
}
enum dmteamTeamInfo {
	dmtColour,
	dmtKills,
	dmtDeaths,
	dmtPlayerCount
}
enum dmteamColoursInfo {
	dmtcName[MAX_TEAM_NAME],
	dmtcColour,
	dmtcGametextColour[4]
}
new dmtPStats[MAX_PLAYERS][dmteamPlayerStatsInfo];
new dmtTStats[MAX_DM_TEAM_MAPS][MAX_DM_TEAMS][dmteamTeamInfo];
new dmtMaps[MAX_DM_TEAM_MAPS][dmteamInfo];
new dmtSpawns[MAX_DM_TEAM_MAPS][MAX_DM_TEAM_SPAWNS][dmteamSpawnInfo];
new dmtColours[MAX_DM_TEAMS][dmteamColoursInfo] = {
	{"Red", 0xFF0000AA, "~r~"},
	{"Blue", 0x0000FFAA, "~b~"},
	{"Green", 0x00FF00AA, "~g~"},
	{"Yellow", 0xFFFF00AA, "~y~"}
};
//new dmtTimer;

new dmtInProgress[MAX_DM_TEAM_MAPS + 1];

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

DeathmatchTeamInit() {
	for (new i; i < MAX_DM_TEAM_MAPS; i++) {
		dmtMaps[i][dmtModeID] = INVALID_MODE_ID;
	}
	for (new i; i < MAX_PLAYERS; i++) {
		dmtPStats[i][dmtpMapID] = INVALID_MAP_ID;
		dmtPStats[i][dmtpTeamID] = INVALID_TEAM_ID;
	}
	dmtInProgress[0] = INVALID_MAP_ID;
	
	LoadDeathMatchTeamMaps();
	
	kEnableTimer(8); // dmtTimer = SetTimer("fdm_dmteam_checkDM", 925, true);
	kEnableTimer(23); // fdm_dmteam_checkDMFast
}

DeathmatchTeamUnload() {
	// Unload all variables
	new tmp1[dmteamPlayerStatsInfo];
	new tmp2[dmteamTeamInfo];
	new tmp3[dmteamInfo];
	new tmp4[dmteamSpawnInfo];
	
	dmteamUpdatePlayerInfoUnload();
	
	for (new a; a < MAX_PLAYERS; a++) {
		dmtPStats[a] = tmp1;
	}
	for (new d; d < MAX_DM_TEAM_MAPS; d++) {
		dmtMaps[d] = tmp3;
		for (new b; b < MAX_DM_TEAMS; b++) {
			dmtTStats[d][b] = tmp2;
		}
		for (new e; e < MAX_DM_TEAM_MAPS; e++) {
			dmtSpawns[d][e] = tmp4;
		}
	}
	
	kDisableTimer(8); // KillTimer(dmtTimer);
	kDisableTimer(23); // fdm_dmteam_checkDMFast
}

LoadDeathMatchTeamMaps() {
	if (!INI_Exist(FILE_DEATHMATCH_TEAM)) {
		printf("[FDM] WARNING: DMTeam Map List Fail: '%s' - File not found.", FILE_DEATHMATCH_TEAM);
		return 0;
	}
	
	if (!INI_Open(FILE_DEATHMATCH_TEAM)) {
		printf("[FDM] WARNING: DMTeam List Fail: '%s' - File cannot be opened.", FILE_DEATHMATCH_TEAM);
		return 0;
	}
	
	new dmFiles[MAX_DM_TEAM_MAPS][MAX_FILENAME];
	new dmcount;
	new dmsLoaded;
	//new key[INI_MAX_KEY];
	
	new i3;
	for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
		if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "DMTEAM", false)) {
			format(dmFiles[i3], MAX_FILENAME, "%s%s", FILE_DEATHMATCH_TEAM_MAPS, gCache[l][E_VALUE]);
			i3++;
			dmcount = i3;
		}
		if (i3 >= MAX_DM_TEAM_MAPS) {
			printf("[FDM] WARNING: Maximum derby maps reached (%i derbies)", MAX_DM_TEAM_MAPS);
			break; // Maximum dmteams reached
		}
	}
	/*for (new i; i < MAX_DM_TEAM_MAPS; i++) { // Read DM list
		format(key, sizeof(key), "DMTEAM%i", i);
		
		if (INI_ReadString(dmFiles[i], key)) {
			// Add the file path to the beginning
			format(dmFiles[i], MAX_FILENAME, "%s%s", FILE_DEATHMATCH_TEAM_MAPS, dmFiles[i]);
		} else {
			dmcount = i;
			break; // End of file
		}
	}*/
	INI_Close();
	
	if (!dmcount) return 0; // There are no DM's to load
	
	// Load deathmatch map files
	for (new i; i < dmcount; i++) {
		// Load deathmatch files
		if (!INI_Exist(dmFiles[i])) {
			printf("[FDM] WARNING: DMTeam Load Fail: '%s' - File not found.", dmFiles[i]);
			continue;
		}
		if (!INI_Open(dmFiles[i])) {
			printf("[FDM] WARNING: DMTeam Load Fail: '%s' - File cannot be opened.", dmFiles[i]);
			continue;
		}
		
		new dmname[MAX_MODE_NAME];
		new tmpstr[MAX_STRING];
		//new minlevel;
		//new cost;
		new idx;
		
		INI_ReadString(dmname, "Name");
		
		if (!dmname[0]) { // Invalid DM name
			INI_Close();
			printf("[FDM] WARNING: DMTeam Load Fail: '%s' - File is corrupt/invalid.", dmFiles[i]);
			continue;
		}
		/* Minimode data
		minlevel = INI_ReadInt("MinLevel");
		cost = INI_ReadInt("JoinCost");
		
		INI_ReadString(tmpstr, "WaitLocation"); // Waiting location
		idx = 0;
		new Float:x = floatstr(split(tmpstr, idx, ','));
		new Float:y = floatstr(split(tmpstr, idx, ','));
		new Float:z = floatstr(split(tmpstr, idx, ','));*/
		
		// DM Team data
		dmtMaps[i][dmtRunTime] = INI_ReadInt("RunTime");
		dmtMaps[i][dmtWinXP] = INI_ReadInt("WinXP");
		dmtMaps[i][dmtWinCash] = INI_ReadInt("WinCash");
		dmtMaps[i][dmtKillXP] = INI_ReadInt("KillXP");
		dmtMaps[i][dmtTeamCount] = INI_ReadInt("TeamCount");
		dmtMaps[i][dmtEnemyBlips] = INI_ReadInt("EnemyBlips");
		dmtMaps[i][dmtBlockJoinMidGame] = INI_ReadInt("BlockJoinMidGame");
		
		dmtMaps[i][dmtPlayerHealth] = INI_ReadFloat("PlayerHealth");
		dmtMaps[i][dmtPlayerArmour] = INI_ReadFloat("PlayerArmour");
		if (dmtMaps[i][dmtPlayerHealth] == 0.0) dmtMaps[i][dmtPlayerHealth] = 100.0;
		
		if (dmtMaps[i][dmtTeamCount] < 2) dmtMaps[i][dmtTeamCount] = 2;
		if (dmtMaps[i][dmtTeamCount] > MAX_DM_TEAMS) dmtMaps[i][dmtTeamCount] = MAX_DM_TEAMS;
		
		if (INI_ReadString(tmpstr, "Bounds")) { // DM Boundaries
			idx = 0;
			dmtMaps[i][dmtBoundsRadius][dmtrUsed] = 1;
			dmtMaps[i][dmtBoundsRadius][dmtCentreX] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRadius][dmtCentreY] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRadius][dmtCentreZ] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRadius][dmtRadius] = floatstr(split(tmpstr, idx, ','));
		} else {
			dmtMaps[i][dmtBoundsRadius][dmtrUsed] = 0;
		}
		
		idx = 0;
		if (INI_ReadString(tmpstr, "BoundRectangle")) { // DM Boundaries
			dmtMaps[i][dmtBoundsRectangle][dmteUsed] = 1;
			dmtMaps[i][dmtBoundsRectangle][dmteMinX] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRectangle][dmteMinY] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRectangle][dmteMinZ] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRectangle][dmteMaxX] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRectangle][dmteMaxY] = floatstr(split(tmpstr, idx, ','));
			dmtMaps[i][dmtBoundsRectangle][dmteMaxZ] = floatstr(split(tmpstr, idx, ','));
			
			if (dmtMaps[i][dmtBoundsRectangle][dmteMinX] > dmtMaps[i][dmtBoundsRectangle][dmteMaxX])
				floatswap(dmtMaps[i][dmtBoundsRectangle][dmteMinX], dmtMaps[i][dmtBoundsRectangle][dmteMaxX]);
			
			if (dmtMaps[i][dmtBoundsRectangle][dmteMinY] > dmtMaps[i][dmtBoundsRectangle][dmteMinY])
				floatswap(dmtMaps[i][dmtBoundsRectangle][dmteMinY], dmtMaps[i][dmtBoundsRectangle][dmteMinY]);
			
			if (dmtMaps[i][dmtBoundsRectangle][dmteMaxZ] > dmtMaps[i][dmtBoundsRectangle][dmteMaxZ])
				floatswap(dmtMaps[i][dmtBoundsRectangle][dmteMaxZ], dmtMaps[i][dmtBoundsRectangle][dmteMaxZ]);
			
		} else {
			dmtMaps[i][dmtBoundsRectangle][dmteUsed] = 0;
		}
		
		new i2;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "BoundPolyPoint", false)) {
				idx = 0;
				dmtMaps[i][dmtBoundPoly][E_VERTEX_X][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmtMaps[i][dmtBoundPoly][E_VERTEX_Y][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_VERTICIES) break; // Maximum polys reached
		}
		dmtMaps[i][dmtBoundPoly][E_VERTICIES] = i2;
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "WPN", false)) {
				idx = 0;
				new weaponidx = strval(split(gCache[l][E_VALUE], idx, ','));
				dmtMaps[i][dmtWeapons][weaponidx] = strval(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_WEAPON_ID) break; // Maximum weapons reached
		}
		/*for (new p; p < MAX_WEAPON_ID; p++) { // Weapons in deathmatch
			format(key, sizeof(key), "W%i", p);
			if (!INI_ReadString(tmpstr, key)) break; // End of weapons
			
			idx = 0;
			new weaponidx = strval(split(tmpstr, idx, ','));
			dmtMaps[i][dmtWeapons][weaponidx] = strval(split(tmpstr, idx, ','));
		}*/
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "SPAWN", false)) {
				idx = 0;
				dmtSpawns[i][i2][dmtTeamID] = strval(split(gCache[l][E_VALUE], idx, ','));
				dmtSpawns[i][i2][dmtX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmtSpawns[i][i2][dmtY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmtSpawns[i][i2][dmtZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				dmtSpawns[i][i2][dmtRot] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_DM_TEAM_SPAWNS) break; // Maximum spawns reached
		}
		/*for (new s; s < MAX_DM_TEAM_SPAWNS; s++) { // Deathmatch spawn loations
			format(key, sizeof(key), "Spawn%i", s);
			if (!INI_ReadString(tmpstr, key)) break; // End of spawns
			
			idx = 0;
			dmtSpawns[i][s][dmtTeamID] = strval(split(tmpstr, idx, ','));
			dmtSpawns[i][s][dmtX] = floatstr(split(tmpstr, idx, ','));
			dmtSpawns[i][s][dmtY] = floatstr(split(tmpstr, idx, ','));
			dmtSpawns[i][s][dmtZ] = floatstr(split(tmpstr, idx, ','));
			dmtSpawns[i][s][dmtRot] = floatstr(split(tmpstr, idx, ','));
		}*/
		
		// Deathmatch map reading complete
		//format(dmname, sizeof(dmname), "Deathmatch Team: %s", dmname);
		//dmtMaps[i][dmtModeID] = CreateMinimode(MODE_TYPE_DMTEAM, dmname, i, minlevel, cost, x, y, z);
		dmtMaps[i][dmtModeID] = LoadMinimode(MODE_TYPE_DMTEAM, dmname, i, "", 2, MAX_PLAYERS, 1);
		INI_Close();
		
		if (dmtMaps[i][dmtModeID] != INVALID_MODE_ID) {
			dmsLoaded++;
		}
	}
	
	printf("[FDM] Deathmatch Team's Loaded: %i", dmsLoaded);
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// OPTIMISATIONS
//-----------------------------------------------------------------------------------------------------

UpdateDMTeamInProgress() {
	new indexLoaded;
	
	for (new mapid; mapid < MAX_DM_TEAM_MAPS; mapid++) {
		if (dmtMaps[mapid][dmtIsLoaded]) {
			dmtInProgress[indexLoaded] = mapid;
			indexLoaded++;
		}
	}
	
	dmtInProgress[indexLoaded] = INVALID_MAP_ID;
	return 1;
}

#define LoopDMTeamLoaded(%1) for (new mmc, %1; dmtInProgress[mmc] != INVALID_MAP_ID; mmc++) if ((%1 = dmtInProgress[mmc]) || true)


//-----------------------------------------------------------------------------------------------------
// ACTUAL TEAM DEATHMATCHES
//-----------------------------------------------------------------------------------------------------

dmteam_IsJoinable(mapid) {
	if (!dmtMaps[mapid][dmtIsLoaded]) return 0;
	if (GetTickCount() - dmtMaps[mapid][dmtStartTime] > dmtMaps[mapid][dmtRunTime] * 500) return 0;
	return 1;
}

dmteam_ShowModeInfo(mapid, playerid) {
	new msg[MAX_STRING];
	
	// Show blipmode/health/armour
	if (dmtMaps[mapid][dmtEnemyBlips] == DM_BLIP_STEALTH) {
		msg = "Blips: Stealth";
	} else if (dmtMaps[mapid][dmtEnemyBlips] == DM_BLIP_SHOWN) {
		msg = "Blips: Always Visible";
	} else {
		msg = "Blips: Hidden";
	}
	
	format(msg, sizeof(msg), "%s | Health: %0.1f | Armour: %0.1f | Time Limit: %s",
		msg, dmtMaps[mapid][dmtPlayerHealth], dmtMaps[mapid][dmtPlayerArmour], GetTimeString(dmtMaps[mapid][dmtRunTime] * 1000, 0));
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	// Show weapons
	msg = "Weapons: ";
	new weaponcount;
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!dmtMaps[mapid][dmtWeapons][i]) continue;
		strcat(msg, ReturnWeaponName(i));
		
		if (!IsWeaponMelee(i)) {
			new strammo[MAX_REAL + 2];
			format(strammo, sizeof(strammo), "(%i)", dmtMaps[mapid][dmtWeapons][i]);
			strcat(msg, strammo);
		}
		
		strcat(msg, ", ");
		
		weaponcount++;
	}
	if (weaponcount) {
		msg[strlen(msg) - 2] = 0;
		SendWrappedMessageToPlayer(playerid, COLOUR_HELP, msg);
	} else {
		SendClientMessage(playerid, COLOUR_HELP, "Weapons: None");
	}
	
	return 1;
}

dmteam_OnStart(mapid, players[], playercount) {
	if (dmtMaps[mapid][dmtIsLoaded]) return 0;
	if (playercount < 2) return 0; // Not enough players
	
	dmtMaps[mapid][dmtIsLoaded] = 1;
	dmtMaps[mapid][dmtStartTime] = GetTickCount();
	dmtMaps[mapid][dmtCountDown] = COUNTDOWN_TIME;
	
	UpdateDMTeamInProgress();
	
	// Reset team data
	for (new i; i < MAX_DM_TEAMS; i++) {
		dmtTStats[mapid][i][dmtColour] = dmtColours[i][dmtcColour];
		dmtTStats[mapid][i][dmtKills] = 0;
		dmtTStats[mapid][i][dmtDeaths] = 0;
	}
	
	LoopPlayers(i) {
		if (players[i]) {
			dmteam_OnPlayerEnter(mapid, i);
		}
	}
	
	return 1;
}

dmteam_OnStop(mapid) {
	if (!dmtMaps[mapid][dmtIsLoaded]) return 0;
	dmtMaps[mapid][dmtIsLoaded] = 0;
	
	UpdateDMTeamInProgress();
	
	new winnerteam = dmteamGetWinnerTeamID(mapid);
	
	// Announce winning team
	new message[MAX_INPUT];
	if (winnerteam == INVALID_TEAM_ID) {
		format(message, sizeof(message), "%s: No team has won this deathmatch.",
			modes[dmtMaps[mapid][dmtModeID]][mName]);
		SendClientMessageToAll(COLOUR_TITLE, message);
		
		new xp = MinimodeProcessFlagsXP(dmtMaps[mapid][dmtModeID], dmtMaps[mapid][dmtWinXP] / 2);
		new money = MinimodeProcessFlagsMoney(dmtMaps[mapid][dmtModeID], dmtMaps[mapid][dmtWinCash] / 2);
		
		LoopPlayers(i) {
			//if (!dmtMaps[mapid][dmtPlayers][i]) continue;
			if (mpData[i][mpModeID] != dmtMaps[mapid][dmtModeID]) continue;
			
			SendXPMessageToPlayer(i, GivePlayerXP(i, xp));
			GivePlayerMoney(i, money);
			SendMoneyMessageToPlayer(i, money);
		}
	} else {
		format(message, sizeof(message), "%s: Team %s has won with a total of %i kills, %i deaths.",
			modes[dmtMaps[mapid][dmtModeID]][mName], dmtColours[winnerteam][dmtcName], dmtTStats[mapid][winnerteam][dmtKills], dmtTStats[mapid][winnerteam][dmtDeaths]);
		SendClientMessageToAll(dmtColours[winnerteam][dmtcColour], message);
		
		new xp = MinimodeProcessFlagsXP(dmtMaps[mapid][dmtModeID], dmtMaps[mapid][dmtWinXP]);
		new money = MinimodeProcessFlagsMoney(dmtMaps[mapid][dmtModeID], dmtMaps[mapid][dmtWinCash]);
		
		LoopPlayers(i) {
			//if (!dmtMaps[mapid][dmtPlayers][i]) continue;
			if (mpData[i][mpModeID] != dmtMaps[mapid][dmtModeID]) continue;
			if (dmtPStats[i][dmtpTeamID] == winnerteam) {
				SendXPMessageToPlayer(i, GivePlayerXP(i, xp));
				GivePlayerMoney(i, money);
				SendMoneyMessageToPlayer(i, money);
			} else {
				SendXPMessageToPlayer(i, GivePlayerXP(i, xp / 2));
				GivePlayerMoney(i, money / 2);
				SendMoneyMessageToPlayer(i, money / 2);
			}
		}
	}
	
	LoopPlayers(p) {
		//if (!dmtMaps[mapid][dmtPlayers][p]) continue;
		if (mpData[p][mpModeID] != dmtMaps[mapid][dmtModeID]) continue;
		
		dmteam_OnPlayerExit(mapid, p);
		
		new Float:x, Float:y, Float:z;
		GetPlayerPos(p, x, y, z);
		PlayerPlaySound(p, SOUND_FINISH, x, y, z);
	}
	
	for (new i; i < MAX_DM_TEAMS; i++) {
		dmtTStats[mapid][i][dmtPlayerCount] = 0;
	}
	return 1;
}

dmteam_OnPlayerEnter(mapid, playerid) {
	if (dmtPStats[playerid][dmtpMapID] != INVALID_MAP_ID) return 0;
	
	if (dmtMaps[mapid][dmtBlockJoinMidGame] && !dmtMaps[mapid][dmtCountDown]) return 0; // Block join mid game
	if (GetTickCount() - dmtMaps[mapid][dmtStartTime] > dmtMaps[mapid][dmtRunTime] * 500) return 0; // Block join mid game when half finished
	
	dmtPStats[playerid][dmtpMapID] = mapid;
	dmtPStats[playerid][dmtpBoundsTime] = 0;
	//dmtMaps[mapid][dmtPlayers][playerid] = 1;
	dmtSetRandomSpawn(mapid, playerid);
	
	if (dmtMaps[mapid][dmtCountDown]) {
		TogglePlayerControllable(playerid, 0); // Freeze player
		EnableGodmode(playerid);
	} else {
		StartGodSpawning(playerid, DM_GODE_TIME, 0);
	}
	
	SetPlayerVirtualWorld(playerid, modes[dmtMaps[mapid][dmtModeID]][mVirtualWorld]);
	SetPlayerHealth(playerid, dmtMaps[mapid][dmtPlayerHealth]);
	SetPlayerArmour(playerid, dmtMaps[mapid][dmtPlayerArmour]);
	
	// Put player in team with least players
	new playercount;
	LoopPlayers(p) { // Count players in each team
		if (dmtPStats[p][dmtpMapID] != mapid) continue;
		if (dmtPStats[p][dmtpTeamID] == INVALID_TEAM_ID) continue;
		playercount = playercount + dmtTStats[mapid][dmtPStats[p][dmtpTeamID]][dmtPlayerCount];
	}
	
	if (playercount) {
		new minimumPlayers = dmtTStats[mapid][0][dmtPlayerCount];
		new samecountPlayers;
		
		dmtPStats[playerid][dmtpTeamID] = 0;
		for (new i = 1; i < dmtMaps[mapid][dmtTeamCount]; i++) {
			if (minimumPlayers == dmtTStats[mapid][i][dmtPlayerCount]) {
				samecountPlayers++;
				continue;
			}
			if (minimumPlayers > dmtTStats[mapid][i][dmtPlayerCount]) {
				minimumPlayers = dmtTStats[mapid][i][dmtPlayerCount];
				dmtPStats[playerid][dmtpTeamID] = i;
				samecountPlayers = 0;
			}
		}
		
		if (samecountPlayers) { // Equal teams
			/*dmtPStats[playerid][dmtpTeamID] = random(dmtMaps[mapid][dmtTeamCount]);
			while (dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtPlayerCount] != minimumPlayers) { // Put the player in a random smallest team
				dmtPStats[playerid][dmtpTeamID] = random(dmtMaps[mapid][dmtTeamCount]);
			}*/
			
			// Put the player in the losing team
			new losekills = 0x7FFFFFFF;
			new losedeaths;
			for (new i; i < dmtMaps[mapid][dmtTeamCount]; i++) {
				if (dmtTStats[mapid][i][dmtKills] - dmtTStats[mapid][i][dmtDeaths] < losekills - losedeaths) {
					dmtPStats[playerid][dmtpTeamID] = i;
					losekills = dmtTStats[mapid][i][dmtKills];
					losedeaths = dmtTStats[mapid][i][dmtDeaths];
				}
			}
			
			// Check for duplicate losers
			new duplicateLosers;
			for (new b; b < dmtMaps[mapid][dmtTeamCount]; b++) {
				if (dmtPStats[playerid][dmtpTeamID] == b) continue;
				if (dmtTStats[mapid][b][dmtKills] - dmtTStats[mapid][b][dmtDeaths] == losekills - losedeaths) {
					duplicateLosers = 1;
					break;
				}
			}
			
			if (duplicateLosers) {
				dmtPStats[playerid][dmtpTeamID] = random(dmtMaps[mapid][dmtTeamCount]);
				while (dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtKills] - dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtDeaths] != losekills - losedeaths) { // Put the player in a random losing team
					dmtPStats[playerid][dmtpTeamID] = random(dmtMaps[mapid][dmtTeamCount]);
				}
			}
		}
	} else { // No players in team DM
		dmtPStats[playerid][dmtpTeamID] = random(dmtMaps[mapid][dmtTeamCount]);
	}
	dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtPlayerCount]++;
	SetPlayerColor(playerid, dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtColour]);
	
	// Hide this player for all other players
	if (dmtMaps[mapid][dmtEnemyBlips] == DM_BLIP_HIDDEN || dmtMaps[mapid][dmtEnemyBlips] == DM_BLIP_STEALTH) {
		LoopPlayers(i) {
			if (dmtPStats[i][dmtpMapID] != mapid ||
				dmtPStats[i][dmtpTeamID] == dmtPStats[playerid][dmtpTeamID]) continue;
			
			SetPlayerMarkerForPlayer(i, playerid, (dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtColour] & 0xFFFFFF00));
			SetPlayerMarkerForPlayer(playerid, i, (dmtTStats[mapid][dmtPStats[i][dmtpTeamID]][dmtColour] & 0xFFFFFF00));
			
			ShowPlayerNameTagForPlayer(i, playerid, 0);
			ShowPlayerNameTagForPlayer(playerid, i, 0);
		}
	}
	
	LoopPlayers(i) {
		if (dmtPStats[i][dmtpMapID] != mapid ||
			dmtPStats[i][dmtpTeamID] != dmtPStats[playerid][dmtpTeamID]) continue;
		
		SetPlayerMarkerForPlayer(i, playerid, dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtColour]);
		SetPlayerMarkerForPlayer(playerid, i, dmtTStats[mapid][dmtPStats[i][dmtpTeamID]][dmtColour]);
		
		ShowPlayerNameTagForPlayer(i, playerid, 1);
		ShowPlayerNameTagForPlayer(playerid, i, 1);
	}
	
	// Spawn Player
	new potentialspawns[MAX_DM_TEAM_SPAWNS];
	new spawncount;
	for (new i; i < MAX_DM_TEAM_SPAWNS; i++) {
		if (dmtSpawns[mapid][i][dmtTeamID] != dmtPStats[playerid][dmtpTeamID]) continue; // Not their own team spawn
		if (dmtSpawns[mapid][i][dmtX] == 0.0 && 
			dmtSpawns[mapid][i][dmtY] == 0.0 &&
			dmtSpawns[mapid][i][dmtZ] == 0.0 &&
			dmtSpawns[mapid][i][dmtRot] == 0.0) continue;
		potentialspawns[spawncount] = i;
		spawncount++;
	}
	
	if (spawncount) {
		new rnd = random(spawncount);
		if (IsPlayerSpawned(playerid)) {
			SetPlayerPos(playerid,
				dmtSpawns[mapid][potentialspawns[rnd]][dmtX], dmtSpawns[mapid][potentialspawns[rnd]][dmtY], dmtSpawns[mapid][potentialspawns[rnd]][dmtZ]);
			SetPlayerFacingAngle(playerid, dmtSpawns[mapid][potentialspawns[rnd]][dmtRot]);
			
			// Give player their weapons
			dmtGivePlayerWeapons(mapid, playerid);
		} else {
			SkinsSetNextSpawnPos(playerid,
				dmtSpawns[mapid][potentialspawns[rnd]][dmtX], dmtSpawns[mapid][potentialspawns[rnd]][dmtY], dmtSpawns[mapid][potentialspawns[rnd]][dmtZ], dmtSpawns[mapid][potentialspawns[rnd]][dmtRot]);
		}
	} else {
		SkinsSetNextSpawnPos(playerid, // Centre DM
			dmtMaps[mapid][dmtBoundsRadius][dmtCentreX], dmtMaps[mapid][dmtBoundsRadius][dmtCentreY], dmtMaps[mapid][dmtBoundsRadius][dmtCentreZ], float(random(360)));
	}
	
	SetCameraBehindPlayer(playerid);
	
	dmteamUpdatePlayerInfoShow(mapid, playerid);
	dmteamUpdatePlayersText(mapid);
	
	LoopPlayers(i) {
		if (GetPlayerSpectatingPlayer(i) != playerid) continue;
		dmteam_OnPlayerSpectate(i, playerid);
	}
	
	return 1;
}

dmteam_OnPlayerExit(mapid, playerid) {
	
	if (dmtPStats[playerid][dmtpKills] < 0) { // Heavy team killing - punish player
		SendClientMessage(playerid, COLOUR_ATTENTION, "You have lost XP and cash for teamkilling in this Deathmatch.");
		SendXPMessageToPlayer(playerid, GivePlayerXP(playerid, dmtMaps[mapid][dmtWinXP] * dmtPStats[playerid][dmtpKills] * 2));
		GivePlayerMoney(playerid, dmtMaps[mapid][dmtWinCash] * dmtPStats[playerid][dmtpKills] * 4);
		SendMoneyMessageToPlayer(playerid, dmtMaps[mapid][dmtWinCash] * dmtPStats[playerid][dmtpKills] * 4);
	}
	
	dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtPlayerCount]--;
	dmtPStats[playerid][dmtpMapID] = INVALID_MAP_ID;
	dmtPStats[playerid][dmtpTeamID] = INVALID_TEAM_ID;
	dmtPStats[playerid][dmtpKills] = 0;
	dmtPStats[playerid][dmtpDeaths] = 0;
	//dmtMaps[mapid][dmtPlayers][playerid] = 0;
	SetLastSpawnTick(playerid);
	if (GetPlayerSpectatingPlayer(playerid) == INVALID_PLAYER_ID) SetPlayerVirtualWorld(playerid, 0);
	
	if (IsPlayerSpawned(playerid)) {
		SetPlayerPosRandom(playerid,
			modes[dmtMaps[mapid][dmtModeID]][mLocation][mX],
			modes[dmtMaps[mapid][dmtModeID]][mLocation][mY],
			modes[dmtMaps[mapid][dmtModeID]][mLocation][mZ], 5);
		SetPlayerHealth(playerid, 100.0);
	} else {
		SkinsSetNextSpawnPos(playerid,
			modes[dmtMaps[mapid][dmtModeID]][mLocation][mX],
			modes[dmtMaps[mapid][dmtModeID]][mLocation][mY],
			modes[dmtMaps[mapid][dmtModeID]][mLocation][mZ], float(random(360)));
	}
	
	if (dmtMaps[mapid][dmtCountDown]) {
		// Player may still be frozen
		TogglePlayerControllable(playerid, 1);
	}
	
	GivePlayerGangColour(playerid); // Reset player colour
	LoopPlayers(i) {
		ShowPlayerNameTagForPlayer(i, playerid, 1);
		ShowPlayerNameTagForPlayer(playerid, i, 1);
	}
	dmteamUpdatePlayerInfoHide(playerid);
	
	return 1;
}

dmteam_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED) &&
		!((oldstate >= PLAYER_STATE_ONFOOT && oldstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || oldstate == PLAYER_STATE_SPAWNED) &&
		oldstate != PLAYER_STATE_SPECTATING) {
		// Basically OnPlayerSpawn, but not called if the player was spectating or in a minimode
		StartGodSpawning(playerid, DM_GODE_TIME, 0);
	}
	return 1;
}

dmteam_OnPlayerSpawn(mapid, playerid) {
	if (dmtPStats[playerid][dmtpTeamID] == INVALID_TEAM_ID) return 1; // Or it may crash
	
	SetPlayerHealth(playerid, dmtMaps[mapid][dmtPlayerHealth]);
	SetPlayerArmour(playerid, dmtMaps[mapid][dmtPlayerArmour]);
	dmtGivePlayerWeapons(mapid, playerid);
	
	if (dmtMaps[mapid][dmtEnemyBlips] == DM_BLIP_HIDDEN) {
		SetPlayerColor(playerid, dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtColour] & 0xFFFFFF00);
		
		LoopPlayers(i) {
			if (dmtPStats[i][dmtpMapID] != mapid ||
				dmtPStats[i][dmtpTeamID] != dmtPStats[playerid][dmtpTeamID]) continue;
			
			SetPlayerMarkerForPlayer(i, playerid, (dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtColour]));
			SetPlayerMarkerForPlayer(playerid, i, (dmtTStats[mapid][dmtPStats[i][dmtpTeamID]][dmtColour]));
			
			ShowPlayerNameTagForPlayer(i, playerid, 0);
			ShowPlayerNameTagForPlayer(playerid, i, 0);
		}
	} else if (dmtMaps[mapid][dmtEnemyBlips] == DM_BLIP_STEALTH) {
		dmteamUpdatePlayerAlpha(mapid, playerid);
	} else {
		SetPlayerColor(playerid, dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtColour]);
	}
	return 1;
}

dmteam_OnPlayerDeath(mapid, playerid, killerid, reason) {
	if (dmtPStats[playerid][dmtpMapID] == INVALID_MAP_ID) return 0;
	if (dmtPStats[playerid][dmtpMapID] != mapid) return 0;
	if (dmtPStats[playerid][dmtpTeamID] == INVALID_TEAM_ID) return 1; // Or it may crash
	
	dmtSetRandomSpawn(mapid, playerid);
	dmtPStats[playerid][dmtpDeaths]++;
	dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtDeaths]++;
	
	if (killerid != INVALID_PLAYER_ID && dmtPStats[killerid][dmtpMapID] == mapid) {
		// Check for team kills
		if (dmtPStats[playerid][dmtpTeamID] == dmtPStats[killerid][dmtpTeamID]) {
			// Team kill
			dmtPStats[killerid][dmtpKills]--;
			dmtTStats[mapid][dmtPStats[killerid][dmtpTeamID]][dmtKills]--;
			SendClientMessage(killerid, COLOUR_ATTENTION, "You have killed a team member!");
			SendXPMessageToPlayer(killerid, GivePlayerXP(killerid, -dmtMaps[mapid][dmtKillXP] * 5));
		} else {
			// Legitimate kill
			dmtPStats[killerid][dmtpKills]++;
			dmtTStats[mapid][dmtPStats[killerid][dmtpTeamID]][dmtKills]++;
			
			new drunkxp = GetPlayerDrunkLevel(playerid);
			if (drunkxp < 0) drunkxp = 0;
			if (drunkxp > 5000) drunkxp = 5000;
			drunkxp = drunkxp * dmtMaps[mapid][dmtKillXP] / 5000;
			
			new xp = MinimodeProcessFlagsXP(dmtMaps[mapid][dmtModeID], dmtMaps[mapid][dmtKillXP] + drunkxp);
			SendXPMessageToPlayer(killerid, GivePlayerXP(killerid, xp));
		}
	} else if (killerid == INVALID_PLAYER_ID && reason == 54) { // Fall death
		dmtPStats[playerid][dmtpDeaths]++;
		//dmtPStats[playerid][dmtpKills]--;
		GameTextForPlayer(killerid, "Don't jump off the cliff..", 3000, 5);
	}
	
	dmteamUpdatePlayersText(mapid);
	
	return 1;
}

/*SendTeamScore(mapid, teamid) {
	if (mapid == INVALID_MAP_ID) return 0;
	
	new message[MAX_INPUT];
	format(message, sizeof(message), "%i Team Kills / %i Team Deaths/nTeam Score: %i",
		dmtTStats[mapid][teamid][dmtKills], dmtTStats[mapid][teamid][dmtDeaths],
		dmtTStats[mapid][teamid][dmtKills] - dmtTStats[mapid][teamid][dmtDeaths]);
	
	LoopPlayers(i) {
		if (dmtPStats[i][dmtpMapID] != mapid) continue;
		if (dmtPStats[i][dmtpTeamID] != teamid) continue;
		
		GameTextForPlayer(i, message, 3000, 5);
	}
	return 1;
}*/

dmtSetRandomSpawn(mapid, playerid) {
	new potentialspawns[MAX_DM_TEAM_SPAWNS];
	new spawncount;
	for (new i; i < MAX_DM_TEAM_SPAWNS; i++) {
		if (dmtSpawns[mapid][i][dmtTeamID] != dmtPStats[playerid][dmtpTeamID]) continue; // Not their own team spawn
		if (dmtSpawns[mapid][i][dmtX] == 0.0 &&
			dmtSpawns[mapid][i][dmtY] == 0.0 &&
			dmtSpawns[mapid][i][dmtZ] == 0.0 &&
			dmtSpawns[mapid][i][dmtRot] == 0.0) continue;
		potentialspawns[spawncount] = i;
		spawncount++;
	}
	
	if (spawncount) {
		new rnd = random(spawncount);
		SkinsSetNextSpawnPos(playerid,
			dmtSpawns[mapid][potentialspawns[rnd]][dmtX], dmtSpawns[mapid][potentialspawns[rnd]][dmtY], dmtSpawns[mapid][potentialspawns[rnd]][dmtZ], dmtSpawns[mapid][potentialspawns[rnd]][dmtRot]);
	} else {
		SkinsSetNextSpawnPos(playerid, // Centre DM
			dmtMaps[mapid][dmtBoundsRadius][dmtCentreX], dmtMaps[mapid][dmtBoundsRadius][dmtCentreY], dmtMaps[mapid][dmtBoundsRadius][dmtCentreZ], float(random(360)));
	}
	return 1;
}

dmtGivePlayerWeapons(mapid, playerid) {
	ResetPlayerWeapons(playerid);
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!dmtMaps[mapid][dmtWeapons][i]) continue;
		GivePlayerWeapon(playerid, i, dmtMaps[mapid][dmtWeapons][i]);
	}
}

dmteamUpdatePlayerAlpha(mapid, playerid) {
	
	LoopPlayers(i) {
		if (dmtPStats[i][dmtpMapID] != mapid ||
			dmtPStats[playerid][dmtpTeamID] == dmtPStats[i][dmtpTeamID]) continue;
		
		new volume = GetPlayerVolumeToPlayerByte(playerid, i);
		
		SetPlayerMarkerForPlayer(i, playerid, (GetPlayerColor(playerid) & 0xFFFFFF00) | volume);
		if (volume) {
			ShowPlayerNameTagForPlayer(i, playerid, 1);
		} else {
			ShowPlayerNameTagForPlayer(i, playerid, 0);
		}
	}
	return 1;
}

dmteam_OnPlayerVolumeChange(mapid, playerid) {
	if (dmtMaps[mapid][dmtEnemyBlips] == DM_BLIP_STEALTH) {
		dmteamUpdatePlayerAlpha(mapid, playerid);
	}
	return 1;
}

forward fdm_dmteam_checkDM();
public fdm_dmteam_checkDM() {
	// End the deathmatches when the time runs out
	LoopDMTeamLoaded(i) {
		if (!dmtMaps[i][dmtIsLoaded]) continue;
		if (GetTickCount() - dmtMaps[i][dmtStartTime] > dmtMaps[i][dmtRunTime] * 1000) {
			// Time for deathmatch to end
			StopMinimode(dmtMaps[i][dmtModeID]);
		} else {
			// Check countdown for DM
			if (dmtMaps[i][dmtCountDown]) {
				dmtMaps[i][dmtCountDown]--;
				
				if (!dmtMaps[i][dmtCountDown]) {
					LoopPlayers(c) {
						if (dmtPStats[c][dmtpMapID] != i) continue;
						GameTextForPlayer(c, "GO GO GO!", 1000, 6);
						TogglePlayerControllable(c, 1);
						StartGodSpawning(c, DM_GODE_TIME, 0);
						dmtMaps[i][dmtStartTime] = GetTickCount();
						
						new Float:x, Float:y, Float:z;
						GetPlayerPos(c, x, y, z);
						PlayerPlaySound(c, SOUND_START, x, y, z);
					}
				} else {
					LoopPlayers(c) {
						if (dmtPStats[c][dmtpMapID] != i) continue;
						new gametext[MAX_INPUT];
						format(gametext, sizeof(gametext), "%i", dmtMaps[i][dmtCountDown]);
						GameTextForPlayer(c, gametext, 1000, 6);
						
						new Float:x, Float:y, Float:z;
						GetPlayerPos(c, x, y, z);
						PlayerPlaySound(c, SOUND_COUNTDOWN, x, y, z);
					}
				}
			}
			
			if (!dmtMaps[i][dmtCountDown]) { // Only after countdown is finished
				LoopPlayers(p) {
					if (dmtPStats[p][dmtpMapID] != i) continue;
					if (!IsPlayerSpawned(p)) continue;
					
					if (dmtMaps[i][dmtBoundsRadius][dmtrUsed] || dmtMaps[i][dmtBoundsRectangle][dmteUsed] || dmtMaps[i][dmtBoundPoly][E_VERTICIES] >= 2) {
						new Float:x, Float:y, Float:z, outofbounds;
						GetPlayerPos(p, x, y, z);
						
						// Rectangular check
						if (dmtMaps[i][dmtBoundsRectangle][dmteUsed]) {
							if (dmtMaps[i][dmtBoundsRectangle][dmteMinX] > x) {
								outofbounds = 1;
							} else if (dmtMaps[i][dmtBoundsRectangle][dmteMaxX] < x) {
								outofbounds = 1;
							} else if (dmtMaps[i][dmtBoundsRectangle][dmteMinY] > y) {
								outofbounds = 1;
							} else if (dmtMaps[i][dmtBoundsRectangle][dmteMaxY] < y) {
								outofbounds = 1;
							} else if (dmtMaps[i][dmtBoundsRectangle][dmteMinZ] > z) {
								outofbounds = 1;
							} else if (dmtMaps[i][dmtBoundsRectangle][dmteMaxZ] < z) {
								outofbounds = 1;
							}
						}
						
						// Ciricular check
						if (!outofbounds && dmtMaps[i][dmtBoundsRadius][dmtrUsed]) {
							new Float:distance = FindDistance(x, y, z,
								dmtMaps[i][dmtBoundsRadius][dmtCentreX], dmtMaps[i][dmtBoundsRadius][dmtCentreY], dmtMaps[i][dmtBoundsRadius][dmtCentreZ]);
							
							// Check player distances for DM's
							if (distance > dmtMaps[i][dmtBoundsRadius][dmtRadius]) {
								outofbounds = 1;
							}
						}
						
						// Polygonal check
						if (!outofbounds && dmtMaps[i][dmtBoundPoly][E_VERTICIES] >= 2) {
							new tmp[E_POLYGON];
							tmp[E_VERTICIES] = dmtMaps[i][dmtBoundPoly][E_VERTICIES];
							for (new j; j < tmp[E_VERTICIES]; j++) {
								tmp[E_VERTEX_X][j] = dmtMaps[i][dmtBoundPoly][E_VERTEX_X][j];
								tmp[E_VERTEX_Y][j] = dmtMaps[i][dmtBoundPoly][E_VERTEX_Y][j];
							}
							if (!IsPointInPoly(x, y, tmp)) {
								outofbounds = 1;
							}
						}
						
						if (outofbounds) {
							if (!dmtPStats[p][dmtpBoundsTime]) {
								dmtPStats[p][dmtpBoundsTime] = MAX_DM_BOUNDS_TIME;
							} else {
								dmtPStats[p][dmtpBoundsTime]--;
								if (!dmtPStats[p][dmtpBoundsTime]) {
									SetPlayerHealth(p, 0.0); // Kill player if they're outside the DM
									SendClientMessage(p, COLOUR_ATTENTION, "DEATHMATCHES: You were killed for being outside the deathmatch zone.");
								} else {
									new message[MAX_INPUT];
									format(message, sizeof(message), "DEATHMATCHES: Stay within the deathmatch zone or be killed!! (%is to go)", dmtPStats[p][dmtpBoundsTime]);
									SendClientMessage(p, COLOUR_ATTENTION, message);
									format(message, sizeof(message), "~r~STAY IN THE DEATHMATCH ZONE [%is]", dmtPStats[p][dmtpBoundsTime]);
									kGameTextForPlayer(p, message, 3000, 320.0, 280.0);
								}
							}
						} else {
							dmtPStats[p][dmtpBoundsTime] = 0;
						}
					}
					
					// Check player colours
					if (GetPlayerColor(p) != dmtTStats[i][dmtPStats[p][dmtpTeamID]][dmtColour]) {
						SetPlayerColor(p, dmtTStats[i][dmtPStats[p][dmtpTeamID]][dmtColour]);
						
						if (dmtMaps[i][dmtEnemyBlips] == DM_BLIP_HIDDEN) {
							LoopPlayersEx(c, bb) {
								if (dmtPStats[c][dmtpMapID] != i ||
									dmtPStats[c][dmtpTeamID] == dmtPStats[p][dmtpTeamID]) continue;
									
								SetPlayerMarkerForPlayer(c, p, (dmtTStats[i][dmtPStats[p][dmtpTeamID]][dmtColour] & 0xFFFFFF00));
								ShowPlayerNameTagForPlayer(c, p, 0);
							}
						}
					}
					
					// Check stealth dms
					/*if (dmtMaps[i][dmtEnemyBlips] == DM_BLIP_STEALTH) {
						dmteamUpdatePlayerAlpha(i, p);
					}*/
				}
				
				dmteamUpdatePlayersTime(i);
			}
		}
	}
}

forward fdm_dmteam_checkDMFast();
public fdm_dmteam_checkDMFast() {
	LoopDMTeamLoaded(i) {
		if (!dmtMaps[i][dmtCountDown]) {
			LoopPlayers(p) {
				if (dmtPStats[p][dmtpMapID] != i) continue;
				if (!IsPlayerSpawned(p)) continue;
				
				if (dmtMaps[i][dmtEnemyBlips] == DM_BLIP_STEALTH) {
					dmteamUpdatePlayerAlpha(i, p);
				}
			}
		}
	}
}

dmteamGetWinnerTeamID(mapid) {
	if (mapid == INVALID_MAP_ID) return INVALID_TEAM_ID;
	
	new winnerteam = INVALID_TEAM_ID;
	new winnerkills;
	new winnerdeaths = 0x7FFFFFFF;
	for (new i; i < dmtMaps[mapid][dmtTeamCount]; i++) {
		if (dmtTStats[mapid][i][dmtKills] - dmtTStats[mapid][i][dmtDeaths] > winnerkills - winnerdeaths) {
			winnerteam = i;
			winnerkills = dmtTStats[mapid][i][dmtKills];
			winnerdeaths = dmtTStats[mapid][i][dmtDeaths];
		}
	}
	if (winnerteam == INVALID_TEAM_ID) return INVALID_TEAM_ID;
	
	// Check for duplicate winners
	for (new b; b < dmtMaps[mapid][dmtTeamCount]; b++) {
		if (winnerteam == b) continue;
		if (dmtTStats[mapid][b][dmtKills] - dmtTStats[mapid][b][dmtDeaths] == winnerkills - winnerdeaths) {
			return INVALID_TEAM_ID;
		}
	}
	return winnerteam;
}

dmteamUpdatePlayersText(mapid) { // Player positions and text draws
	
	new message[MAX_INPUT];
	
	new winnerteam = dmteamGetWinnerTeamID(mapid);
	new winnername[MAX_TEAM_NAME];
	if (winnerteam == INVALID_TEAM_ID) {
		winnername = "~w~Nil";
	} else {
		format(winnername, sizeof(winnername), "%s%s", dmtColours[winnerteam][dmtcGametextColour], dmtColours[winnerteam][dmtcName]);
	}
	
	LoopPlayers(i) {
		if (dmtPStats[i][dmtpMapID] != mapid) continue;
		if (!dmtPStats[i][dmtIsTextDrawOn]) continue;
		if (dmtPStats[i][dmtpTeamID] == INVALID_TEAM_ID) continue;
		
		format(message, sizeof(message), "%sKills: ~w~%i  %sDeaths: ~w~%i  %sWinning Team: %s",
			dmtColours[dmtPStats[i][dmtpTeamID]][dmtcGametextColour], dmtPStats[i][dmtpKills],
			dmtColours[dmtPStats[i][dmtpTeamID]][dmtcGametextColour], dmtPStats[i][dmtpDeaths],
			dmtColours[dmtPStats[i][dmtpTeamID]][dmtcGametextColour], winnername);
		
		if (!dmtPStats[i][dmtTextString][0] || strcmp(dmtPStats[i][dmtTextString], message)) {
			TextDrawSetString(dmtPStats[i][dmtText], message);
			strcpy(dmtPStats[i][dmtTextString], message, MAX_INPUT);
		}
	}
	return 1;
}

dmteamUpdatePlayersTime(mapid) {
	new text[MAX_INPUT];
	format(text, sizeof(text), "~g~Time Remaining: ~w~%s", GetTimeString2(dmtMaps[mapid][dmtRunTime] * 1000 - (GetTickCount() - dmtMaps[mapid][dmtStartTime])));
	
	LoopPlayers(i) {
		if (dmtPStats[i][dmtpMapID] != mapid ||
			!dmtPStats[i][dmtIsTextDrawOn] ||
			dmtPStats[i][dmtpTeamID] == INVALID_TEAM_ID) continue;
		
		text[1] = dmtColours[dmtPStats[i][dmtpTeamID]][dmtcGametextColour][1];
		TextDrawSetString(dmtPStats[i][dmtpTime], text);
	}
	return 1;
}

dmteamUpdatePlayerInfoShow(mapid, playerid) {
	#pragma unused mapid
	if (dmtPStats[playerid][dmtIsTextDrawOn]) return 1;
	
	dmtPStats[playerid][dmtIsTextDrawOn] = 1;
	dmtPStats[playerid][dmtText] = TextDrawCreate(150.0, 430.0, " ");
	dmtPStats[playerid][dmtpTime] = TextDrawCreate(20.0, 300.0, " ");
	
	//new text[MAX_INPUT];
	//format(text, sizeof(text), "~g~~h~~h~Team: %s%s", dmtColours[dmtPStats[playerid][dmtpTeamID]][dmtcGametextColour], dmtColours[dmtPStats[playerid][dmtpTeamID]][dmtcName]);
	
	//dmtPStats[playerid][dmtpTeam] = TextDrawCreate(20.0, 320.0, text);
	//TextDrawColor(dmtPStats[playerid][dmtpTeam], dmtTStats[mapid][dmtPStats[playerid][dmtpTeamID]][dmtColour]);
	
	TextDrawShowForPlayer(playerid, dmtPStats[playerid][dmtText]);
	TextDrawShowForPlayer(playerid, dmtPStats[playerid][dmtpTime]);
	//TextDrawShowForPlayer(playerid, dmtPStats[playerid][dmtpTeam]);
	dmtPStats[playerid][dmtTextString][0] = ' ';
	return 1;
}

dmteamUpdatePlayerInfoHide(playerid) {
	if (!dmtPStats[playerid][dmtIsTextDrawOn]) return 1;
	
	dmtPStats[playerid][dmtIsTextDrawOn] = 0;
	
	new Text:tmp;
	TextDrawDestroy(dmtPStats[playerid][dmtText]);
	TextDrawDestroy(dmtPStats[playerid][dmtpTime]);
	//TextDrawDestroy(dmtPStats[playerid][dmtpTeam]);
	
	dmtPStats[playerid][dmtText] = tmp;
	dmtPStats[playerid][dmtpTime] = tmp;
	//dmtPStats[playerid][dmtpTeam] = tmp;
	strclr(dmtPStats[playerid][dmtTextString]);
	return 1;
}

dmteamUpdatePlayerInfoUnload() { // Destroy all textdraws
	LoopPlayers(i) {
		dmteamUpdatePlayerInfoHide(i);
	}
}

kcmd:t(playerid, text[]) {
	// Send team chat to team
	if (dmtPStats[playerid][dmtpMapID] == INVALID_MAP_ID || dmtPStats[playerid][dmtpTeamID] == INVALID_TEAM_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "TEAMCHAT: You must be in a team deathmatch to use /t.");
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /t [message]");
		return 1;
	}
	
	new message[MAX_STRING];
	format(message, sizeof(message), "<TeamChat> %s: %s", pData[playerid][pLoginName], text);
	
	new Bit:players[Bit_Bits(MAX_PLAYERS)];
	LoopPlayers(i) {
		if (dmtPStats[playerid][dmtpMapID] == dmtPStats[i][dmtpMapID] && // Save map
			dmtPStats[playerid][dmtpTeamID] == dmtPStats[i][dmtpTeamID]) { // Same team
			
			Bit_Set(players, i, 1, sizeof(players));
		}
	}
	SendWrappedMessageToClients(players, dmtTStats[dmtPStats[playerid][dmtpMapID]][dmtPStats[playerid][dmtpTeamID]][dmtColour], message);
	
	printf("[FDM] %s", message);
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// SPECTATE EVENTS
//-----------------------------------------------------------------------------------------------------

dmteam_OnPlayerSpectate(playerid, specid) {
	if (dmtPStats[playerid][dmtpMapID] != INVALID_MAP_ID && dmtPStats[playerid][dmtIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, dmtPStats[playerid][dmtText]);
		TextDrawHideForPlayer(playerid, dmtPStats[playerid][dmtpTime]);
		//TextDrawHideForPlayer(playerid, dmtPStats[playerid][dmtpTeam]);
	}
	if (dmtPStats[specid][dmtpMapID] != INVALID_MAP_ID && dmtPStats[specid][dmtIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, dmtPStats[specid][dmtText]);
		TextDrawShowForPlayer(playerid, dmtPStats[specid][dmtpTime]);
		//TextDrawShowForPlayer(playerid, dmtPStats[specid][dmtpTeam]);
	}
	return 1;
}

dmteam_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid) {
	if (dmtPStats[oldspecid][dmtpMapID] != INVALID_MAP_ID && dmtPStats[oldspecid][dmtIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, dmtPStats[oldspecid][dmtText]);
		TextDrawHideForPlayer(playerid, dmtPStats[oldspecid][dmtpTime]);
		//TextDrawHideForPlayer(playerid, dmtPStats[oldspecid][dmtpTeam]);
	}
	if (dmtPStats[newspecid][dmtpMapID] != INVALID_MAP_ID && dmtPStats[newspecid][dmtIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, dmtPStats[newspecid][dmtText]);
		TextDrawShowForPlayer(playerid, dmtPStats[newspecid][dmtpTime]);
		//TextDrawShowForPlayer(playerid, dmtPStats[newspecid][dmtpTeam]);
	}
	return 1;
}

dmteam_OnPlayerUnspectate(playerid, specid) {
	if (dmtPStats[playerid][dmtpMapID] != INVALID_MAP_ID && dmtPStats[playerid][dmtIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, dmtPStats[playerid][dmtText]);
		TextDrawShowForPlayer(playerid, dmtPStats[playerid][dmtpTime]);
		//TextDrawShowForPlayer(playerid, dmtPStats[playerid][dmtpTeam]);
	}
	if (dmtPStats[specid][dmtpMapID] != INVALID_MAP_ID && dmtPStats[specid][dmtIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, dmtPStats[specid][dmtText]);
		TextDrawHideForPlayer(playerid, dmtPStats[specid][dmtpTime]);
		//TextDrawHideForPlayer(playerid, dmtPStats[specid][dmtpTeam]);
	}
	return 1;
}

