#include <a_samp>

#define COLOUR_XPMESSAGE 0xAADDFFAA
#define COLOUR_XPGAIN 0xDDFF11AA
#define COLOUR_XPLOSS 0xFF1111AA

#define XP_LOSS_DISVISOR 50
#define XP_GANG_DIVISOR 10

#define FILE_LEVELS "FDM/Settings/Levels.ini"
#define MAX_LEVELS 1024

enum levelPlayerInfo {
	lpTextExist,
	Text:lpBackRectangle,
	Text:lpFrontRectangle,
	Text:lpLevel
}
enum levelDataInfo {
	lMax,
	lList[MAX_LEVELS],
	lMaxLevelGain
}
new lpData[MAX_PLAYERS][levelPlayerInfo];
new lData[levelDataInfo];

//-----------------------------------------------------------------------------------------------------
// LEVELS / EXPERIENCE HANDLING
//-----------------------------------------------------------------------------------------------------

LevelsInit() {
	// Load levels data
	LoadLevelsList();
	LevelsTextInit();
	return 1;
}

LoadLevelsList() {
	if (!INI_Open(FILE_LEVELS)) return 0;
	
	new key[MAX_REAL + 1];
	new maxlevel = MAX_LEVELS;
	new xp;
	
	for (new i = 1; i < MAX_LEVELS; i++) {		
		format(key, sizeof(key), "L%i", i);
		xp = INI_ReadInt(key);
		
		if (xp) {
			lData[lList][i] = xp;
		} else {
			maxlevel = i;
			break;
		}
	}
	lData[lMax] = maxlevel - 1;
	
	lData[lMaxLevelGain] = INI_ReadInt("MaxLevelXpGain");
	
	INI_Close();
	return 1;
}

LevelsExit() {
	LevelsTextExit();
	return 1;
}

UnloadLevelsList() {
	new tmp[levelDataInfo];
	lData = tmp;
	lData[lList][1] = 0xFFFFFFFF;
	lData[lMax] = 1;
	return 1;
}

levels_OnPlayerLogin(playerid) {
	UpdatePlayerLevel(playerid);
	GetNewSpawnPointIfInvalid(playerid);
	LevelsShowPlayerText(playerid);
	return 1;
}

levels_OnPlayerDisconnect(playerid) {
	LevelsHidePlayerText(playerid);
	return 1;
}

levels_OnPlayerSpawn(playerid) {
	// Set armour according to level
	SetPlayerArmour(playerid, (pData[playerid][pLevel] * 100) / lData[lMax]);
	return 1;
}

levels_OnPlayerDeath(playerid, killerid) {
	new xpgainloss;
	
	xpgainloss = GivePlayerXP(playerid, -(pData[playerid][pXP] / XP_LOSS_DISVISOR)); // Player loses XP for dying
	SendXPMessageToPlayer(playerid, xpgainloss);
	
	if (killerid == INVALID_PLAYER_ID) return 1;
	if (xpgainloss >= 0) return 1;
	
	// The killer 'xp-life steals', effiency depends on level
	new killxp = GivePlayerXP(killerid, floatround((float(-xpgainloss) * float(pData[killerid][pLevel])) / float(lData[lMax]), floatround_round), 0);
	SendXPMessageToPlayer(killerid, killxp);
	
	return 1;
}

UpdatePlayerLevel(playerid) {
	new oldlevel = pData[playerid][pLevel];
	if (pData[playerid][pXP] >= lData[lList][lData[lMax]]) {
		pData[playerid][pLevel] = lData[lMax];
	} else {
		for (new i = 1; i < lData[lMax] + 1; i++) {
			if (pData[playerid][pXP] >= lData[lList][i]) continue;
			pData[playerid][pLevel] = i - 1;
			break;
		}
	}
	if (oldlevel != pData[playerid][pLevel] &&
		GetPlayerScore(playerid) != pData[playerid][pLevel]) {
		
		SetPlayerScore(playerid, pData[playerid][pLevel]);
	}
	return 1;
}

stock GivePlayerXP(playerid, xp, gangxp = 1) {
	if (!pData[playerid][pIsLoggedIn]) return 0; // Not registered
	if (xp == 0) return 0;
	
	new minxp = lData[lList][pData[playerid][pLevel]];
	new maxxp = lData[lList][lData[lMax]];
	
	if (lData[lMaxLevelGain]) {
		maxxp = 0x7FFFFFFF; // Allow xp gain at maximum level
	} else if (pData[playerid][pLevel] == lData[lMax]) {
		return 0; // Block xp gain at max level
	}
	
	new newxp;
	
	newxp = pData[playerid][pXP] + xp;
	if (newxp > maxxp) newxp = maxxp;
	if (newxp < minxp) newxp = minxp;
	if (xp > 0 && newxp < pData[playerid][pXP]) newxp = maxxp; // Integer overflow protection
	
	new xpgain = newxp - pData[playerid][pXP];
	pData[playerid][pXP] = newxp;
	
	new oldlevel = pData[playerid][pLevel];
	
	if (xpgain <= 0) {
		if (xpgain) LevelsUpdatePlayerText(playerid);
		return xpgain;
	} else {
		UpdatePlayerLevel(playerid);
		LevelsUpdatePlayerText(playerid);
	}
	
	if (oldlevel != pData[playerid][pLevel]) {
		new message[MAX_INPUT];
		format(message, sizeof(message), "You are now level %i", pData[playerid][pLevel]);
		SendClientMessage(playerid, COLOUR_XPMESSAGE, message);
		
		OnPlayerLevelChange(playerid, oldlevel, pData[playerid][pLevel]);
	}
	
	// Gang XP
	if (!gangxp) return xpgain;
	if (cData[playerid][cGangID] == INVALID_GANG_ID) return xpgain;
	
	LoopPlayers(i) {
		if (playerid == i) continue; // Cannot get gang xp from self
		if (cData[playerid][cGangID] == cData[i][cGangID]) {
			GivePlayerXP(i, xpgain / XP_GANG_DIVISOR, 0);
		}
		/*if (cData[playerid][cGangID] == cData[i][cGangID] && pData[i][pXP] < maxxp) {
			oldlevel = pData[i][pLevel];
			
			pData[i][pXP] = pData[i][pXP] + (xpgain / XP_GANG_DIVISOR);
			if (pData[i][pXP] > maxxp) pData[i][pXP] = maxxp;
			
			UpdatePlayerLevel(i);
			
			if (oldlevel != pData[i][pLevel]) {
				new message[MAX_INPUT];
				format(message, sizeof(message), "You are now level %i", pData[i][pLevel]);
				SendClientMessage(i, COLOUR_XPMESSAGE, message);
				
				OnPlayerLevelChange(i, oldlevel, pData[i][pLevel]);
			}
			
			LevelsUpdatePlayerText(i);
		}*/
	}
	
	return xpgain;
}

stock SendXPMessageToPlayer(playerid, xp, type = 0) {
	// type = 0 = text message
	// type = 1 = big text
	
	if (!xp) return 0;
	
	new message[MAX_INPUT];
	if (xp > 0) {
		if (!type) {
			format(message, sizeof(message), "You have gained %s XP", kvalstr(xp));
			SendClientMessage(playerid, COLOUR_XPGAIN, message);
		} else {
			format(message, sizeof(message), "+%s XP", kvalstr(xp));
			GameTextForPlayer(playerid, message, 1, 1);
		}
	} else {
		if (!type) {
			format(message, sizeof(message), "You have lost %s XP", kvalstr(-xp));
			SendClientMessage(playerid, COLOUR_XPLOSS, message);
		} else {
			format(message, sizeof(message), "-%s XP", kvalstr(-xp));
			GameTextForPlayer(playerid, message, 1, 1);
		}
	}
	return 1;
}

stock SetPlayerLevel(playerid, level) {
	if (level < 0) return 0;
	if (level > lData[lMax]) return 0;
	new oldlevel = pData[playerid][pLevel];
	pData[playerid][pXP] = lData[lList][level];
	UpdatePlayerLevel(playerid);
	
	if (oldlevel != pData[playerid][pLevel]) {
		OnPlayerLevelChange(playerid, oldlevel, pData[playerid][pLevel]);
	}
	
	LevelsUpdatePlayerText(playerid);
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// TEXT DRAW HANDLING
//-----------------------------------------------------------------------------------------------------

#define LEVEL_REC_MIN 505.0
#define LEVEL_REC_MAX 601.0

LevelsTextInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		if (pData[i][pIsLoggedIn]) {
			LevelsShowPlayerText(i);
		}
	}
	return 1;
}

LevelsTextExit() {
	for (new i; i < MAX_PLAYERS; i++) {
		LevelsHidePlayerText(i);
	}
	return 1;
}

LevelsShowPlayerText(playerid) {
	if (!lpData[playerid][lpTextExist]) {
		lpData[playerid][lpBackRectangle] = TextDrawCreate(504.0, 14.0, " ");
		TextDrawTextSize(lpData[playerid][lpBackRectangle], 603.0, 20.0);
		TextDrawBoxColor(lpData[playerid][lpBackRectangle], 0x22661166);
		TextDrawUseBox(lpData[playerid][lpBackRectangle], 1);
		
		lpData[playerid][lpFrontRectangle] = TextDrawCreate(505.0, 15.0, " ");
		TextDrawTextSize(lpData[playerid][lpFrontRectangle], 505.0, 20.0);
		TextDrawBoxColor(lpData[playerid][lpFrontRectangle], 0x66FF4466);
		TextDrawUseBox(lpData[playerid][lpFrontRectangle], 1);
		
		lpData[playerid][lpLevel] = TextDrawCreate(555.0, 12.0, " ");
		TextDrawAlignment(lpData[playerid][lpLevel], 2);
		TextDrawLetterSize(lpData[playerid][lpLevel], 0.265, 0.85);
		TextDrawSetShadow(lpData[playerid][lpLevel], 1);
		
		lpData[playerid][lpTextExist] = 1;
	}
	
	LevelsUpdatePlayerText(playerid);
	
	TextDrawShowForPlayer(playerid, lpData[playerid][lpBackRectangle]);
	TextDrawShowForPlayer(playerid, lpData[playerid][lpFrontRectangle]);
	TextDrawShowForPlayer(playerid, lpData[playerid][lpLevel]);
	
	return 1;
}

LevelsHidePlayerText(playerid) {
	if (lpData[playerid][lpTextExist]) {
		TextDrawDestroy(lpData[playerid][lpBackRectangle]);
		TextDrawDestroy(lpData[playerid][lpFrontRectangle]);
		TextDrawDestroy(lpData[playerid][lpLevel]);
		lpData[playerid][lpTextExist] = 0;
	}
	return 1;
}

LevelsUpdatePlayerText(playerid) {
	if (lpData[playerid][lpTextExist]) {
		new msg[MAX_INPUT];
		
		if (pData[playerid][pLevel] < lData[lMax]) {
			// Show as normal
			
			new xpcurrent = pData[playerid][pXP] - lData[lList][pData[playerid][pLevel]],
				xplevel = lData[lList][pData[playerid][pLevel] + 1] - lData[lList][pData[playerid][pLevel]];
			
			if (xplevel >= 100000) {
				format(msg, sizeof(msg), "Level %i [%ik/%ik]", pData[playerid][pLevel], xpcurrent / 1000, xplevel / 1000);
			} else {
				format(msg, sizeof(msg), "Level %i [%i/%i]", pData[playerid][pLevel], xpcurrent, xplevel);
			}
			
			TextDrawSetString(lpData[playerid][lpLevel], msg);
			
			if (xplevel > 0) {
				TextDrawTextSize(lpData[playerid][lpFrontRectangle],
					LEVEL_REC_MIN + (float(xpcurrent) / float(xplevel)) * (LEVEL_REC_MAX - LEVEL_REC_MIN), 20.0);
			} else {
				TextDrawTextSize(lpData[playerid][lpFrontRectangle], LEVEL_REC_MAX, 20.0);
			}
		} else {
			// Max level reached
			
			if (!lData[lMaxLevelGain]) {
				// Not allowed to gain xp
				format(msg, sizeof(msg), "Level %i", pData[playerid][pLevel]);
			} else {
				// Allowed to gain xp
				new xpcurrent = pData[playerid][pXP] - lData[lList][pData[playerid][pLevel]];
				
				if (xpcurrent >= 100000) {
					format(msg, sizeof(msg), "Level %i [+%ik]", pData[playerid][pLevel], xpcurrent / 1000);
				} else {
					format(msg, sizeof(msg), "Level %i [+%i]", pData[playerid][pLevel], xpcurrent);
				}
			}
			
			TextDrawSetString(lpData[playerid][lpLevel], msg);
			TextDrawTextSize(lpData[playerid][lpFrontRectangle], LEVEL_REC_MAX, 20.0);
		}
		
		if (GetPlayerSpectatingPlayer(playerid) == INVALID_PLAYER_ID) {
			TextDrawShowForPlayer(playerid, lpData[playerid][lpFrontRectangle]);
		}
		
		// Spectator support
		LoopPlayers(i) {
			if (playerid != i && GetPlayerSpectatingPlayer(i) == playerid) {
				TextDrawShowForPlayer(i, lpData[playerid][lpFrontRectangle]);
			}
		}
		return 1;
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------------------------------
// SPECTATE SUPPORT
//-----------------------------------------------------------------------------------------------------

levels_OnPlayerSpectate(playerid, specid) {
	if (lpData[playerid][lpTextExist]) {
		TextDrawHideForPlayer(playerid, lpData[playerid][lpBackRectangle]);
		TextDrawHideForPlayer(playerid, lpData[playerid][lpFrontRectangle]);
		TextDrawHideForPlayer(playerid, lpData[playerid][lpLevel]);
	}
	if (lpData[specid][lpTextExist]) {
		TextDrawShowForPlayer(playerid, lpData[specid][lpBackRectangle]);
		TextDrawShowForPlayer(playerid, lpData[specid][lpFrontRectangle]);
		TextDrawShowForPlayer(playerid, lpData[specid][lpLevel]);
	}
	return 1;
}

levels_OnPlayerSpecChangePlayer(playerid, oldspecid, newspecid) {
	if (lpData[oldspecid][lpTextExist]) {
		TextDrawHideForPlayer(playerid, lpData[oldspecid][lpBackRectangle]);
		TextDrawHideForPlayer(playerid, lpData[oldspecid][lpFrontRectangle]);
		TextDrawHideForPlayer(playerid, lpData[oldspecid][lpLevel]);
	}
	if (lpData[newspecid][lpTextExist]) {
		TextDrawShowForPlayer(playerid, lpData[newspecid][lpBackRectangle]);
		TextDrawShowForPlayer(playerid, lpData[newspecid][lpFrontRectangle]);
		TextDrawShowForPlayer(playerid, lpData[newspecid][lpLevel]);
	}
	return 1;
}

levels_OnPlayerUnspectate(playerid, specid) {
	if (lpData[playerid][lpTextExist]) {
		TextDrawShowForPlayer(playerid, lpData[playerid][lpBackRectangle]);
		TextDrawShowForPlayer(playerid, lpData[playerid][lpFrontRectangle]);
		TextDrawShowForPlayer(playerid, lpData[playerid][lpLevel]);
	}
	if (lpData[specid][lpTextExist]) {
		TextDrawHideForPlayer(playerid, lpData[specid][lpBackRectangle]);
		TextDrawHideForPlayer(playerid, lpData[specid][lpFrontRectangle]);
		TextDrawHideForPlayer(playerid, lpData[specid][lpLevel]);
	}
	return 1;
}

