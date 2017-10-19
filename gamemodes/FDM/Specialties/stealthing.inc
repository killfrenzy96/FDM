
new pStealthEnabled[MAX_PLAYERS];

//-----------------------------------------------------------------------------------------------------
// LOADING / UNLOADING
//-----------------------------------------------------------------------------------------------------

StealthingInit() {
	kEnableTimer(24); // fdm_stealthing_Check
	return 1;
}

StealthingExit() {
	kDisableTimer(24); // fdm_stealthing_Check
	LoopPlayers(i) {
		StealthingDisableForPlayer(i);
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// EVENTS
//-----------------------------------------------------------------------------------------------------

stealthing_OnPlayerDisconnect(playerid) {
	StealthingDisableForPlayer(playerid);
	return 1;
}

stealthing_OnPlayerSpawn(playerid) {
	if (pStealthEnabled[playerid]) {
		if (!IsPlayerAbilityAllowed(playerid, SP_ABILITY_STEALTH)) {
			StealthingDisableForPlayer(playerid);
		} else if (!IsPlayerInMinimode(playerid)) {
			StealthingEnableForPlayer(playerid);
		}
	}
	return 1;
}

forward fdm_stealthing_Check();
public fdm_stealthing_Check() {
	LoopPlayers(i) {
		if (pStealthEnabled[i] && !IsPlayerInMinimode(i) && IsPlayerSpawned(i)) {
			StealthingUpdatePlayerAlpha(i);
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------------------------------

StealthingEnableForPlayer(playerid) {
	pStealthEnabled[playerid] = 1;
	
	SetPlayerColor(playerid, GetPlayerColor(playerid) & 0xFFFFFF00);
	LoopPlayers(i) {
		if (playerid != i && !IsPlayerInMinimode(i)) {
			if (ArePlayersInSameGang(playerid, i)) {
				SetPlayerMarkerForPlayer(i, playerid, GetPlayerColor(playerid) | 0x00000060);
				ShowPlayerNameTagForPlayer(i, playerid, 1);
			} else {
				ShowPlayerNameTagForPlayer(i, playerid, 0);
			}
		}
	}
	
	return 1;
}

StealthingDisableForPlayer(playerid) {
	pStealthEnabled[playerid] = 0;
	GivePlayerGangColour(playerid);
	return 1;
}

StealthingUpdatePlayerAlpha(playerid) {
	LoopPlayers(i) {
		if (IsPlayerInMinimode(i)) continue;
		if (playerid == i) continue;
		
		if (!ArePlayersInSameGang(playerid, i)) {
			new volume = GetPlayerVolumeToPlayerByte(playerid, i);
			
			SetPlayerMarkerForPlayer(i, playerid, (GetPlayerColor(playerid) & 0xFFFFFF00) | volume);
			if (volume) {
				ShowPlayerNameTagForPlayer(i, playerid, 1);
			} else {
				ShowPlayerNameTagForPlayer(i, playerid, 0);
			}
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// COMMANDS
//-----------------------------------------------------------------------------------------------------

kcmd:sl(playerid, text[]) return kcmd_stealth(playerid, text);
kcmd:stealth(playerid, text[]) {
	if (!SpecialtyAllowedCheck(playerid, SP_ABILITY_STEALTH, "stealth")) return 1;
	
	if (pStealthEnabled[playerid]) {
		if (StealthingDisableForPlayer(playerid)) {
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPECIALTIES: Stealth disabled.");
		} else {
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Could not disable stealth.");
		}
	} else {
		if (StealthingEnableForPlayer(playerid)) {
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, "SPECIALTIES: Stealth enabled. Be quiet!");
		} else {
			SendClientMessage(playerid, COLOUR_ERROR, "SPECIALTIES: Could not enable stealth.");
		}
	}
	
	return 1;
}
