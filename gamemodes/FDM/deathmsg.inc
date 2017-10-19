#include <a_samp>

new pIsSuicide[MAX_PLAYERS];
new pSuicideIcon[MAX_PLAYERS];

deathmsg_OnPlayerDeath(playerid, &killerid, &reason, &noloss) {
	
	if (killerid != INVALID_PLAYER_ID) {
		if (racePStats[playerid][rcMapID] != INVALID_MAP_ID || derbyPStats[playerid][dpMapID] != INVALID_MAP_ID ||
			racePStats[killerid][rcMapID] != INVALID_MAP_ID || derbyPStats[killerid][dpMapID] != INVALID_MAP_ID) {
			
			killerid = INVALID_PLAYER_ID;
			if (reason != 54) {
				reason = 255;
			}
		}
	}
	
	#define pmodeid mpData[playerid][mpLastJoinModeID]
	#define kmodeid mpData[killerid][mpLastJoinModeID]
	new pillegal, killegal, pcolour, kcolour, forcelegal;
	
	if (!pIsSuicide[playerid]) {
		if (mpData[playerid][mpModeID] == INVALID_MODE_ID) {
			if (skinPlayerData[playerid][pSelecting] || GetTickCount() - skinPlayerData[playerid][pLastSpawn] < SPAWN_GODMODE_TIME + 1000) {
				pillegal = 1;
				noloss = 1;
			} else {
				if (pmodeid != INVALID_MODE_ID && modes[pmodeid][mState] == MODE_STATE_WAITING && !GetPlayerVirtualWorld(playerid)) { // Not lose xp on dying if waiting at minimode
					new Float:px, Float:py, Float:pz, Float:distance;
					GetPlayerPos(playerid, px, py, pz);
					distance = FindDistance(px, py, pz,
						cpData[modes[pmodeid][mCheckpointID]][cX], cpData[modes[pmodeid][mCheckpointID]][cY], cpData[modes[pmodeid][mCheckpointID]][cZ]);
					if (distance < cpData[modes[pmodeid][mCheckpointID]][cSize] * 2.5) {
						pillegal = 1;
						noloss = 1;
					}
				}
			}
		} else {
			if (killerid != INVALID_PLAYER_ID) {
				if (mpData[playerid][mpModeID] != mpData[killerid][mpModeID]) {
					//pillegal = 1;
					killegal = 1;
					noloss = 1;
				} else {
					forcelegal = 1;
				}
			}
		}
		
		if (!forcelegal && killerid != INVALID_PLAYER_ID) {
			if (mpData[playerid][mpModeID] != mpData[killerid][mpModeID] ||
				skinPlayerData[killerid][pSelecting] || GetTickCount() - skinPlayerData[killerid][pLastSpawn] < SPAWN_GODMODE_TIME + 1000) {
				killegal = 1;
				noloss = 1;
			} else {
				if (kmodeid != INVALID_MODE_ID && modes[kmodeid][mState] == MODE_STATE_WAITING && !GetPlayerVirtualWorld(killerid)) { // Not lose xp on dying if waiting at minimode
					new Float:px, Float:py, Float:pz, Float:distance;
					GetPlayerPos(killerid, px, py, pz);
					distance = FindDistance(px, py, pz,
						cpData[modes[kmodeid][mCheckpointID]][cX], cpData[modes[kmodeid][mCheckpointID]][cY], cpData[modes[kmodeid][mCheckpointID]][cZ]);
					if (distance < cpData[modes[kmodeid][mCheckpointID]][cSize] * 2.5) {
						killegal = 1;
						noloss = 1;
					}
				}
			}
		}
	}
	
	if (kIsPlayerOnGodmode(playerid)) {
		pillegal = 1;
		noloss = 1;
	}
	if (killerid != INVALID_PLAYER_ID && kIsPlayerOnGodmode(killerid)) {
		killegal = 1;
		noloss = 1;
	}
	
	if (!forcelegal) {
		if (pillegal) {
			pcolour = GetPlayerColor(playerid);
			SetPlayerColor(playerid, 0);
		}
		if (killegal) {
			kcolour = GetPlayerColor(killerid);
			SetPlayerColor(killerid, 0);
		}
	} else {
		noloss = 0;
	}
	
	if (pIsSuicide[playerid]) {
		new showKiller = killerid;
		if (killerid == INVALID_PLAYER_ID) showKiller = playerid;
		
		if (pSuicideIcon[playerid] == -1) {
			SendDeathMessage(showKiller, playerid, reason);
		} else {
			SendDeathMessage(showKiller, playerid, pSuicideIcon[playerid]);
		}
		pIsSuicide[playerid] = 0;
	} else {
		SendDeathMessage(killerid, playerid, reason);
	}
	
	if (!forcelegal) {
		if (pillegal) SetPlayerColor(playerid, pcolour);
		if (killegal) SetPlayerColor(killerid, kcolour);
	}
	
	#undef pmodeid
	#undef kmodeid
	return 1;
}

KillPlayer(playerid, suicideicon = -1) {
	if (!IsPlayerSpawned(playerid)) return 0;
	
	if (suicideicon == -1) {
		pIsSuicide[playerid] = 0;
	} else {
		pIsSuicide[playerid] = 1;
		pSuicideIcon[playerid] = suicideicon;
	}
	
	SetPlayerHealth(playerid, 0.0);
	return 1;
}
