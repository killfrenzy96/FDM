/*

By KillFrenzy.

The gamemode's anti-cheat system. Designed to work with the ALAR admin script.

*/

#include <alar>
#include <a_samp>

#define ADMIN_LEVEL_ANTICHEAT 8
#define INVALID_WEAPON_ID -1
#define INVALID_WEAPON_SLOT -1
#define MAX_WEAPON_SLOTS 13
#define MAX_WEAPON_ID 47

#define HEALTH_BUFFER 256.0

#define CHEAT_FLAG_NORMAL 0
#define CHEAT_FLAG_LAG 1
#define CHEAT_FLAG_DEFINITE 2

#define COLOUR_LOG_SUSPECT_LAG 0xDD5500AA
#define COLOUR_LOG_SUSPECT_DEFINATE 0xFF0000AA
#define COLOUR_LOG_SUSPECT 0xDD0000AA
#define COLOUR_LOG_PAUSED 0xDD9900AA
#define COLOUR_LOG_ERROR 0xFF0000AA
#define COLOUR_KICK 0xEE0000AA

#define MAX_ALERT_INTERVAL 10000 // The interval between anti-cheat warnings of the same type
#define MAX_MONEY_TOLERANCE 1000 // $1000 = Pimp mobile money

// SYNC_DELAY
// Increase to compensate for people with higher pings - keep it high, around 2000 to 5000
// A higher value means a less responsive anti-cheat
// Minimum = SYNC_TIMER_INTERVAL * 2
#define SYNC_DELAY 2000
#define MONEY_SYNC_DELAY 4000 // Money updates much more slowly, grant it more time. It also has worser lag detection.

// SYNC_TIMER_INTERVAL
// This is the timer interval
#define DEFAULT_SYNC_TIMER_INTERVAL 50

#define MAX_AMMOPERPURCHASE 450

#define MAX_CHEAT_IP 256
#define CHEAT_BAN_TIME 1500000 // 25 minutes

#define MAX_MONEYPERWIN 32000000
#define MAX_MONEYTOTALWIN 50000000

#define COLOUR_KICK 0xEE0000AA
#define WARNING_RESET_INTERVAL 600000
#define MAX_WARNINGS 12
#define MAX_WARNINGS_WITH_MISC_ADMINS 32
#define MAX_WARNINGS_WITH_MODERATOR_ADMINS 64
#define MAX_WARNINGS_WITH_MASTER_ADMINS 128

#define GODMODE_HP 0xFFFFFF
#define GODMODE_ARMOUR 0.0
#define GODMODE_VEHICLE_HP 0xFFFFFF

#define PURCHASETYPE_HEALTH 0
#define PURCHASETYPE_ARMOUR 1
#define PURCHASETYPE_WEAPONS 2
#define PURCHASETYPE_VEHICLEHP 3

enum locationInfo {
	Float:fx,
	Float:fy,
	Float:fz
}

enum playerInfo {
	Float:pHealth,
	Float:pLastHealth,
	pHealthSyncing,
	Float:pArmour,
	Float:pLastArmour,
	pArmourSyncing,
	pMoney,
	pLastMoney,
	pMoneySyncing,
	pVehicleID,
	Float:pLastVehicleHP,
	pVehicleSyncing,
	pVehiclePartSyncing,
	pVehicleWarning,
	pLastSuspect,
	pWarnings,
	pLastWarning,
	pPauseWarning,
	pLastPurchase,
	pLastPurchaseType,
	pLastMoneyLoss,
	pLastMoneyLossWarnings,
	pBeenToAmmunation,
	pJetpack,
	pJetpackSyncing,
	pSpectating,
	pSpectateSyncing,
	pWeaponSlotWpn[MAX_WEAPON_SLOTS],
	pWeaponSlotAmmo[MAX_WEAPON_SLOTS],
	pLastWeaponSlotWpn[MAX_WEAPON_SLOTS], // TODO
	pWeaponSyncing,
	pVisitedAmmunation,
	pHealthLastCheat,
	pArmourLastCheat,
	pMoneyLastCheat,
	pVehicleLastCheat,
	pVehiclePartLastCheat,
	pJetpackLastCheat,
	pSpectateLastCheat,
	pWeaponsLastCheat,
	pWeaponSlotWpnLastCheat[MAX_WEAPON_SLOTS],
	pWeaponSlotAmmoLastCheat[MAX_WEAPON_SLOTS],
	pCasinoWinCash,
	pGodMode,
	pWeaponsDisabled,
	pWeaponLastWpn,
	pWeaponLastAmmo,
	pWeaponSwitching,
	pUpdated,
	pInteriorWeaponsEnabled,
	pDisableResist,
	pWeaponsLoaded,
	pPauseStartTick,
	pWeaponHoldTick,
	pLastDamagedPlayer,
	pLastDamagedWeapon,
	pLastDamagedTick
}

enum vehicleInfo {
	vDriver,
	Float:vHealth,
	vDamage[4],
	vLastDamage[4]
}

enum playerHealthTextInfo {
	ptTextExist,
	Text:ptBackRectangle,
	Text:ptFrontRectangle,
	Float:pCurrentHealth
}

enum CheatIPInfo {
	Unsigned:cIP,
	cTime
}

new pStats[MAX_PLAYERS][playerInfo];
new phtData[MAX_PLAYERS][playerHealthTextInfo];
new vStats[MAX_VEHICLES][vehicleInfo];
new CheatIPs[MAX_CHEAT_IP][CheatIPInfo];
// new anticheatTimer;
new anticheatCrashedSection;
new anticheatCrashedPlayer;
new anticheatCrashTick;
new anticheatSyncInterval;

stock AntiCheatInit() {
	for (new i; i < MAX_VEHICLES; i++) {
		ac_ResetVehicleData(i);
		VehicleAcceptVehicleData(i);
	}
	
	for (new i; i < MAX_PLAYERS; i++) {
		ac_ResetPlayerData(i);
		
		// Accept all current data..
		if (!IsPlayerConnected(i)) continue;
		ArmourAcceptPlayerData(i);
		HealthAcceptPlayerData(i);
		JetpackAcceptPlayerData(i);
		MoneyAcceptPlayerData(i);
		SpectateAcceptPlayerData(i);
		VehicleAcceptPlayerData(i);
		WeaponAcceptPlayerData(i);
		
		ac_ShowPlayerTextDraw(i);
	}
	
	EnableStuntBonusForAll(0);
	
	// if (anticheatTimer) KillTimer(anticheatTimer);
	// anticheatTimer = SetTimer("fdm_anticheatEnableSync", 10000, false); // Compensates for time taken to start up
	
	kEnableTimer(0);
}

stock AntiCheatSettingsInit() {
	anticheatSyncInterval = INI_ReadInt("AntiCheatSyncInterval");
	if (!anticheatSyncInterval) {
		anticheatSyncInterval = DEFAULT_SYNC_TIMER_INTERVAL;
	}
	
	/*if (anticheatTimer) {
		KillTimer(anticheatTimer);
		anticheatTimer = SetTimer("fdm_anticheatSync", anticheatSyncInterval, true);
	}*/
	kEnableTimer(1);
	return 1;
}

AntiCheatExit() {
	LoopPlayers(i) {
		ac_HidePlayerTextDraw(i);
	}
	return 1;
}

forward fdm_anticheatEnableSync();
public fdm_anticheatEnableSync() {
	kEnableTimer(1); // anticheatTimer = SetTimer("fdm_anticheatSync", anticheatSyncInterval, true);
	return 1;
}

forward fdm_anticheatSync();
public fdm_anticheatSync() {
	
	if (anticheatCrashedSection) {
		new tick = GetTickCount();
		if (!anticheatCrashTick || tick - anticheatCrashTick > 1000) {
			new msg[MAX_INPUT];
			
			switch (anticheatCrashedSection) {
				case 1: msg = "Vehicle";
				case 2: msg = "Money";
				case 3: msg = "Weapon";
				case 4: msg = "Health";
				case 5: msg = "Armour";
				case 6: msg = "Jetpack";
				case 7: msg = "Spectate";
				case 8: msg = "Pause";
			}
			
			if (anticheatCrashedPlayer == INVALID_PLAYER_ID) {
				format(msg, sizeof(msg), "WARNING: Anti-cheat script crash: Section: %s, Player: None - Continuing anyway...", msg);
			} else {
				format(msg, sizeof(msg), "WARNING: Anti-cheat script crash: Section: %s, Player: %s - Continuing anyway...", msg, ReturnPlayerName(anticheatCrashedPlayer));
			}
			
			printf("[FDM] %s", msg);
			AddAdminLogLine(COLOUR_LOG_ERROR, msg);
		}
		
		anticheatCrashTick = tick;
	}
	
	anticheatCrashedSection = 1;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	VehicleSync();
	
	anticheatCrashedSection = 2;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	MoneySync();
	
	anticheatCrashedSection = 3;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	WeaponSync();
	
	anticheatCrashedSection = 4;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	HealthSync();
	
	anticheatCrashedSection = 5;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	ArmourSync();
	
	anticheatCrashedSection = 6;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	JetpackSync();
	
	anticheatCrashedSection = 7;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	SpectateSync();
	
	anticheatCrashedSection = 8;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	PauseUpdate();
	
	LoopPlayers(i) {
		pStats[i][pUpdated] = 0;
		ac_UpdatePlayerTextDraw(i);
	}
	
	anticheatCrashedSection = 0;
	anticheatCrashedPlayer = INVALID_PLAYER_ID;
	
	return 1;
}

ac_ResetPlayerData(playerid) {
	
	if (pStats[playerid][pVehicleID]) {
		vStats[pStats[playerid][pVehicleID]][vDriver] = INVALID_PLAYER_ID;
	}
	
	new tmpdata[playerInfo];
	pStats[playerid] = tmpdata;
	
	pStats[playerid][pHealth] = 100.0;
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		pStats[playerid][pWeaponSlotWpn][i] = INVALID_WEAPON_ID;
		pStats[playerid][pWeaponLastWpn] = INVALID_WEAPON_ID;
		pStats[playerid][pLastWeaponSlotWpn][i] = INVALID_WEAPON_ID;
	}
	
	pStats[playerid][pLastDamagedPlayer] = INVALID_PLAYER_ID;
	pStats[playerid][pLastDamagedWeapon] = INVALID_WEAPON_ID;
	pStats[playerid][pLastDamagedTick] = 0;
	
	kStartSyncDelay(playerid);
	/*pStats[playerid][pArmourSyncing] = SYNC_DELAY;
	pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
	pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
	pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
	pStats[playerid][pJetpackSyncing] = SYNC_DELAY;
	pStats[playerid][pSpectateSyncing] = SYNC_DELAY;
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;*/
	return 1;
}

ac_ResetVehicleData(vehicleid) {
	//new tmpdata[vehicleInfo];
	//vStats[vehicleid] = tmpdata;
	
	vStats[vehicleid][vDriver] = INVALID_PLAYER_ID;
	vStats[vehicleid][vHealth] = 1000.0;
	for (new i; i < 4; i++) {
		vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
		vStats[vehicleid][vDamage][i] = 0;
	}
	
	return 1;
}

Float:ac_GetHealthBuffer() {
	return HEALTH_BUFFER;
}

ac_SetPlayerHealth(playerid, Float:health) {
	if (health <= 0.0) {
		return SetPlayerHealth(playerid, 0.0);
	} else {
		return SetPlayerHealth(playerid, health + ac_GetHealthBuffer());
	}
}

ac_GetPlayerHealth(playerid, &Float:health) {
	new rtn = GetPlayerHealth(playerid, health);
	if (health > 100.0) health = 0.0;
	return rtn;
}

//-----------------------------------------------------------------------------------------------------
// HEALTH TEXT DRAW
//-----------------------------------------------------------------------------------------------------

#define AC_BAR_BACK_COLOUR 0x661111FF
#define AC_BAR_FRONT_COLOUR 0xFF111199
#define AC_BAR_GODMODE_BACK_COLOUR 0xAA3311FF
#define AC_BAR_GODMODE_FRONT_COLOUR 0xFF8811FF
#define AC_BAR_MIN 548.0
#define AC_BAR_MAX 606.0

ac_ShowPlayerTextDraw(playerid) {
	if (IsPlayerNPC(playerid)) return 1;
	
	if (!phtData[playerid][ptTextExist]) {
		new Text:txtDraw = TextDrawCreate(AC_BAR_MIN - 1.0, 67.0, "~n~"); // Health background
		phtData[playerid][ptBackRectangle] = txtDraw;
		
		TextDrawUseBox(txtDraw, 1);
		TextDrawTextSize(txtDraw, AC_BAR_MAX + 1.0, 20.0);
		TextDrawBoxColor(txtDraw, AC_BAR_BACK_COLOUR);
		TextDrawLetterSize(txtDraw, 0.1, 0.8);
		
		txtDraw = TextDrawCreate(AC_BAR_MIN, 68.0, "~n~"); // Health foreground
		phtData[playerid][ptFrontRectangle] = txtDraw;
		
		TextDrawUseBox(txtDraw, 1);
		TextDrawTextSize(txtDraw, AC_BAR_MIN, 20.0);
		TextDrawBoxColor(txtDraw, AC_BAR_FRONT_COLOUR);
		TextDrawLetterSize(txtDraw, 0.1, 0.8);
		
		phtData[playerid][pCurrentHealth] = -1.0;
		phtData[playerid][ptTextExist] = 1;
	}
	
	ac_UpdatePlayerTextDraw(playerid);
	
	TextDrawShowForPlayer(playerid, phtData[playerid][ptBackRectangle]);
	TextDrawShowForPlayer(playerid, phtData[playerid][ptFrontRectangle]);
	
	return 1;
}

ac_HidePlayerTextDraw(playerid) {
	if (phtData[playerid][ptTextExist]) {
		TextDrawDestroy(phtData[playerid][ptBackRectangle]);
		TextDrawDestroy(phtData[playerid][ptFrontRectangle]);
		phtData[playerid][ptTextExist] = 0;
	}
	return 1;
}

ac_UpdatePlayerTextDraw(playerid) {
	if (phtData[playerid][ptTextExist]) {
		if (!IsPlayerSpawned(playerid)) {
			// player is not spawned
			if (phtData[playerid][pCurrentHealth] != -1.0) {
				TextDrawHideForPlayer(playerid, phtData[playerid][ptFrontRectangle]);
				phtData[playerid][pCurrentHealth] = -1.0;
			}
		
		} else if (pStats[playerid][pGodMode]) {
			// godmode
			if (phtData[playerid][pCurrentHealth] != -2.0) {
				TextDrawBoxColor(phtData[playerid][ptBackRectangle], AC_BAR_GODMODE_BACK_COLOUR);
				TextDrawShowForPlayer(playerid, phtData[playerid][ptBackRectangle]);
				
				TextDrawTextSize(phtData[playerid][ptFrontRectangle], AC_BAR_MAX, 40.0);
				TextDrawBoxColor(phtData[playerid][ptFrontRectangle], AC_BAR_GODMODE_FRONT_COLOUR);
				TextDrawShowForPlayer(playerid, phtData[playerid][ptFrontRectangle]);
				
				phtData[playerid][pCurrentHealth] = -2.0;
			}
			
		} else if (pStats[playerid][pHealth] > phtData[playerid][pCurrentHealth] + 1.0 || pStats[playerid][pHealth] < phtData[playerid][pCurrentHealth] - 1.0) {
			// hp changed
			if (phtData[playerid][pCurrentHealth] == -2.0) {
				TextDrawBoxColor(phtData[playerid][ptBackRectangle], AC_BAR_BACK_COLOUR);
				TextDrawShowForPlayer(playerid, phtData[playerid][ptBackRectangle]);
				TextDrawBoxColor(phtData[playerid][ptFrontRectangle], AC_BAR_FRONT_COLOUR);
			}
			
			new Float:rectWidth = AC_BAR_MIN + (pStats[playerid][pHealth] / 100.0) * (AC_BAR_MAX - AC_BAR_MIN);
			TextDrawTextSize(phtData[playerid][ptFrontRectangle], rectWidth, 40.0);
			TextDrawShowForPlayer(playerid, phtData[playerid][ptFrontRectangle]);
			
			phtData[playerid][pCurrentHealth] = pStats[playerid][pHealth];
		}
	}
}


//-----------------------------------------------------------------------------------------------------
// SUSPECTED CHEATER HANDLING
//-----------------------------------------------------------------------------------------------------

CheatSuspect(playerid, reason[], cheatflag = CHEAT_FLAG_NORMAL) { // Designed for use with the alar admin script
	if (IsPlayerNPC(playerid)) {
		return 0;
	}
	
	new playername[MAX_NAME];
	new message[MAX_INPUT];
	
	GetPlayerName(playerid, playername, sizeof(playername));
	
	if (GetTickCount() - pStats[playerid][pLastWarning] > WARNING_RESET_INTERVAL) pStats[playerid][pWarnings] = 0;
	pStats[playerid][pLastWarning] = GetTickCount();
	
	// Cheat flags
	switch (cheatflag) {
		case CHEAT_FLAG_LAG: {
			format(message, sizeof(message), "%s(%i) suspect (Possible Lag): %s", playername, playerid, reason);
			AddAdminLogLine(COLOUR_LOG_SUSPECT_LAG, message);
			pStats[playerid][pWarnings]++;
		}
		case CHEAT_FLAG_DEFINITE: {
			format(message, sizeof(message), "%s(%i) suspect (Definate): %s", playername, playerid, reason);
			AddAdminLogLine(COLOUR_LOG_SUSPECT_DEFINATE, message);
			pStats[playerid][pWarnings] += 8;
		}
		default: {
			format(message, sizeof(message), "%s(%i) suspect: %s", playername, playerid, reason);
			AddAdminLogLine(COLOUR_LOG_SUSPECT, message);
			pStats[playerid][pWarnings] += 4;
		}
	}
	
	printf("[FDM] ANTICHEAT: %s", message);
	
	new maxAdminLevel = GetMaxAdminLevel();
	new maxWarnings = 0;
	
	if (maxAdminLevel >= ADMIN_LEVEL_MASTER) {
		maxWarnings = MAX_WARNINGS_WITH_MASTER_ADMINS;
	
	} else if (maxAdminLevel >= ADMIN_LEVEL_MODERATOR) {
		maxWarnings = MAX_WARNINGS_WITH_MODERATOR_ADMINS;
		
	} else if (maxAdminLevel >= ADMIN_LEVEL_MISC) {
		maxWarnings = MAX_WARNINGS_WITH_MISC_ADMINS;
		
	} else {
		maxWarnings = MAX_WARNINGS;
		
	}
	
	if (pStats[playerid][pWarnings] >= maxWarnings && !pStats[playerid][pDisableResist]) {
		// Maximum warnings reached
		//format(message, sizeof(message), "Anti-cheat - Suspected Cheats - %s", reason);
		//SuspendPlayerid(playerid, SUSPEND_TIME, message, "AntiCheat_Protection");
		SendClientMessage(playerid, COLOUR_KICK, "You were auto-kicked for suspected cheats.");
		
		format(message, sizeof(message), "You must wait %i minutes before rejoining the server.", CHEAT_BAN_TIME / 60000);
		SendClientMessage(playerid, COLOUR_KICK, message);
		
		format(message, sizeof(message), "Anti-cheat Autokick: %s(%i) - Suspected Cheats - %s", playername, playerid, reason);
		printf("[FDM] ANTICHEAT: %s", message);
		AddAdminLogLine(COLOUR_KICK, message);
		
		format(message, sizeof(message), "*** Anti-cheat Autokick: %s(%i) - Suspected Cheats", playername, playerid);
		//AddJoinLine(playerid, COLOUR_KICK, message);
		SendClientMessageToAll(COLOUR_KICK, message);
		
		#if defined _irc_included
			format(message, sizeof(message), "2*** Anti-cheat Autokick: \2;%s\2;(%i) - Suspected Cheats", playername, playerid);
			iSendClientMessageToAll(message);
		#endif
		
		AddPlayerToCheatIPs(playerid);
	}
	
	return 1;
}

GetMaxAdminLevel() {
	new maxLevel;
	
	LoopPlayers(i) {
		new adminLevel = GetAdminLevel(i);
		if (adminLevel > maxLevel) {
			maxLevel = adminLevel;
		}
	}
	
	return maxLevel;
}

IsPlayerTempBanned(playerid) {
	new ip[MAX_IP];
	GetPlayerIp(playerid, ip, sizeof(ip));
	return IsCheatIP(ip);
}

AddPlayerToCheatIPs(playerid) {
	new ip[MAX_IP];
	GetPlayerIp(playerid, ip, sizeof(ip));
	return AddCheatIP(ip);
}

AddCheatIP(IP[]) {
	new minimum = 0x80000000;
	new cheatipslot;
	new Unsigned:ipcode = IP2Code(IP);
	
	for (new i; i < MAX_CHEAT_IP; i++) {
		if (CheatIPs[i][cIP] && ipcode == CheatIPs[i][cIP]) {
			cheatipslot = i; // IP already exists
			break;
		} else {
			if (CheatIPs[i][cTime] < minimum) {
				minimum = CheatIPs[i][cTime];
				cheatipslot = i;
			}
		}
	}
	
	// This does not use LoopPlayers because kicking players during the loop can cause problems
	for (new i; i < MAX_PLAYERS; i++) {
		if (IsPlayerConnected(i)) {
			new playerip[MAX_IP];
			GetPlayerIp(i, playerip, sizeof(playerip));
			if (!strcmp(playerip, IP)) {
				Kick(i);
			}
		}
	}
	
	CheatIPs[cheatipslot][cIP] = ipcode;
	CheatIPs[cheatipslot][cTime] = GetTickCount() + CHEAT_BAN_TIME;
	return 1;
}

RemoveCheatIP(IP[]) {
	new Unsigned:ipcode = IP2Code(IP);
	
	for (new i; i < MAX_CHEAT_IP; i++) {
		if (CheatIPs[i][cIP] && CheatIPs[i][cIP] == ipcode) {
			if (CheatIPs[i][cTime] > GetTickCount()) {
				CheatIPs[i][cIP] = Unsigned:0;
				CheatIPs[i][cTime] = 0;
				return 1;
			} else {
				return 0;
			}
		}
	}
	return 0;
}

IsCheatIP(IP[]) {
	new Unsigned:ipcode = IP2Code(IP);
	
	for (new i; i < MAX_CHEAT_IP; i++) {
		if (CheatIPs[i][cIP] && CheatIPs[i][cIP] == ipcode) {
			if (CheatIPs[i][cTime] > GetTickCount()) {
				return 1;
			} else {
				return 0;
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------------
// EVENTS
//-----------------------------------------------------------------------------------------------------

ac_OnPlayerConnect(playerid) {
	if (IsPlayerTempBanned(playerid)) {
		SendClientMessage(playerid, COLOUR_KICK, "You are temporarily suspended for suspected cheats.");
		
		new pname[MAX_NAME];
		new message[MAX_INPUT];
		
		GetPlayerName(playerid, pname, sizeof(pname));
		GetPlayerIp(playerid, message, sizeof(message));
		
		format(message, sizeof(message), "%s [-:%s] is suspended (AntiCheat: Suspected Cheats)", pname, message);
		AddAdminLogLine(COLOUR_LOG, message);
		printf("[FDM] %s", message);
		
		Kick(playerid);
		return 0;
	}
	ac_ResetPlayerData(playerid);
	ac_ShowPlayerTextDraw(playerid);
	EnableStuntBonusForPlayer(playerid, 0);
	return 1;
}

ac_OnPlayerDisconnect(playerid) {
	ac_ResetPlayerData(playerid);
	ac_HidePlayerTextDraw(playerid);
	return 1;
}

ac_OnPlayerSpawn(playerid) {
	if (IsPlayerSpawned(playerid)) ac_SetPlayerHealth(playerid, 100.0);
	return 1;
}

ac_OnPlayerUpdate(playerid) {
	pStats[playerid][pUpdated] = 1;
	
	// Weapon cheat firing disable
	
	if (!IsPlayerSpawned(playerid) || pStats[playerid][pDisableResist]) {
		pStats[playerid][pWeaponHoldTick] = 0;
		return 1;
	}
	
	if (pStats[playerid][pWeaponsDisabled] && !IsPlayerNPC(playerid)) {
		pStats[playerid][pWeaponHoldTick] = 0;
		if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT || !GetPlayerWeapon(playerid)) return 1;
		
		new keys, updown, leftright;
		GetPlayerKeys(playerid, keys, updown, leftright);
		//if (keys & KEY_FIRE || (keys & (KEY_SPRINT | KEY_HANDBRAKE))) return 0;
		if (keys & KEY_FIRE) return 0;
		
	} else {
		new weaponid = GetPlayerWeapon(playerid);
		if (!weaponid) return 1;
		
		switch (weaponid) {
			case 43,44,45: {}
			default: {
				if (IsPlayerNPC(playerid) || pStats[playerid][pWeaponSlotWpn][GetWeaponSlot(weaponid)] == weaponid) {
					pStats[playerid][pWeaponHoldTick] = 0;
					return 1;
				}
			}
		}
		
		new keys, updown, leftright;
		GetPlayerKeys(playerid, keys, updown, leftright);
		if (keys & KEY_FIRE) {
			new tickCount = GetTickCount();
			if (!pStats[playerid][pWeaponHoldTick]) {
				pStats[playerid][pWeaponHoldTick] = tickCount;
			} else if (tickCount - pStats[playerid][pWeaponHoldTick] > 300) {
				SetPlayerArmedWeapon(playerid, 0);
				pStats[playerid][pWeaponHoldTick] = 0;
			}
			return 0;
		}
	}
	
	return 1;
}

ac_OnPlayerDeath(playerid, &killerid, &reason) {
	//SetPlayerHealth(playerid, 1.0); // Fixes that walking around fire bug
	
	new money = pStats[playerid][pMoney];
	new warnings = pStats[playerid][pWarnings];
	new lastWarning = pStats[playerid][pLastWarning];
	
	new lastMoney = pStats[playerid][pLastMoney];
	new Float:lastHealth = pStats[playerid][pLastHealth];
	new Float:lastArmour = pStats[playerid][pLastArmour];
	new lastWeaponSlotWpn[MAX_WEAPON_SLOTS];
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		lastWeaponSlotWpn[i] = pStats[playerid][pLastWeaponSlotWpn];
	}
	
	if (pStats[playerid][pLastDamagedPlayer] != INVALID_PLAYER_ID && pStats[playerid][pLastDamagedWeapon] != INVALID_WEAPON_ID &&
		pStats[playerid][pLastDamagedTick] != 0 && GetTickCount() - pStats[playerid][pLastDamagedTick] < 10000) {
		
		killerid = pStats[playerid][pLastDamagedPlayer];
		reason = pStats[playerid][pLastDamagedWeapon];
	}
	
	new validweapon;
	if (killerid != INVALID_PLAYER_ID) {
		validweapon = (pStats[killerid][pWeaponSlotWpn][GetWeaponSlot(reason)] == reason);
	}
	
	ac_ResetPlayerData(playerid);
	
	// Keep last lag values
	pStats[playerid][pLastMoney] = lastMoney;
	pStats[playerid][pLastHealth] = lastHealth;
	pStats[playerid][pLastArmour] = lastArmour;
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		pStats[playerid][pLastWeaponSlotWpn] = lastWeaponSlotWpn[i];
	}
	
	if (pStats[playerid][pLastHealth] < 100.0) pStats[playerid][pLastHealth] = 100.0;
	
	// Players lose $100 on death by default
	if (pStats[playerid][pLastMoney] < money) pStats[playerid][pLastMoney] = money;
	if (money < 0) {
		pStats[playerid][pMoney] = money;
	} else {
		if (money > 100) {
			pStats[playerid][pMoney] = money - 100;
		} else {
			pStats[playerid][pMoney] = 0;
		}
	}
	
	// Warnings
	pStats[playerid][pWarnings] = warnings;
	pStats[playerid][pLastWarning] = lastWarning;
	
	if (IsPlayerNPC(playerid) || IsPlayerNPC(killerid)) {
		return 1;
	}
	
	// Check distances between players
	if (killerid == INVALID_PLAYER_ID) {
		return 1;
	} else {
		if (GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(killerid)) {
			killerid = INVALID_PLAYER_ID;
			reason = 255;
			return 1;
		}
		
		switch (reason) {
			case 2..15,22..34,38,41,42: { // Valid weapons and normal kill messages
				if (!validweapon && IsPlayerSpawned(killerid)) { // Player does not have that weapon
					new vehicleid = GetPlayerVehicleID(killerid);
					if (vehicleid) {
						if (GetVehicleModel(vehicleid) == 425) {
							if (reason != 38) {
								killerid = INVALID_PLAYER_ID;
								reason = 255;
								return 1;
							}
						} else {
							switch (reason) {
								case 28,29,32: {}
								default: {
									killerid = INVALID_PLAYER_ID;
									reason = 255;
									return 1;
								}
							}
						}
					} else {
						killerid = INVALID_PLAYER_ID;
						reason = 255;
						return 1;
					}
				}
			}
			case 16..21,35,36,39,40,43..47,55..255: { // Impossible kill reasons
				CheatSuspect(playerid, "Fake Kill Message");
				//killerid = INVALID_PLAYER_ID;
				//reason = 255;
				return 1;
			}
		}
		
		new Float: px1, Float:py1, Float:pz1;
		new Float: px2, Float:py2, Float:pz2;
		new Float: distance;
		
		GetPlayerPos(playerid, px1, py1, pz1);
		GetPlayerPos(killerid, px2, py2, pz2);
		
		distance = FindDistance(px1, py1, pz1, px2, py2, pz2);
		if (distance > 300.0) { // Too far
			killerid = INVALID_PLAYER_ID;
			reason = 255;
			return 1;
		}
		
		return 1;
	}
}

ac_OnPlayerGiveDamage(playerid, damagedid, Float:amount, weaponid) {
	if (playerid == INVALID_PLAYER_ID || damagedid == INVALID_PLAYER_ID || pStats[playerid][pWeaponsDisabled] || pStats[damagedid][pGodMode]) return 1;
	//printf("DEBUG GIVE: Damaged playerid %i, damagedid %i, amount %f, weaponid %i", playerid, damagedid, amount, weaponid);
	new valid = 0;
	
	// Hacked weapons check
	if (playerid == INVALID_PLAYER_ID) {
		valid = 1;
		
	} else if (IsPlayerNPC(damagedid)) {
		new Float: health;
		GetPlayerHealth(damagedid, health);
		SetPlayerHealth(damagedid, health - amount);
		
	} else {
		if (!weaponid) { // no weapon
			valid = 1;
			
		} else {
			// Hacked/lagged weapons check
			switch (weaponid) {
				case 43,44,45: {}
				default: {
					if (pStats[playerid][pWeaponSlotWpn][GetWeaponSlot(weaponid)] == weaponid) {
						new Float: weaponDamage = GetWeaponMaxDamage(weaponid);
						if (weaponDamage > 0.1 && amount > weaponDamage + 0.1) {
							// weapon damage is hacked
							
							if (GetTickCount() - pStats[playerid][pWeaponsLastCheat] > MAX_ALERT_INTERVAL) {
								new tmpstr[MAX_INPUT];
								GetWeaponName(weaponid, tmpstr, sizeof(tmpstr));
								format(tmpstr, sizeof(tmpstr), "Weapon: Did %f damage with the weapon %s", amount, tmpstr);
								
								CheatSuspect(playerid, tmpstr);
								pStats[playerid][pWeaponsLastCheat] = GetTickCount();
							}
							
							valid = 0;
						} else {
							// weapon damage modifications
							switch (weaponid) {
								case 2..4: amount = amount * 4.0;
								case 5: amount = amount * 6.0;
								case 6..7: amount = amount * 4.0;
								case 8: amount = amount * 8.0;
								case 9: if (amount > 6.7) amount = amount * 4.0;
								case 10..15: amount = amount * 4.0;
								case 22: amount = amount * 2.0;
								case 25: amount = amount * 1.35;
								case 30: amount = amount * 1.5;
								case 33: amount = amount * 3.0;
								case 34: amount = amount * 3.0;
							}
							
							valid = 1; // weapon is not hacked
						}
					}
				}
			}
			
		}
	}
	
	// Distance check
	if (valid) {
		new Float: px1, Float:py1, Float:pz1;
		new Float: px2, Float:py2, Float:pz2;
		new Float: distance;
		
		GetPlayerPos(playerid, px1, py1, pz1);
		GetPlayerPos(damagedid, px2, py2, pz2);
		
		distance = FindDistance(px1, py1, pz1, px2, py2, pz2);
		if (distance > 300.0) { // Too far
			valid = 0;
		} else {
			// Diminishing bullet damage over distance
			new Float: damageMultiplier = (200.0 - distance) / 200.0;
			if (damageMultiplier <= 0.33) damageMultiplier = 0.33;
			if (damageMultiplier >= 1.0) damageMultiplier = 1.0;
			amount = amount * damageMultiplier;
		}
	}
	
	if (valid && IsPlayerSpawned(playerid) && IsPlayerSpawned(damagedid)) { // Damage the player
		if (pStats[damagedid][pArmour] > 0.0) {
			if (amount > pStats[damagedid][pArmour]) {
				amount -= pStats[damagedid][pArmour];
				pStats[damagedid][pArmour] = 0.0;
			} else {
				pStats[damagedid][pArmour] -= amount;
				amount = 0.0;
			}
			
			pStats[damagedid][pArmourSyncing] = SYNC_DELAY;
			SetPlayerArmour(damagedid, pStats[damagedid][pArmour]);
			pStats[damagedid][pLastDamagedTick] = GetTickCount();
		}
		
		if (amount > 0.1) {
			pStats[damagedid][pHealth] -= amount;
			if (pStats[damagedid][pHealth] < 0.0) pStats[damagedid][pHealth] = 0.0;
			
			pStats[damagedid][pHealthSyncing] = SYNC_DELAY;
			ac_SetPlayerHealth(damagedid, pStats[damagedid][pHealth]);
		}
		
		pStats[damagedid][pLastDamagedPlayer] = playerid;
		pStats[damagedid][pLastDamagedWeapon] = weaponid;
		pStats[damagedid][pLastDamagedTick] = GetTickCount();
	}
	
	return 1;
}

ac_OnPlayerTakeDamage(playerid, issuerid, weaponid) {
	if (playerid == INVALID_PLAYER_ID || pStats[playerid][pGodMode]) return 1;
	//printf("DEBUG TAKE: Damaged playerid %i, issuerid %i, weaponid %i", playerid, issuerid, weaponid);
	new fixplayer = 0;
	
	if (issuerid != INVALID_PLAYER_ID) {
		if (pStats[issuerid][pWeaponsDisabled]) fixplayer = 1;
		
		new vehicleid = GetPlayerVehicleID(issuerid);
		
		if (vehicleid > 0) {
			new vehicletype = GetVehicleModel(vehicleid);
			switch (vehicletype) {
				case 425,432,447,464,476,520: { // Not working vehicles
					fixplayer = 0;
				}
			}
		}
	}
	
	// Hacked weapons check
	if (!fixplayer) {
		if (issuerid == INVALID_PLAYER_ID) {
			fixplayer = 0;
			
		} else if (pStats[issuerid][pWeaponsDisabled]) { // weapons disabled
			fixplayer = 1;
			
		} else {
			if (weaponid) {
				// Hacked/lagged weapons check
				fixplayer = 1; // assume weapon hacked
				
				switch (weaponid) {
					case 2..15,22..36,38,41,42: { // working weapons
						fixplayer = 1;
						/*switch (weaponid) {
							case 43,44,45: {}
							default: {
								if (pStats[issuerid][pWeaponSlotWpn][GetWeaponSlot(weaponid)] == weaponid) {
									fixplayer = 1; // weapon is not hacked
								}
							}
						}*/
					}
					
					default: {
						fixplayer = 0; // not working weapons
					}
				}
				
			}
		}
	}
	
	// Distance check
	if (!fixplayer && playerid != INVALID_PLAYER_ID && issuerid != INVALID_PLAYER_ID) {
		new Float: px1, Float:py1, Float:pz1;
		new Float: px2, Float:py2, Float:pz2;
		new Float: distance;
		
		GetPlayerPos(playerid, px1, py1, pz1);
		GetPlayerPos(issuerid, px2, py2, pz2);
		
		distance = FindDistance(px1, py1, pz1, px2, py2, pz2);
		if (distance > 110.0) { // Too far
			fixplayer = 1;
		}
	}
	
	if (fixplayer && IsPlayerSpawned(playerid)) { // Heal the player
		pStats[playerid][pHealthSyncing] = SYNC_DELAY;
		ac_SetPlayerHealth(playerid, pStats[playerid][pHealth]);
		
		pStats[playerid][pArmourSyncing] = SYNC_DELAY;
		SetPlayerArmour(playerid, pStats[playerid][pArmour]);
	}
	
	return 1;
}

ac_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger) {
	
	new vehiclemodel = GetVehicleModel(vehicleid);
	if (!vehiclemodel) return 1;
	
	if (!ispassenger) {
		
		SetVehicleHealth(vehicleid, vStats[vehicleid][vHealth]);
		UpdateVehicleDamageStatus(vehicleid, 0, 0, 0, 0);
		UpdateVehicleDamageStatus(vehicleid, vStats[vehicleid][vDamage][0], vStats[vehicleid][vDamage][1], vStats[vehicleid][vDamage][2], vStats[vehicleid][vDamage][3]);
		
		switch (vehiclemodel) {
			case 416,588: {
				// Ambulance - gain 20 hp when entering
				// Hotdog - gain 40 hp when entering
				pStats[playerid][pHealthSyncing] = SYNC_DELAY + 5000;
			}
			case 427: {
				// Enforcer - gain full armour when entering
				pStats[playerid][pArmourSyncing] = SYNC_DELAY + 5000;
			}
			case 420,438,575: {
				// Taxi - gain $12 when getting in one
				// Pimp - gain $1000 when getting in one
				pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY + 5000;
			}
		}
		
	}
	
	return 1;
}

ac_OnVehicleMod(playerid, vehicleid, componentid) {
	if (!IsPlayerNearVehicleFix(playerid, 1)) {
		if (!pStats[playerid][pDisableResist]) RemoveVehicleComponent(vehicleid, componentid);
		new tmpstr[MAX_INPUT];
		format(tmpstr, sizeof(tmpstr), "Vehicle: Added component %i without mod shop", componentid);
		CheatSuspect(playerid, tmpstr);
		return 0;
	}
	
	return 1;
}

ac_OnVehiclePaintjob(playerid, paintjobid) {
	if (!IsPlayerNearVehicleFix(playerid, 1)) {
		new tmpstr[MAX_INPUT];
		format(tmpstr, sizeof(tmpstr), "Vehicle: Added paintjob %i without mod shop", paintjobid);
		CheatSuspect(playerid, tmpstr);
		return 0;
	}
	
	return 1;
}

ac_OnPlayerStateChange(playerid, newstate, oldstate) {
	
	if (newstate == PLAYER_STATE_SPECTATING || oldstate == PLAYER_STATE_SPECTATING) return 1;
	if (newstate == PLAYER_STATE_DRIVER) {
		new vehicleid = GetPlayerVehicleID(playerid);
		if (!vehicleid || !GetVehicleModel(vehicleid)) return 1;
		
		if (pStats[playerid][pVehicleID] == vehicleid) return 1;
		
		if (vStats[vehicleid][vDriver] != INVALID_PLAYER_ID &&
			vStats[vehicleid][vDriver] != playerid &&
			pStats[vStats[vehicleid][vDriver]][pVehicleID] == vehicleid &&
			GetPlayerState(vStats[vehicleid][vDriver]) == PLAYER_STATE_DRIVER &&
			GetPlayerVehicleID(vStats[vehicleid][vDriver]) == vehicleid) {
			
			// Anti-vehicle hack stealing
			ClearAnimations(playerid);
			
			pStats[vStats[vehicleid][vDriver]][pVehicleWarning] = 0;
			PutPlayerInVehicle(vStats[vehicleid][vDriver], vehicleid, 0);
			return 0;
			
		} else {
			if (vStats[vehicleid][vDriver] != INVALID_PLAYER_ID) {
				pStats[vStats[vehicleid][vDriver]][pVehicleID] = 0;
			}
			
			vStats[vehicleid][vDriver] = playerid;
			pStats[playerid][pVehicleID] = vehicleid;
			
			if (pStats[playerid][pGodMode]) {
				SetVehicleHealth(vehicleid, GODMODE_VEHICLE_HP);
			} else {
				SetVehicleHealth(vehicleid, vStats[vehicleid][vHealth]);
			}
			
			UpdateVehicleDamageStatus(vehicleid, 0, 0, 0, 0);
			UpdateVehicleDamageStatus(vehicleid, vStats[vehicleid][vDamage][0], vStats[vehicleid][vDamage][1], vStats[vehicleid][vDamage][2], vStats[vehicleid][vDamage][3]);
			
			new Float:vehiclehp;
			GetVehicleHealth(vehicleid, vehiclehp);
			if (pStats[playerid][pLastVehicleHP] < vehiclehp) {
				pStats[playerid][pLastVehicleHP] = vehiclehp;
			}
			if (pStats[playerid][pLastVehicleHP] < vStats[vehicleid][vHealth]) {
				pStats[playerid][pLastVehicleHP] = vStats[vehicleid][vHealth];
			}
			
			pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
			pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
			pStats[playerid][pVehicleWarning] = 0;
			
			VehicleEnterPerformActions(playerid, vehicleid, 0);
		}
		
	} else if (oldstate == PLAYER_STATE_DRIVER) {
		if (!pStats[playerid][pVehicleID]) return 1;
		
		if (GetVehicleModel(pStats[playerid][pVehicleID])) {
			VehicleExitPerformActions(playerid, pStats[playerid][pVehicleID]);
		}
		
		if (pStats[playerid][pGodMode]) {
			SetVehicleHealth(pStats[playerid][pVehicleID], vStats[pStats[playerid][pVehicleID]][vHealth]);
			UpdateVehicleDamageStatus(pStats[playerid][pVehicleID], 0, 0, 0, 0);
			UpdateVehicleDamageStatus(pStats[playerid][pVehicleID], vStats[pStats[playerid][pVehicleID]][vDamage][0], vStats[pStats[playerid][pVehicleID]][vDamage][1], vStats[pStats[playerid][pVehicleID]][vDamage][2], vStats[pStats[playerid][pVehicleID]][vDamage][3]);
		}
		
		vStats[pStats[playerid][pVehicleID]][vDriver] = INVALID_PLAYER_ID;
		pStats[playerid][pVehicleID] = 0;
		
	} else if (newstate == PLAYER_STATE_PASSENGER) {
		new vehicleid = GetPlayerVehicleID(playerid);
		if (!vehicleid || !GetVehicleModel(vehicleid)) return 1;
		
		VehicleEnterPerformActions(playerid, vehicleid, 1);
		
	}/* else if (oldstate == PLAYER_STATE_PASSENGER) {
		VehicleExitPerformActions(playerid, vehicleid);
		
	}*/
	
	return 1;
}

ac_OnVehicleSpawn(vehicleid) {
	new playerid = vStats[vehicleid][vDriver];
	
	if (playerid != INVALID_PLAYER_ID) {
		ac_OnPlayerStateChange(playerid, PLAYER_STATE_ONFOOT, PLAYER_STATE_DRIVER);
		
		pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
		pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
	}
	
	vStats[vehicleid][vHealth] = 1000.0;
	for (new i; i < 4; i++) {
		vStats[vehicleid][vDamage][i] = 0;
		vStats[vehicleid][vLastDamage][i] = 0;
	}
	
	return 1;
}


//-----------------------------------------------------------------------------------------------------
// COMMANDS
//-----------------------------------------------------------------------------------------------------

kcmd:acban(playerid, text[]) {
	new msg[MAX_INPUT];
	new adminlevel = GetAdminLevel(playerid);
	
	if (adminlevel < ADMIN_LEVEL_MISC) {
		format(msg, sizeof(msg), "AC BAN: You must be at least admin level %i to use /acban.", ADMIN_LEVEL_MISC);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /acban [player] (reason)");
		return 1;
	}
	
	new idx;
	new banid = FindPlayer(split(text, idx, ' '));
	
	if (banid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "AC BAN: Invalid player ID/Name.");
		return 1;
	}
	
	if (banid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "AC BAN: You cannot ban yourself.");
		return 1;
	}
	
	new banadminlevel = GetAdminLevel(banid);
	
	if (adminlevel <= banadminlevel) {
		format(msg, sizeof(msg), "AC BAN: You cannot ban level %i admins.", banadminlevel);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	new banname[MAX_NAME];
	GetPlayerName(banid, banname, sizeof(banname));
	
	if (pStats[banid][pWarnings] < 3) {
		format(msg, sizeof(msg), "AC BAN: %s does not have enough anti-cheat warnings to ban.", banname);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	new adminname[MAX_NAME];
	new adminip[MAX_IP];
	new reason[MAX_INPUT];
	GetPlayerName(playerid, adminname, sizeof(adminname));
	GetPlayerIp(playerid, adminip, sizeof(adminip));
	splitcpy(reason, text, idx, 0);
	
	if (!reason[0]) {
		reason = "Cheats";
	}
	
	//format(msg, sizeof(msg), "%s has been banned (%s)", banname, reason);
	//SendClientMessageToAll(COLOUR_KICK, msg);
	
	if (fdm_IsPlayerRegistered(banid)) {
		BanPlayerid(banid, reason, adminname, adminip, bool:(GetAdminState(playerid) & ADMIN_STATE_HIDDEN));
	} else {
		new banip[MAX_IP];
		GetPlayerIp(banid, banip, sizeof(banip));
		BanPlayer("\01", banip, reason, adminname, adminip, bool:(GetAdminState(playerid) & ADMIN_STATE_HIDDEN));
	}
	
	format(msg, sizeof(msg), "%s has banned %s using the anti-cheat (%s)", adminname, banname, reason);
	//AddAdminLogLine(COLOUR_LOG, msg);
	printf("[FDM] %s", msg);
	
	return 1;
}

kcmd:acresist(playerid, text[]) {
	new msg[MAX_INPUT];
	new adminlevel = GetAdminLevel(playerid);
	
	if (adminlevel < ADMIN_LEVEL_MODERATOR) {
		format(msg, sizeof(msg), "AC RESIST: You must be at least admin level %i to use /acresist.", ADMIN_LEVEL_MODERATOR);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /acresist [player]");
		return 1;
	}
	
	new idx, disable;
	new targetid = FindPlayer(split(text, idx, ' '));
	
	if (targetid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "AC RESIST: Invalid player ID/Name.");
		return 1;
	}
	
	splitcpy(msg, text, idx, ' ');
	if (!msg[0]) {
		disable = !pStats[targetid][pDisableResist];
	} else {
		if (!strcmp(msg, "true", true) || strval(msg)) {
			disable = 0;
		} else {
			disable = 1;
		}
	}
	
	if (pStats[targetid][pDisableResist] == disable) {
		GetPlayerName(targetid, msg, sizeof(msg));
		if (disable) {
			format(msg, sizeof(msg), "AC RESIST: Anti-cheat resistance for %s is already disabled.", msg);
		} else {
			format(msg, sizeof(msg), "AC RESIST: Anti-cheat resistance for %s is already enabled.", msg);
		}
		
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		printf("[FDM] %s", msg);
		
		return 1;
	}
	
	new playerName[MAX_NAME], targetName[MAX_NAME];
	pStats[targetid][pDisableResist] = disable;
	GetPlayerName(playerid, playerName, sizeof(playerName));
	GetPlayerName(targetid, targetName, sizeof(targetName));
	
	if (disable) {
		format(msg, sizeof(msg), "AC RESIST: Anti-cheat resistance for %s disabled.", targetName);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		
		format(msg, sizeof(msg), "%s disabled anti-cheat resistance for %s", playerName, targetName);
		AddAdminLogLine(COLOUR_LOG, msg);
		printf("[FDM] %s", msg);
	} else {
		format(msg, sizeof(msg), "AC RESIST: Anti-cheat resistance for %s enabled.", targetName);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		
		format(msg, sizeof(msg), "%s enabled anti-cheat resistance for %s", playerName, targetName);
		AddAdminLogLine(COLOUR_LOG, msg);
		printf("[FDM] %s", msg);
	}
	
	return 1;
}

kcmd:acblock(playerid, text[]) {
	new msg[MAX_INPUT];
	new adminlevel = GetAdminLevel(playerid);
	
	if (adminlevel < ADMIN_LEVEL_MISC) {
		format(msg, sizeof(msg), "AC BLOCK: You must be at least admin level %i to use /acunblock.", ADMIN_LEVEL_MISC);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /acblock [player]");
		return 1;
	}
	
	new idx;
	new blockid = FindPlayer(split(text, idx, ' '));
	
	if (blockid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "AC BLOCK: Invalid player ID/Name.");
		return 1;
	}
	
	if (blockid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "AC BLOCK: You cannot block yourself.");
		return 1;
	}
	
	new banadminlevel = GetAdminLevel(blockid);
	
	if (adminlevel <= banadminlevel) {
		format(msg, sizeof(msg), "AC BLOCK: You cannot block level %i admins.", banadminlevel);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}

	
	new reason[MAX_NAME];
	splitcpy(reason, text, idx, ' ');
	
	if (isnull(reason)) {
		reason = "Suspected Cheats";
	}
	
	format(msg, sizeof(msg), "AC BLOCK: You have blocked %s temporarily using the nti-cheat.", ReturnPlayerName(blockid));
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	
	format(msg, sizeof(msg), "%s has blocked %s temporarily using the anti-cheat (%s).", ReturnPlayerName(playerid), ReturnPlayerName(blockid), reason);
	AddAdminLogLine(COLOUR_LOG, msg);
	printf("[FDM] %s", msg);
	
	AddPlayerToCheatIPs(blockid);
	return 1;
}

//RemoveCheatIP(IP[])
kcmd:acunblock(playerid, text[]) {
	new msg[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MISC) {
		format(msg, sizeof(msg), "AC UNBLOCKIP: You must be at least admin level %i to use /acunblock.", ADMIN_LEVEL_MISC);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /acunblock [ip]");
		return 1;
	}
	
	new IP[MAX_IP], idx;
	splitcpy(IP, text, idx, ' ');
	
	if (!IP[0]) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /acunblock [ip]");
		return 1;
	}
	
	if (!RemoveCheatIP(IP)) {
		format(msg, sizeof(msg), "AC UNBLOCKIP: The IP '%s' is not blocked.", IP);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	format(msg, sizeof(msg), "AC UNBLOCKIP: The IP %s was unblocked.", IP);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
	
	format(msg, sizeof(msg), "%s unblocked the IP %s from the anti-cheat", ReturnPlayerName(playerid), IP);
	AddAdminLogLine(COLOUR_LOG, msg);
	printf("[FDM] %s", msg);
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// PAUSING
//-----------------------------------------------------------------------------------------------------

PauseUpdate() {
	LoopPlayers(i) {
		anticheatCrashedPlayer = i;
		PausePlayerUpdate(i);
	}
	return 1;
}

PausePlayerUpdate(playerid) {
	if (!IsPlayerSpawned(playerid) || GetPlayerInterior(playerid) || IsPlayerNPC(playerid)) {
		pStats[playerid][pPauseWarning] = 0;
		return 1;
	}
	
	new tickCount = GetTickCount();
	new playerlastupdate = GetPlayerLastUpdateTime(playerid);
	if (!playerlastupdate) return 1;
	
	if (tickCount - playerlastupdate > 60000) { // Paused for 1 minute outside
		if (pStats[playerid][pPauseWarning]) return 1;
		
		new playername[MAX_NAME];
		new message[MAX_INPUT];
		
		GetPlayerName(playerid, playername, sizeof(playername));
		
		if (GetMaxAdminLevel() >= ADMIN_LEVEL_ANTICHEAT) {
			// Warn admins
			format(message, sizeof(message), "%s(%i) has paused for over 1 minute outside", playername, playerid);
			AddAdminLogLine(COLOUR_LOG_PAUSED, message);
			printf("[FDM] %s", message);
			
			pStats[playerid][pPauseWarning] = 1;
			pStats[playerid][pPauseStartTick] = tickCount - 60000;
		} else {
			// Just kick the player
			SendClientMessage(playerid, COLOUR_KICK, "You were auto-kicked for pausing outside.");
			format(message, sizeof(message), "Autokick: %s(%i) - Pausing Outside", playername, playerid);
			//AddJoinLine(playerid, COLOUR_KICK, message);
			AddAdminLogLine(COLOUR_LOG_SUSPECT, message);
			SendClientMessageToAll(COLOUR_KICK, message);
			printf("[FDM] %s", message);
			
			pStats[playerid][pPauseWarning] = 0;
			pStats[playerid][pPauseStartTick] = 0;
			
			Kick(playerid);
		}
		return 1;
	} else {
		if (!pStats[playerid][pPauseWarning]) return 1;
		
		new playername[MAX_NAME];
		new message[MAX_INPUT];
		
		GetPlayerName(playerid, playername, sizeof(playername));
		
		format(message, sizeof(message), "%s(%i) has unpaused (Time: %s)", playername, playerid, GetTimeString2(tickCount - pStats[playerid][pPauseStartTick]));
		AddAdminLogLine(COLOUR_LOG_PAUSED, message);
		printf("[FDM] %s", message);
		
		pStats[playerid][pPauseWarning] = 0;
		pStats[playerid][pPauseStartTick] = 0;
	}
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// HEALTH
//-----------------------------------------------------------------------------------------------------

new foodLocations[][locationInfo] = {
	// Food stores
	{375.5660,-68.2220,1001.5151}, //burger
	{374.0000,-119.6410,1001.4922}, //pizza
	{368.7890,-6.8570,1001.8516}, //clukin
	
	// Vending machines
	{199.7696,-107.7419,1.5510},
	{662.3007,-551.2833,16.3359},
	{1279.1410,372.0853,19.5547},
	{1787.7739,-1369.1648,15.7578},
	{1729.6689,-1943.8674,13.5687},
	{2060.0315,-1898.4545,13.5538},
	{2480.8713,-1958.5214,13.5830},
	{2325.9187,-1645.9760,14.8270},
	{199.7696,-107.7419,1.5510},
	{2352.9946,-1357.2770,24.3984},
	{2224.4084,-1153.4156,1025.7969},
	{2140.3635,-1161.4850,23.9922},
	{2153.8108,-1015.6655,62.8849},
	{2271.9146,-77.5772,26.5741},
	{2320.1633,2531.8755,10.8203},
	{2086.7495,2071.5354,11.0579},
	{1659.6379,1721.8336,10.8281},
	{1519.1349,1055.0989,10.8203},
	{2502.9512,1244.6552,10.8203},
	{2647.8584,1128.6964,11.1797},
	{2845.9268,1294.0793,11.3906},
	{1153.7367,-1465.0555,15.7969},
	{-2119.6689,-422.3703,35.5313},
	{-2119.9944,-423.5487,35.5313},
	{-2093.4387,-393.3176,35.5313},
	{-2068.7302,-397.3119,35.5313},
	{-2040.023,-397.3755,35.5313},
	{-2011.3458,-397.3716,35.5313},
	{-2005.4456,-491.0871,35.5313},
	{-2034.2781,-491.0196,35.5313},
	{-2064.1082,-491.0554,35.5313},
	{-2091.9265,-487.0424,35.5313},
	{-1981.8063,142.5103,27.6875},
	{-2229.3704,288.3852,35.3203},
	{-1349.1316,492.4591,11.1953},
	{-1349.1272,494.0255,11.1953},
	{-2419.2222,984.7466,45.2969},
	{-2419.2151,986.1569,45.2969},
	{-1455.2714,2592.6504,55.8359},
	{-252.7562,2598.1252,62.8582},
	{-252.7703,2599.9373,62.8582},
	{-861.9953,1537.5873,22.5870},
	{-75.0634,1228.1898,19.7321},
	{-14.8740,1176.3368,19.5634},
	{1634.2598,-2238.5327,13.5073},
	{-2091.9041,-491.0815,35.5313},
	{-2097.4304,-397.3233,35.5313},
	{-2087.9290,-487.0315,35.5313},
	{-17.7140,-90.6940,1003.5469},
	{-16.3014,-90.6721,1003.5469},
	{1929.7532,-1772.2109,13.5469},
	{374.0078,-179.1724,1000.6328},
	{378.0044,-179.0817,1000.6328},
	{-36.3695,-56.9197,1003.5469},
	{-19.2225,-56.8625,1003.5469},
	{1153.7667,-1461.0714,15.7969},
	{495.7806,-23.3484,1000.6797},
	{501.9810,-2.4421,1000.6797},
	{500.7921,-2.3424,1000.6797},
	{2156.8982,1606.9410,999.9717},
	{2156.8389,1608.0439,999.9691},
	{2202.2229,1617.9775,999.9766},
	{2209.4075,1620.2117,999.9827},
	{2223.1653,1606.9534,999.9758},
	{2223.3345,1602.8253,999.9789},
	{2208.9355,1607.0234,999.9689},
	{2575.6729,-1284.6438,1060.9844},
	{330.8416,177.5134,1019.9912},
	{332.0887,177.5154,1019.9844},
	{371.7842,177.4837,1019.9844},
	{375.1282,187.9473,1008.3893},
	{351.9388,206.2890,1008.3828}
};

HealthSync() {
	LoopPlayers(i) {
		if (IsPlayerNPC(i)) continue;
		anticheatCrashedPlayer = i;
		HealthPlayerSync(i);
	}
	return 1;
}

HealthPlayerSync(playerid) {
	if (!IsPlayerSpawned(playerid)) {
		pStats[playerid][pHealth] = 100.0;
		pStats[playerid][pHealthSyncing] = SYNC_DELAY;
		return 1;
	}
	
	if (IsPlayerPaused(playerid, SYNC_DELAY)) {
		pStats[playerid][pHealthSyncing] = SYNC_DELAY;
		return 1;
	}
	
	new Float:playerhealth;
	GetPlayerHealth(playerid, playerhealth);
	if (playerhealth < 0.0) playerhealth = 0.0;
	
	if (pStats[playerid][pGodMode]) {
		if (pStats[playerid][pHealth] > 0.0 && playerhealth < 255.0) {
			SetPlayerHealth(playerid, GODMODE_HP);
		}
		return 1;
	}
	
	if (pStats[playerid][pHealthSyncing]) {
		pStats[playerid][pHealthSyncing] = pStats[playerid][pHealthSyncing] - anticheatSyncInterval;
		if (pStats[playerid][pHealthSyncing] < 0) pStats[playerid][pHealthSyncing] = 0;
		//return 1;
	}
	
	if (!pStats[playerid][pUpdated]) return 1;
	
	if (playerhealth > pStats[playerid][pHealth] + 0.1) {
		if (IsPlayerNearHealth(playerid)) {
			if (playerhealth > 100.0) {
				if (!pStats[playerid][pHealthSyncing]) {
					// More healing than possible
					if (!pStats[playerid][pDisableResist]) {
						ac_SetPlayerHealth(playerid, pStats[playerid][pHealth]);
					}
					
					if (!pStats[playerid][pHealthLastCheat] || GetTickCount() - pStats[playerid][pHealthLastCheat] > MAX_ALERT_INTERVAL) {
						new tmpstr[MAX_INPUT];
						format(tmpstr, sizeof(tmpstr), "Health: Increased %0.1f > %0.1f HP when healing", pStats[playerid][pHealth], playerhealth);
						CheatSuspect(playerid, tmpstr);
						pStats[playerid][pHealthLastCheat] = GetTickCount();
					}
				}
			} else {
				pStats[playerid][pHealth] = playerhealth;
				pStats[playerid][pLastPurchase] = GetTickCount();
				pStats[playerid][pLastPurchaseType] = PURCHASETYPE_HEALTH;
				pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
			}
		} else {
			if (!pStats[playerid][pHealthSyncing]) {
				if (!pStats[playerid][pDisableResist]) {
					ac_SetPlayerHealth(playerid, pStats[playerid][pHealth]);
				}
				
				// In GTASA, if the health is below 5, the player will heal themselves
				if ((pStats[playerid][pHealth] > 5.0) || (playerhealth - pStats[playerid][pHealth] > 5.0)) {
					//if (playerhealth > 100.0) { // The 255.0 looparound - kill player
					if (playerhealth > 91.0) { // The 255.0 looparound - kill player
						ac_SetPlayerHealth(playerid, 0.0);
						pStats[playerid][pHealth] = 0.0;
						pStats[playerid][pHealthSyncing] = SYNC_DELAY;
						
					} else if (!pStats[playerid][pHealthLastCheat] || GetTickCount() - pStats[playerid][pHealthLastCheat] > MAX_ALERT_INTERVAL) {
						new tmpstr[MAX_INPUT];
						format(tmpstr, sizeof(tmpstr), "Health: Increased %0.1f > %0.1f HP", pStats[playerid][pHealth], playerhealth);
						//format(tmpstr, sizeof(tmpstr), "Health: Increased %0.1f > %0.1f HP (Lag HP: %0.1f)", pStats[playerid][pHealth], playerhealth, pStats[playerid][pLastHealth]);
						
						if (playerhealth <= pStats[playerid][pLastHealth] + 0.1) {
							CheatSuspect(playerid, tmpstr, CHEAT_FLAG_LAG);
						} else {
							CheatSuspect(playerid, tmpstr);
						}
						
						pStats[playerid][pHealthLastCheat] = GetTickCount();
					}
				}
			}
		}
		
	} else if (playerhealth < pStats[playerid][pHealth] - 0.1) {
		if (!pStats[playerid][pHealthSyncing]) {
			if (pStats[playerid][pHealth] > pStats[playerid][pLastHealth] - 0.1) pStats[playerid][pLastHealth] = pStats[playerid][pHealth];
			pStats[playerid][pHealth] = playerhealth;
		}
		
	} else {
		if (!pStats[playerid][pHealthSyncing] && (!pStats[playerid][pHealthLastCheat] || GetTickCount() - pStats[playerid][pHealthLastCheat] > MAX_ALERT_INTERVAL)) {
			pStats[playerid][pLastHealth] = playerhealth;
		}
	}
	
	return 1;
}

HealthAcceptPlayerData(playerid) {
	if (pStats[playerid][pGodMode]) return 1;
	ac_GetPlayerHealth(playerid, pStats[playerid][pHealth]);
	pStats[playerid][pHealthLastCheat] = 0;
	pStats[playerid][pHealthSyncing] = SYNC_DELAY;
	return 1;
}

IsPlayerNearHealth(playerid) {
	if (!IsPlayerSpawned(playerid)) return 0;
	
	for (new i; i < sizeof(foodLocations); i++) {
		if (IsPlayerInRangeOfPoint(playerid, 3.0, foodLocations[i][fx], foodLocations[i][fy], foodLocations[i][fz])) return 1;
	}
	return 0;
}

stock kSetPlayerHealth(playerid, Float:health, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	new rtn = 1;
	
	if (health < 0.0) health = 0.0;
	if (!pStats[playerid][pGodMode] || health == 0.0) {
		pStats[playerid][pGodMode] = 0;
		if (!cancel) ac_SetPlayerHealth(playerid, health); else rtn = 0;
	}
	pStats[playerid][pHealth] = health;
	pStats[playerid][pHealthSyncing] = SYNC_DELAY;
	
	if (health > pStats[playerid][pLastHealth] - 0.1) {
		pStats[playerid][pLastHealth] = health;
	}
	
	return rtn;
}

stock kGetPlayerHealth(playerid, &Float:health) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	if (pStats[playerid][pGodMode]) {
		health = pStats[playerid][pHealth];
	} else {
		ac_GetPlayerHealth(playerid, health);
		if (health > pStats[playerid][pHealth]) {
			health = pStats[playerid][pHealth];
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// ARMOUR
//-----------------------------------------------------------------------------------------------------

new armourLocations[][locationInfo] = {
	{295.5747,-80.8067,1001.5156}, //ammunation1
	{290.0636,-109.7777,1001.5156}, //ammunation2
	{296.3013,-38.4593,1001.5156}, //ammunation3
	{312.8451,-166.0348,999.6010}, //ammunation4
	{308.2652,-140.9205,999.6016} // hidden ammunation
};

ArmourSync() {
	LoopPlayers(i) {
		if (IsPlayerNPC(i)) continue;
		anticheatCrashedPlayer = i;
		ArmourPlayerSync(i);
	}
	return 1;
}

ArmourPlayerSync(playerid) {
	if (!IsPlayerSpawned(playerid)) {
		pStats[playerid][pArmour] = 0.0;
		pStats[playerid][pArmourSyncing] = SYNC_DELAY;
		return 1;
	}
	
	if (IsPlayerPaused(playerid, SYNC_DELAY)) {
		return 1;
	}
	
	new Float:playerarmour;
	GetPlayerArmour(playerid, playerarmour);
	if (playerarmour < 0) playerarmour = 0;
	
	if (pStats[playerid][pGodMode]) {
		if (playerarmour != GODMODE_ARMOUR) SetPlayerArmour(playerid, GODMODE_ARMOUR);
		//SetPlayerArmour(playerid, GODMODE_ARMOUR);
		return 1;
	}
	
	if (pStats[playerid][pArmourSyncing]) {
		pStats[playerid][pArmourSyncing] = pStats[playerid][pArmourSyncing] - anticheatSyncInterval;
		if (pStats[playerid][pArmourSyncing] < 0) pStats[playerid][pArmourSyncing] = 0;
		//return 1;
	}
	
	if (!pStats[playerid][pUpdated]) return 1;
	
	if (playerarmour > pStats[playerid][pArmour] + 0.1) {
		
		if (IsPlayerNearArmour(playerid)) {
			if (playerarmour > 100.0) {
				if (!pStats[playerid][pArmourSyncing] && (!pStats[playerid][pArmourLastCheat] || GetTickCount() - pStats[playerid][pArmourLastCheat] > MAX_ALERT_INTERVAL)) {
					new message[MAX_INPUT];
					format(message, sizeof(message), "Armour: Increased %0.1f > %0.1f armour at ammunation", pStats[playerid][pArmour], playerarmour);
					CheatSuspect(playerid, message);
					pStats[playerid][pArmourLastCheat] = GetTickCount();
				}
			} else {
				pStats[playerid][pLastPurchase] = GetTickCount();
				pStats[playerid][pLastPurchaseType] = PURCHASETYPE_ARMOUR;
				pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
				pStats[playerid][pArmour] = playerarmour;
			}
		} else {
			if (!pStats[playerid][pArmourSyncing]) {
				if (!pStats[playerid][pDisableResist]) {
					SetPlayerArmour(playerid, pStats[playerid][pArmour]);
				}
				
				if (!pStats[playerid][pArmourLastCheat] || GetTickCount() - pStats[playerid][pArmourLastCheat] > MAX_ALERT_INTERVAL) {
					new message[MAX_INPUT];
					format(message, sizeof(message), "Armour: Increased %0.1f > %0.1f armour", pStats[playerid][pArmour], playerarmour);
					//format(message, sizeof(message), "Armour: Increased %0.1f > %0.1f armour (Lag Armour: %0.1f)", pStats[playerid][pArmour], playerarmour, pStats[playerid][pLastArmour]);
					
					if (playerarmour <= pStats[playerid][pLastArmour] + 0.1) {
						CheatSuspect(playerid, message, CHEAT_FLAG_LAG);
					} else {
						CheatSuspect(playerid, message);
					}
					
					pStats[playerid][pArmourLastCheat] = GetTickCount();
				}
			}
		}
		
	} else if (playerarmour < pStats[playerid][pArmour] - 0.1) {
		if (!pStats[playerid][pArmourSyncing]) {
			if (pStats[playerid][pArmour] > pStats[playerid][pLastArmour] - 0.1) pStats[playerid][pLastArmour] = pStats[playerid][pArmour];
			pStats[playerid][pArmour] = playerarmour;
		}
		
	} else {
		if (!pStats[playerid][pArmourSyncing] && (!pStats[playerid][pArmourLastCheat] || GetTickCount() - pStats[playerid][pArmourLastCheat] > MAX_ALERT_INTERVAL)) {
			pStats[playerid][pLastArmour] = playerarmour;
		}
	}
	
	return 1;
}

ArmourAcceptPlayerData(playerid) {
	if (pStats[playerid][pGodMode]) return 1;
	GetPlayerArmour(playerid, pStats[playerid][pArmour]);
	pStats[playerid][pArmourLastCheat] = 0;
	pStats[playerid][pArmourSyncing] = SYNC_DELAY;
	return 1;
}

IsPlayerNearArmour(playerid) {
	if (!IsPlayerSpawned(playerid)) return 0;
	
	for (new i; i < sizeof(armourLocations); i++) {
		if (IsPlayerInRangeOfPoint(playerid, 10.0, armourLocations[i][fx], armourLocations[i][fy], armourLocations[i][fz])) return 1;
	}
	return 0;
}

stock kSetPlayerArmour(playerid, Float:armour, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	new rtn = 1;
	
	if (armour < 0.0) armour = 0.0;
	if (!pStats[playerid][pGodMode]) if (!cancel) SetPlayerArmour(playerid, armour); else rtn = 0;
	pStats[playerid][pArmour] = armour;
	pStats[playerid][pArmourSyncing] = SYNC_DELAY;
	
	if (armour > pStats[playerid][pLastArmour] - 0.1) {
		pStats[playerid][pLastArmour] = armour;
	}
	
	return rtn;
}

stock kGetPlayerArmour(playerid, &Float:armour) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	if (pStats[playerid][pGodMode]) {
		armour = pStats[playerid][pArmour];
	} else {
		GetPlayerArmour(playerid, armour);
		if (armour > pStats[playerid][pArmour]) {
			armour = pStats[playerid][pArmour];
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// MONEY
//-----------------------------------------------------------------------------------------------------

new moneyLocations[][locationInfo] = {
	// 4 Dragons Casino
	{1962.3431,1010.1740,992.4688},
	{1958.0288,1010.1550,992.4688},
	{1958.7371,1015.6306,992.4688},
	{1963.9728,1015.7000,992.4688},
	{1963.9725,1020.1454,992.4688},
	{1958.7369,1020.2162,992.4688},
	{1958.0288,1025.8231,992.4688},
	{1962.3407,1025.5699,992.4688},
	{1966.5991,1029.6577,992.4745},
	{1969.5425,1029.7184,992.4745},
	{1969.5437,1021.7614,992.4688},
	{1969.5422,1014.0912,992.4688},
	{1969.5450,1006.3813,992.4745},
	{1966.5953,1006.3868,992.4745},
	{1966.5963,1014.0477,992.4688},
	{1966.5986,1021.6609,992.4688},
	{1945.9033,986.5529,992.4688},
	{1940.6799,990.9129,992.4609},
	{1935.6685,986.6136,992.4745},
	{1940.1447,1005.7741,992.4688},
	{1942.8999,1006.8035,992.4688},
	{1941.8329,1014.2010,992.4688},
	{1938.8879,1014.2562,992.4688},
	{1938.8827,1021.5009,992.4688},
	{1941.8306,1021.3914,992.4688},
	{1942.6344,1028.9602,992.4688},
	{1939.7578,1029.6250,992.4688},
	
	// Caligulas Casino
	{2275.2136,1589.7646,1006.1797},
	{2275.2188,1596.5017,1006.1797},
	{2271.5913,1589.7173,1006.1797},
	{2266.5378,1589.7927,1006.1797},
	{2271.6785,1596.4629,1006.1797},
	{2266.6589,1596.4768,1006.1797},
	{2261.6287,1589.7120,1006.1797},
	{2261.4431,1596.5834,1006.1797},
	{2256.1807,1589.9916,1006.1797},
	{2256.2876,1596.5729,1006.1797},
	{2242.1169,1596.1278,1006.1852},
	{2242.0771,1590.5620,1006.1824},
	{2230.3428,1590.5573,1006.1824},
	{2230.3486,1596.1256,1006.1852},
	{2241.3359,1601.1166,1006.1797},
	{2244.7559,1604.4454,1006.1860},
	{2241.3601,1607.9033,1006.1797},
	{2237.7944,1604.4478,1006.1860},
	{2241.2747,1601.1187,1006.1797},
	{2241.2793,1615.9233,1006.1807},
	{2241.3945,1618.2382,1006.1790},
	{2230.9968,1618.2875,1006.1790},
	{2230.6187,1615.9630,1006.1806},
	{2230.3015,1607.9055,1006.1860},
	{2233.8179,1604.4104,1006.1797},
	{2230.4392,1601.1161,1006.1860},
	{2226.8618,1604.3907,1006.1797},
	
	// Casino
	{1133.8165,-2.7623,1000.6797},
	{1126.4077,3.0247,1000.6797},
	{1133.9337,-0.3960,1000.6797},
	{1128.7096,-1.6074,1000.6797},
	{1125.0798,-5.0441,1000.6797},
	{1125.1041,1.5979,1000.6797},
	{1119.0232,-1.7199,1000.6913}
};

MoneySync() {
	LoopPlayers(i) {
		if (IsPlayerNPC(i)) continue;
		anticheatCrashedPlayer = i;
		MoneyPlayerSync(i);
	}
	return 1;
}

MoneyPlayerSync(playerid) {
	if (!IsPlayerSpawned(playerid) || IsPlayerPaused(playerid, SYNC_DELAY)) {
		pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
		return 1;
	}
	
	new playermoney = GetPlayerMoney(playerid);
	
	if (pStats[playerid][pMoneySyncing]) {
		pStats[playerid][pMoneySyncing] = pStats[playerid][pMoneySyncing] - anticheatSyncInterval;
		if (pStats[playerid][pMoneySyncing] < 0) pStats[playerid][pMoneySyncing] = 0;
		//return 1;
	}
	
	// Player must lose money within the sync delay //pStats[playerid][pLastPurchaseType]
	/*if (!pStats[playerid][pMoneySyncing] && pStats[playerid][pLastPurchase]) {
		if (GetTickCount() - pStats[playerid][pLastPurchase] < MONEY_SYNC_DELAY) {
			if (GetTickCount() - pStats[playerid][pLastMoneyLoss] < MONEY_SYNC_DELAY) {
				// Player has lost money
				pStats[playerid][pLastPurchase] = 0;
			}
		} else {
			// Player has not lost money
			switch (pStats[playerid][pLastPurchaseType]) {
				case PURCHASETYPE_HEALTH: {
					pStats[playerid][pLastMoneyLossWarnings]++; // You lose only 50c at a time
					if (pStats[playerid][pLastMoneyLossWarnings] > 1) {
						CheatSuspect(playerid, "Money/Health: Gained health at store/vending without losing money");
					}
				}
				case PURCHASETYPE_ARMOUR: {
					CheatSuspect(playerid, "Money/Armour: Gained armour at ammunation without losing money");
				}
				case PURCHASETYPE_WEAPONS: {
					CheatSuspect(playerid, "Money/Weapon: Gained weapons at ammunation without losing money");
				}
				case PURCHASETYPE_VEHICLEHP: {
					CheatSuspect(playerid, "Money/Vehicle: Gained vehicle HP at respray/modshop without losing money");
				}
			}
			pStats[playerid][pLastPurchase] = 0;
		}
	}*/
	
	if (playermoney == pStats[playerid][pMoney]) {
		if (!pStats[playerid][pMoneySyncing] && (!pStats[playerid][pMoneyLastCheat] || GetTickCount() - pStats[playerid][pMoneyLastCheat] > MAX_ALERT_INTERVAL)) {
			pStats[playerid][pLastMoney] = playermoney;
		}
		return 1;
	}
	
	if (playermoney > pStats[playerid][pMoney]) {
		new moneygain = playermoney - pStats[playerid][pMoney];
		if (IsPlayerNearMoney(playerid)) {
			if (moneygain > MAX_MONEYPERWIN) { // TODO: Perhaps better casino anti-cheat
				if (!pStats[playerid][pMoneySyncing]) {
					if (!pStats[playerid][pDisableResist]) {
						ResetPlayerMoney(playerid);
						GivePlayerMoney(playerid, pStats[playerid][pMoney]);
					}
					
					if (!pStats[playerid][pMoneyLastCheat] || GetTickCount() - pStats[playerid][pMoneyLastCheat] > MAX_ALERT_INTERVAL) {
						new tmpstr[MAX_INPUT];
						format(tmpstr, sizeof(tmpstr), "Money: Increased $%i > $%i in one win at casino", pStats[playerid][pMoney], playermoney);
						CheatSuspect(playerid, tmpstr);
						pStats[playerid][pMoneyLastCheat] = GetTickCount();
					}
				}
				return 1;
			} else {
				new tmp;
				tmp = pStats[playerid][pCasinoWinCash];
				pStats[playerid][pCasinoWinCash] = pStats[playerid][pCasinoWinCash] + moneygain;
				
				if (pStats[playerid][pCasinoWinCash] < tmp) { // Overflow protection
					pStats[playerid][pCasinoWinCash] = 0x7FFFFFFF;
				}
				
				if (pStats[playerid][pCasinoWinCash] < MAX_MONEYTOTALWIN) {
					pStats[playerid][pMoney] = playermoney;
				} else {
					if (!pStats[playerid][pMoneySyncing]) {
						if (!pStats[playerid][pDisableResist]) {
							ResetPlayerMoney(playerid);
							GivePlayerMoney(playerid, pStats[playerid][pMoney]);
						}
						
						if (!pStats[playerid][pMoneyLastCheat] || GetTickCount() - pStats[playerid][pMoneyLastCheat] > MAX_ALERT_INTERVAL) {
							new tmpstr[MAX_INPUT];
							format(tmpstr, sizeof(tmpstr), "Money: Has won over $%i total at casino", pStats[playerid][pCasinoWinCash]);
							CheatSuspect(playerid, tmpstr);
							pStats[playerid][pMoneyLastCheat] = GetTickCount();
						}
					}
				}
			}
		} else {
			if (!pStats[playerid][pMoneySyncing]) {
				if (!pStats[playerid][pDisableResist]) {
					ResetPlayerMoney(playerid);
					GivePlayerMoney(playerid, pStats[playerid][pMoney]);
				}
				
				#if MAX_MONEY_TOLERANCE > 0
					if (moneygain < MAX_MONEY_TOLERANCE) return 1;
				#endif
				
				if (!pStats[playerid][pMoneyLastCheat] || GetTickCount() - pStats[playerid][pMoneyLastCheat] > MAX_ALERT_INTERVAL) {
					new tmpstr[MAX_INPUT];
					format(tmpstr, sizeof(tmpstr), "Money: Increased $%i > $%i", pStats[playerid][pMoney], playermoney);
					//format(tmpstr, sizeof(tmpstr), "Money: Increased $%i > $%i (Lag Money: %i)", pStats[playerid][pMoney], playermoney, pStats[playerid][pLastMoney]);
					
					if (playermoney <= pStats[playerid][pLastMoney]) {
						CheatSuspect(playerid, tmpstr, CHEAT_FLAG_LAG);
					} else {
						CheatSuspect(playerid, tmpstr);
					}
					pStats[playerid][pMoneyLastCheat] = GetTickCount();
				}
			}
		}
		return 1;
	}
	
	if (playermoney < pStats[playerid][pMoney]) {
		if (!pStats[playerid][pMoneySyncing]) {
			if (pStats[playerid][pMoney] > pStats[playerid][pLastMoney]) pStats[playerid][pLastMoney] = pStats[playerid][pMoney];
			
			if (IsPlayerNearMoney(playerid)) { // Player is in casino
				pStats[playerid][pCasinoWinCash] = pStats[playerid][pCasinoWinCash] - (pStats[playerid][pMoney] - playermoney);
			}
			
			pStats[playerid][pMoney] = playermoney;
			pStats[playerid][pLastMoneyLoss] = GetTickCount();
			pStats[playerid][pLastMoneyLossWarnings] = 0;
		}
		return 1;
	}
	return 1;
}

MoneyAcceptPlayerData(playerid) {
	pStats[playerid][pMoney] = GetPlayerMoney(playerid);
	pStats[playerid][pMoneyLastCheat] = 0;
	pStats[playerid][pLastPurchase] = 0;
	pStats[playerid][pCasinoWinCash] = 0;
	pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
	return 1;
}

IsPlayerNearMoney(playerid) {
	if (!IsPlayerSpawned(playerid)) return 0;
	
	for (new i; i < sizeof(moneyLocations); i++) {
		if (IsPlayerInRangeOfPoint(playerid, 7.5, moneyLocations[i][fx], moneyLocations[i][fy], moneyLocations[i][fz])) return 1;
	}
	return 0;
}

stock kResetPlayerMoney(playerid, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	pStats[playerid][pMoney] = 0;
	pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
	
	if (!cancel) return ResetPlayerMoney(playerid);
	return 0;
}

stock kGivePlayerMoney(playerid, money, cancel = 0) {
	if (!money) return 0;
	if (!IsPlayerConnected(playerid)) return 0;
	
	// Overflow protection
	if (money > 0) {
		if (pStats[playerid][pMoney] + money < pStats[playerid][pMoney]) {
			money = 0x7FFFFFFF - pStats[playerid][pMoney];
		}
	} else {
		if (pStats[playerid][pMoney] + money > pStats[playerid][pMoney]) {
			money = pStats[playerid][pMoney] + 0x80000000;
		}
	}
	
	
	pStats[playerid][pMoney] = pStats[playerid][pMoney] + money;
	pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
	
	if (pStats[playerid][pMoney] > pStats[playerid][pLastMoney]) {
		pStats[playerid][pLastMoney] = pStats[playerid][pMoney];
	}
	
	if (!cancel) return GivePlayerMoney(playerid, money);
	return 0;
}

stock kGetPlayerMoney(playerid) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	return pStats[playerid][pMoney];
}

//-----------------------------------------------------------------------------------------------------
// WEAPONS
//-----------------------------------------------------------------------------------------------------

new weaponLocations[][locationInfo] = {
	{295.5747,-80.8067,1001.5156}, //ammunation1
	{290.0636,-109.7777,1001.5156}, //ammunation2
	{296.3013,-38.4593,1001.5156}, //ammunation3
	{312.8451,-166.0348,999.6010}, //ammunation4
	{308.2652,-140.9205,999.6016} // hidden ammunation
};

// Weapons that ammunation sells
new ammunationWeapons[] = {
	22,23,24,32,28,29,25,26,27,30,31
};

// Weapon ammo considered as definate hacks
/*new definateCheatAmmo[] = {
	69,198,500
};*/

WeaponSync() {
	LoopPlayers(i) {
		if (IsPlayerNPC(i)) continue;
		anticheatCrashedPlayer = i;
		WeaponPlayerSync(i);
	}
	return 1;
}

WeaponPlayerSync(playerid) {
	
	if (!IsPlayerSpawned(playerid)) {
		pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
		return 1;
	}
	
	if (!pStats[playerid][pVisitedAmmunation] && IsPlayerNearWeapons(playerid)) {
		pStats[playerid][pVisitedAmmunation] = 1;
	}
	
	if (!pStats[playerid][pWeaponsDisabled]) {
		new curwpn = GetPlayerWeapon(playerid);
		new curammo = GetPlayerAmmo(playerid);
		if (pStats[playerid][pWeaponLastWpn] != curwpn) {
			// Different weapon
			pStats[playerid][pWeaponLastWpn] = curwpn;
			pStats[playerid][pWeaponLastAmmo] = curammo;
			pStats[playerid][pWeaponSwitching] = 1;
		} else {
			// Same weapon
			if (pStats[playerid][pWeaponSwitching] && pStats[playerid][pWeaponLastAmmo] != curammo) {
				//weapon recently switched
				pStats[playerid][pWeaponLastAmmo] = curammo;
				pStats[playerid][pWeaponSwitching] = 0;
			}
		}
		
		if (curwpn && !pStats[playerid][pWeaponSyncing] && pStats[playerid][pUpdated]) {
			pStats[playerid][pWeaponsLoaded] = 1;
		}
	}
	
	if (IsPlayerPaused(playerid, SYNC_DELAY) || IsPlayerInAnyVehicle(playerid)) {
		pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
		return 1;
	}
	
	if (pStats[playerid][pWeaponSyncing]) {
		pStats[playerid][pWeaponSyncing] = pStats[playerid][pWeaponSyncing] - anticheatSyncInterval;
		if (pStats[playerid][pWeaponSyncing] < 0) pStats[playerid][pWeaponSyncing] = 0;
		return 1;
	}
	
	if (!pStats[playerid][pUpdated]) return 1;
	
	new weaponid, ammo;
	new resetweapons;
	new cheatwpns[MAX_WEAPON_SLOTS];
	new cheatammo[MAX_WEAPON_SLOTS];
	new cheatlag[MAX_WEAPON_SLOTS];
	
	// Scan player for weapon cheats, record in cheatwpns
	for (new weaponslot; weaponslot < MAX_WEAPON_SLOTS; weaponslot++) {
		GetPlayerWeaponData(playerid, weaponslot, weaponid, ammo);
		
		// Checks if the player has no ammo remaining for this weapon slot - this part may be causing some bugs
		if (!weaponid && !ammo && !pStats[playerid][pVisitedAmmunation] && !pStats[playerid][pWeaponsDisabled] && !pStats[playerid][pWeaponSwitching] && pStats[playerid][pWeaponsLoaded] && pStats[playerid][pWeaponSlotAmmo][weaponslot] < 150) {
			pStats[playerid][pLastWeaponSlotWpn][weaponslot] = pStats[playerid][pWeaponSlotWpn][weaponslot];
			//pStats[playerid][pWeaponSlotWpn][weaponslot] = INVALID_WEAPON_SLOT;
			pStats[playerid][pWeaponSlotAmmo][weaponslot] = 0;
			continue;
		}
		//if (!weaponid) continue;
		
		if (IsWeaponMelee(weaponid)) {
			ammo = -1;
		} else if (ammo < 0) {
			ammo += 0x8000;
		}
		
		// Different weapon from what they're supposed to have
		if (weaponid != pStats[playerid][pWeaponSlotWpn][weaponslot]) {
			if (pStats[playerid][pVisitedAmmunation] && IsAmmunationWeapon(weaponid)) {
				// Player got the gun from ammunation
				pStats[playerid][pWeaponSlotWpn][weaponslot] = weaponid;
				
				if (!IsWeaponSlotShared(weaponslot)) {
					pStats[playerid][pWeaponSlotAmmo][weaponslot] = 0;
				}
			} else {
				cheatwpns[weaponslot] = weaponid;
				if (!IsWeaponSlotShared(weaponslot)) {
					if (ammo == -1) {
						cheatammo[weaponslot] = -1;
					} else {
						cheatammo[weaponslot] = ammo;
					}
				} else {
					if (ammo > pStats[playerid][pWeaponSlotAmmo][weaponslot]) {
						if (ammo == -1) {
							cheatammo[weaponslot] = -1;
						} else {
							cheatammo[weaponslot] = ammo - pStats[playerid][pWeaponSlotAmmo][weaponslot];
						}
					}
				}
				if (!pStats[playerid][pWeaponsDisabled]) {
					if (weaponid == pStats[playerid][pLastWeaponSlotWpn][weaponslot]) {
						cheatlag[weaponslot] = 1;
					}
				}
			}
		}
		
		if (ammo == pStats[playerid][pWeaponSlotAmmo][weaponslot]) {
			if (!pStats[playerid][pWeaponsDisabled]) {
				continue;
			}
		}
		
		if (ammo < pStats[playerid][pWeaponSlotAmmo][weaponslot]) {
			if (!pStats[playerid][pWeaponsDisabled]) {
				if (pStats[playerid][pWeaponSwitching]) continue;
				if (pStats[playerid][pWeaponLastWpn] != weaponid) {
					if (!ammo && pStats[playerid][pWeaponSlotAmmo][weaponslot] < 150) {
						pStats[playerid][pWeaponSlotAmmo][weaponslot] = 0;
					}
				} else {
					if (ammo == -1) continue;
					pStats[playerid][pWeaponSlotAmmo][weaponslot] = ammo;
				}
				continue;
			}
		}
		
		if (pStats[playerid][pWeaponsDisabled]) {
			if (pStats[playerid][pVisitedAmmunation]) {
				if (IsAmmunationWeapon(weaponid)) {
					if (ammo < MAX_AMMOPERPURCHASE && (ammo != -1 || IsWeaponMelee(weaponid))) {
						pStats[playerid][pWeaponSlotAmmo][weaponslot] = pStats[playerid][pWeaponSlotAmmo][weaponslot] + ammo;
						pStats[playerid][pWeaponSyncing] = GetPlayerPing(playerid) + 150;
						resetweapons = 1;
						
						pStats[playerid][pLastPurchase] = GetTickCount();
						pStats[playerid][pLastPurchaseType] = PURCHASETYPE_WEAPONS;
						pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
						
						new tmpstr[MAX_INPUT];
						format(tmpstr, sizeof(tmpstr), "%s Ammo: %i", ReturnWeaponName(weaponid), pStats[playerid][pWeaponSlotAmmo][weaponslot]);
						GameTextForPlayer(playerid, tmpstr, 5000, 3);
					}
					continue;
				}
			}
			
			cheatwpns[weaponslot] = weaponid;
			if (ammo == -1) {
				cheatammo[weaponslot] = -1;
			} else {
				cheatammo[weaponslot] = ammo - pStats[playerid][pWeaponSlotAmmo][weaponslot];
			}
		} else {
			if (ammo > pStats[playerid][pWeaponSlotAmmo][weaponslot]) {
				if (pStats[playerid][pVisitedAmmunation]) {
					if (IsAmmunationWeapon(weaponid)) {
						/*if (!pStats[playerid][pWeaponSlotAmmo][weaponslot]) {
							pStats[playerid][pLastPurchase] = GetTickCount();
							pStats[playerid][pLastPurchaseType] = PURCHASETYPE_WEAPONS;
							pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
						}*/
						pStats[playerid][pWeaponSlotAmmo][weaponslot] = ammo;
						continue;
					}
				}
				cheatwpns[weaponslot] = weaponid;
				if (ammo == -1) {
					cheatammo[weaponslot] = -1;
				} else {
					cheatammo[weaponslot] = ammo - pStats[playerid][pWeaponSlotAmmo][weaponslot];
				}
				continue;
			}
		}
	}
	
	if (resetweapons) {
		ResetPlayerWeapons(playerid);
		pStats[playerid][pWeaponsLoaded] = 0;
		return 1;
	}
	
	new ischeatwpnheld;
	new cheatstring[MAX_INPUT];
	new ischangedfromlast;
	new islag = 1;
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		
		if (!cheatwpns[i]) {
			pStats[playerid][pWeaponSlotWpnLastCheat][i] = 0;
			pStats[playerid][pWeaponSlotAmmoLastCheat][i] = 0;
			continue;
		}
		
		if (cheatwpns[i] == 40 && pStats[playerid][pWeaponSlotWpn][8] == 39) { // Disables satchel charge cheat
			cheatwpns[i] = 0;
			cheatammo[i] = 0;
			continue;
		}
		
		if (!ischeatwpnheld) {
			if (pStats[playerid][pWeaponsDisabled]) {
				//if (GetPlayerWeapon(playerid) == cheatwpns[i] && (!pStats[playerid][pWeaponSlotAmmo][i] || pStats[playerid][pWeaponSlotWpn][i] != cheatwpns[i])) {
				if (GetPlayerWeapon(playerid) == cheatwpns[i]) {
					ischeatwpnheld = 1;
				}
			} else {
				//if (pStats[playerid][pWeaponLastWpn] == cheatwpns[i] && (!pStats[playerid][pWeaponSlotAmmo][i] || pStats[playerid][pWeaponSlotWpn][i] != cheatwpns[i])) {
				if (pStats[playerid][pWeaponLastWpn] == cheatwpns[i]) {
					ischeatwpnheld = 1;
				}
			}
		}
		
		if (islag) {
			if (pStats[playerid][pWeaponsDisabled]) {
				if (!cheatlag[i] && (pStats[playerid][pWeaponSlotWpn][i] == INVALID_WEAPON_ID)) {
					islag = 0;
				}
			} else {
				if (!cheatlag[i] && (cheatammo[i] + pStats[playerid][pWeaponSlotAmmo][i])) {
					islag = 0;
				}
			}
		}
	}
	
	if (!ischeatwpnheld) {
		return 1;
	}
	
	if (pStats[playerid][pWeaponsLastCheat] && GetTickCount() - pStats[playerid][pWeaponsLastCheat] < MAX_ALERT_INTERVAL) {
		return 1;
	}
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		if (!cheatwpns[i]) continue;
		
		//if (pStats[playerid][pWeaponSlotAmmo][i] && cheatwpns[i] == pStats[playerid][pWeaponSlotWpn][i]) continue; // Disables ammo counting
		if (cheatwpns[i] == pStats[playerid][pWeaponSlotWpn][i]) continue; // Disables ammo counting
		
		if (!ischangedfromlast && (
			pStats[playerid][pWeaponSlotWpnLastCheat][i] != cheatwpns[i] ||
			(
				!pStats[playerid][pWeaponSlotAmmoLastCheat][i] ||
				pStats[playerid][pWeaponSlotAmmoLastCheat][i] + 1 < cheatammo[i]
			)
			)) {
			
			ischangedfromlast = 1;
		}
		
		pStats[playerid][pWeaponSlotWpnLastCheat][i] = cheatwpns[i];
		pStats[playerid][pWeaponSlotAmmoLastCheat][i] = cheatammo[i];
		
		new fromammo;
		if (cheatwpns[i] == pStats[playerid][pWeaponSlotWpn][i]) {
			fromammo = pStats[playerid][pWeaponSlotAmmo][i];
		}
		
		if (!cheatstring[0]) {
			if (cheatammo[i] == -1) {
				format(cheatstring, sizeof(cheatstring), "%s", ReturnWeaponName(cheatwpns[i]));
			} else {
				if (fromammo) {
					format(cheatstring, sizeof(cheatstring), "%s(%i>%i)", ReturnWeaponName(cheatwpns[i]), fromammo, pStats[playerid][pWeaponSlotAmmo][i] + cheatammo[i]);
				} else {
					format(cheatstring, sizeof(cheatstring), "%s(%i)", ReturnWeaponName(cheatwpns[i]), cheatammo[i]);
				}
			}
		} else {
			if (cheatammo[i] == -1) {
				format(cheatstring, sizeof(cheatstring), "%s, %s", cheatstring, ReturnWeaponName(cheatwpns[i]));
			} else {
				if (fromammo) {
					format(cheatstring, sizeof(cheatstring), "%s, %s(%i>%i)", cheatstring, ReturnWeaponName(cheatwpns[i]), fromammo, pStats[playerid][pWeaponSlotAmmo][i] + cheatammo[i]);
				} else {
					format(cheatstring, sizeof(cheatstring), "%s, %s(%i)", cheatstring, ReturnWeaponName(cheatwpns[i]), cheatammo[i]);
				}
			}
		}
	}
	
	/*if (cheatstring[0] && ischangedfromlast) {
		GiveServerSideWeapons(playerid);
	}*/
	
	if (cheatstring[0] && (ischangedfromlast || GetMaxAdminLevel() < ADMIN_LEVEL_ANTICHEAT)) {
		format(cheatstring, sizeof(cheatstring), "Weapon: %s", cheatstring);
		
		if (islag) {
			CheatSuspect(playerid, cheatstring, CHEAT_FLAG_LAG);
		} else {
			CheatSuspect(playerid, cheatstring);
		}
		pStats[playerid][pWeaponsLastCheat] = GetTickCount();
	}
	
	return 1;
}

WeaponAcceptPlayerData(playerid) {
	if (pStats[playerid][pWeaponsDisabled]) return 1;
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		GetPlayerWeaponData(playerid, i, pStats[playerid][pWeaponSlotWpn][i], pStats[playerid][pWeaponSlotAmmo][i]);
		
		if (!pStats[playerid][pWeaponSlotWpn][i]) pStats[playerid][pWeaponSlotWpn][i] = INVALID_WEAPON_ID;
		if (!pStats[playerid][pWeaponSlotAmmo][i]) pStats[playerid][pWeaponSlotAmmo][i] = INVALID_WEAPON_ID;
		
		if (IsWeaponMelee(pStats[playerid][pWeaponSlotWpn][i])) {
			pStats[playerid][pWeaponSlotAmmo][i] = -1;
		} else if (pStats[playerid][pWeaponSlotAmmo][i] < 0) {
			pStats[playerid][pWeaponSlotAmmo][i] += 0x8000;
		}
		
		pStats[playerid][pWeaponSlotWpnLastCheat][i] = 0;
		pStats[playerid][pWeaponSlotAmmoLastCheat][i] = 0;
	}
	
	pStats[playerid][pWeaponsLastCheat] = 0;
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
	return 1;
}

IsPlayerNearWeapons(playerid) {
	if (!IsPlayerSpawned(playerid)) return 0;
	if (!GetPlayerInterior(playerid)) return 0;
	
	for (new i; i < sizeof(weaponLocations); i++) {
		if (IsPlayerInRangeOfPoint(playerid, 10.0, weaponLocations[i][fx], weaponLocations[i][fy], weaponLocations[i][fz])) return 1;
	}
	return 0;
}

IsAmmunationWeapon(weaponid) {
	for (new i; i < sizeof(ammunationWeapons); i++) {
		if (ammunationWeapons[i] == weaponid) return 1;
	}
	return 0;
}

stock CleanPlayerWeaponsArray(playerid) {
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		if (!pStats[playerid][pWeaponSlotAmmo][i]) {
			pStats[playerid][pLastWeaponSlotWpn][i] = pStats[playerid][pWeaponSlotWpn][i];
			pStats[playerid][pWeaponSlotWpn][i] = INVALID_WEAPON_ID;
		}
	}
	return 1;
}

stock GiveServerSideWeapons(playerid) {
	if (pStats[playerid][pWeaponsDisabled]) return 0;
	
	ResetPlayerWeapons(playerid);
	CleanPlayerWeaponsArray(playerid);
	
	pStats[playerid][pWeaponsLoaded] = 0;
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		if (!pStats[playerid][pWeaponSlotWpn][i]) continue;
		if (!pStats[playerid][pWeaponSlotAmmo][i]) continue;
		
		GivePlayerWeapon(playerid, pStats[playerid][pWeaponSlotWpn][i], pStats[playerid][pWeaponSlotAmmo][i]);
	}
	
	SetPlayerArmedWeapon(playerid, pStats[playerid][pWeaponLastWpn]);
	
	pStats[playerid][pVisitedAmmunation] = 0;
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
	return 1;
}

stock kGetPlayerWeapon(playerid) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	return pStats[playerid][pWeaponLastWpn];
}

stock kSetPlayerAmmo(playerid, weaponslot, ammo, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	if (pStats[playerid][pWeaponSlotWpn][weaponslot] == INVALID_WEAPON_ID) {
		return 0;
	} else {
		pStats[playerid][pWeaponSlotAmmo][weaponslot] = ammo;
		
		if (pStats[playerid][pWeaponsDisabled]) {
			if (!cancel) return 0;
			return 1;
		} else {
			pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
			if (!cancel) return SetPlayerAmmo(playerid, weaponslot, ammo);
			return 0;
		}
	}
}

stock kGetPlayerAmmo(playerid) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	return pStats[playerid][pWeaponSlotAmmo][GetWeaponSlot(GetPlayerWeapon(playerid))];
}

stock kGivePlayerWeapon(playerid, weaponid, ammo, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	new weaponslot = GetWeaponSlot(weaponid);
	
	if (weaponid != pStats[playerid][pWeaponSlotWpn][weaponslot]) {
		pStats[playerid][pLastWeaponSlotWpn][weaponslot] = pStats[playerid][pWeaponSlotWpn][weaponslot];
		pStats[playerid][pWeaponSlotWpn][weaponslot] = weaponid;
		if (!IsWeaponSlotShared(weaponslot)) pStats[playerid][pWeaponSlotAmmo][weaponslot] = 0;
	}
	if (IsWeaponMelee(weaponid)) {
		pStats[playerid][pWeaponSlotAmmo][weaponslot] = -1;
	} else {
		pStats[playerid][pWeaponSlotAmmo][weaponslot] = pStats[playerid][pWeaponSlotAmmo][weaponslot] + ammo;
	}
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
	
	if (pStats[playerid][pWeaponsDisabled]) {
		pStats[playerid][pWeaponLastWpn] = weaponid;
		return 1;
	}
	
	if (!cancel) return GivePlayerWeapon(playerid, weaponid, ammo);
	return 0;
}

stock kResetPlayerWeapons(playerid, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	for (new a; a < MAX_WEAPON_SLOTS; a++) {
		pStats[playerid][pLastWeaponSlotWpn][a] = pStats[playerid][pWeaponSlotWpn][a];
		pStats[playerid][pWeaponSlotWpn][a] = INVALID_WEAPON_ID;
		pStats[playerid][pWeaponSlotAmmo][a] = 0;
		pStats[playerid][pWeaponSlotWpnLastCheat][a] = 0;
		pStats[playerid][pWeaponSlotAmmoLastCheat][a] = 0;
	}
	
	pStats[playerid][pVisitedAmmunation] = 0;
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
	
	if (pStats[playerid][pWeaponsDisabled]) return 1;
	
	pStats[playerid][pWeaponsLoaded] = 0;
	
	if (!cancel) return ResetPlayerWeapons(playerid);
	return 0;
}

stock kSetPlayerArmedWeapon(playerid, weaponid, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	if (pStats[playerid][pWeaponsDisabled]) {
		if (!weaponid || pStats[playerid][pWeaponSlotWpn][GetWeaponSlot(weaponid)] != INVALID_WEAPON_ID) {
			pStats[playerid][pWeaponLastWpn] = weaponid;
			return 1;
		} else {
			return 0;
		}
	} else {
		if (!cancel) return SetPlayerArmedWeapon(playerid, weaponid);
		return 0;
	}
}

stock kGetPlayerWeaponData(playerid, slot, &weapons, &ammo) {
	if (!IsPlayerConnected(playerid) ||
		slot < 0 || slot >= MAX_WEAPON_SLOTS) {
		
		weapons = 0;
		ammo = 0;
		
		return 0;
	}
	
	if (pStats[playerid][pWeaponSlotWpn][slot] == INVALID_WEAPON_ID) {
		weapons = 0;
		ammo = 0;
	} else {
		weapons = pStats[playerid][pWeaponSlotWpn][slot];
		ammo = pStats[playerid][pWeaponSlotAmmo][slot];
	}
	
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// VEHICLES
//-----------------------------------------------------------------------------------------------------

new vehicleFixLocations[][locationInfo] = {
	// Mod shops
	{615.2842,-124.2390,997.6938}, // Wheel Arch Angels
	{617.5360,-1.9900,1000.6607}, // Transfender
	{616.7897,-74.8150,997.8911}, // Lowrider Modshop
	
	// Pay and Spray 
	{-1904.3903,284.4834,40.7517},
	{-2425.7927,1020.9296,50.1030},
	{-1420.6332,2585.4141,55.5480},
	{-99.8577,1117.5374,19.4465},
	{1975.0107,2162.5005,10.7770},
	{2393.8582,1491.2083,10.5272},
	{720.1977,-457.7859,16.1675},
	{1024.8751,-1023.7849,31.9332},
	{487.4547,-1740.9866,10.9642},
	{2065.2773,-1831.4730,13.3854}
};
new vehicleModShopCount = 3;

VehicleSync() {
	LoopPlayers(i) {
		anticheatCrashedPlayer = i;
		VehiclePlayerSync(i);
	}
}

VehiclePlayerSync(playerid) {
	if (!IsPlayerSpawned(playerid)) {
		if (pStats[playerid][pVehicleID]) {
			ac_OnPlayerStateChange(playerid, PLAYER_STATE_ONFOOT, PLAYER_STATE_DRIVER);
		}
		return 1;
	}
	
	if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) {
		if (pStats[playerid][pVehicleID]) {
			ac_OnPlayerStateChange(playerid, PLAYER_STATE_ONFOOT, PLAYER_STATE_DRIVER);
		}
		return 1;
	}
	
	new vehicleid = GetPlayerVehicleID(playerid);
	
	if (!vehicleid) {
		if (pStats[playerid][pVehicleID]) {
			ac_OnPlayerStateChange(playerid, PLAYER_STATE_ONFOOT, PLAYER_STATE_DRIVER);
		}
		return 1;
	}
	
	if (IsPlayerPaused(playerid, SYNC_DELAY)) {
		pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
		pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
		return 1;
	}
	
	if (pStats[playerid][pVehicleSyncing]) {
		pStats[playerid][pVehicleSyncing] = pStats[playerid][pVehicleSyncing] - anticheatSyncInterval;
		if (pStats[playerid][pVehicleSyncing] < 0) pStats[playerid][pVehicleSyncing] = 0;
	}
	
	if (pStats[playerid][pVehiclePartSyncing]) {
		pStats[playerid][pVehiclePartSyncing] = pStats[playerid][pVehiclePartSyncing] - anticheatSyncInterval;
		if (pStats[playerid][pVehiclePartSyncing] < 0) pStats[playerid][pVehiclePartSyncing] = 0;
	}
	
	if (!pStats[playerid][pUpdated]) return 1;
	
	if (!GetVehicleModel(vehicleid)) {
		return 1;
	}
	
	if (vehicleid != pStats[playerid][pVehicleID]) {
		if (pStats[playerid][pVehicleID]) {
			ac_OnPlayerStateChange(playerid, PLAYER_STATE_ONFOOT, PLAYER_STATE_DRIVER);
		}
		ac_OnPlayerStateChange(playerid, PLAYER_STATE_DRIVER, PLAYER_STATE_ONFOOT);
	}
	
	new panels, doors, lights, tyres, repaired, lagparts, tick = GetTickCount();	
	GetVehicleDamageStatus(vehicleid, panels, doors, lights, tyres);
	
	if (panels != vStats[vehicleid][vDamage][0] ||
		doors != vStats[vehicleid][vDamage][1] ||
		lights != vStats[vehicleid][vDamage][2] ||
		tyres != vStats[vehicleid][vDamage][3]) {
		
		// Check if the player repaired anything
		// Panels
		/*for (new i = 0xF; i != 0; i = i << 4) {
			if (!(panels & i) && vStats[vehicleid][vDamage][0] & i) {
				repaired = repaired | 1; // Panel repaired
				if (panels == vStats[vehicleid][vLastDamage][0]) lagparts = lagparts | 1;
				break;
			}
		}*/
		if (!panels && vStats[vehicleid][vDamage][0]) {
			repaired = repaired | 1; // Panel repaired
			if (panels == vStats[vehicleid][vLastDamage][0]) lagparts = lagparts | 1;
		}
		
		// Doors
		for (new i = 8; i <= 32; i += 8) {
			new checkBits = 0xFF << i;
			if (!(doors & checkBits) && (vStats[vehicleid][vDamage][1] & checkBits) && ((vStats[vehicleid][vDamage][1] & checkBits) >> i) != 1) {
				repaired = repaired | 2; // Doors repaired
				if (doors == vStats[vehicleid][vLastDamage][1]) lagparts = lagparts | 2;
				break;
			}
		}
		
		// Lights
		/*for (new i = 1; i != 0b10000; i = i << 1) {
			if (!(lights & i) && vStats[vehicleid][vDamage][2] & i) {
				repaired = repaired | 4; // Lights repaired
				if (lights == vStats[vehicleid][vLastDamage][2]) lagparts = lagparts | 4;
				break;
			}
		}*/
		if (!lights && vStats[vehicleid][vDamage][2]) {
			repaired = repaired | 4; // Lights repaired
			if (panels == vStats[vehicleid][vLastDamage][2]) lagparts = lagparts | 4;
		}
		
		// Tyres
		for (new i = 1; i != 0b10000; i = i << 1) {
			if (!(tyres & i) && vStats[vehicleid][vDamage][3] & i) {
				repaired = repaired | 8; // Tyres repaired
				if (tyres == vStats[vehicleid][vLastDamage][3]) lagparts = lagparts | 8;
				break;
			}
		}
		
		if (repaired) {
			if (IsPlayerNearVehicleFix(playerid)) {
				for (new i; i < 4; i++) vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
				vStats[vehicleid][vDamage][0] = panels;
				vStats[vehicleid][vDamage][1] = doors;
				vStats[vehicleid][vDamage][2] = lights;
				vStats[vehicleid][vDamage][3] = tyres;
				
			} else if (!pStats[playerid][pVehiclePartSyncing] && (!pStats[playerid][pVehiclePartLastCheat] || tick - pStats[playerid][pVehiclePartLastCheat] > MAX_ALERT_INTERVAL)) {
				new tmpstr[MAX_INPUT], lag = 1;
				tmpstr = "Vehicle: Repaired ";
				
				if (repaired & 1) {
					strcat(tmpstr, "Panels, ");
					if (!(lagparts & 1)) lag = 0;
				}
				if (repaired & 2) {
					strcat(tmpstr, "Doors, ");
					if (!(lagparts & 2)) lag = 0;
				}
				if (repaired & 4) {
					strcat(tmpstr, "Lights, ");
					if (!(lagparts & 4)) lag = 0;
				}
				if (repaired & 8) {
					strcat(tmpstr, "Tyres, ");
					if (!(lagparts & 8)) lag = 0;
				}
				tmpstr[strlen(tmpstr) - 2] = 0;
				
				if (lag) {
					CheatSuspect(playerid, tmpstr, CHEAT_FLAG_LAG);
				} else {
					CheatSuspect(playerid, tmpstr);
				}
				pStats[playerid][pVehiclePartLastCheat] = tick;
				if (!pStats[playerid][pDisableResist]) {
					UpdateVehicleDamageStatus(vehicleid, 0, 0, 0, 0);
					UpdateVehicleDamageStatus(vehicleid, vStats[vehicleid][vDamage][0], vStats[vehicleid][vDamage][1], vStats[vehicleid][vDamage][2], vStats[vehicleid][vDamage][3]);
				}
			}
		} else {
			if (pStats[playerid][pGodMode]) {
				UpdateVehicleDamageStatus(vehicleid, 0, 0, 0, 0);
				UpdateVehicleDamageStatus(vehicleid, vStats[vehicleid][vDamage][0], vStats[vehicleid][vDamage][1], vStats[vehicleid][vDamage][2], vStats[vehicleid][vDamage][3]);
				pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
				
			} else if (!pStats[playerid][pVehiclePartSyncing]) {
				for (new i; i < 4; i++) vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
				vStats[vehicleid][vDamage][0] = panels;
				vStats[vehicleid][vDamage][1] = doors;
				vStats[vehicleid][vDamage][2] = lights;
				vStats[vehicleid][vDamage][3] = tyres;
			}
		}
	} else {
		if (!pStats[playerid][pVehiclePartSyncing]) {
			for (new i; i < 4; i++) vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
		}
	}
	
	new Float:vehiclehp;
	GetVehicleHealth(vehicleid, vehiclehp);
	
	if (pStats[playerid][pGodMode]) {
		if (vehiclehp != GODMODE_VEHICLE_HP) SetVehicleHealth(vehicleid, GODMODE_VEHICLE_HP);
		return 1;
	}
	
	if (vehiclehp > vStats[vehicleid][vHealth] + 0.1) {
		if (IsPlayerNearVehicleFix(playerid)) {
			if (vehiclehp > 1000.0) {
				if (!pStats[playerid][pVehicleSyncing]) {
					if (!pStats[playerid][pDisableResist]) {
						SetVehicleHealth(vehicleid, vStats[vehicleid][vHealth]);
					}
					
					if (!pStats[playerid][pVehicleLastCheat] || tick - pStats[playerid][pVehicleLastCheat] > MAX_ALERT_INTERVAL) {
						new tmpstr[MAX_INPUT];
						format(tmpstr, sizeof(tmpstr), "Vehicle: Increased %0.1f > %0.1f HP after respray", vStats[vehicleid][vHealth], vehiclehp);
						CheatSuspect(playerid, tmpstr);
						pStats[playerid][pVehicleLastCheat] = tick;
					}
				}
			} else {
				vStats[vehicleid][vHealth] = vehiclehp;
				race_OnVehicleRepair(vehicleid);
				derby_OnVehicleRepair(vehicleid);
			}
		} else {
			if (!pStats[playerid][pVehicleSyncing]) {
				if (!pStats[playerid][pDisableResist]) {
					SetVehicleHealth(vehicleid, vStats[vehicleid][vHealth]);
				}
				
				if (!pStats[playerid][pVehicleWarning]) {
					pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
					pStats[playerid][pVehicleWarning] = 1;
					
				} else {
					if (!pStats[playerid][pVehicleLastCheat] || tick - pStats[playerid][pVehicleLastCheat] > MAX_ALERT_INTERVAL) {
						new tmpstr[MAX_INPUT];
						format(tmpstr, sizeof(tmpstr), "Vehicle: Increased %0.1f > %0.1f HP", vStats[vehicleid][vHealth], vehiclehp);
						
						if (vehiclehp <= pStats[playerid][pLastVehicleHP] + 0.1) {
							CheatSuspect(playerid, tmpstr, CHEAT_FLAG_LAG);
						} else {
							CheatSuspect(playerid, tmpstr);
						}
						
						pStats[playerid][pVehicleLastCheat] = tick;
					}
					
				}
			}
		}
		
	} else if (vehiclehp < vStats[vehicleid][vHealth] - 0.1) {
		if (!pStats[playerid][pVehicleSyncing]) {
			vStats[vehicleid][vHealth] = vehiclehp;
		}
		
	} else {
		if (!pStats[playerid][pVehicleSyncing] && (!pStats[playerid][pVehicleLastCheat] || tick - pStats[playerid][pVehicleLastCheat] > MAX_ALERT_INTERVAL)) {
			pStats[playerid][pLastVehicleHP] = vehiclehp;
		}
		
	}
	
	return 1;
}

VehicleAcceptPlayerData(playerid) {
	new vehicleid = GetPlayerVehicleID(playerid);
	if (vehicleid) {
		pStats[playerid][pVehicleID] = vehicleid;
		vStats[vehicleid][vDriver] = playerid;
		GetVehicleHealth(vehicleid, vStats[vehicleid][vHealth]);
		pStats[playerid][pLastVehicleHP] = vStats[vehicleid][vHealth];
		GetVehicleDamageStatus(vehicleid, vStats[vehicleid][vDamage][0], vStats[vehicleid][vDamage][1], vStats[vehicleid][vDamage][2], vStats[vehicleid][vDamage][3]);
	} else {
		pStats[playerid][pVehicleID] = 0;
	}
	return 1;
}

VehicleAcceptVehicleData(vehicleid) {
	GetVehicleHealth(vehicleid, vStats[vehicleid][vHealth]);
	if (vStats[vehicleid][vDriver] == INVALID_PLAYER_ID && vStats[vehicleid][vHealth] < 1000.0) {
		vStats[vehicleid][vHealth] = 1000.0;
	}
	return 1;
}

VehicleEnterPerformActions(playerid, vehicleid, ispassenger) {
	
	new vehiclemodel = GetVehicleModel(vehicleid);
	
	switch (vehiclemodel) {
		case 416: {
			// Ambulance - gain 20 hp when entering
			if (!ispassenger && pStats[playerid][pHealth] < 100.0) {
				pStats[playerid][pHealth] = pStats[playerid][pHealth] + 20.0;
				if (pStats[playerid][pHealth] > 100.0) pStats[playerid][pHealth] = 100.0;
				pStats[playerid][pHealthSyncing] = SYNC_DELAY;
			}
		}
		case 588: {
			// Hotdog - gain 20 hp when entering
			if (!ispassenger && pStats[playerid][pHealth] < 100.0) {
				pStats[playerid][pHealth] = pStats[playerid][pHealth] + 40.0;
				if (pStats[playerid][pHealth] > 100.0) pStats[playerid][pHealth] = 100.0;
				pStats[playerid][pHealthSyncing] = SYNC_DELAY;
			}
		}
		case 427: {
			// Enforcer - gain full armour when entering
			if (!ispassenger && pStats[playerid][pArmour] < 100.0) {
				pStats[playerid][pArmour] = 100.0;
				pStats[playerid][pArmourSyncing] = SYNC_DELAY;
			}
		}
		case 420,438: {
			// Taxi - gain $12 when getting in one
			if (!ispassenger) {
				pStats[playerid][pMoney] = pStats[playerid][pMoney] + 12;
				pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
			}
		}
		case 575: {
			// Pimp - gain $1000 when getting in one
			if (!ispassenger) {
				pStats[playerid][pMoney] = pStats[playerid][pMoney] + 1000;
				pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
			}
		}
		case 596,598,597,599: {
			// Police - gain 5 shotgun when exiting vehicle
			if (!ispassenger) {
				if (pStats[playerid][pWeaponSlotWpn][3] == INVALID_WEAPON_ID) {
					pStats[playerid][pWeaponSlotWpn][3] = WEAPON_SHOTGUN;
				}
				pStats[playerid][pWeaponSlotAmmo][3] = pStats[playerid][pWeaponSlotAmmo][3] + 5;
				pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
			}
		}
		case 592,577,511,512,593,520,553,476,519,460,513,548,425,417,487,488,497,563,447,469,464,465,501,539: {
			// Planes/Helicopters - can get a parachute at high altitudes
			if (pStats[playerid][pWeaponSlotWpn][11] != WEAPON_PARACHUTE) {
				if (!pStats[playerid][pWeaponSlotWpn][11]) {
					pStats[playerid][pLastWeaponSlotWpn][11] = pStats[playerid][pWeaponSlotWpn][11];
				}
				pStats[playerid][pWeaponSlotWpn][11] = WEAPON_PARACHUTE;
				pStats[playerid][pWeaponSlotAmmo][11] = 0;
				pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
			}
		}
		case 457: {
			// Caddy - gain a golf club
			if (!ispassenger && pStats[playerid][pWeaponSlotWpn][1] == INVALID_WEAPON_ID) {
				pStats[playerid][pWeaponSlotWpn][1] = WEAPON_GOLFCLUB;
				pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
				pStats[playerid][pWeaponSlotAmmo][1] = -1;
			}
		}
	}
	
	pStats[playerid][pVehicleWarning] = 0;
	
	return 1;
}

VehicleExitPerformActions(playerid, vehicleid) {
	
	new vehiclemodel = GetVehicleModel(vehicleid);
	
	switch (vehiclemodel) {
		case 592,577,511,512,593,520,553,476,519,460,513,548,425,417,487,488,497,563,447,469,464,465,501,539: {
			// Planes/Helicopters - can get a parachute at high altitudes
			if (pStats[playerid][pWeaponSlotWpn][11] != WEAPON_PARACHUTE) {
				if (!pStats[playerid][pWeaponSlotWpn][11]) {
					pStats[playerid][pLastWeaponSlotWpn][11] = pStats[playerid][pWeaponSlotWpn][11];
				}
				pStats[playerid][pWeaponSlotWpn][11] = WEAPON_PARACHUTE;
				pStats[playerid][pWeaponSlotAmmo][11] = 0;
				pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
			}
		}
	}
	
	return 1;
}

IsPlayerNearVehicleFix(playerid, onlymodshop = 0) {
	if (!IsPlayerSpawned(playerid)) return 0;
	
	new loopCount = (onlymodshop ? vehicleModShopCount : sizeof(vehicleFixLocations));
	
	for (new i; i < loopCount; i++) {
		if (IsPlayerInRangeOfPoint(playerid, 10.0, vehicleFixLocations[i][fx], vehicleFixLocations[i][fy], vehicleFixLocations[i][fz])) return 1;
	}
	return 0;
}

stock kRepairVehicle(vehicleid, cancel = 0) {
	if (!GetVehicleModel(vehicleid)) return 0;
	new rtn = 1;
	
	if (vStats[vehicleid][vDriver] != INVALID_PLAYER_ID) {
		new playerid = vStats[vehicleid][vDriver];
		
		vStats[vehicleid][vHealth] = 1000.0;
		pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
		pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
		
		if (!pStats[playerid][pGodMode]) {
			if (1000.0 > pStats[playerid][pLastVehicleHP]) {
				pStats[playerid][pLastVehicleHP] = 1000.0;
			}
			
			for (new i; i < 4; i++) {
				vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
				vStats[vehicleid][vDamage][i] = 0;
			}
			if (!cancel) RepairVehicle(vehicleid); else rtn = 0;
			
		} else {
			for (new i; i < 4; i++) {
				vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
				vStats[vehicleid][vDamage][i] = 0;
			}
			if (!cancel) UpdateVehicleDamageStatus(vehicleid, 0, 0, 0, 0); else rtn = 0;
			
		}
		
	} else {
		vStats[vehicleid][vHealth] = 1000.0;
		for (new i; i < 4; i++) {
			vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
			vStats[vehicleid][vDamage][i] = 0;
		}
		if (!cancel) RepairVehicle(vehicleid); else rtn = 0;
		
	}
	
	return rtn;
}

kUpdateVehicleDamageStatus(vehicleid, panels, doors, lights, tyres, cancel = 0) {
	if (!GetVehicleModel(vehicleid)) return 0;
	
	if (vStats[vehicleid][vDriver] != INVALID_PLAYER_ID) {
		pStats[vStats[vehicleid][vDriver]][pVehiclePartSyncing] = SYNC_DELAY;
	}
	
	for (new i; i < 4; i++) {
		vStats[vehicleid][vLastDamage][i] = vStats[vehicleid][vDamage][i];
	}
	vStats[vehicleid][vDamage][0] = panels;
	vStats[vehicleid][vDamage][1] = doors;
	vStats[vehicleid][vDamage][2] = lights;
	vStats[vehicleid][vDamage][3] = tyres;
	
	UpdateVehicleDamageStatus(vehicleid, panels, doors, 0, tyres);
	if (!cancel) return UpdateVehicleDamageStatus(vehicleid, panels, doors, lights, tyres);
	return 0;
}

stock kSetVehicleHealth(vehicleid, Float:health, cancel = 0) {
	if (!GetVehicleModel(vehicleid)) return 0;
	new rtn = 1;
	
	if (vStats[vehicleid][vDriver] != INVALID_PLAYER_ID) {
		new playerid = vStats[vehicleid][vDriver];
		
		vStats[vehicleid][vHealth] = health;
		pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
		
		if (!pStats[playerid][pGodMode]) {
			if (health > pStats[playerid][pLastVehicleHP]) {
				pStats[playerid][pLastVehicleHP] = health;
			}
			
			if (!cancel) SetVehicleHealth(vehicleid, health); else rtn = 0;
		}
		
	} else {
		
		vStats[vehicleid][vHealth] = health;
		if (!cancel) SetVehicleHealth(vehicleid, health); else rtn = 0;
	}
	
	return rtn;
}

stock kGetVehicleHealth(vehicleid, &Float:health) {
	if (!GetVehicleModel(vehicleid)) return 0;
	
	if (vStats[vehicleid][vDriver] == INVALID_PLAYER_ID) {
		health = vStats[vehicleid][vHealth];
	} else {
		if (pStats[vStats[vehicleid][vDriver]][pGodMode]) {
			health = vStats[vehicleid][vHealth];
		} else {
			GetVehicleHealth(vehicleid, health);
			if (health > vStats[vehicleid][vHealth]) {
				health = vStats[vehicleid][vHealth];
			}
		}
	}
	return 1;
}

stock kDestroyVehicle(vehicleid, cancel = 0) {
	if (!GetVehicleModel(vehicleid)) return 0;
	
	new playerid = vStats[vehicleid][vDriver];
	
	if (playerid != INVALID_PLAYER_ID) {
		ac_OnPlayerStateChange(playerid, PLAYER_STATE_ONFOOT, PLAYER_STATE_DRIVER);
		
		vStats[vehicleid][vDriver] = INVALID_PLAYER_ID;
		pStats[playerid][pVehicleID] = 0;
		pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
		pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
	}
	
	vStats[vehicleid][vHealth] = 1000.0;
	for (new i; i < 4; i++) {
		vStats[vehicleid][vLastDamage][i] = 0;
		vStats[vehicleid][vDamage][i] = 0;
	}
	
	race_OnVehicleDestroyed(vehicleid);
	if (!cancel) return DestroyVehicle(vehicleid);
	return 0;
}

//------------------------------------------------------------------------------------------------------
// JETPACKS
//------------------------------------------------------------------------------------------------------

JetpackSync() {
	LoopPlayers(i) {
		if (IsPlayerNPC(i)) continue;
		anticheatCrashedPlayer = i;
		JetpackPlayerSync(i);
	}
}

JetpackPlayerSync(playerid) {
	
	if (IsPlayerPaused(playerid, SYNC_DELAY)) {
		pStats[playerid][pJetpackSyncing] = SYNC_DELAY;
		return 1;
	}
	
	if (pStats[playerid][pJetpackSyncing]) {
		pStats[playerid][pJetpackSyncing] = pStats[playerid][pJetpackSyncing] - anticheatSyncInterval;
		if (pStats[playerid][pJetpackSyncing] < 0) pStats[playerid][pJetpackSyncing] = 0;
		return 1;
	}
	
	new playeractionid = GetPlayerSpecialAction(playerid);
	
	if (playeractionid == SPECIAL_ACTION_USEJETPACK) {
		if (pStats[playerid][pJetpack]) return 1;
		
		// Remove jetpack
		ClearAnimations(playerid);
		
		if (!pStats[playerid][pJetpackLastCheat] || GetTickCount() - pStats[playerid][pJetpackLastCheat] > MAX_ALERT_INTERVAL) {
			CheatSuspect(playerid, "Jetpack Hack");
			pStats[playerid][pJetpackLastCheat] = GetTickCount();
		}
		return 1;
	}
	return 1;
}

JetpackAcceptPlayerData(playerid) {
	if (GetPlayerSpecialAction(playerid) == SPECIAL_ACTION_USEJETPACK) {
		pStats[playerid][pJetpack] = 1;
	} else {
		pStats[playerid][pJetpack] = 0;
	}
	pStats[playerid][pJetpackLastCheat] = 0;
	pStats[playerid][pJetpackSyncing] = SYNC_DELAY;
	return 1;
}

stock kSetPlayerSpecialAction(playerid, actionid, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	switch (actionid) {
		case SPECIAL_ACTION_USEJETPACK: {
			pStats[playerid][pJetpack] = 1;
			pStats[playerid][pJetpackSyncing] = SYNC_DELAY;
		}
	}
	if (!cancel) return SetPlayerSpecialAction(playerid, actionid);
	return 0;
}

//------------------------------------------------------------------------------------------------------
// SPECTATE
//------------------------------------------------------------------------------------------------------

SpectateSync() {
	LoopPlayers(i) {
		if (IsPlayerNPC(i)) continue;
		anticheatCrashedPlayer = i;
		SpectatePlayerSync(i);
	}
}

SpectatePlayerSync(playerid) {
	
	if (pStats[playerid][pSpectateSyncing]) {
		pStats[playerid][pSpectateSyncing] = pStats[playerid][pSpectateSyncing] - anticheatSyncInterval;
		if (pStats[playerid][pSpectateSyncing] < 0) pStats[playerid][pSpectateSyncing] = 0;
		return 1;
	}
	
	if (pStats[playerid][pSpectating] == 2) {
		if (IsPlayerSpawned(playerid) && !IsPlayerPaused(playerid, 5000)) {
			pStats[playerid][pSpectating] = 0;
		}
		return 1;
	}
	
	new playerstate = GetPlayerState(playerid);
	
	if (playerstate == PLAYER_STATE_SPECTATING) {
		if (pStats[playerid][pSpectating]) return 1;
		
		// Stop them spectating
		/*new Float:px, Float:py, Float:pz, Float:prot;
		GetPlayerPos(playerid, px, py, pz);
		GetPlayerFacingAngle(playerid, prot);
		SkinsSetNextSpawnPos(playerid, px, py, pz, prot);*/
		TogglePlayerSpectating(playerid, 0);
		SpawnPlayer(playerid);
		
		if (!pStats[playerid][pSpectateLastCheat] || GetTickCount() - pStats[playerid][pSpectateLastCheat] > MAX_ALERT_INTERVAL) {
			CheatSuspect(playerid, "Spectate Hack");
			pStats[playerid][pSpectateLastCheat] = GetTickCount();
		}
	}
	
	return 1;
}

SpectateAcceptPlayerData(playerid) {
	if (GetPlayerState(playerid) == PLAYER_STATE_SPECTATING) {
		pStats[playerid][pSpectating] = 1;
	} else {
		pStats[playerid][pSpectating] = 0;
	}
	pStats[playerid][pSpectateLastCheat] = 0;
	pStats[playerid][pSpectateSyncing] = SYNC_DELAY;
	return 1;
}

stock kTogglePlayerSpectating(playerid, toggle, cancel = 0) {
	if (!IsPlayerConnected(playerid)) return 0;
	
	if (toggle) {
		pStats[playerid][pSpectating] = 1;
		pStats[playerid][pSpectateSyncing] = SYNC_DELAY;
	} else {
		if (pStats[playerid][pSpectating]) {
			pStats[playerid][pSpectating] = 2;
			pStats[playerid][pSpectateSyncing] = SYNC_DELAY;
		}
	}
	
	if (!cancel) return TogglePlayerSpectating(playerid, toggle);
	return 0;
}

//------------------------------------------------------------------------------------------------------
// NEW FEATURES
//------------------------------------------------------------------------------------------------------

stock kGetVehicleDriver(vehicleid) {
	if (vStats[vehicleid][vDriver] == INVALID_PLAYER_ID) {
		return INVALID_PLAYER_ID;
	} else {
		if (GetPlayerState(vStats[vehicleid][vDriver]) != PLAYER_STATE_DRIVER ||
			GetPlayerVehicleID(vStats[vehicleid][vDriver]) != vehicleid) {
			
			return INVALID_PLAYER_ID;
		} else {
			return vStats[vehicleid][vDriver];
		}
	}
}

//------------------------------------------------------------------------------------------------------
// GODMODE FUNCTIONS
//------------------------------------------------------------------------------------------------------

stock kTogglePlayerGodmode(playerid, toggle) {
	if (!IsPlayerConnected(playerid)) return 0;
	if (toggle) {
		if (pStats[playerid][pGodMode]) return 1;
		pStats[playerid][pGodMode] = 1;
		SetPlayerHealth(playerid, GODMODE_HP);
		SetPlayerArmour(playerid, GODMODE_ARMOUR);
		
		if (pStats[playerid][pVehicleID]) {
			SetVehicleHealth(pStats[playerid][pVehicleID], GODMODE_VEHICLE_HP);
		}
		
	} else {
		if (!pStats[playerid][pGodMode]) return 1;
		pStats[playerid][pGodMode] = 0;
		pStats[playerid][pHealthSyncing] = SYNC_DELAY;
		pStats[playerid][pArmourSyncing] = SYNC_DELAY;
		pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
		pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
		
		ac_SetPlayerHealth(playerid, pStats[playerid][pHealth]);
		SetPlayerArmour(playerid, pStats[playerid][pArmour]);
		
		if (pStats[playerid][pHealth] > GODMODE_HP) {
			pStats[playerid][pLastHealth] = pStats[playerid][pHealth];
		} else {
			pStats[playerid][pLastHealth] = GODMODE_HP;
		}
		
		if (pStats[playerid][pArmour] > GODMODE_ARMOUR) {
			pStats[playerid][pLastArmour] = pStats[playerid][pArmour];
		} else {
			pStats[playerid][pLastArmour] = GODMODE_ARMOUR;
		}
		
		new vehiclereset;
		if (pStats[playerid][pVehicleID]) {
			if (vStats[pStats[playerid][pVehicleID]][vHealth] > GODMODE_VEHICLE_HP) {
				pStats[playerid][pLastVehicleHP] = vStats[pStats[playerid][pVehicleID]][vHealth];
			} else {
				pStats[playerid][pLastVehicleHP] = GODMODE_VEHICLE_HP;
			}
			if (GetVehicleModel(pStats[playerid][pVehicleID])) {
				SetVehicleHealth(pStats[playerid][pVehicleID], vStats[pStats[playerid][pVehicleID]][vHealth]);
			}
			vehiclereset = 1;
		}
		
		SetTimerEx("fdm_ResetPlayerStats", SYNC_DELAY / 6, 0, "iiii",
			playerid, 1, 1, vehiclereset);
		SetTimerEx("fdm_ResetPlayerStats", SYNC_DELAY / 3, 0, "iiii",
			playerid, 1, 1, vehiclereset);
		SetTimerEx("fdm_ResetPlayerStats", SYNC_DELAY / 2, 0, "iiii",
			playerid, 1, 1, vehiclereset);
	}
	return 1;
}

stock kIsPlayerOnGodmode(playerid) {
	return pStats[playerid][pGodMode];
}

forward fdm_ResetPlayerStats(playerid, health, armour, vehicle);
public fdm_ResetPlayerStats(playerid, health, armour, vehicle) {
	if (!IsPlayerConnected(playerid)) return 0;
	if (pStats[playerid][pGodMode]) return 0;
	if (!IsPlayerSpawned(playerid)) return 0;
	
	new Float:tmp;
	if (health) {
		ac_GetPlayerHealth(playerid, tmp);
		if (pStats[playerid][pHealth] < tmp - 0.1) ac_SetPlayerHealth(playerid, pStats[playerid][pHealth]);
	}
	if (armour) {
		GetPlayerArmour(playerid, tmp);
		if (pStats[playerid][pArmour] < tmp - 0.1) SetPlayerArmour(playerid, pStats[playerid][pArmour]);
	}
	if (vehicle) {
		if (pStats[playerid][pVehicleID] && GetVehicleModel(pStats[playerid][pVehicleID])) {
			GetVehicleHealth(pStats[playerid][pVehicleID], tmp);
			
			if (vStats[pStats[playerid][pVehicleID]][vHealth] < tmp - 0.1) {
				SetVehicleHealth(pStats[playerid][pVehicleID], vStats[pStats[playerid][pVehicleID]][vHealth]);
			}
		}
	}
	return 1;
}

//------------------------------------------------------------------------------------------------------
// WEAPON DISABLING FUNCTIONS
//------------------------------------------------------------------------------------------------------

stock kDisablePlayerWeapons(playerid) {
	if (!IsPlayerConnected(playerid)) return 0;
	if (pStats[playerid][pWeaponsDisabled]) return 1;
	
	ResetPlayerWeapons(playerid);
	CleanPlayerWeaponsArray(playerid);
	
	pStats[playerid][pWeaponsLoaded] = 0;
	pStats[playerid][pWeaponsDisabled] = 1;
	pStats[playerid][pVisitedAmmunation] = 0;
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
	return 1;
}

stock kEnablePlayerWeapons(playerid) {
	if (!IsPlayerConnected(playerid)) return 0;
	if (!pStats[playerid][pWeaponsDisabled]) return 1;
	
	ResetPlayerWeapons(playerid);
	CleanPlayerWeaponsArray(playerid);
	
	for (new i; i < MAX_WEAPON_SLOTS; i++) {
		if (pStats[playerid][pWeaponSlotWpn][i] != INVALID_WEAPON_ID && pStats[playerid][pWeaponSlotAmmo][i]) {
			GivePlayerWeapon(playerid, pStats[playerid][pWeaponSlotWpn][i], pStats[playerid][pWeaponSlotAmmo][i]);
		}
	}
	
	if (pStats[playerid][pWeaponLastWpn] != INVALID_WEAPON_ID) {
		SetPlayerArmedWeapon(playerid, pStats[playerid][pWeaponLastWpn]);
	}
	
	pStats[playerid][pWeaponsLoaded] = 0;
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
	pStats[playerid][pWeaponsDisabled] = 0;
	pStats[playerid][pVisitedAmmunation] = 0;
	return 1;
}

stock kIsPlayerWeaponsDisabled(playerid) {
	return pStats[playerid][pWeaponsDisabled];
}

//------------------------------------------------------------------------------------------------------
// MISC FUNCTIONS
//------------------------------------------------------------------------------------------------------

stock kStartSyncDelay(playerid) {
	pStats[playerid][pArmourSyncing] = SYNC_DELAY;
	pStats[playerid][pMoneySyncing] = MONEY_SYNC_DELAY;
	pStats[playerid][pVehicleSyncing] = SYNC_DELAY;
	pStats[playerid][pVehiclePartSyncing] = SYNC_DELAY;
	pStats[playerid][pJetpackSyncing] = SYNC_DELAY;
	pStats[playerid][pSpectateSyncing] = SYNC_DELAY;
	pStats[playerid][pWeaponSyncing] = SYNC_DELAY;
	return 1;
}

//-----------------------------------------------------------------------------------------------------
// FUNCTIONS REPLACER
//-----------------------------------------------------------------------------------------------------

#if defined _ALS_SetPlayerHealth
	#undef SetPlayerHealth
#else
	#define _ALS_SetPlayerHealth
#endif
#define SetPlayerHealth kSetPlayerHealth

#if defined _ALS_GetPlayerHealth
	#undef GetPlayerHealth
#else
	#define _ALS_GetPlayerHealth
#endif
#define GetPlayerHealth kGetPlayerHealth

#if defined _ALS_SetPlayerArmour
	#undef SetPlayerArmour
#else
	#define _ALS_SetPlayerArmour
#endif
#define SetPlayerArmour kSetPlayerArmour

#if defined _ALS_GetPlayerArmour
	#undef GetPlayerArmour
#else
	#define _ALS_GetPlayerArmour
#endif
#define GetPlayerArmour kGetPlayerArmour


#if defined _ALS_ResetPlayerMoney
	#undef ResetPlayerMoney
#else
	#define _ALS_ResetPlayerMoney
#endif
#define ResetPlayerMoney kResetPlayerMoney

#if defined _ALS_GivePlayerMoney
	#undef GivePlayerMoney
#else
	#define _ALS_GivePlayerMoney
#endif
#define GivePlayerMoney kGivePlayerMoney

#if defined _ALS_GetPlayerMoney
	#undef GetPlayerMoney
#else
	#define _ALS_GetPlayerMoney
#endif
#define GetPlayerMoney kGetPlayerMoney


#if defined _ALS_GetPlayerWeapon
	#undef GetPlayerWeapon
#else
	#define _ALS_GetPlayerWeapon
#endif
#define GetPlayerWeapon kGetPlayerWeapon

#if defined _ALS_SetPlayerAmmo
	#undef SetPlayerAmmo
#else
	#define _ALS_SetPlayerAmmo
#endif
#define SetPlayerAmmo kSetPlayerAmmo


#if defined _ALS_GetPlayerAmmo
	#undef GetPlayerAmmo
#else
	#define _ALS_GetPlayerAmmo
#endif
#define GetPlayerAmmo kGetPlayerAmmo

#if defined _ALS_GivePlayerWeapon
	#undef GivePlayerWeapon
#else
	#define _ALS_GivePlayerWeapon
#endif
#define GivePlayerWeapon kGivePlayerWeapon

#if defined _ALS_ResetPlayerWeapons
	#undef ResetPlayerWeapons
#else
	#define _ALS_ResetPlayerWeapons
#endif
#define ResetPlayerWeapons kResetPlayerWeapons

#if defined _ALS_GetPlayerWeaponData
	#undef GetPlayerWeaponData
#else
	#define _ALS_GetPlayerWeaponData
#endif
#define GetPlayerWeaponData kGetPlayerWeaponData

#if defined _ALS_SetPlayerArmedWeapon
	#undef SetPlayerArmedWeapon
#else
	#define _ALS_SetPlayerArmedWeapon
#endif
#define SetPlayerArmedWeapon kSetPlayerArmedWeapon


#if defined _ALS_SetVehicleHealth
	#undef SetVehicleHealth
#else
	#define _ALS_SetVehicleHealth
#endif
#define SetVehicleHealth kSetVehicleHealth

#if defined _ALS_GetVehicleHealth
	#undef GetVehicleHealth
#else
	#define _ALS_GetVehicleHealth
#endif
#define GetVehicleHealth kGetVehicleHealth

#if defined _ALS_RepairVehicle
	#undef RepairVehicle
#else
	#define _ALS_RepairVehicle
#endif
#define RepairVehicle kRepairVehicle

#if defined _ALS_UpdateVehicleDamageStatus
	#undef UpdateVehicleDamageStatus
#else
	#define _ALS_UpdateVehicleDamageStatus
#endif
#define UpdateVehicleDamageStatus kUpdateVehicleDamageStatus

#if defined _ALS_DestroyVehicle
	#undef DestroyVehicle
#else
	#define _ALS_DestroyVehicle
#endif
#define DestroyVehicle kDestroyVehicle


#if defined _ALS_SetPlayerSpecialAction
	#undef SetPlayerSpecialAction
#else
	#define _ALS_SetPlayerSpecialAction
#endif
#define SetPlayerSpecialAction kSetPlayerSpecialAction

#if defined _ALS_TogglePlayerSpectating
	#undef TogglePlayerSpectating
#else
	#define _ALS_TogglePlayerSpectating
#endif
#define TogglePlayerSpectating kTogglePlayerSpectating
