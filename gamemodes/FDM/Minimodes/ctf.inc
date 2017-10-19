#include <a_samp>

#define FILE_CTF_TEAM "FDM/Minimodes/DeathMatchTeamList.ini"
#define FILE_CTF_TEAM_MAPS "FDM/Minimodes/Deathmatch_Team/"

#define MAX_CTF_TEAMS 4
#if defined CRIPPLE_PLAYERS
	#define MAX_CTF_MAPS 2
#else
	#define MAX_CTF_MAPS 2 // Maximum ctf which can be loaded
#endif
#define MAX_CTF_SPAWNS 64
#define MAX_CTF_BOUNDS_TIME 5
#define MAX_CTF_BOUND_POLYS 32

#define MAX_CTF_NAME 12

#define COLOUR_CTF_TEAMCHAT 0x6600CCAA

#if !defined MAX_WEAPON_ID
	#define MAX_WEAPON_ID 47
#endif

#define CTF_FLAG_OBJECT 2914


enum ctfSpawnInfo {
	ctfTeamID,
	Float:ctfX,
	Float:ctfY,
	Float:ctfZ,
	Float:ctfRot
}
enum ctfBoundsRadiusInfo {
	ctfrUsed,
	Float:ctfCentreX,
	Float:ctfCentreY,
	Float:ctfCentreZ,
	Float:ctfRadius,
}
enum ctfBoundsRectangleInfo {
	ctfeUsed,
	Float:ctfeMinX,
	Float:ctfeMinY,
	Float:ctfeMinZ,
	Float:ctfeMaxX,
	Float:ctfeMaxY,
	Float:ctfeMaxZ
}
enum ctfInfo {
	ctfIsLoaded,
	ctfModeID,
	ctfStartTime,
	ctfRunTime,
	ctfWinXP,
	ctfWinCash,
	ctfKillXP,
	ctfTeamCount,
	ctfBoundsRadius[ctfBoundsRadiusInfo],
	ctfBoundsRectangle[ctfBoundsRectangleInfo],
	ctfBoundPoly[E_POLYGON],
	ctfWeapons[MAX_WEAPON_ID],
	//ctfPlayers[MAX_PLAYERS],
	ctfCountDown,
	Float:ctfPlayerHealth,
	Float:ctfPlayerArmour,
	ctfEnemyBlips,
	ctfBlockJoinMidGame
}
enum ctfPlayerStatsInfo {
	ctfpMapID,
	ctfpTeamID,
	ctfpBoundsTime,
	ctfIsTextDrawOn,
	Text:ctfpTime,
	//Text:ctfpTeam,
	Text:ctfText,
	ctfTextString[MAX_INPUT]
}
enum ctfTeamInfo {
	ctfColour,
	ctfScore,
	ctfPlayerCount
}
enum ctfColoursInfo {
	ctfcName[MAX_CTF_NAME],
	ctfcColour,
	ctfcGametextColour[4]
}
new ctfPStats[MAX_PLAYERS][ctfPlayerStatsInfo];
new ctfTStats[MAX_CTF_MAPS][MAX_CTF_TEAMS][ctfTeamInfo];
new ctfMaps[MAX_CTF_MAPS][ctfInfo];
new ctfSpawns[MAX_CTF_MAPS][MAX_CTF_SPAWNS][ctfSpawnInfo];
new ctfColours[MAX_CTF_TEAMS][ctfColoursInfo] = {
	{"Red", 0xFF0000AA, "~r~"},
	{"Blue", 0x0000FFAA, "~b~"},
	{"Green", 0x00FF00AA, "~g~"},
	{"Yellow", 0xFFFF00AA, "~y~"}
};
//new dmtTimer;

new ctfInProgress[MAX_CTF_MAPS + 1];

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

CTFInit() {
	for (new i; i < MAX_CTF_MAPS; i++) {
		ctfMaps[i][ctfModeID] = INVALID_MODE_ID;
	}
	for (new i; i < MAX_PLAYERS; i++) {
		ctfPStats[i][ctfpMapID] = INVALID_MAP_ID;
		ctfPStats[i][ctfpTeamID] = INVALID_TEAM_ID;
	}
	ctfInProgress[0] = INVALID_MAP_ID;
	
	LoadCTFMaps();
	
	kEnableTimer(25);
	kEnableTimer(26);
}

CTFUnload() {
	// Unload all variables
	new tmp1[ctfPlayerStatsInfo];
	new tmp2[ctfTeamInfo];
	new tmp3[ctfInfo];
	new tmp4[ctfSpawnInfo];
	
	ctfUpdatePlayerInfoUnload();
	
	for (new a; a < MAX_PLAYERS; a++) {
		ctfPStats[a] = tmp1;
	}
	for (new d; d < MAX_CTF_MAPS; d++) {
		ctfMaps[d] = tmp3;
		for (new b; b < MAX_CTF_TEAMS; b++) {
			ctfTStats[d][b] = tmp2;
		}
		for (new e; e < MAX_CTF_MAPS; e++) {
			ctfSpawns[d][e] = tmp4;
		}
	}
	
	kDisableTimer(25);
	kDisableTimer(26);
}

LoadCTFMaps() {
	if (!INI_Exist(FILE_CTF_TEAM)) {
		//printf("[FDM] WARNING: CTF Map List Fail: '%s' - File not found.", FILE_CTF_TEAM);
		return 0;
	}
	
	if (!INI_Open(FILE_CTF_TEAM)) {
		//printf("[FDM] WARNING: CTF List Fail: '%s' - File cannot be opened.", FILE_CTF_TEAM);
		return 0;
	}
	
	new dmFiles[MAX_CTF_MAPS][MAX_FILENAME];
	new dmcount;
	new dmsLoaded;
	//new key[INI_MAX_KEY];
	
	new i3;
	for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
		if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "CTF", false)) {
			format(dmFiles[i3], MAX_FILENAME, "%s%s", FILE_CTF_TEAM_MAPS, gCache[l][E_VALUE]);
			i3++;
			dmcount = i3;
		}
		if (i3 >= MAX_CTF_MAPS) {
			printf("[FDM] WARNING: Maximum ctf maps reached (%i derbies)", MAX_CTF_MAPS);
			break; // Maximum CTF reached
		}
	}
	/*for (new i; i < MAX_CTF_MAPS; i++) { // Read DM list
		format(key, sizeof(key), "CTF%i", i);
		
		if (INI_ReadString(dmFiles[i], key)) {
			// Add the file path to the beginning
			format(dmFiles[i], MAX_FILENAME, "%s%s", FILE_CTF_TEAM_MAPS, dmFiles[i]);
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
			printf("[FDM] WARNING: CTF Load Fail: '%s' - File not found.", dmFiles[i]);
			continue;
		}
		if (!INI_Open(dmFiles[i])) {
			printf("[FDM] WARNING: CTF Load Fail: '%s' - File cannot be opened.", dmFiles[i]);
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
			printf("[FDM] WARNING: CTF Load Fail: '%s' - File is corrupt/invalid.", dmFiles[i]);
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
		ctfMaps[i][ctfRunTime] = INI_ReadInt("RunTime");
		ctfMaps[i][ctfWinXP] = INI_ReadInt("WinXP");
		ctfMaps[i][ctfWinCash] = INI_ReadInt("WinCash");
		ctfMaps[i][ctfKillXP] = INI_ReadInt("KillXP");
		ctfMaps[i][ctfTeamCount] = INI_ReadInt("TeamCount");
		ctfMaps[i][ctfEnemyBlips] = INI_ReadInt("EnemyBlips");
		ctfMaps[i][ctfBlockJoinMidGame] = INI_ReadInt("BlockJoinMidGame");
		
		ctfMaps[i][ctfPlayerHealth] = INI_ReadFloat("PlayerHealth");
		ctfMaps[i][ctfPlayerArmour] = INI_ReadFloat("PlayerArmour");
		if (ctfMaps[i][ctfPlayerHealth] == 0.0) ctfMaps[i][ctfPlayerHealth] = 100.0;
		
		if (ctfMaps[i][ctfTeamCount] < 2) ctfMaps[i][ctfTeamCount] = 2;
		if (ctfMaps[i][ctfTeamCount] > MAX_CTF_TEAMS) ctfMaps[i][ctfTeamCount] = MAX_CTF_TEAMS;
		
		if (INI_ReadString(tmpstr, "Bounds")) { // DM Boundaries
			idx = 0;
			ctfMaps[i][ctfBoundsRadius][ctfrUsed] = 1;
			ctfMaps[i][ctfBoundsRadius][ctfCentreX] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRadius][ctfCentreY] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRadius][ctfCentreZ] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRadius][ctfRadius] = floatstr(split(tmpstr, idx, ','));
		} else {
			ctfMaps[i][ctfBoundsRadius][ctfrUsed] = 0;
		}
		
		idx = 0;
		if (INI_ReadString(tmpstr, "BoundRectangle")) { // DM Boundaries
			ctfMaps[i][ctfBoundsRectangle][ctfeUsed] = 1;
			ctfMaps[i][ctfBoundsRectangle][ctfeMinX] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRectangle][ctfeMinY] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRectangle][ctfeMinZ] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRectangle][ctfeMaxX] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRectangle][ctfeMaxY] = floatstr(split(tmpstr, idx, ','));
			ctfMaps[i][ctfBoundsRectangle][ctfeMaxZ] = floatstr(split(tmpstr, idx, ','));
			
			if (ctfMaps[i][ctfBoundsRectangle][ctfeMinX] > ctfMaps[i][ctfBoundsRectangle][ctfeMaxX])
				floatswap(ctfMaps[i][ctfBoundsRectangle][ctfeMinX], ctfMaps[i][ctfBoundsRectangle][ctfeMaxX]);
			
			if (ctfMaps[i][ctfBoundsRectangle][ctfeMinY] > ctfMaps[i][ctfBoundsRectangle][ctfeMinY])
				floatswap(ctfMaps[i][ctfBoundsRectangle][ctfeMinY], ctfMaps[i][ctfBoundsRectangle][ctfeMinY]);
			
			if (ctfMaps[i][ctfBoundsRectangle][ctfeMaxZ] > ctfMaps[i][ctfBoundsRectangle][ctfeMaxZ])
				floatswap(ctfMaps[i][ctfBoundsRectangle][ctfeMaxZ], ctfMaps[i][ctfBoundsRectangle][ctfeMaxZ]);
			
		} else {
			ctfMaps[i][ctfBoundsRectangle][ctfeUsed] = 0;
		}
		
		new i2;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "BoundPolyPoint", false)) {
				idx = 0;
				ctfMaps[i][ctfBoundPoly][E_VERTEX_X][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				ctfMaps[i][ctfBoundPoly][E_VERTEX_Y][i2] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_VERTICIES) break; // Maximum polys reached
		}
		ctfMaps[i][ctfBoundPoly][E_VERTICIES] = i2;
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "WPN", false)) {
				idx = 0;
				new weaponidx = strval(split(gCache[l][E_VALUE], idx, ','));
				ctfMaps[i][ctfWeapons][weaponidx] = strval(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_WEAPON_ID) break; // Maximum weapons reached
		}
		/*for (new p; p < MAX_WEAPON_ID; p++) { // Weapons in deathmatch
			format(key, sizeof(key), "W%i", p);
			if (!INI_ReadString(tmpstr, key)) break; // End of weapons
			
			idx = 0;
			new weaponidx = strval(split(tmpstr, idx, ','));
			ctfMaps[i][ctfWeapons][weaponidx] = strval(split(tmpstr, idx, ','));
		}*/
		
		i2 = 0;
		for (new l; gCache[l][E_VALUE][0] && l < INI_MAX_LINES; l++) { // Read directly from the cache
			if (gCache[l][E_KEY][0] && !strcmp(gCache[l][E_KEY], "SPAWN", false)) {
				idx = 0;
				ctfSpawns[i][i2][ctfTeamID] = strval(split(gCache[l][E_VALUE], idx, ','));
				ctfSpawns[i][i2][ctfX] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				ctfSpawns[i][i2][ctfY] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				ctfSpawns[i][i2][ctfZ] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				ctfSpawns[i][i2][ctfRot] = floatstr(split(gCache[l][E_VALUE], idx, ','));
				i2++;
			}
			if (i2 >= MAX_CTF_SPAWNS) break; // Maximum spawns reached
		}
		/*for (new s; s < MAX_CTF_SPAWNS; s++) { // Deathmatch spawn loations
			format(key, sizeof(key), "Spawn%i", s);
			if (!INI_ReadString(tmpstr, key)) break; // End of spawns
			
			idx = 0;
			ctfSpawns[i][s][ctfTeamID] = strval(split(tmpstr, idx, ','));
			ctfSpawns[i][s][ctfX] = floatstr(split(tmpstr, idx, ','));
			ctfSpawns[i][s][ctfY] = floatstr(split(tmpstr, idx, ','));
			ctfSpawns[i][s][ctfZ] = floatstr(split(tmpstr, idx, ','));
			ctfSpawns[i][s][ctfRot] = floatstr(split(tmpstr, idx, ','));
		}*/
		
		// Deathmatch map reading complete
		//format(dmname, sizeof(dmname), "CTF: %s", dmname);
		//ctfMaps[i][ctfModeID] = CreateMinimode(MODE_TYPE_CTF, dmname, i, minlevel, cost, x, y, z);
		ctfMaps[i][ctfModeID] = LoadMinimode(MODE_TYPE_CTF, dmname, i, "", 2, MAX_PLAYERS, 1);
		INI_Close();
		
		if (ctfMaps[i][ctfModeID] != INVALID_MODE_ID) {
			dmsLoaded++;
		}
	}
	
	printf("[FDM] CTF's Loaded: %i", dmsLoaded);
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// OPTIMISATIONS
//-----------------------------------------------------------------------------------------------------

CTFProgress() {
	new indexLoaded;
	
	for (new mapid; mapid < MAX_CTF_MAPS; mapid++) {
		if (ctfMaps[mapid][ctfIsLoaded]) {
			ctfInProgress[indexLoaded] = mapid;
			indexLoaded++;
		}
	}
	
	ctfInProgress[indexLoaded] = INVALID_MAP_ID;
	return 1;
}

#define CTFTeamLoaded(%1) for (new mmc, %1; ctfInProgress[mmc] != INVALID_MAP_ID; mmc++) if ((%1 = ctfInProgress[mmc]) || true)


//-----------------------------------------------------------------------------------------------------
// ACTUAL TEAM DEATHMATCHES
//-----------------------------------------------------------------------------------------------------

ctf_IsJoinable(mapid) {
	if (!ctfMaps[mapid][ctfIsLoaded]) return 0;
	if (GetTickCount() - ctfMaps[mapid][ctfStartTime] > ctfMaps[mapid][ctfRunTime] * 500) return 0;
	return 1;
}

ctf_ShowModeInfo(mapid, playerid) {
	new msg[MAX_STRING];
	
	// Show blipmode/health/armour
	if (ctfMaps[mapid][ctfEnemyBlips] == DM_BLIP_STEALTH) {
		msg = "Blips: Stealth";
	} else if (ctfMaps[mapid][ctfEnemyBlips] == DM_BLIP_SHOWN) {
		msg = "Blips: Always Visible";
	} else {
		msg = "Blips: Hidden";
	}
	
	format(msg, sizeof(msg), "%s | Health: %0.1f | Armour: %0.1f | Time Limit: %s",
		msg, ctfMaps[mapid][ctfPlayerHealth], ctfMaps[mapid][ctfPlayerArmour], GetTimeString(ctfMaps[mapid][ctfRunTime] * 1000, 0));
	SendClientMessage(playerid, COLOUR_HELP, msg);
	
	// Show weapons
	msg = "Weapons: ";
	new weaponcount;
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!ctfMaps[mapid][ctfWeapons][i]) continue;
		strcat(msg, ReturnWeaponName(i));
		
		if (!IsWeaponMelee(i)) {
			new strammo[MAX_REAL + 2];
			format(strammo, sizeof(strammo), "(%i)", ctfMaps[mapid][ctfWeapons][i]);
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

ctf_OnStart(mapid, players[], playercount) {
	if (ctfMaps[mapid][ctfIsLoaded]) return 0;
	if (playercount < 2) return 0; // Not enough players
	
	ctfMaps[mapid][ctfIsLoaded] = 1;
	ctfMaps[mapid][ctfStartTime] = GetTickCount();
	ctfMaps[mapid][ctfCountDown] = COUNTDOWN_TIME;
	
	CTFProgress();
	
	// Reset team data
	for (new i; i < MAX_CTF_TEAMS; i++) {
		ctfTStats[mapid][i][ctfColour] = ctfColours[i][ctfcColour];
		ctfTStats[mapid][i][ctfScore] = 0;
	}
	
	LoopPlayers(i) {
		if (players[i]) {
			ctf_OnPlayerEnter(mapid, i);
		}
	}
	
	return 1;
}

ctf_OnStop(mapid) {
	if (!ctfMaps[mapid][ctfIsLoaded]) return 0;
	ctfMaps[mapid][ctfIsLoaded] = 0;
	
	CTFProgress();
	
	new winnerteam = ctfGetWinnerTeamID(mapid);
	
	// Announce winning team
	new message[MAX_INPUT];
	if (winnerteam == INVALID_TEAM_ID) {
		format(message, sizeof(message), "%s: No team has won this deathmatch.",
			modes[ctfMaps[mapid][ctfModeID]][mName]);
		SendClientMessageToAll(COLOUR_TITLE, message);
	} else {
		format(message, sizeof(message), "%s: Team %s has won with a score of %i.",
			modes[ctfMaps[mapid][ctfModeID]][mName], ctfColours[winnerteam][ctfcName], ctfTStats[mapid][winnerteam][ctfScore]);
		SendClientMessageToAll(ctfColours[winnerteam][ctfcColour], message);
		
		LoopPlayers(i) {
			//if (!ctfMaps[mapid][ctfPlayers][i]) continue;
			if (mpData[i][mpModeID] != ctfMaps[mapid][ctfModeID]) continue;
			if (ctfPStats[i][ctfpTeamID] != winnerteam) continue;
			
			new xp = MinimodeProcessFlagsXP(ctfMaps[mapid][ctfModeID], ctfMaps[mapid][ctfWinXP]);
			SendXPMessageToPlayer(i, GivePlayerXP(i, xp));
			
			new money = MinimodeProcessFlagsMoney(ctfMaps[mapid][ctfModeID], ctfMaps[mapid][ctfWinCash]);
			GivePlayerMoney(i, money);
			SendMoneyMessageToPlayer(i, money);
		}
	}
	
	LoopPlayers(p) {
		//if (!ctfMaps[mapid][ctfPlayers][p]) continue;
		if (mpData[p][mpModeID] != ctfMaps[mapid][ctfModeID]) continue;
		
		ctf_OnPlayerExit(mapid, p);
		
		new Float:x, Float:y, Float:z;
		GetPlayerPos(p, x, y, z);
		PlayerPlaySound(p, SOUND_FINISH, x, y, z);
	}
	
	for (new i; i < MAX_CTF_TEAMS; i++) {
		ctfTStats[mapid][i][ctfPlayerCount] = 0;
	}
	return 1;
}

ctf_OnPlayerEnter(mapid, playerid) {
	if (ctfPStats[playerid][ctfpMapID] != INVALID_MAP_ID) return 0;
	
	if (ctfMaps[mapid][ctfBlockJoinMidGame] && !ctfMaps[mapid][ctfCountDown]) return 0; // Block join mid game
	if (GetTickCount() - ctfMaps[mapid][ctfStartTime] > ctfMaps[mapid][ctfRunTime] * 500) return 0; // Block join mid game when half finished
	
	ctfPStats[playerid][ctfpMapID] = mapid;
	ctfPStats[playerid][ctfpBoundsTime] = 0;
	//ctfMaps[mapid][ctfPlayers][playerid] = 1;
	ctfSetRandomSpawn(mapid, playerid);
	
	if (ctfMaps[mapid][ctfCountDown]) {
		TogglePlayerControllable(playerid, 0); // Freeze player
		EnableGodmode(playerid);
	} else {
		StartGodSpawning(playerid, DM_GODE_TIME, 0);
	}
	
	SetPlayerVirtualWorld(playerid, modes[ctfMaps[mapid][ctfModeID]][mVirtualWorld]);
	SetPlayerHealth(playerid, ctfMaps[mapid][ctfPlayerHealth]);
	SetPlayerArmour(playerid, ctfMaps[mapid][ctfPlayerArmour]);
	
	// Put player in team with least players
	new playercount;
	LoopPlayers(p) { // Count players in each team
		if (ctfPStats[p][ctfpMapID] != mapid) continue;
		if (ctfPStats[p][ctfpTeamID] == INVALID_TEAM_ID) continue;
		playercount = playercount + ctfTStats[mapid][ctfPStats[p][ctfpTeamID]][ctfPlayerCount];
	}
	
	if (playercount) {
		new minimumPlayers = ctfTStats[mapid][0][ctfPlayerCount];
		new samecountPlayers;
		
		ctfPStats[playerid][ctfpTeamID] = 0;
		for (new i = 1; i < ctfMaps[mapid][ctfTeamCount]; i++) {
			if (minimumPlayers == ctfTStats[mapid][i][ctfPlayerCount]) {
				samecountPlayers++;
				continue;
			}
			if (minimumPlayers > ctfTStats[mapid][i][ctfPlayerCount]) {
				minimumPlayers = ctfTStats[mapid][i][ctfPlayerCount];
				ctfPStats[playerid][ctfpTeamID] = i;
				samecountPlayers = 0;
			}
		}
		
		if (samecountPlayers) { // Equal teams
			/*ctfPStats[playerid][ctfpTeamID] = random(ctfMaps[mapid][ctfTeamCount]);
			while (ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfPlayerCount] != minimumPlayers) { // Put the player in a random smallest team
				ctfPStats[playerid][ctfpTeamID] = random(ctfMaps[mapid][ctfTeamCount]);
			}*/
			
			// Put the player in the losing team
			new losescore = 0x7FFFFFFF;
			for (new i; i < ctfMaps[mapid][ctfTeamCount]; i++) {
				if (ctfTStats[mapid][i][ctfScore] < losescore) {
					ctfPStats[playerid][ctfpTeamID] = i;
					losescore = ctfTStats[mapid][i][ctfScore];
				}
			}
			
			// Check for duplicate losers
			new duplicateLosers;
			for (new b; b < ctfMaps[mapid][ctfTeamCount]; b++) {
				if (ctfPStats[playerid][ctfpTeamID] == b) continue;
				if (ctfTStats[mapid][b][ctfScore] == losescore) {
					duplicateLosers = 1;
					break;
				}
			}
			
			if (duplicateLosers) {
				ctfPStats[playerid][ctfpTeamID] = random(ctfMaps[mapid][ctfTeamCount]);
				while (ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfScore] != losescore) { // Put the player in a random losing team
					ctfPStats[playerid][ctfpTeamID] = random(ctfMaps[mapid][ctfTeamCount]);
				}
			}
		}
	} else { // No players in team DM
		ctfPStats[playerid][ctfpTeamID] = random(ctfMaps[mapid][ctfTeamCount]);
	}
	ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfPlayerCount]++;
	SetPlayerColor(playerid, ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfColour]);
	
	// Hide this player for all other players
	if (ctfMaps[mapid][ctfEnemyBlips] == DM_BLIP_HIDDEN || ctfMaps[mapid][ctfEnemyBlips] == DM_BLIP_STEALTH) {
		LoopPlayers(i) {
			if (ctfPStats[i][ctfpMapID] != mapid ||
				ctfPStats[i][ctfpTeamID] == ctfPStats[playerid][ctfpTeamID]) continue;
			
			SetPlayerMarkerForPlayer(i, playerid, (ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfColour] & 0xFFFFFF00));
			SetPlayerMarkerForPlayer(playerid, i, (ctfTStats[mapid][ctfPStats[i][ctfpTeamID]][ctfColour] & 0xFFFFFF00));
			
			ShowPlayerNameTagForPlayer(i, playerid, 0);
			ShowPlayerNameTagForPlayer(playerid, i, 0);
		}
	}
	
	LoopPlayers(i) {
		if (ctfPStats[i][ctfpMapID] != mapid ||
			ctfPStats[i][ctfpTeamID] != ctfPStats[playerid][ctfpTeamID]) continue;
		
		SetPlayerMarkerForPlayer(i, playerid, ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfColour]);
		SetPlayerMarkerForPlayer(playerid, i, ctfTStats[mapid][ctfPStats[i][ctfpTeamID]][ctfColour]);
		
		ShowPlayerNameTagForPlayer(i, playerid, 1);
		ShowPlayerNameTagForPlayer(playerid, i, 1);
	}
	
	// Spawn Player
	new potentialspawns[MAX_CTF_SPAWNS];
	new spawncount;
	for (new i; i < MAX_CTF_SPAWNS; i++) {
		if (ctfSpawns[mapid][i][ctfTeamID] != ctfPStats[playerid][ctfpTeamID]) continue; // Not their own team spawn
		if (ctfSpawns[mapid][i][ctfX] == 0.0 && 
			ctfSpawns[mapid][i][ctfY] == 0.0 &&
			ctfSpawns[mapid][i][ctfZ] == 0.0 &&
			ctfSpawns[mapid][i][ctfRot] == 0.0) continue;
		potentialspawns[spawncount] = i;
		spawncount++;
	}
	
	if (spawncount) {
		new rnd = random(spawncount);
		if (IsPlayerSpawned(playerid)) {
			SetPlayerPos(playerid,
				ctfSpawns[mapid][potentialspawns[rnd]][ctfX], ctfSpawns[mapid][potentialspawns[rnd]][ctfY], ctfSpawns[mapid][potentialspawns[rnd]][ctfZ]);
			SetPlayerFacingAngle(playerid, ctfSpawns[mapid][potentialspawns[rnd]][ctfRot]);
			
			// Give player their weapons
			ctfGivePlayerWeapons(mapid, playerid);
		} else {
			SkinsSetNextSpawnPos(playerid,
				ctfSpawns[mapid][potentialspawns[rnd]][ctfX], ctfSpawns[mapid][potentialspawns[rnd]][ctfY], ctfSpawns[mapid][potentialspawns[rnd]][ctfZ], ctfSpawns[mapid][potentialspawns[rnd]][ctfRot]);
		}
	} else {
		SkinsSetNextSpawnPos(playerid, // Centre DM
			ctfMaps[mapid][ctfBoundsRadius][ctfCentreX], ctfMaps[mapid][ctfBoundsRadius][ctfCentreY], ctfMaps[mapid][ctfBoundsRadius][ctfCentreZ], float(random(360)));
	}
	
	SetCameraBehindPlayer(playerid);
	
	ctfUpdatePlayerInfoShow(mapid, playerid);
	ctfUpdatePlayersText(mapid);
	
	LoopPlayers(i) {
		if (GetPlayerSpectatingPlayer(i) != playerid) continue;
		ctf_OnPlayerSpectate(i, playerid);
	}
	
	return 1;
}

ctf_OnPlayerExit(mapid, playerid) {
	
	/*if (ctfPStats[playerid][ctfpKills] < 0) { // Heavy team killing - punish player
		SendClientMessage(playerid, COLOUR_ATTENTION, "You have lost XP and cash for teamkilling in this Deathmatch.");
		SendXPMessageToPlayer(playerid, GivePlayerXP(playerid, ctfMaps[mapid][ctfWinXP] * ctfPStats[playerid][ctfpKills] * 2));
		GivePlayerMoney(playerid, ctfMaps[mapid][ctfWinCash] * ctfPStats[playerid][ctfpKills] * 4);
		SendMoneyMessageToPlayer(playerid, ctfMaps[mapid][ctfWinCash] * ctfPStats[playerid][ctfpKills] * 4);
	}*/
	
	ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfPlayerCount]--;
	ctfPStats[playerid][ctfpMapID] = INVALID_MAP_ID;
	ctfPStats[playerid][ctfpTeamID] = INVALID_TEAM_ID;
	//ctfMaps[mapid][ctfPlayers][playerid] = 0;
	SetLastSpawnTick(playerid);
	if (GetPlayerSpectatingPlayer(playerid) == INVALID_PLAYER_ID) SetPlayerVirtualWorld(playerid, 0);
	
	if (IsPlayerSpawned(playerid)) {
		SetPlayerPosRandom(playerid,
			modes[ctfMaps[mapid][ctfModeID]][mLocation][mX],
			modes[ctfMaps[mapid][ctfModeID]][mLocation][mY],
			modes[ctfMaps[mapid][ctfModeID]][mLocation][mZ], 5);
		SetPlayerHealth(playerid, 100.0);
	} else {
		SkinsSetNextSpawnPos(playerid,
			modes[ctfMaps[mapid][ctfModeID]][mLocation][mX],
			modes[ctfMaps[mapid][ctfModeID]][mLocation][mY],
			modes[ctfMaps[mapid][ctfModeID]][mLocation][mZ], float(random(360)));
	}
	
	if (ctfMaps[mapid][ctfCountDown]) {
		// Player may still be frozen
		TogglePlayerControllable(playerid, 1);
	}
	
	GivePlayerGangColour(playerid); // Reset player colour
	LoopPlayers(i) {
		ShowPlayerNameTagForPlayer(i, playerid, 1);
		ShowPlayerNameTagForPlayer(playerid, i, 1);
	}
	ctfUpdatePlayerInfoHide(playerid);
	
	return 1;
}

ctf_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED) &&
		!((oldstate >= PLAYER_STATE_ONFOOT && oldstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || oldstate == PLAYER_STATE_SPAWNED) &&
		oldstate != PLAYER_STATE_SPECTATING) {
		// Basically OnPlayerSpawn, but not called if the player was spectating or in a minimode
		StartGodSpawning(playerid, DM_GODE_TIME, 0);
	}
	return 1;
}

ctf_OnPlayerSpawn(mapid, playerid) {
	if (ctfPStats[playerid][ctfpTeamID] == INVALID_TEAM_ID) return 1; // Or it may crash
	
	SetPlayerHealth(playerid, ctfMaps[mapid][ctfPlayerHealth]);
	SetPlayerArmour(playerid, ctfMaps[mapid][ctfPlayerArmour]);
	ctfGivePlayerWeapons(mapid, playerid);
	
	if (ctfMaps[mapid][ctfEnemyBlips] == DM_BLIP_HIDDEN) {
		SetPlayerColor(playerid, ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfColour] & 0xFFFFFF00);
		
		LoopPlayers(i) {
			if (ctfPStats[i][ctfpMapID] != mapid ||
				ctfPStats[i][ctfpTeamID] != ctfPStats[playerid][ctfpTeamID]) continue;
			
			SetPlayerMarkerForPlayer(i, playerid, (ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfColour]));
			SetPlayerMarkerForPlayer(playerid, i, (ctfTStats[mapid][ctfPStats[i][ctfpTeamID]][ctfColour]));
			
			ShowPlayerNameTagForPlayer(i, playerid, 0);
			ShowPlayerNameTagForPlayer(playerid, i, 0);
		}
	} else if (ctfMaps[mapid][ctfEnemyBlips] == DM_BLIP_STEALTH) {
		ctfUpdatePlayerAlpha(mapid, playerid);
	} else {
		SetPlayerColor(playerid, ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfColour]);
	}
	return 1;
}

ctf_OnPlayerDeath(mapid, playerid, killerid, reason) {
	if (ctfPStats[playerid][ctfpMapID] == INVALID_MAP_ID) return 0;
	if (ctfPStats[playerid][ctfpMapID] != mapid) return 0;
	if (ctfPStats[playerid][ctfpTeamID] == INVALID_TEAM_ID) return 1; // Or it may crash
	
	ctfSetRandomSpawn(mapid, playerid);
	
	if (killerid != INVALID_PLAYER_ID && ctfPStats[killerid][ctfpMapID] == mapid) {
		// Check for team kills
		if (ctfPStats[playerid][ctfpTeamID] == ctfPStats[killerid][ctfpTeamID]) {
			// Team kill
			SendClientMessage(killerid, COLOUR_ATTENTION, "You have killed a team member!");
			SendXPMessageToPlayer(killerid, GivePlayerXP(killerid, -ctfMaps[mapid][ctfKillXP] * 5));
		} else {
			// Legitimate kill
			new drunkxp = GetPlayerDrunkLevel(playerid);
			if (drunkxp < 0) drunkxp = 0;
			if (drunkxp > 5000) drunkxp = 5000;
			drunkxp = drunkxp * ctfMaps[mapid][ctfKillXP] / 5000;
			
			new xp = MinimodeProcessFlagsXP(ctfMaps[mapid][ctfModeID], ctfMaps[mapid][ctfKillXP] + drunkxp);
			SendXPMessageToPlayer(killerid, GivePlayerXP(killerid, xp));
		}
	} else if (killerid == INVALID_PLAYER_ID && reason == 54) { // Fall death
		GameTextForPlayer(killerid, "Don't jump off the cliff..", 3000, 5);
	}
	
	ctfUpdatePlayersText(mapid);
	
	return 1;
}

/*SendTeamScore(mapid, teamid) {
	if (mapid == INVALID_MAP_ID) return 0;
	
	new message[MAX_INPUT];
	format(message, sizeof(message), "%i Team Kills / %i Team Deaths/nTeam Score: %i",
		ctfTStats[mapid][teamid][ctfKills], ctfTStats[mapid][teamid][ctfDeaths],
		ctfTStats[mapid][teamid][ctfKills] - ctfTStats[mapid][teamid][ctfDeaths]);
	
	LoopPlayers(i) {
		if (ctfPStats[i][ctfpMapID] != mapid) continue;
		if (ctfPStats[i][ctfpTeamID] != teamid) continue;
		
		GameTextForPlayer(i, message, 3000, 5);
	}
	return 1;
}*/

ctfSetRandomSpawn(mapid, playerid) {
	new potentialspawns[MAX_CTF_SPAWNS];
	new spawncount;
	for (new i; i < MAX_CTF_SPAWNS; i++) {
		if (ctfSpawns[mapid][i][ctfTeamID] != ctfPStats[playerid][ctfpTeamID]) continue; // Not their own team spawn
		if (ctfSpawns[mapid][i][ctfX] == 0.0 &&
			ctfSpawns[mapid][i][ctfY] == 0.0 &&
			ctfSpawns[mapid][i][ctfZ] == 0.0 &&
			ctfSpawns[mapid][i][ctfRot] == 0.0) continue;
		potentialspawns[spawncount] = i;
		spawncount++;
	}
	
	if (spawncount) {
		new rnd = random(spawncount);
		SkinsSetNextSpawnPos(playerid,
			ctfSpawns[mapid][potentialspawns[rnd]][ctfX], ctfSpawns[mapid][potentialspawns[rnd]][ctfY], ctfSpawns[mapid][potentialspawns[rnd]][ctfZ], ctfSpawns[mapid][potentialspawns[rnd]][ctfRot]);
	} else {
		SkinsSetNextSpawnPos(playerid, // Centre DM
			ctfMaps[mapid][ctfBoundsRadius][ctfCentreX], ctfMaps[mapid][ctfBoundsRadius][ctfCentreY], ctfMaps[mapid][ctfBoundsRadius][ctfCentreZ], float(random(360)));
	}
	return 1;
}

ctfGivePlayerWeapons(mapid, playerid) {
	ResetPlayerWeapons(playerid);
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!ctfMaps[mapid][ctfWeapons][i]) continue;
		GivePlayerWeapon(playerid, i, ctfMaps[mapid][ctfWeapons][i]);
	}
}

ctfUpdatePlayerAlpha(mapid, playerid) {
	
	LoopPlayers(i) {
		if (ctfPStats[i][ctfpMapID] != mapid ||
			ctfPStats[playerid][ctfpTeamID] == ctfPStats[i][ctfpTeamID]) continue;
		
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

ctf_OnPlayerVolumeChange(mapid, playerid) {
	if (ctfMaps[mapid][ctfEnemyBlips] == DM_BLIP_STEALTH) {
		ctfUpdatePlayerAlpha(mapid, playerid);
	}
	return 1;
}

forward fdm_ctf_checkDM();
public fdm_ctf_checkDM() {
	// End the deathmatches when the time runs out
	CTFTeamLoaded(i) {
		if (!ctfMaps[i][ctfIsLoaded]) continue;
		if (GetTickCount() - ctfMaps[i][ctfStartTime] > ctfMaps[i][ctfRunTime] * 1000) {
			// Time for deathmatch to end
			StopMinimode(ctfMaps[i][ctfModeID]);
		} else {
			// Check countdown for DM
			if (ctfMaps[i][ctfCountDown]) {
				ctfMaps[i][ctfCountDown]--;
				
				if (!ctfMaps[i][ctfCountDown]) {
					LoopPlayers(c) {
						if (ctfPStats[c][ctfpMapID] != i) continue;
						GameTextForPlayer(c, "GO GO GO!", 1000, 6);
						TogglePlayerControllable(c, 1);
						StartGodSpawning(c, DM_GODE_TIME, 0);
						ctfMaps[i][ctfStartTime] = GetTickCount();
						
						new Float:x, Float:y, Float:z;
						GetPlayerPos(c, x, y, z);
						PlayerPlaySound(c, SOUND_START, x, y, z);
					}
				} else {
					LoopPlayers(c) {
						if (ctfPStats[c][ctfpMapID] != i) continue;
						new gametext[MAX_INPUT];
						format(gametext, sizeof(gametext), "%i", ctfMaps[i][ctfCountDown]);
						GameTextForPlayer(c, gametext, 1000, 6);
						
						new Float:x, Float:y, Float:z;
						GetPlayerPos(c, x, y, z);
						PlayerPlaySound(c, SOUND_COUNTDOWN, x, y, z);
					}
				}
			}
			
			if (!ctfMaps[i][ctfCountDown]) { // Only after countdown is finished
				LoopPlayers(p) {
					if (ctfPStats[p][ctfpMapID] != i) continue;
					if (!IsPlayerSpawned(p)) continue;
					
					if (ctfMaps[i][ctfBoundsRadius][ctfrUsed] || ctfMaps[i][ctfBoundsRectangle][ctfeUsed] || ctfMaps[i][ctfBoundPoly][E_VERTICIES] >= 2) {
						new Float:x, Float:y, Float:z, outofbounds;
						GetPlayerPos(p, x, y, z);
						
						// Rectangular check
						if (ctfMaps[i][ctfBoundsRectangle][ctfeUsed]) {
							if (ctfMaps[i][ctfBoundsRectangle][ctfeMinX] > x) {
								outofbounds = 1;
							} else if (ctfMaps[i][ctfBoundsRectangle][ctfeMaxX] < x) {
								outofbounds = 1;
							} else if (ctfMaps[i][ctfBoundsRectangle][ctfeMinY] > y) {
								outofbounds = 1;
							} else if (ctfMaps[i][ctfBoundsRectangle][ctfeMaxY] < y) {
								outofbounds = 1;
							} else if (ctfMaps[i][ctfBoundsRectangle][ctfeMinZ] > z) {
								outofbounds = 1;
							} else if (ctfMaps[i][ctfBoundsRectangle][ctfeMaxZ] < z) {
								outofbounds = 1;
							}
						}
						
						// Ciricular check
						if (!outofbounds && ctfMaps[i][ctfBoundsRadius][ctfrUsed]) {
							new Float:distance = FindDistance(x, y, z,
								ctfMaps[i][ctfBoundsRadius][ctfCentreX], ctfMaps[i][ctfBoundsRadius][ctfCentreY], ctfMaps[i][ctfBoundsRadius][ctfCentreZ]);
							
							// Check player distances for DM's
							if (distance > ctfMaps[i][ctfBoundsRadius][ctfRadius]) {
								outofbounds = 1;
							}
						}
						
						// Polygonal check
						if (!outofbounds && ctfMaps[i][ctfBoundPoly][E_VERTICIES] >= 2) {
							new tmp[E_POLYGON];
							tmp[E_VERTICIES] = ctfMaps[i][ctfBoundPoly][E_VERTICIES];
							for (new j; j < tmp[E_VERTICIES]; j++) {
								tmp[E_VERTEX_X][j] = ctfMaps[i][ctfBoundPoly][E_VERTEX_X][j];
								tmp[E_VERTEX_Y][j] = ctfMaps[i][ctfBoundPoly][E_VERTEX_Y][j];
							}
							if (!IsPointInPoly(x, y, tmp)) {
								outofbounds = 1;
							}
						}
						
						if (outofbounds) {
							if (!ctfPStats[p][ctfpBoundsTime]) {
								ctfPStats[p][ctfpBoundsTime] = MAX_DM_BOUNDS_TIME;
							} else {
								ctfPStats[p][ctfpBoundsTime]--;
								if (!ctfPStats[p][ctfpBoundsTime]) {
									SetPlayerHealth(p, 0.0); // Kill player if they're outside the DM
									SendClientMessage(p, COLOUR_ATTENTION, "DEATHMATCHES: You were killed for being outside the deathmatch zone.");
								} else {
									new message[MAX_INPUT];
									format(message, sizeof(message), "DEATHMATCHES: Stay within the deathmatch zone or be killed!! (%is to go)", ctfPStats[p][ctfpBoundsTime]);
									SendClientMessage(p, COLOUR_ATTENTION, message);
									format(message, sizeof(message), "~r~STAY IN THE DEATHMATCH ZONE [%is]", ctfPStats[p][ctfpBoundsTime]);
									kGameTextForPlayer(p, message, 3000, 320.0, 280.0);
								}
							}
						} else {
							ctfPStats[p][ctfpBoundsTime] = 0;
						}
					}
					
					// Check player colours
					if (GetPlayerColor(p) != ctfTStats[i][ctfPStats[p][ctfpTeamID]][ctfColour]) {
						SetPlayerColor(p, ctfTStats[i][ctfPStats[p][ctfpTeamID]][ctfColour]);
						
						if (ctfMaps[i][ctfEnemyBlips] == DM_BLIP_HIDDEN) {
							LoopPlayersEx(c, bb) {
								if (ctfPStats[c][ctfpMapID] != i ||
									ctfPStats[c][ctfpTeamID] == ctfPStats[p][ctfpTeamID]) continue;
									
								SetPlayerMarkerForPlayer(c, p, (ctfTStats[i][ctfPStats[p][ctfpTeamID]][ctfColour] & 0xFFFFFF00));
								ShowPlayerNameTagForPlayer(c, p, 0);
							}
						}
					}
					
					// Check stealth dms
					/*if (ctfMaps[i][ctfEnemyBlips] == DM_BLIP_STEALTH) {
						ctfUpdatePlayerAlpha(i, p);
					}*/
				}
				
				ctfUpdatePlayersTime(i);
			}
		}
	}
}

forward fdm_ctf_checkDMFast();
public fdm_ctf_checkDMFast() {
	CTFTeamLoaded(i) {
		if (!ctfMaps[i][ctfCountDown]) {
			LoopPlayers(p) {
				if (ctfPStats[p][ctfpMapID] != i) continue;
				if (!IsPlayerSpawned(p)) continue;
				
				if (ctfMaps[i][ctfEnemyBlips] == DM_BLIP_STEALTH) {
					ctfUpdatePlayerAlpha(i, p);
				}
			}
		}
	}
}

ctfGetWinnerTeamID(mapid) {
	if (mapid == INVALID_MAP_ID) return INVALID_TEAM_ID;
	
	new winnerteam = INVALID_TEAM_ID;
	new winnerscore = 0x7FFFFFFF;
	for (new i; i < ctfMaps[mapid][ctfTeamCount]; i++) {
		if (ctfTStats[mapid][i][ctfScore] > winnerscore) {
			winnerteam = i;
			winnerscore = ctfTStats[mapid][i][ctfScore];
		}
	}
	if (winnerteam == INVALID_TEAM_ID) return INVALID_TEAM_ID;
	
	// Check for duplicate winners
	for (new b; b < ctfMaps[mapid][ctfTeamCount]; b++) {
		if (winnerteam == b) continue;
		if (ctfTStats[mapid][b][ctfScore] == winnerscore) {
			return INVALID_TEAM_ID;
		}
	}
	return winnerteam;
}

ctfUpdatePlayersText(mapid) { // Player positions and text draws
	
	new message[MAX_INPUT];
	
	new winnerteam = ctfGetWinnerTeamID(mapid);
	new winnername[MAX_CTF_NAME];
	if (winnerteam == INVALID_TEAM_ID) {
		winnername = "~w~Nil";
	} else {
		format(winnername, sizeof(winnername), "%s%s", ctfColours[winnerteam][ctfcGametextColour], ctfColours[winnerteam][ctfcName]);
	}
	
	LoopPlayers(i) {
		if (ctfPStats[i][ctfpMapID] != mapid) continue;
		if (!ctfPStats[i][ctfIsTextDrawOn]) continue;
		if (ctfPStats[i][ctfpTeamID] == INVALID_TEAM_ID) continue;
		
		format(message, sizeof(message), "%sTeam Score: ~w~%i  %sWinning Team: %s",
			//ctfColours[ctfPStats[i][ctfpTeamID]][ctfcGametextColour], ctfPStats[i][ctfpKills],
			//ctfColours[ctfPStats[i][ctfpTeamID]][ctfcGametextColour], ctfPStats[i][ctfpDeaths],
			ctfColours[ctfPStats[i][ctfpTeamID]][ctfcGametextColour], winnername);
		
		if (!ctfPStats[i][ctfTextString][0] || strcmp(ctfPStats[i][ctfTextString], message)) {
			TextDrawSetString(ctfPStats[i][ctfText], message);
			strcpy(ctfPStats[i][ctfTextString], message, MAX_INPUT);
		}
	}
	return 1;
}

ctfUpdatePlayersTime(mapid) {
	new text[MAX_INPUT];
	format(text, sizeof(text), "~g~Time Remaining: ~w~%s", GetTimeString2(ctfMaps[mapid][ctfRunTime] * 1000 - (GetTickCount() - ctfMaps[mapid][ctfStartTime])));
	
	LoopPlayers(i) {
		if (ctfPStats[i][ctfpMapID] != mapid ||
			!ctfPStats[i][ctfIsTextDrawOn] ||
			ctfPStats[i][ctfpTeamID] == INVALID_TEAM_ID) continue;
		
		text[1] = ctfColours[ctfPStats[i][ctfpTeamID]][ctfcGametextColour][1];
		TextDrawSetString(ctfPStats[i][ctfpTime], text);
	}
	return 1;
}

ctfUpdatePlayerInfoShow(mapid, playerid) {
	#pragma unused mapid
	if (ctfPStats[playerid][ctfIsTextDrawOn]) return 1;
	
	ctfPStats[playerid][ctfIsTextDrawOn] = 1;
	ctfPStats[playerid][ctfText] = TextDrawCreate(150.0, 430.0, " ");
	ctfPStats[playerid][ctfpTime] = TextDrawCreate(20.0, 300.0, " ");
	
	//new text[MAX_INPUT];
	//format(text, sizeof(text), "~g~~h~~h~Team: %s%s", ctfColours[ctfPStats[playerid][ctfpTeamID]][ctfcGametextColour], ctfColours[ctfPStats[playerid][ctfpTeamID]][ctfcName]);
	
	//ctfPStats[playerid][ctfpTeam] = TextDrawCreate(20.0, 320.0, text);
	//TextDrawColor(ctfPStats[playerid][ctfpTeam], ctfTStats[mapid][ctfPStats[playerid][ctfpTeamID]][ctfColour]);
	
	TextDrawShowForPlayer(playerid, ctfPStats[playerid][ctfText]);
	TextDrawShowForPlayer(playerid, ctfPStats[playerid][ctfpTime]);
	//TextDrawShowForPlayer(playerid, ctfPStats[playerid][ctfpTeam]);
	ctfPStats[playerid][ctfTextString][0] = ' ';
	return 1;
}

ctfUpdatePlayerInfoHide(playerid) {
	if (!ctfPStats[playerid][ctfIsTextDrawOn]) return 1;
	
	ctfPStats[playerid][ctfIsTextDrawOn] = 0;
	
	new Text:tmp;
	TextDrawDestroy(ctfPStats[playerid][ctfText]);
	TextDrawDestroy(ctfPStats[playerid][ctfpTime]);
	//TextDrawDestroy(ctfPStats[playerid][ctfpTeam]);
	
	ctfPStats[playerid][ctfText] = tmp;
	ctfPStats[playerid][ctfpTime] = tmp;
	//ctfPStats[playerid][ctfpTeam] = tmp;
	strclr(ctfPStats[playerid][ctfTextString]);
	return 1;
}

ctfUpdatePlayerInfoUnload() { // Destroy all textdraws
	LoopPlayers(i) {
		ctfUpdatePlayerInfoHide(i);
	}
}

kcmd:tf(playerid, text[]) { // FIX this later
	// Send team chat to team
	if (ctfPStats[playerid][ctfpMapID] == INVALID_MAP_ID || ctfPStats[playerid][ctfpTeamID] == INVALID_TEAM_ID) {
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
		if (ctfPStats[playerid][ctfpMapID] == ctfPStats[i][ctfpMapID] && // Save map
			ctfPStats[playerid][ctfpTeamID] == ctfPStats[i][ctfpTeamID]) { // Same team
			
			Bit_Set(players, i, 1, sizeof(players));
		}
	}
	SendWrappedMessageToClients(players, ctfTStats[ctfPStats[playerid][ctfpMapID]][ctfPStats[playerid][ctfpTeamID]][ctfColour], message);
	
	printf("[FDM] %s", message);
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// SPECTATE EVENTS
//-----------------------------------------------------------------------------------------------------

ctf_OnPlayerSpectate(playerid, specid) {
	if (ctfPStats[playerid][ctfpMapID] != INVALID_MAP_ID && ctfPStats[playerid][ctfIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, ctfPStats[playerid][ctfText]);
		TextDrawHideForPlayer(playerid, ctfPStats[playerid][ctfpTime]);
		//TextDrawHideForPlayer(playerid, ctfPStats[playerid][ctfpTeam]);
	}
	if (ctfPStats[specid][ctfpMapID] != INVALID_MAP_ID && ctfPStats[specid][ctfIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, ctfPStats[specid][ctfText]);
		TextDrawShowForPlayer(playerid, ctfPStats[specid][ctfpTime]);
		//TextDrawShowForPlayer(playerid, ctfPStats[specid][ctfpTeam]);
	}
	return 1;
}

ctf_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid) {
	if (ctfPStats[oldspecid][ctfpMapID] != INVALID_MAP_ID && ctfPStats[oldspecid][ctfIsTextDrawOn]) {
		TextDrawHideForPlayer(playerid, ctfPStats[oldspecid][ctfText]);
		TextDrawHideForPlayer(playerid, ctfPStats[oldspecid][ctfpTime]);
		//TextDrawHideForPlayer(playerid, ctfPStats[oldspecid][ctfpTeam]);
	}
	if (ctfPStats[newspecid][ctfpMapID] != INVALID_MAP_ID && ctfPStats[newspecid][ctfIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, ctfPStats[newspecid][ctfText]);
		TextDrawShowForPlayer(playerid, ctfPStats[newspecid][ctfpTime]);
		//TextDrawShowForPlayer(playerid, ctfPStats[newspecid][ctfpTeam]);
	}
	return 1;
}

ctf_OnPlayerUnspectate(playerid, specid) {
	if (ctfPStats[playerid][ctfpMapID] != INVALID_MAP_ID && ctfPStats[playerid][ctfIsTextDrawOn]) {
		TextDrawShowForPlayer(playerid, ctfPStats[playerid][ctfText]);
		TextDrawShowForPlayer(playerid, ctfPStats[playerid][ctfpTime]);
		//TextDrawShowForPlayer(playerid, ctfPStats[playerid][ctfpTeam]);
	}
	if (ctfPStats[specid][ctfpMapID] != INVALID_MAP_ID && ctfPStats[specid][ctfIsTextDrawOn]) {
		print("test1");
		TextDrawHideForPlayer(playerid, ctfPStats[specid][ctfText]);
		TextDrawHideForPlayer(playerid, ctfPStats[specid][ctfpTime]);
		//TextDrawHideForPlayer(playerid, ctfPStats[specid][ctfpTeam]);
		print("test2");
	}
	return 1;
}
