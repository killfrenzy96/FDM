#include <a_samp>

#define FILE_WEAPONS "FDM/Settings/Weapons.ini"
#define INVALID_WEAPON_LEVEL -1
#define INVALID_PICKUP_ID -1

#define COLOUR_WEAPON_ENABLED 0xBBEE00AA
#define COLOUR_WEAPON_DISABLED 0xDD5500AA
#define MAX_AMMO 0x7FFF

enum ammunationLocationInfo {
	wCheckpointID,
	Float:wX,
	Float:wY,
	Float:wZ
}
enum weaponDataInfo {
	wMinLevel,
	wCost,
	wStartAmmo
}
new weaponLevelsData[MAX_WEAPON_ID][MAX_INPUT];
new wData[MAX_WEAPON_ID][weaponDataInfo];
new ammuLocations[][ammunationLocationInfo] = {
	{INVALID_CHECKPOINT_ID, 291.5192, -84.0591, 1001.5156}, // ammunation1
	{INVALID_CHECKPOINT_ID, 287.6172, -106.3156, 1001.5156}, // ammunation2
	{INVALID_CHECKPOINT_ID, 293.6840, -34.9384, 1001.5156}, // ammunation3
	{INVALID_CHECKPOINT_ID, 315.2399, -168.1577, 999.5938} //ammunation4
};

new wpDisableNextSpawnWpn[MAX_PLAYERS];

WeaponsInit() {
	new i;
	for (i = 0; i < MAX_WEAPON_ID; i++) { // Set vars
		wData[i][wMinLevel] = INVALID_WEAPON_LEVEL;
	}
	for (i = 0; i < sizeof(ammuLocations); i++) { // Create checkpoints
		ammuLocations[i][wCheckpointID] = AddCheckpoint(
			ammuLocations[i][wX], ammuLocations[i][wY], ammuLocations[i][wZ], 10.0);
	}
	LoadWeaponSettings();
	return 1;
}

LoadWeaponSettings() {
	// Read weapon levels
	if (!INI_Open(FILE_WEAPONS)) return 0;
	
	new i;
	new key[INI_MAX_KEY];
	new levels[MAX_WEAPON_ID];
	
	for (i = 0; i < MAX_WEAPON_ID; i++) {
		key = ReturnWeaponName(i);
		if (!key[0]) continue;
		
		new readstr[MAX_WEAPONNAME];
		if (!INI_ReadString(readstr, key)) continue; // Possibly invalid data
		if (!readstr[0]) continue;
		
		new idx;
		new minlevel = strval(split(readstr, idx, ','));
		new cost = strval(split(readstr, idx, ','));
		
		if (!cost) continue; // Possibly invalid data
		
		wData[i][wMinLevel] = minlevel;
		wData[i][wCost] = cost;
		levels[i] = minlevel;
	}
	
	// Read weapon starting ammo
	for (i = 0; i < MAX_WEAPON_ID; i++) {
		new readstr[MAX_WEAPONNAME];
		format(key, sizeof(key), "StartWeapon%i", i);
		
		if (!INI_ReadString(readstr, key)) break; // End of starting weapon data
		
		new idx;
		new weaponid = strval(split(readstr, idx, ','));
		new ammo = strval(split(readstr, idx, ','));
		
		if (!weaponid || !ammo) break; // End of starting weapon data
		
		wData[weaponid][wStartAmmo] = ammo;
	}
	
	INI_Close();
	
	// Create data for /wlevels
	//level
	//weaponLevelsData
	
	for (i = 0; i < MAX_WEAPON_ID; i++) {
		if (wData[i][wMinLevel] == INVALID_WEAPON_LEVEL) continue;
		
		new weaponname[MAX_WEAPONNAME];
		strcpy(weaponname, ReturnWeaponName(i));
		if (!weaponname[0]) continue; // Invalid weapon
		
		if (IsWeaponMelee(i)) {
			format(weaponLevelsData[i], MAX_INPUT, "Level %i - ID:%i - %s - $%s", wData[i][wMinLevel], i, ReturnWeaponName(i), kvalstr(wData[i][wCost]));
		} else {
			format(weaponLevelsData[i], MAX_INPUT, "Level %i - ID:%i - %s - $%s per ammo", wData[i][wMinLevel], i, ReturnWeaponName(i), kvalstr(wData[i][wCost]));
		}
	}
	
	i = 1;
	new j = 2;
	while (i < MAX_WEAPON_ID) {
		if (levels[i-1] <= levels[i]) {
			i = j;
			j = j + 1;
		} else {
			// swap
			new tmp[MAX_INPUT];
			new tmp2;
			tmp2 = levels[i];
			levels[i] = levels[i-1];
			levels[i-1] = tmp2;
			strcpy(tmp, weaponLevelsData[i]);
			strcpy(weaponLevelsData[i], weaponLevelsData[i-1], MAX_INPUT);
			strcpy(weaponLevelsData[i-1], tmp, MAX_INPUT);
			// end of swap
			
			i = i - 1;
			if (!i) i = 1;
		}
	}
	
	return 1;
}

UnloadWeaponSettings() {
	for (new i; i < MAX_WEAPON_ID; i++) {
		wData[i][wMinLevel] = INVALID_WEAPON_LEVEL;
		wData[i][wCost] = 0;
		wData[i][wStartAmmo] = 0;
		strclr(weaponLevelsData[i]);
	}
	return 1;
}

weapons_OnPlayerConnect(playerid) {
	wpDisableNextSpawnWpn[playerid] = 0;
	return 1;
}

weapons_OnPlayerLoaded(playerid) {
	if (!pData[playerid][pIsLoggedIn]) {
		for (new i; i < MAX_WEAPON_ID; i++) {
			if (wData[i][wStartAmmo] > pData[playerid][pWeapons][i]) {
				pData[playerid][pWeapons][i] = wData[i][wStartAmmo];
				pData[playerid][pActiveWeapons][i] = 1;
			}
		}
	}
	return 1;
}

weapons_OnPlayerRegister(playerid) {
	// Give player spawn starting weapons
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (wData[i][wStartAmmo] > pData[playerid][pWeapons][i]) {
			pData[playerid][pWeapons][i] = wData[i][wStartAmmo];
			pData[playerid][pActiveWeapons][i] = 1;
		}
	}
	if (IsPlayerSpawned(playerid)) {
		GivePlayerSpawnWeapons(playerid);
	}
	return 1;
}

weapons_OnPlayerLogin(playerid) {
	if (!IsPlayerSpawned(playerid)) return 1;
	GivePlayerSpawnWeapons(playerid); // Give player their own weapons
	return 1;
}

weapons_OnPlayerSpawn(playerid) {
	if (wpDisableNextSpawnWpn[playerid]) {
		wpDisableNextSpawnWpn[playerid] = 0;
		return 1;
	}
	ResetPlayerWeapons(playerid);
	GivePlayerSpawnWeapons(playerid);
	SetPlayerArmedWeapon(playerid, 0);
	return 1;
}

weapons_OnPlayerEnterCheckpoint(playerid, checkpointid) {
	for (new i; i < sizeof(ammuLocations); i++) {
		if (ammuLocations[i][wCheckpointID] != checkpointid) continue;
		
		// Player has entered ammunation checkpointid
		SendClientMessage(playerid, COLOUR_TITLE, "Spawn Weapons:");
		SendClientMessage(playerid, COLOUR_HELP, "/weaponlist - A list of weapons you can buy (Short: /wl).");
		SendClientMessage(playerid, COLOUR_HELP, "/weaponbuy [weapon] [ammo] - Buy a spawn weapon (Short: /wb).");
		SendClientMessage(playerid, COLOUR_HELP, "/weaponenable [weapon] - Enable a spawn weapon (Short: /we).");
		SendClientMessage(playerid, COLOUR_HELP, "/weapondisable [weapon] - Disable a spawn weapon (Short: /wd).");
		SendClientMessage(playerid, COLOUR_HELP, "/weapons - A list of spawn weapons you own.");
		
		return 0;
	}
	return 1;
}

weapons_OnPlayerLevelChange(playerid, oldlevel, newlevel) {
	if (oldlevel <= newlevel) return 1;
	
	// Send player new weapons
	new message[MAX_INPUT];
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (wData[i][wMinLevel] <= oldlevel) continue;
		if (wData[i][wMinLevel] > newlevel) continue;
		
		format(message, sizeof(message), "New weapon: %s - $%s per ammo", ReturnWeaponName(i), kvalstr(wData[i][wCost]));
		SendClientMessage(playerid, COLOUR_HELP, message);
	}
	return 1;
}

GivePlayerSpawnWeapons(playerid) {
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!pData[playerid][pActiveWeapons][i]) continue; // Inactive weapon
		if (!pData[playerid][pWeapons][i]) continue; // Must have ammo
		if (wData[i][wMinLevel] == INVALID_WEAPON_LEVEL) continue; // Weapon must be enabled
		
		// Weapon data integrity
		if (IsWeaponMelee(i)) {
			if (pData[playerid][pWeapons][i] < 0 || pData[playerid][pWeapons][i] > 1) {
				pData[playerid][pWeapons][i] = 1;
			}
		} else {
			if (pData[playerid][pWeapons][i] < 0) pData[playerid][pWeapons][i] = MAX_AMMO;
			if (pData[playerid][pWeapons][i] > MAX_AMMO) pData[playerid][pWeapons][i] = MAX_AMMO;
		}
		
		if (wData[i][wMinLevel] > pData[playerid][pLevel]) continue; // Player must have a level high enough
		
		// Give the player their weapon
		GivePlayerWeapon(playerid, i, pData[playerid][pWeapons][i]);
	}
	return 1;
}

stock DisableNextSpawnWeapons(playerid) {
	if (IsPlayerConnected(playerid))
		wpDisableNextSpawnWpn[playerid] = 1;
	return 1;
}

IsPlayerAtSpawnAmmu(playerid) {
	for (new i; i < sizeof(ammuLocations); i++) {
		if (IsPlayerInCheckpointID(playerid, ammuLocations[i][wCheckpointID])) return 1;
	}
	return 0;
}

kcmd:wlevels(playerid, text[]) return kcmd_weaponlevels(playerid, text);
kcmd:weaponlevels(playerid, text[]) {
	SendClientMessage(playerid, COLOUR_TITLE, "Weapons Levels List:");
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (!weaponLevelsData[i][0]) continue;
		SendClientMessage(playerid, COLOUR_HELP, weaponLevelsData[i]);
	}
	SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	return 1;
}

kcmd:wl(playerid, text[]) return kcmd_weaponlist(playerid, text);
kcmd:weaponlist(playerid, text[]) {
	new message[MAX_INPUT];
	new weaponname[MAX_WEAPONNAME];
	
	SendClientMessage(playerid, COLOUR_TITLE, "Spawn Weapons List:");
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (wData[i][wMinLevel] == INVALID_WEAPON_LEVEL) continue;
		if (pData[playerid][pLevel] < wData[i][wMinLevel]) continue;
		
		strcpy(weaponname, ReturnWeaponName(i));
		if (!weaponname[0]) continue; // Invalid weapon
		
		if (IsWeaponMelee(i)) {
			format(message, sizeof(message), "ID:%i - %s - $%s", i, ReturnWeaponName(i), kvalstr(wData[i][wCost]));
		} else {
			format(message, sizeof(message), "ID:%i - %s - $%s per ammo", i, ReturnWeaponName(i), kvalstr(wData[i][wCost]));
		}
		SendClientMessage(playerid, COLOUR_HELP, message);
	}
	SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	return 1;
}

kcmd:wb(playerid, text[]) return kcmd_weaponbuy(playerid, text);
kcmd:weaponbuy(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /weaponbuy [weapon] [ammo]");
		return 1;
	}
	if (!IsPlayerAtSpawnAmmu(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "WEAPONS: You must be at ammunation to use this command.");
		return 1;
	}
	
	new message[MAX_INPUT];
	new wname[MAX_WEAPONNAME];
	new weaponstr[MAX_WEAPONNAME];
	new weaponid;
	new ammo;
	new idx;
	
	splitcpy(weaponstr, text, idx, ' ');
	ammo = strval(split(text, idx, ' '));
	
	if (!weaponstr[0]) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /weaponbuy [weapon] [ammo]");
		return 1;
	}
	
	weaponid = FindWeaponID(weaponstr);
	strcpy(wname, ReturnWeaponName(weaponid));
	if (!weaponid) {
		SendClientMessage(playerid, COLOUR_ERROR, "WEAPONS: Invalid weapon.");
		return 1;
	}
	if (ammo <= 0 && !IsWeaponMelee(weaponid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "WEAPONS: Invalid ammo.");
		return 1;
	}
	if (ammo >= 0x0FFFFFFF / wData[weaponid][wCost]) { // Overflow protection
		SendClientMessage(playerid, COLOUR_ERROR, "WEAPONS: Invalid ammo.");
		return 1;
	}
	if (IsWeaponMelee(weaponid)) ammo = 1;
	if (wData[weaponid][wMinLevel] == INVALID_WEAPON_LEVEL) {
		format(message, sizeof(message), "WEAPONS: The weapon '%s' cannot be purchased.", wname);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (pData[playerid][pLevel] < wData[weaponid][wMinLevel]) {
		format(message, sizeof(message), "WEAPONS: Level % is needed to buy a '%s'.", wData[weaponid][wMinLevel], wname);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (pData[playerid][pWeapons][weaponid] + ammo > MAX_AMMO) {
		ammo = MAX_AMMO - pData[playerid][pWeapons][weaponid];
		if (!ammo) {
			format(message, sizeof(message), "WEAPONS: You have reached the maximum ammo for the %s.", wname);
			SendClientMessage(playerid, COLOUR_ERROR, message);
			return 1;
		}
	}
	new cost = wData[weaponid][wCost] * ammo;
	if (GetPlayerMoney(playerid) < cost) {
		format(message, sizeof(message), "WEAPONS: $%s is needed to make this purchase.", kvalstr(cost));
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	// Perform purchase
	GivePlayerMoney(playerid, -cost);
	GivePlayerWeapon(playerid, weaponid, ammo);
	
	pData[playerid][pWeapons][weaponid] = pData[playerid][pWeapons][weaponid] + ammo;
	pData[playerid][pActiveWeapons][weaponid] = 1;
	// Disable all other weapons of the same slot
	new weaponslot = GetWeaponSlot(weaponid);
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (weaponid == i) continue;
		if (GetWeaponSlot(i) != weaponslot) continue;
		pData[playerid][pActiveWeapons][i] = 0;
	}
	
	// Send Success message
	format(message, sizeof(message), "WEAPONS: Purchased %i spawn %s for $%s", ammo, wname, kvalstr(cost));
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	return 1;
}

kcmd:we(playerid, text[]) return kcmd_weaponenable(playerid, text);
kcmd:weaponenable(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /weaponenable [weapon]");
		return 1;
	}
	
	new weaponid = FindWeaponID(text);
	new wname[MAX_WEAPONNAME];
	new message[MAX_INPUT];
	wname = ReturnWeaponName(weaponid);
	if (!weaponid) {
		SendClientMessage(playerid, COLOUR_ERROR, "WEAPON ENABLE: Invalid weapon.");
		return 1;
	}
	if (wData[weaponid][wMinLevel] == -1) {
		format(message, sizeof(message), "WEAPONS: The weapon %s cannot be enabled.", wname);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (pData[playerid][pLevel] < wData[weaponid][wMinLevel]) {
		format(message, sizeof(message), "WEAPONS: Level %i is needed to enable a %s.", wData[weaponid][wMinLevel], wname);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (!pData[playerid][pWeapons][weaponid]) {
		format(message, sizeof(message), "WEAPONS: You do not have any %s ammo.", wname);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (pData[playerid][pActiveWeapons][weaponid]) {
		format(message, sizeof(message), "WEAPONS: The %s is already enabled.", wname);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		return 1;
	}
	
	pData[playerid][pActiveWeapons][weaponid] = 1;
	new weaponslot = GetWeaponSlot(weaponid);
	for (new i; i < MAX_WEAPON_ID; i++) {
		if (weaponid == i) continue;
		if (GetWeaponSlot(i) != weaponslot) continue;
		pData[playerid][pActiveWeapons][i] = 0;
	}
	format(message, sizeof(message), "WEAPONS: %s spawn weapon enabled.", wname);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	return 1;
}

kcmd:wd(playerid, text[]) return kcmd_weapondisable(playerid, text);
kcmd:weapondisable(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /weapondisable [weapon]");
		return 1;
	}
	
	new weaponid = FindWeaponID(text);
	new wname[MAX_WEAPONNAME];
	new message[MAX_INPUT];
	wname = ReturnWeaponName(weaponid);
	if (!weaponid) {
		SendClientMessage(playerid, COLOUR_ERROR, "WEAPON ENABLE: Invalid weapon.");
		return 1;
	}
	if (!pData[playerid][pActiveWeapons][weaponid]) {
		format(message, sizeof(message), "WEAPONS: The %s is already disabled.", wname);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		return 1;
	}
	
	pData[playerid][pActiveWeapons][weaponid] = 0;
	format(message, sizeof(message), "WEAPONS: %s spawn weapon disabled.", wname);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	return 1;
}

kcmd:weapons(playerid, text[]) {
	new wname[MAX_WEAPONNAME];
	new message[MAX_INPUT];
	new tmpstr[25];
	new colour;
	new count;
	
	SendClientMessage(playerid, COLOUR_TITLE, "Spawn Weapons Owned:");
	for (new i; i < MAX_WEAPON_ID; i++) {
		wname = ReturnWeaponName(i);
		if (!wname[0]) continue;
		
		if (!pData[playerid][pWeapons][i]) continue;
		
		if (pData[playerid][pActiveWeapons][i]) {
			if (pData[playerid][pLevel] < wData[i][wMinLevel] || wData[i][wMinLevel] == -1) {
				tmpstr = "Enabled, but not allowed";
				colour = COLOUR_WEAPON_DISABLED;
			} else {
				tmpstr = "Enabled";
				colour = COLOUR_WEAPON_ENABLED;
			}
		} else {
			tmpstr = "Disabled";
			colour = COLOUR_WEAPON_DISABLED;
		}
		
		if (IsWeaponMelee(i)) {
			format(message, sizeof(message), "ID:%i - %s [%s]", i, wname, tmpstr);
		} else {
			format(message, sizeof(message), "ID:%i - %s - %i Ammo [%s]", i, wname, pData[playerid][pWeapons][i], tmpstr);
		}
		SendClientMessage(playerid, colour, message);
		
		count++;
	}
	if (!count) {
		SendClientMessage(playerid, COLOUR_HELP, "You have no spawn weapons.");
	} else if (count > 7) {
		SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	}
	return 1;
}

weapons_OnPlayerCommandText(playerid, cmd[]) {
	if (strcmp(cmd, "/weapon", true, 7)) return 0;
	
	SendClientMessage(playerid, COLOUR_TITLE, "Spawn Weapons Help:");
	SendCommandSetToPlayer(playerid, CMD_SET_WEAPONS);
	return 1;
}
