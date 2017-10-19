
// Core
forward fdm_IsLoaded();
public fdm_IsLoaded() return 1;


// Anti-cheat
forward fdm_SetPlayerHealth(playerid, Float:health, cancel);
public fdm_SetPlayerHealth(playerid, Float:health, cancel) return kSetPlayerHealth(playerid, health, cancel);
forward fdm_GetPlayerHealth(playerid);
public fdm_GetPlayerHealth(playerid) {
	new Float:health;
	kGetPlayerHealth(playerid, health);
	return floatround(health, floatround_floor);
}
forward Float:fdm_GetPlayerHealthFloat(playerid);
public Float:fdm_GetPlayerHealthFloat(playerid) {
	new Float:health;
	kGetPlayerHealth(playerid, health);
	return health;
}

forward fdm_SetPlayerArmour(playerid, Float:armour, cancel);
public fdm_SetPlayerArmour(playerid, Float:armour, cancel) return kSetPlayerArmour(playerid, armour, cancel);
forward fdm_GetPlayerArmour(playerid);
public fdm_GetPlayerArmour(playerid) {
	new Float:armour;
	kGetPlayerArmour(playerid, armour);
	return floatround(armour, floatround_floor);
}
forward Float:fdm_GetPlayerArmourFloat(playerid);
public Float:fdm_GetPlayerArmourFloat(playerid) {
	new Float:armour;
	kGetPlayerArmour(playerid, armour);
	return armour;
}

forward fdm_ResetPlayerMoney(playerid, cancel);
public fdm_ResetPlayerMoney(playerid, cancel) return kResetPlayerMoney(playerid, cancel);
forward fdm_GivePlayerMoney(playerid, money, cancel);
public fdm_GivePlayerMoney(playerid, money, cancel) return kGivePlayerMoney(playerid, money, cancel);
forward fdm_GetPlayerMoney(playerid);
public fdm_GetPlayerMoney(playerid) return kGetPlayerMoney(playerid);

forward fdm_GetPlayerWeapon(playerid);
public fdm_GetPlayerWeapon(playerid) return kGetPlayerWeapon(playerid);
forward fdm_SetPlayerAmmo(playerid, weaponslot, ammo, cancel);
public fdm_SetPlayerAmmo(playerid, weaponslot, ammo, cancel) return kSetPlayerAmmo(playerid, weaponslot, ammo, cancel);
forward fdm_GetPlayerAmmo(playerid);
public fdm_GetPlayerAmmo(playerid) return kGetPlayerAmmo(playerid);
forward fdm_GivePlayerWeapon(playerid, weaponid, ammo, cancel);
public fdm_GivePlayerWeapon(playerid, weaponid, ammo, cancel) return kGivePlayerWeapon(playerid, weaponid, ammo, cancel);
forward fdm_ResetPlayerWeapons(playerid, cancel);
public fdm_ResetPlayerWeapons(playerid, cancel) return kResetPlayerWeapons(playerid, cancel);
forward fdm_SetPlayerArmedWeapon(playerid, weaponid, cancel);
public fdm_SetPlayerArmedWeapon(playerid, weaponid, cancel) return kSetPlayerArmedWeapon(playerid, weaponid, cancel);

forward fdm_GetPlayerWeaponDataWpn(playerid, slot);
public fdm_GetPlayerWeaponDataWpn(playerid, slot) {
	new weapons, ammo;
	kGetPlayerWeaponData(playerid, slot, weapons, ammo);
	return weapons;
}
forward fdm_GetPlayerWeaponDataAmmo(playerid, slot);
public fdm_GetPlayerWeaponDataAmmo(playerid, slot) {
	new weapons, ammo;
	kGetPlayerWeaponData(playerid, slot, weapons, ammo);
	return ammo;
}
forward fdm_GetPlayerWeaponData(playerid, slot);
public fdm_GetPlayerWeaponData(playerid, slot) {
	new weapons, ammo;
	kGetPlayerWeaponData(playerid, slot, weapons, ammo);
	return (weapons << 16) | (ammo + 0x7FFF);
}

forward fdm_RepairVehicle(vehicleid, cancel);
public fdm_RepairVehicle(vehicleid, cancel) return kRepairVehicle(vehicleid, cancel);
forward fdm_UpdateVehicleDamageStatus(vehicleid, panels, doors, lights, tyres, cancel);
public fdm_UpdateVehicleDamageStatus(vehicleid, panels, doors, lights, tyres, cancel) return kUpdateVehicleDamageStatus(vehicleid, panels, doors, lights, tyres, cancel);
forward fdm_SetVehicleHealth(vehicleid, Float:health, cancel);
public fdm_SetVehicleHealth(vehicleid, Float:health, cancel) return kSetVehicleHealth(vehicleid, health, cancel);
forward fdm_GetVehicleHealth(vehicleid);
public fdm_GetVehicleHealth(vehicleid) {
	new Float:health;
	kGetVehicleHealth(vehicleid, health);
	return floatround(health, floatround_floor);
}
forward Float:fdm_GetVehicleHealthFloat(vehicleid);
public Float:fdm_GetVehicleHealthFloat(vehicleid) {
	new Float:health;
	kGetVehicleHealth(vehicleid, health);
	return health;
}
forward fdm_DestroyVehicle(vehicleid, cancel);
public fdm_DestroyVehicle(vehicleid, cancel) return kDestroyVehicle(vehicleid, cancel);

forward fdm_SetPlayerSpecialAction(playerid, actionid, cancel);
public fdm_SetPlayerSpecialAction(playerid, actionid, cancel) return kSetPlayerSpecialAction(playerid, actionid, cancel);

forward fdm_TogglePlayerSpectating(playerid, toggle, cancel);
public fdm_TogglePlayerSpectating(playerid, toggle, cancel) return kTogglePlayerSpectating(playerid, toggle, cancel);

forward fdm_TogglePlayerGodmode(playerid, toggle);
public fdm_TogglePlayerGodmode(playerid, toggle) return kTogglePlayerGodmode(playerid, toggle);
forward fdm_IsPlayerOnGodmode(playerid);
public fdm_IsPlayerOnGodmode(playerid) return kIsPlayerOnGodmode(playerid);
forward fdm_DisablePlayerWeapons(playerid);
public fdm_DisablePlayerWeapons(playerid) return kDisablePlayerWeapons(playerid);
forward fdm_EnablePlayerWeapons(playerid);
public fdm_EnablePlayerWeapons(playerid) return kEnablePlayerWeapons(playerid);
forward fdm_IsPlayerWeaponsDisabled(playerid);
public fdm_IsPlayerWeaponsDisabled(playerid) return kIsPlayerWeaponsDisabled(playerid);

forward fdm_AddCheatIP(IP[]);
public fdm_AddCheatIP(IP[]) return AddCheatIP(IP);
forward fdm_IsPlayerTempBanned(playerid);
public fdm_IsPlayerTempBanned(playerid) return IsPlayerTempBanned(playerid);
forward fdm_StartSyncDelay(playerid);
public fdm_StartSyncDelay(playerid) return kStartSyncDelay(playerid);


//Godmode
forward fdm_EnableGodmode(playerid);
public fdm_EnableGodmode(playerid) return EnableGodmode(playerid);
forward fdm_DisableGodmode(playerid);
public fdm_DisableGodmode(playerid) return DisableGodmode(playerid);
forward fdm_StartGodSpawning(playerid, time, disableweapons);
public fdm_StartGodSpawning(playerid, time, disableweapons) return StartGodSpawning(playerid, time, disableweapons);


// Skins
forward fdm_IsPlayerSelectingSkin(playerid);
public fdm_IsPlayerSelectingSkin(playerid) return skinPlayerData[playerid][pSelecting];


// Players
forward fdm_IsPlayerRegistered(playerid);
public fdm_IsPlayerRegistered(playerid) return pData[playerid][pIsRegistered];
forward fdm_IsPlayerLoggedIn(playerid);
public fdm_IsPlayerLoggedIn(playerid) return pData[playerid][pIsLoggedIn];
forward fdm_GetPlayerLoginNameByProp(playerid);
public fdm_GetPlayerLoginNameByProp(playerid) return SetReturnString(pData[playerid][pLoginName]);


// Experience
forward fdm_GetPlayerLevel(playerid);
public fdm_GetPlayerLevel(playerid) return pData[playerid][pLevel];
forward fdm_SetPlayerLevel(playerid, level);
public fdm_SetPlayerLevel(playerid, level) return SetPlayerLevel(playerid, level);

forward fdm_GetPlayerXP(playerid);
public fdm_GetPlayerXP(playerid) return pData[playerid][pXP];
forward fdm_GivePlayerXP(playerid, xp);
public fdm_GivePlayerXP(playerid, xp) return GivePlayerXP(playerid, xp);
forward fdm_SendXPMessageToPlayer(playerid, xp, type); //type 0 = chatbox, type 1 = gametext
public fdm_SendXPMessageToPlayer(playerid, xp, type) return SendXPMessageToPlayer(playerid, xp, type);


// Money
forward fdm_SendMoneyMessageToPlayer(playerid, money, type); //type 0 = chatbox, type 1 = gametext
public fdm_SendMoneyMessageToPlayer(playerid, money, type) return SendMoneyMessageToPlayer(playerid, money, type);


// Weapons
forward fdm_GivePlayerSpawnWeapons(playerid);
public fdm_GivePlayerSpawnWeapons(playerid) return GivePlayerSpawnWeapons(playerid);
forward fdm_GetSpawnWeaponMinLevel(weaponid);
public fdm_GetSpawnWeaponMinLevel(weaponid) return wData[weaponid][wMinLevel];


// Minimodes
forward fdm_StartMinimode(modeid, flags);
public fdm_StartMinimode(modeid, flags) return StartMinimode(modeid, flags);
forward fdm_StopMinimode(modeid);
public fdm_StopMinimode(modeid) return StopMinimode(modeid);
forward fdm_PutPlayerInMinimode(playerid, modeid);
public fdm_PutPlayerInMinimode(playerid, modeid) return PutPlayerInMode(playerid, modeid);
forward fdm_RemovePlayerFromMinimode(modeid);
public fdm_RemovePlayerFromMinimode(modeid) return RemovePlayerFromMode(modeid);
forward fdm_FindMinimode(modename[]);
public fdm_FindMinimode(modename[]) return FindMinimode(modename);
forward fdm_GetPlayerMinimodeID(playerid);
public fdm_GetPlayerMinimodeID(playerid) return mpData[playerid][mpModeID];
forward fdm_GetMinimodeType(modeid);
public fdm_GetMinimodeType(modeid) return modes[modeid][mType];
forward fdm_GetMinimodeMax();
public fdm_GetMinimodeMax() return MAX_MINIMODES;
forward fdm_GetMinimodeState(modeid);
public fdm_GetMinimodeState(modeid) return modes[modeid][mState];
forward fdm_GetMinimodePlayerCount(modeid);
public fdm_GetMinimodePlayerCount(modeid) return modes[modeid][mPlayerCount];
forward fdm_GetMinimodeVirtualWorld(modeid);
public fdm_GetMinimodeVirtualWorld(modeid) return modes[modeid][mVirtualWorld];
forward fdm_IsPlayerInMinimode(playerid);
public fdm_IsPlayerInMinimode(playerid) return IsPlayerInMinimode(playerid);
forward fdm_IsPlayerInMinimodeWaiting(playerid);
public fdm_IsPlayerInMinimodeWaiting(playerid) {
	if (mpData[playerid][mpLastJoinModeID] == INVALID_MODE_ID) return 0;
	if (IsPlayerInCheckpointID(playerid, modes[mpData[playerid][mpLastJoinModeID]][mCheckpointID])) return 1;
	return 0;
}
forward fdm_GetPlayerMinimodeWaiting(playerid);
public fdm_GetPlayerMinimodeWaiting(playerid) {
	if (mpData[playerid][mpLastJoinModeID] == INVALID_MODE_ID) return INVALID_MODE_ID;
	if (IsPlayerInCheckpointID(playerid, modes[mpData[playerid][mpLastJoinModeID]][mCheckpointID])) return mpData[playerid][mpLastJoinModeID];
	return INVALID_MODE_ID;
}
forward fdm_GetMinimodeNameByProp(modeid);
public fdm_GetMinimodeNameByProp(modeid) {
	if (modes[modeid][mType] == MODE_TYPE_INVALID) return -1;
	return SetReturnString(modes[modeid][mName]);
}
forward fdm_GetMinimodeNameByPVar(modeid);
public fdm_GetMinimodeNameByPVar(modeid) {
	SetPVarString(0, "fdmTempString", modes[modeid][mName]);
	return 1;
}


// Race data
forward fdm_GetPlayerRaceCheckpointID(playerid);
public fdm_GetPlayerRaceCheckpointID(playerid) return racePStats[playerid][rcCheckpointID];
forward fdm_GetRaceCurrentTime(modeid);
public fdm_GetRaceCurrentTime(modeid) {
	if (modes[modeid][mState] == MODE_STATE_PROGRESS && !raceMaps[modes[modeid][mMapID]][rcCountDown]) {
		return GetTickCount() - raceMaps[modes[modeid][mMapID]][rcStartTime];
	} else {
		return 0;
	}
}
forward fdm_GetRaceCheckpointCount(modeid);
public fdm_GetRaceCheckpointCount(modeid) return raceMaps[modes[modeid][mMapID]][rcCheckpointCount];


// Race record reading
forward fdm_GetRecordPlayerNameByProp(modeid, position);
public fdm_GetRecordPlayerNameByProp(modeid, position) {
	if (raceMapRecords[modes[modeid][mMapID]][position][rcPlayer][0]) {
		return SetReturnString(raceMapRecords[modes[modeid][mMapID]][position][rcPlayer]);
	} else {
		return 0;
	}
}
forward fdm_GetRecordPlayerNameByPVar(modeid, position);
public fdm_GetRecordPlayerNameByPVar(modeid, position) {
	SetPVarString(0, "fdmTempString", raceMapRecords[modes[modeid][mMapID]][position][rcPlayer]);
	return 1;
}
forward fdm_FindRecord(modeid, playerName[]);
public fdm_FindRecord(modeid, playerName[]) {
	new mapid = modes[modeid][mMapID];
	
	for (new i; i < MAX_RACE_RECORDS; i++) {
		if (raceMapRecords[mapid][i][rcTime]) {
			if (raceMapRecords[mapid][i][rcPlayer][0] && !strcmp(raceMapRecords[mapid][i][rcPlayer], playerName)) {
				// Equal names
				return i;
			}
		}
	}
	
	return -1;
}

forward fdm_GetRecordPlayerTime(modeid, position);
public fdm_GetRecordPlayerTime(modeid, position) {
	return raceMapRecords[modes[modeid][mMapID]][position][rcTime];
}


// Race record modifications
forward fdm_AddRecord(modeid, playername[], time);
public fdm_AddRecord(modeid, playername[], time) {
	new tmp = -1;
	return AddRecord(modes[modeid][mMapID], playername, time, tmp, tmp, 1);
}
forward fdm_RemoveRecord(modeid, playername[]);
public fdm_RemoveRecord(modeid, playername[]) return RemoveRecord(modes[modeid][mMapID], playername, 1);
forward fdm_RemovePlayerRecords(playername[]);
public fdm_RemovePlayerRecords(playername[]) return RemovePlayerRecords(playername, 1);
forward fdm_RemoveRaceRecords(modeid);
public fdm_RemoveRaceRecords(modeid) return RemoveRaceRecords(modes[modeid][mMapID], 1);


// Sidechat
forward fdm_SideChatAddLine(text[], colour);
public fdm_SideChatAddLine(text[], colour) return SideChatAddLine(text, colour);


// Time
forward fdm_SetWorldTime(hour);
public fdm_SetWorldTime(hour) return kSetWorldTime(hour);
forward fdm_SetPlayerTime(playerid, hour, minute);
public fdm_SetPlayerTime(playerid, hour, minute) return kSetPlayerTime(playerid, hour, minute);
forward fdm_ResetPlayerTime(playerid);
public fdm_ResetPlayerTime(playerid) return kResetPlayerTime(playerid);


// Weather
forward fdm_GetWeather();
public fdm_GetWeather() return kGetWeather();
forward fdm_SetWeather(weatherid);
public fdm_SetWeather(weatherid) return kSetWeather(weatherid);
forward fdm_GetPlayerWeather(playerid);
public fdm_GetPlayerWeather(playerid) return kGetPlayerWeather(playerid);
forward fdm_SetPlayerWeather(playerid, weatherid);
public fdm_SetPlayerWeather(playerid, weatherid) return kSetPlayerWeather(playerid, weatherid);
forward fdm_ResetPlayerWeather(playerid);
public fdm_ResetPlayerWeather(playerid) return kResetPlayerWeather(playerid);

//forward fdm_GetPlayerTime(playerid, &hour, &minute);
//public fdm_GetPlayerTime(playerid, &hour, &minute) return kGetPlayerTime(playerid, hour, minute);
forward fdm_GetPlayerTimeHour(playerid);
public fdm_GetPlayerTimeHour(playerid) {
	new hour, minute;
	kGetPlayerTime(playerid, hour, minute);
	return hour;
}
forward fdm_GetPlayerTimeMinute(playerid);
public fdm_GetPlayerTimeMinute(playerid) {
	new hour, minute;
	kGetPlayerTime(playerid, hour, minute);
	return minute;
}


// Interior / VirtualWorld Handling
forward fdm_StopNextInteriorChanges(playerid);
public fdm_StopNextInteriorChanges(playerid) return DisableNextInteriorChange(playerid);


// Checkpoint Streaming
forward fdm_IsPlayerInCheckpointID(playerid, checkpointid);
public fdm_IsPlayerInCheckpointID(playerid, checkpointid) return IsPlayerInCheckpointID(playerid, checkpointid);
forward fdm_AddCheckpoint(Float:x, Float:y, Float:z, Float:size, virtualworld);
public fdm_AddCheckpoint(Float:x, Float:y, Float:z, Float:size, virtualworld) return AddCheckpoint(x, y, z, size, virtualworld);
forward fdm_RemoveCheckpoint(checkpointid);
public fdm_RemoveCheckpoint(checkpointid) return RemoveCheckpoint(checkpointid);


// Specialties
forward fdm_GetPlayerSpecialtyByProp(playerid);
public fdm_GetPlayerSpecialtyByProp(playerid) return SetReturnString(sTypes[sppData[playerid][spType]][stName]);
forward fdm_GetPlayerSpecialtyByPVar(playerid);
public fdm_GetPlayerSpecialtyByPVar(playerid) {
	SetPVarString(0, "fdmTempString", sTypes[sppData[playerid][spType]][stName]);
	return 1;
}

// In-combat
forward fdm_GetPlayerCombatPlayer(playerid);
public fdm_GetPlayerCombatPlayer(playerid) {
	return GetPlayerCombatPlayer(playerid);
}
