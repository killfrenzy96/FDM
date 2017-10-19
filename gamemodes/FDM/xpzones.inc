#include <a_samp>

#define FILE_XPZONES "FDM/Settings/XP Zones.ini"
#define INVALID_ZONE_ID -1
#define MAX_XPZONES 48
#define COLOUR_XPZONEHELP 0x44CCFFAA

#define MAX_XPZONE_NAME 32

enum playerXPCacheInfo {
	cZoneID,
	cXP,
	cCash
}
enum xpZoneInfo {
	zCheckpointID,
	zName[MAX_XPZONE_NAME],
	zMinPlayers,
	zXP,
	zCash,
	Float:zX,
	Float:zY,
	Float:zZ,
	Float:zRadius,
	zWorld
}
new pxpData[MAX_PLAYERS][playerXPCacheInfo];
new xpZones[MAX_XPZONES][xpZoneInfo];

XPZoneInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		pxpData[i][cZoneID] = INVALID_ZONE_ID;
	}
	for (new i; i < MAX_XPZONES; i++) {
		xpZones[i][zCheckpointID] = INVALID_CHECKPOINT_ID;
	}
	LoadXPZones();
	kEnableTimer(21); // SetTimer("fdm_XPZoneProcess", 6250, true);
	return 1;
}

zones_OnPlayerDisconnect(playerid) {
	pxpData[playerid][cZoneID] = INVALID_ZONE_ID;
	pxpData[playerid][cXP] = 0;
	pxpData[playerid][cCash] = 0;
	return 1;
}

LoadXPZones() {
	if (!INI_Open(FILE_XPZONES)) return 0;
	
	//new key[MAX_REAL + 1];
	//new readstring[MAX_STRING];
	
	new i2;
	for (new i; gCache[i][E_VALUE][0] && i < INI_MAX_LINES; i++) { // Read directly from the cache
		if (gCache[i][E_KEY][0] && !strcmp(gCache[i][E_KEY], "XP", false)) {
			// Get the data
			new idx;
			new tmpdata[xpZoneInfo];
			splitcpy(tmpdata[zName], gCache[i][E_VALUE], idx, ',', MAX_XPZONE_NAME);
			tmpdata[zXP] = strval(split(gCache[i][E_VALUE], idx, ','));
			tmpdata[zCash] = strval(split(gCache[i][E_VALUE], idx, ','));
			tmpdata[zX] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			tmpdata[zY] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			tmpdata[zZ] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			tmpdata[zRadius] = floatstr(split(gCache[i][E_VALUE], idx, ','));
			tmpdata[zWorld] = strval(split(gCache[i][E_VALUE], idx, ','));
			tmpdata[zMinPlayers] = strval(split(gCache[i][E_VALUE], idx, ','));
			
			// Check the data's integrity
			if (!tmpdata[zName][0]) continue;
			if (tmpdata[zRadius] == 0.0) continue;
			if (tmpdata[zMinPlayers] < 0) tmpdata[zMinPlayers] = 0;
			
			// All good, put that data into the xp zone spots
			xpZones[i2] = tmpdata;
			
			// Create a checkpoint for it
			xpZones[i2][zCheckpointID] = AddCheckpoint(xpZones[i2][zX], xpZones[i2][zY], xpZones[i2][zZ], 10.0, xpZones[i2][zWorld]);
			
			i2++;
		}
		if (i2 >= MAX_XPZONES) break; // Maximum objects reached
	}
	
	/*for (new i; i < MAX_XPZONES; i++) {
		format(key, sizeof(key), "XP%i", i);
		if (!INI_ReadString(readstring, key)) continue;
		
		// Get the data
		new idx;
		new tmpdata[xpZoneInfo];
		splitcpy(tmpdata[zName], readstring, idx, ',', MAX_XPZONE_NAME);
		tmpdata[zXP] = strval(split(readstring, idx, ','));
		tmpdata[zCash] = strval(split(readstring, idx, ','));
		tmpdata[zX] = floatstr(split(readstring, idx, ','));
		tmpdata[zY] = floatstr(split(readstring, idx, ','));
		tmpdata[zZ] = floatstr(split(readstring, idx, ','));
		tmpdata[zRadius] = floatstr(split(readstring, idx, ','));
		
		// Check the data's integrity
		if (!tmpdata[zName][0]) continue;
		if (tmpdata[zRadius] == 0.0) continue;
		
		// All good, put that data into the xp zone spots
		xpZones[i] = tmpdata;
		
		// Create a checkpoint for it
		xpZones[i][zCheckpointID] = AddCheckpoint(xpZones[i][zX], xpZones[i][zY], xpZones[i][zZ]);
	}*/
	INI_Close();
	return 1;
}

UnloadXPZones() {
	new tmpdata[xpZoneInfo];
	for (new i; i < MAX_XPZONES; i++) {
		RemoveCheckpoint(xpZones[i][zCheckpointID]);
		xpZones[i] = tmpdata;
		xpZones[i][zCheckpointID] = INVALID_CHECKPOINT_ID;
	}
	
	new tmpdata2[playerXPCacheInfo];
	for (new i; i < MAX_XPZONES; i++) {
		pxpData[i] = tmpdata2;
		pxpData[i][cZoneID] = INVALID_ZONE_ID;
	}
	return 1;
}

/*GetXPZoneIDFromCPID(checkpointid) {
	if (checkpointid == INVALID_CHECKPOINT_ID) return INVALID_ZONE_ID;
	for (new i; i < MAX_XPZONES; i++) {
		if (xpZones[i][zCheckpointID] == checkpointid) return i;
	}
	return INVALID_ZONE_ID;
}*/

stock IsPlayerNearAnyZone(playerid) {
	for (new xpzoneid; xpzoneid < MAX_XPZONES; xpzoneid++) {
		if (GetPlayerVirtualWorld(playerid) == xpZones[xpzoneid][zWorld]) {
			
			new Float:distance;
			new Float:px;
			new Float:py;
			new Float:pz;
			
			GetPlayerPos(playerid, px, py, pz);
			
			distance = FindDistance(px, py, pz,
				xpZones[xpzoneid][zX], xpZones[xpzoneid][zY], xpZones[xpzoneid][zZ]);
			
			if (distance < xpZones[xpzoneid][zRadius] * 3) return 1; // They are in the zone
			
		}
	}
	return 0;
}

forward fdm_XPZoneProcess();
public fdm_XPZoneProcess() {
	new playerisinanyxp[MAX_PLAYERS];
	new Float:distance;
	new Float:px;
	new Float:py;
	new Float:pz;
	
	for (new xpzoneid; xpzoneid < MAX_XPZONES; xpzoneid++) {
		if (xpZones[xpzoneid][zCheckpointID] == INVALID_CHECKPOINT_ID) continue; // No xp zone exists there
		
		// Loop through all players to check if they're within range of the xp zone
		new playersinxp[MAX_PLAYERS];
		new rivalgangs;
		
		LoopPlayers(playerid) {
			if (!IsPlayerConnected(playerid)) continue;
			if (!IsPlayerSpawned(playerid)) continue;
			if (IsPlayerPaused(playerid, 5000)) continue; // Can't get xp while paused
			if (GetPlayerVirtualWorld(playerid) != xpZones[xpzoneid][zWorld]) continue;
			if (IsPlayerInMinimode(playerid)) continue; // Can't get xp while in minimode
			
			// Check distance
			GetPlayerPos(playerid, px, py, pz);
			
			new Float:radius = xpZones[xpzoneid][zRadius];
			if (px < xpZones[xpzoneid][zX] - radius ||
				px > xpZones[xpzoneid][zX] + radius ||
				py < xpZones[xpzoneid][zY] - radius ||
				py > xpZones[xpzoneid][zY] + radius ||
				pz < xpZones[xpzoneid][zZ] - radius ||
				pz > xpZones[xpzoneid][zZ] + radius) {
				
				continue;
			} else {
				distance = FindDistance(px, py, pz,
					xpZones[xpzoneid][zX], xpZones[xpzoneid][zY], xpZones[xpzoneid][zZ]);
				
				if (distance > radius) continue; // Check if the xp zone is in range
			}
			
			playersinxp[playerid] = 1; // Player is in xp zone
			playerisinanyxp[playerid] = 1;
		}
		
		// Check if there are rival gangs/players
		new allyplayers[MAX_PLAYERS];
		
		LoopPlayers(playerid) {
			if (!playersinxp[playerid]) continue; // Not in xp zone
			allyplayers[playerid]++;
			
			LoopPlayersEx(rivalid, bb) {
				if (!playersinxp[rivalid]) continue; // Not in xp zone
				if (playerid == rivalid) continue; // Player cannot rival themselves
				
				if (cData[rivalid][cGangID] == INVALID_GANG_ID) { // Someone not in a gang
					rivalgangs = 1;
					break;
				}
				
				if (cData[playerid][cGangID] != cData[rivalid][cGangID]) { // Someone not the player's gang
					rivalgangs = 1;
					break;
				}
				
				allyplayers[playerid]++;
			}
		}
		
		// TODO: Adjust player zoneid/xp/cash accordingly
		LoopPlayers(playerid) {
			if (!playersinxp[playerid]) continue;
			
			if ((pxpData[playerid][cZoneID] == INVALID_ZONE_ID) || pxpData[playerid][cZoneID] != xpzoneid) {
				// Player has just entered XP zone
				pxpData[playerid][cZoneID] = xpzoneid;
				pxpData[playerid][cXP] = 0;
				pxpData[playerid][cCash] = 0;
				
				new tmpstr[MAX_STRING];
				
				if (xpZones[pxpData[playerid][cZoneID]][zMinPlayers]) {
					format(tmpstr, sizeof(tmpstr), "You entered the zone '%s' | XP Gain: %i | Cash Gain: %i | Min Members: %i", xpZones[pxpData[playerid][cZoneID]][zName], xpZones[pxpData[playerid][cZoneID]][zXP], xpZones[pxpData[playerid][cZoneID]][zCash], xpZones[pxpData[playerid][cZoneID]][zMinPlayers]);
				} else {
					format(tmpstr, sizeof(tmpstr), "You entered the zone '%s' | XP Gain: %i | Cash Gain: %i", xpZones[pxpData[playerid][cZoneID]][zName], xpZones[pxpData[playerid][cZoneID]][zXP], xpZones[pxpData[playerid][cZoneID]][zCash]);
				}
				
				SendClientMessage(playerid, COLOUR_XPZONEHELP, tmpstr);
				
				if (xpZones[xpzoneid][zXP] && !pData[playerid][pIsLoggedIn]) {
					SendClientMessage(playerid, COLOUR_ERROR, "ZONES: You must be registered to gain XP - /register [password]");
				}
			}
			
			pxpData[playerid][cXP] = pxpData[playerid][cXP] + (xpZones[xpzoneid][zXP] / 20);
			pxpData[playerid][cCash] = pxpData[playerid][cCash] + (xpZones[xpzoneid][zCash] / 20);
			if (pxpData[playerid][cXP] > xpZones[xpzoneid][zXP]) pxpData[playerid][cXP] = xpZones[xpzoneid][zXP];
			if (pxpData[playerid][cCash] > xpZones[xpzoneid][zCash]) pxpData[playerid][cCash] = xpZones[xpzoneid][zCash];
			
			if (rivalgangs) {
				// Someone is rivaling their zone
				pxpData[playerid][cXP] = pxpData[playerid][cXP] / 2;
				pxpData[playerid][cCash] = pxpData[playerid][cCash] / 2;
				
				new tmpstr[MAX_STRING];
				format(tmpstr, sizeof(tmpstr), "Someone is rivaling your zone '%s'", xpZones[pxpData[playerid][cZoneID]][zName]);
				SendClientMessage(playerid, COLOUR_XPZONEHELP, tmpstr);
			}
			
			if (xpZones[pxpData[playerid][cZoneID]][zMinPlayers] && allyplayers[playerid] < xpZones[pxpData[playerid][cZoneID]][zMinPlayers]) {
				pxpData[playerid][cXP] = pxpData[playerid][cXP] / 2;
				pxpData[playerid][cCash] = pxpData[playerid][cCash] / 2;
				
				new tmpstr[MAX_STRING];
				format(tmpstr, sizeof(tmpstr), "You must have at least %i allies in your zone '%s'", xpZones[pxpData[playerid][cZoneID]][zMinPlayers], xpZones[pxpData[playerid][cZoneID]][zName]);
				SendClientMessage(playerid, COLOUR_XPZONEHELP, tmpstr);
			}
			
			// Give xp/cash
			SendXPMessageToPlayer(playerid, GivePlayerXP(playerid, pxpData[playerid][cXP]), 1);
			GivePlayerMoney(playerid, pxpData[playerid][cCash]);
		}
	}
	
	// Reset player data for anyone not in an xp zone
	LoopPlayers(playerid) {
		if (!playerisinanyxp[playerid]) {
			
			if (pxpData[playerid][cZoneID] != INVALID_ZONE_ID) {
				// Player just exited XP zone
				new tmpstr[MAX_STRING];
				format(tmpstr, sizeof(tmpstr), "You have left the zone '%s'", xpZones[pxpData[playerid][cZoneID]][zName]);
				SendClientMessage(playerid, COLOUR_XPZONEHELP, tmpstr);
			}
			
			pxpData[playerid][cZoneID] = INVALID_ZONE_ID;
			pxpData[playerid][cXP] = 0;
			pxpData[playerid][cCash] = 0;
			continue;
		}
	}
}
