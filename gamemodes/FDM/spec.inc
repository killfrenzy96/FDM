#include <a_samp>

#define COLOUR_SPECTATE 0xFF7700AA

#define KEY_SPEC_LEFT KEY_FIRE
#define KEY_SPEC_RIGHT KEY_HANDBRAKE
#define KEY_SPEC_EXIT KEY_SPRINT
#define KEY_SPEC_EXIT2 KEY_SECONDARY_ATTACK

#define ALAR_SPEC_ID -1

#if defined _alar_included
	#undef PutPlayerInVehicle
#endif
#define PutPlayerInVehicle kPutPlayerInVehicle

enum specPlayerInfo {
	spAllowedMode,
	spSpecPlayer,
	spSpecVehicle,
	spAlarSpecPlayer,
	spSpecWhileInMode,
	spIsAdminSpec,
	Float:spX,
	Float:spY,
	Float:spZ,
	Float:spRotZ
}
new spData[MAX_PLAYERS][specPlayerInfo];

SpecInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		spec_ResetPlayerData(i);
		/*new specid = GetAdminSpectating(i);
		if (specid != INVALID_PLAYER_ID) {
			SetTimerEx("OnAdminSpectate", 0, false, "iii", i, specid, INVALID_PLAYER_ID);
		}*/
	}
	return 1;
}

SpecExit() {
	LoopPlayers(i) {
		if (spData[i][spSpecPlayer] != INVALID_PLAYER_ID) {
			if (spData[i][spSpecPlayer] != ALAR_SPEC_ID && GetPlayerState(i) == PLAYER_STATE_SPECTATING) {
				TogglePlayerSpectating(i, 0);
				OnPlayerUnspectate(i, GetPlayerSpectatingPlayer(i), 1);
			}
		}
		spec_ResetPlayerData(i);
	}
	return 1;
}

spec_OnPlayerConnect(playerid) {
	spec_ResetPlayerData(playerid);
	return 1;
}

spec_OnPlayerDisconnect(playerid) {
	spec_ResetPlayerData(playerid);
	return 1;
}

spec_OnPlayerLeaveMinimode(playerid, modeid) {
	new specid = FindFirstPlayerInMinimode(modeid);
	
	LoopPlayers(i) {
		if (playerid == i) continue;
		
		if (spData[i][spSpecPlayer] == playerid) {
			if (specid == INVALID_PLAYER_ID) {
				UnspecPlayer(i);
			} else {
				SpecPlayer(i, specid);
			}
		}
	}
	
	spData[playerid][spAllowedMode] = modeid;
	
	return 1;
}

spec_OnMinimodeStop(modeid) {
	LoopPlayers(i) {
		if (spData[i][spSpecWhileInMode] == modeid) {
			spData[i][spSpecWhileInMode] = 0xF0000000;
		}
		
		if (spData[i][spAllowedMode] == modeid) {
			if (spData[i][spSpecPlayer] != INVALID_PLAYER_ID && spData[i][spSpecPlayer] != ALAR_SPEC_ID) {
				UnspecPlayer(i);
			}
			spData[i][spAllowedMode] = INVALID_MODE_ID;
		}
	}
	return 1;
}

spec_ResetPlayerData(playerid) {
	spData[playerid][spAllowedMode] = INVALID_MODE_ID;
	spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
	spData[playerid][spAlarSpecPlayer] = INVALID_PLAYER_ID;
	spData[playerid][spSpecWhileInMode] = INVALID_MODE_ID;
	spData[playerid][spIsAdminSpec] = 0;
	//spData[playerid][spSpecVehicle] = 0;
	return 1;
}

spec_OnPlayerStream(playerid, forplayerid) {
	if (spData[forplayerid][spSpecPlayer] == playerid && GetPlayerState(forplayerid) == PLAYER_STATE_SPECTATING) {
		new vid = GetPlayerVehicleID(playerid);
		if (vid) {
			PlayerSpectateVehicle(forplayerid, vid);
			spData[forplayerid][spSpecVehicle] = vid;
		} else {
			PlayerSpectatePlayer(forplayerid, playerid);
			spData[forplayerid][spSpecVehicle] = 0;
		}
	}
	return 1;
}

spec_OnVehicleStream(vehicleid, forplayerid) {
	if (spData[forplayerid][spSpecVehicle] == vehicleid && GetPlayerState(forplayerid) == PLAYER_STATE_SPECTATING) {
		new vid = GetPlayerVehicleID(spData[forplayerid][spSpecVehicle]);
		if (vid) {
			PlayerSpectateVehicle(forplayerid, vid);
			spData[forplayerid][spSpecVehicle] = vid;
		} else {
			PlayerSpectatePlayer(forplayerid, spData[forplayerid][spSpecVehicle]);
			spData[forplayerid][spSpecVehicle] = 0;
		}
	}
	return 1;
}

SpecPlayer(playerid, specid) {
	if (playerid == specid) return 0;
	if (!IsPlayerSpawned(specid)) return 0;
	
	if (IsPlayerSpawned(playerid)) {
		OnPlayerSpectate(playerid, specid);
	} else {
		if (spData[playerid][spSpecPlayer] != specid) {
			OnPlayerSpectateChangePlayer(playerid, spData[playerid][spSpecPlayer], specid);
		}
	}
	
	spData[playerid][spSpecPlayer] = specid;
	TogglePlayerSpectating(playerid, 1);
	SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(specid));
	SetPlayerInterior(playerid, GetPlayerInterior(specid));
	
	new vid = GetPlayerVehicleID(specid);
	if (vid) {
		PlayerSpectateVehicle(playerid, vid);
		spData[playerid][spSpecVehicle] = vid;
	} else {
		PlayerSpectatePlayer(playerid, specid);
		spData[playerid][spSpecVehicle] = 0;
	}
	
	new msg[128];
	format(msg, sizeof(msg), "~g~~h~~h~Now Spectating: ~w~%s", ReturnPlayerName(specid));
	kGameTextForPlayer(playerid, msg, 10000, 320.0, 300.0);
	
	return 1;
}

UnspecPlayer(playerid) {
	if (!IsPlayerConnected(playerid)) return 0;
	if (GetPlayerState(playerid) != PLAYER_STATE_SPECTATING) return 1;
	//spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
	
	SkinsSetNextSpawnPos(playerid,
		spData[playerid][spX], spData[playerid][spY], spData[playerid][spZ], spData[playerid][spRotZ]);
	
	SetPlayerVirtualWorld(playerid, 0);
	TogglePlayerSpectating(playerid, 0);
	//SpawnPlayer(playerid);
	
	kGameTextHide(playerid);
	return 1;
}

stock kPutPlayerInVehicle(playerid, vehicleid, seatid) {
	#if defined _alar_included
		new rtn = alarPutPlayerInVehicle(playerid, vehicleid, seatid);
	#else
		new rtn = PutPlayerInVehicle(playerid, vehicleid, seatid);
	#endif
	
	LoopPlayers(i) {
		if (spData[i][spSpecPlayer] == playerid) {
			TogglePlayerSpectating(i, 1);
			PlayerSpectateVehicle(i, vehicleid);
			spData[i][spSpecVehicle] = vehicleid;
		}
	}
	
	return rtn;
}

spec_OnPlayerSpectate(playerid, specid) {
	#pragma unused specid
	if (spData[playerid][spSpecPlayer] == ALAR_SPEC_ID) return 0;
	
	spData[playerid][spSpecWhileInMode] = mpData[playerid][mpModeID];
	
	SavePlayerInfo(playerid);
	
	if (mpData[specid][mpModeID] == INVALID_MODE_ID) {
		new vid = GetPlayerVehicleID(playerid);
		if (vid) {
			GetVehiclePos(vid, spData[playerid][spX], spData[playerid][spY], spData[playerid][spZ]);
			GetVehicleZAngle(vid, spData[playerid][spRotZ]);
		} else {
			GetPlayerPos(playerid, spData[playerid][spX], spData[playerid][spY], spData[playerid][spZ]);
			GetPlayerFacingAngle(playerid, spData[playerid][spRotZ]);
		}
		
	} else {
		spData[playerid][spX] = modes[mpData[specid][mpModeID]][mLocation][mX] + (random(5) - 2.5);
		spData[playerid][spY] = modes[mpData[specid][mpModeID]][mLocation][mY] + (random(5) - 2.5);
		spData[playerid][spZ] = modes[mpData[specid][mpModeID]][mLocation][mZ];
		
		GetPlayerFacingAngle(playerid, spData[playerid][spRotZ]);
	}
	return 1;
}

spec_OnPlayerUnspectate(playerid) {
	GetNewSpawnPoint(playerid);
	
	if (spData[playerid][spSpecPlayer] == ALAR_SPEC_ID) {
		spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
		//DisableNextSpawnWeapons(playerid);
		StartGodSpawning(playerid, 2000);
		return 1;
	}
	spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
	
	new loadinterior;
	if (spData[playerid][spSpecWhileInMode] == INVALID_MODE_ID || spData[playerid][spSpecWhileInMode] == 0xF0000000) {
		SetPlayerVirtualWorld(playerid, 0);
		if (spData[playerid][spAllowedMode] == INVALID_MODE_ID) {
			loadinterior = 1;
		} else {
			SetPlayerInterior(playerid, modes[spData[playerid][spAllowedMode]][mLocation][mwInteriorID]);
			loadinterior = 0;
		}
	} else {
		SetPlayerVirtualWorld(playerid, modes[spData[playerid][spSpecWhileInMode]][mVirtualWorld]);
		loadinterior = 1;
	}
	
	if (spData[playerid][spSpecWhileInMode] != mpData[playerid][mpModeID]) {
		return 0;
	} else {
		StartGodSpawning(playerid);
		SetPlayerPos(playerid, spData[playerid][spX], spData[playerid][spY], spData[playerid][spZ]);
		SetPlayerFacingAngle(playerid, spData[playerid][spRotZ]);
		LoadPlayerInfo(playerid, loadinterior);
		return 1;
	}
}

spec_OnPlayerSpawn(playerid) {
	if (spData[playerid][spSpecPlayer] != INVALID_PLAYER_ID && spData[playerid][spSpecPlayer] != ALAR_SPEC_ID) {
		if (OnPlayerUnspectate(playerid, spData[playerid][spSpecPlayer])) {
			spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
			return 1;
		} else {
			return 0;
		}
	}
	return 0;
}

spec_OnPlayerDeath(playerid, killerid) {
	/*if (killerid == INVALID_PLAYER_ID) {
		if (spData[playerid][spAllowedMode] == INVALID_MODE_ID) {
			UnspecPlayer(playerid);
		} else if (!PlayerSpecNext(playerid)) {
			UnspecPlayer(playerid);
		}
		return 1;
	}*/
	
	LoopPlayers(i) {
		if (spData[i][spAllowedMode] == INVALID_MODE_ID) continue;
		if (spData[i][spSpecPlayer] != playerid) continue;
		if (playerid == i) continue;
		
		if (killerid == INVALID_PLAYER_ID) {
			if (!PlayerSpecNext(i)) {
				UnspecPlayer(i);
			}
		} else {
			if (IsPlayerSpawned(killerid)) {
				if (IsPlayerInMinimode(killerid)) {
					SpecPlayer(i, killerid);
				} else {
					if (!PlayerSpecNext(i)) {
						UnspecPlayer(i);
					}
				}
			} else {
				UnspecPlayer(i);
			}
		}
	}
	return 1;
}

spec_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (newstate == PLAYER_STATE_SPECTATING && spData[playerid][spAllowedMode] == INVALID_MODE_ID && spData[playerid][spSpecPlayer] != INVALID_PLAYER_ID && spData[playerid][spSpecPlayer] != ALAR_SPEC_ID) {
		UnspecPlayer(playerid);
		OnPlayerUnspectate(playerid, spData[playerid][spSpecPlayer]);
		spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
		
	} else if (newstate == PLAYER_STATE_ONFOOT && (oldstate == PLAYER_STATE_DRIVER || oldstate == PLAYER_STATE_PASSENGER)) {
		LoopPlayers(i) {
			if (spData[i][spSpecPlayer] != playerid || playerid == i || spData[i][spAllowedMode] == INVALID_MODE_ID) continue;
			PlayerSpectatePlayer(i, playerid);
		}
		
	} else if (oldstate == PLAYER_STATE_ONFOOT && (newstate == PLAYER_STATE_DRIVER || newstate == PLAYER_STATE_PASSENGER)) {
		new vehicleid = GetPlayerVehicleID(playerid);
		
		if (vehicleid) {
			LoopPlayers(i) {
				if (spData[i][spSpecPlayer] != playerid || playerid == i || spData[i][spAllowedMode] == INVALID_MODE_ID) continue;
				PlayerSpectateVehicle(i, vehicleid);
			}
		}
		
	} else if (newstate != PLAYER_STATE_WASTED && !((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED)) {
		LoopPlayers(i) {
			if (spData[i][spSpecPlayer] != playerid) continue;
			if (playerid == i) continue;
			
			if (spData[i][spAllowedMode] == INVALID_MODE_ID) {
				UnspecPlayer(i);
				continue;
			}
			
			if (!PlayerSpecNext(i)) {
				UnspecPlayer(i);
			}
		}
		
	//} else if (spData[playerid][spSpecPlayer] != INVALID_PLAYER_ID && oldstate == PLAYER_STATE_SPECTATING && ((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED)) {
	} else if (spData[playerid][spSpecPlayer] != INVALID_PLAYER_ID && newstate == PLAYER_STATE_SPAWNED) {
		if (spData[playerid][spSpecPlayer] != ALAR_SPEC_ID && !IsPlayerInMinimode(playerid)) {
			OnPlayerUnspectate(playerid, spData[playerid][spSpecPlayer]);
			spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
		//} else if (spData[playerid][spSpecPlayer] == ALAR_SPEC_ID) {
			//DisableNextSpawnWeapons(playerid);
		}
	}
	return 1;
}

spec_OnPlayerInteriorChange(playerid, newinteriorid) {
	LoopPlayers(i) {
		if (spData[i][spSpecPlayer] != playerid) continue;
		if (playerid == i) continue;
		
		SetPlayerInterior(i, newinteriorid);
	}
	return 1;
}

spec_OnPlayerKeyStateChange(playerid, newkeys, oldkeys) {
	if (spData[playerid][spSpecPlayer] != INVALID_PLAYER_ID && spData[playerid][spSpecPlayer] != ALAR_SPEC_ID && GetPlayerState(playerid) == PLAYER_STATE_SPECTATING) {
		if (IsKeyJustDown(KEY_SPEC_LEFT, newkeys, oldkeys)) {
			PlayerSpecPrev(playerid);
		} else if (IsKeyJustDown(KEY_SPEC_RIGHT, newkeys, oldkeys)) {
			PlayerSpecNext(playerid);
		} else if (IsKeyJustDown(KEY_SPEC_EXIT, newkeys, oldkeys) || IsKeyJustDown(KEY_SPEC_EXIT2, newkeys, oldkeys)) {
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPEC: You have left spectate mode.");
			UnspecPlayer(playerid);
			
			new message[MAX_INPUT];
			format(message, sizeof(message), "%s left spectate mode", pData[playerid][pLoginName]);
			SideChatAddLine(message, COLOUR_SPECTATE);
			printf("[FDM] %s", message);
		}
	}
	return 1;
}

PlayerSpecNext(playerid) {
	if (GetPlayerState(playerid) != PLAYER_STATE_SPECTATING) return 0;
	if (spData[playerid][spAllowedMode] == INVALID_MODE_ID) return 0;
	
	new specid = INVALID_PLAYER_ID;
	for (new i = spData[playerid][spSpecPlayer] + 1; i < MAX_PLAYERS; i++) {
		if (mpData[i][mpModeID] != spData[playerid][spAllowedMode]) continue;
		if (!IsPlayerSpawned(i)) continue;
		
		specid = i;
		break;
	}
	if (specid == INVALID_PLAYER_ID) {
		for (new i = 0; i <= spData[playerid][spSpecPlayer]; i++) {
			if (mpData[i][mpModeID] != spData[playerid][spAllowedMode]) continue;
			if (!IsPlayerSpawned(i)) continue;
			
			specid = i;
			break;
		}
	}
	if (specid == INVALID_PLAYER_ID) {
		//UnspecPlayer(playerid);
		return 0;
	}
	SpecPlayer(playerid, specid);
	return 1;
}

PlayerSpecPrev(playerid) {
	if (GetPlayerState(playerid) != PLAYER_STATE_SPECTATING) return 0;
	if (spData[playerid][spAllowedMode] == INVALID_MODE_ID) return 0;
	
	new specid = INVALID_PLAYER_ID;
	for (new i = spData[playerid][spSpecPlayer] - 1; i >= 0; i--) {
		if (mpData[i][mpModeID] != spData[playerid][spAllowedMode]) continue;
		if (!IsPlayerSpawned(i)) continue;
		
		specid = i;
		break;
	}
	if (specid == INVALID_PLAYER_ID) {
		for (new i = MAX_PLAYERS - 1; i >= spData[playerid][spSpecPlayer]; i--) {
			if (mpData[i][mpModeID] != spData[playerid][spAllowedMode]) continue;
			if (!IsPlayerSpawned(i)) continue;
			
			specid = i;
			break;
		}
	}
	if (specid == INVALID_PLAYER_ID) {
		//UnspecPlayer(playerid);
		return 0;
	}
	SpecPlayer(playerid, specid);
	return 1;
}

FindFirstPlayerInMinimode(modeid) {
	if (modes[modeid][mType] == MODE_TYPE_INVALID ||
		modes[modeid][mState] != MODE_STATE_PROGRESS) return INVALID_PLAYER_ID;
	
	LoopPlayers(i) {
		if (mpData[i][mpModeID] != modeid) continue;
		if (!IsPlayerSpawned(i)) continue;
		return i;
	}
	
	return INVALID_PLAYER_ID;
}

//-----------------------------------------------------------------------------------------------------
// ALAR SUPPORT
//-----------------------------------------------------------------------------------------------------

stock GetPlayerSpectatingPlayer(playerid) {
	if (spData[playerid][spSpecPlayer] == ALAR_SPEC_ID) {
		if (spData[playerid][spAlarSpecPlayer] == FREE_SPECTATE_ID) {
			return INVALID_PLAYER_ID;
		} else {
			return spData[playerid][spAlarSpecPlayer];
		}
	} else {
		return spData[playerid][spSpecPlayer];
	}
}

IsAdminSpectating(playerid) {
	return spData[playerid][spIsAdminSpec];
}

public OnAdminSpectate(playerid, newid, oldid) {
	
	if (oldid == INVALID_PLAYER_ID) {
		spData[playerid][spIsAdminSpec] = 1;
	} else if (newid == INVALID_PLAYER_ID) {
		spData[playerid][spIsAdminSpec] = 0;
	}
	
	spData[playerid][spAlarSpecPlayer] = newid;
	
	if ( // Unspectate
		spData[playerid][spSpecPlayer] == ALAR_SPEC_ID &&
		(newid == INVALID_PLAYER_ID || newid == FREE_SPECTATE_ID)
		) {
		
		OnPlayerUnspectate(playerid, oldid);
		spData[playerid][spSpecPlayer] = INVALID_PLAYER_ID;
		//return -3;
		
	} else if ( // Spectate
		spData[playerid][spSpecPlayer] != ALAR_SPEC_ID &&
		(oldid == INVALID_PLAYER_ID || oldid == FREE_SPECTATE_ID) &&
		(newid != INVALID_PLAYER_ID && newid != FREE_SPECTATE_ID)
		) {
		
		spData[playerid][spSpecPlayer] = ALAR_SPEC_ID;
		OnPlayerSpectate(playerid, newid);
		//return -3;
		
	} else if ( // Spectate change player
		spData[playerid][spSpecPlayer] == ALAR_SPEC_ID && oldid != newid &&
		(newid != INVALID_PLAYER_ID && newid != FREE_SPECTATE_ID) &&
		(oldid != INVALID_PLAYER_ID && oldid != FREE_SPECTATE_ID)
		) {
		
		spData[playerid][spSpecPlayer] = ALAR_SPEC_ID;
		OnPlayerSpectateChangePlayer(playerid, oldid, newid);
		//return -3;
	
	} else if ( // Free spec to unspec
		oldid == FREE_SPECTATE_ID && newid == INVALID_PLAYER_ID
		) {
		
		GetNewSpawnPoint(playerid);
		DisableNextSpawnWeapons(playerid);
		StartGodSpawning(playerid, 2000);
	}
	
	return -4;
}

new playerSaveWeaponsWpn[MAX_PLAYERS][MAX_WEAPON_SLOTS];
new playerSaveWeaponsAmmo[MAX_PLAYERS][MAX_WEAPON_SLOTS];
new Float:playerSaveHP[MAX_PLAYERS];
new Float:playerSaveArmour[MAX_PLAYERS];
new playerSaveInterior[MAX_PLAYERS];
new playerCurrentWpn[MAX_PLAYERS];

SavePlayerInfo(playerid) {
	//if (!IsPlayerSpawned(playerid)) return 0;
	
	for (new l; l < MAX_WEAPON_SLOTS; l++) {
		GetPlayerWeaponData(playerid, l, playerSaveWeaponsWpn[playerid][l], playerSaveWeaponsAmmo[playerid][l]);
	}
	playerCurrentWpn[playerid] = GetPlayerWeapon(playerid);
	
	GetPlayerHealth(playerid, playerSaveHP[playerid]);
	GetPlayerArmour(playerid, playerSaveArmour[playerid]);
	
	playerSaveInterior[playerid] = GetPlayerInterior(playerid);
	
	return 1;
}

LoadPlayerInfo(playerid, loadinterior = 1) {
	//if (!IsPlayerSpawned(playerid)) return 0;
	ResetPlayerWeapons(playerid);
	new curslot = INVALID_WEAPON_SLOT;
	
	for (new l = 1; l < MAX_WEAPON_SLOTS; l++) {
		if (playerSaveWeaponsWpn[playerid][l]) {
			if (playerSaveWeaponsWpn[playerid][l] == playerCurrentWpn[playerid]) {
				curslot = l;
			} else {
				GivePlayerWeapon(playerid, playerSaveWeaponsWpn[playerid][l], playerSaveWeaponsAmmo[playerid][l]);
			}
		}
	}
	if (curslot != INVALID_WEAPON_SLOT && playerCurrentWpn[playerid]) {
		GivePlayerWeapon(playerid, playerCurrentWpn[playerid], playerSaveWeaponsAmmo[playerid][curslot]);
	}
	
	SetPlayerHealth(playerid, playerSaveHP[playerid]);
	SetPlayerArmour(playerid, playerSaveArmour[playerid]);
	
	if (loadinterior) {
		SetPlayerInterior(playerid, playerSaveInterior[playerid]);
	}
	
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// COMMANDS
//-----------------------------------------------------------------------------------------------------

kcmd:spec(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPEC: You must be registered to use /spec.");
		return 1;
	}
	
	new message[MAX_INPUT];
	
	if (spData[playerid][spSpecPlayer] != INVALID_PLAYER_ID && spData[playerid][spSpecPlayer] != ALAR_SPEC_ID && isnull(text)) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPEC: You have left spectate mode.");
		UnspecPlayer(playerid);
		
		format(message, sizeof(message), "%s left spectate mode", pData[playerid][pLoginName]);
		SideChatAddLine(message, COLOUR_SPECTATE);
		printf("[FDM] %s", message);
		return 1;
	}
	
	new modeid = INVALID_MODE_ID;
	
	if (mpData[playerid][mpModeID] != INVALID_MODE_ID) {
		//SendClientMessage(playerid, COLOUR_ERROR, "SPEC: You are not allowed to spectate while in a minimode.");
		//return 1;
		modeid = mpData[playerid][mpModeID];
		
		if (modes[modeid][mPlayerCount] < 2) {
			SendClientMessage(playerid, COLOUR_ERROR, "SPEC: There is no-one to spectate in this minimode.");
			return 1;
		}
		
		if (!RemovePlayerFromMode(playerid)) { // Leave minimode
			format(message, sizeof(message), "MINIMODES: %s cannot be left.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		} else {
			SetPlayerInterior(playerid, 0);
			format(message, sizeof(message), "MINIMODES: %s left.", modes[modeid][mName]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		}
	}
	
	if (!IsPlayerSpawned(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPEC: You must be spawned to spectate.");
		return 1;
	}
	
	if (modeid == INVALID_MODE_ID) {
		modeid = FindPlayerMinimodeWithMsg(playerid, "USAGE: /spec [minimode id/name]", "SPEC: Invalid minimode ID/Name.", text);
		if (modeid == INVALID_MODE_ID) return 1;
	}
	
	new pid = FindFirstPlayerInMinimode(modeid);
	if (pid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPEC: There are no spawned players in this minimode.");
		return 1;
	}
	
	if (combatBlockTeleport && GetPlayerCombatPlayer(playerid) != INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SPEC: You cannot spectate minimodes while in combat.");
		return 1;
	}
	
	spData[playerid][spAllowedMode] = modeid;
	if (!SpecPlayer(playerid, pid)) {
		format(message, sizeof(message), "SPEC: Cannot spectate %s.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
	} else {
		format(message, sizeof(message), "SPEC: You are spectating %s.", modes[modeid][mName]);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s is spectating %s", pData[playerid][pLoginName], modes[modeid][mName]);
		SideChatAddLine(message, COLOUR_SPECTATE);
		printf("[FDM] %s", message);
	}
	return 1;
}
