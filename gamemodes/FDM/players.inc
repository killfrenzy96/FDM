#include <a_samp>

#define FILE_PLAYERS "FDM/Players/player_"
#if !defined MAX_WEAPON_ID
	#define MAX_WEAPON_ID 47
#endif

#define COLOUR_LOGIN 0xFFDD00AA
#define COLOUR_LOGOUT 0xFFDD00AA

#define LOGIN_NORMAL 0
#define LOGIN_AUTO 1
#define LOGIN_REGISTER 2
#define LOGIN_FORCED 3

#define JOINSTYLE_ALL 0
#define JOINSTYLE_LOGINS 1
#define JOINSTYLE_NONE 2

#define MIN_LASTLOGIN_DELETE_TIME 31104000 // 1 year before login can be overridden (Player account expiration time)

#define MAX_PASS_LEN 48

#define KEY_PASSWORD "Password"
#define KEY_LASTLOGIN "LastLogin"
#define KEY_LASTIP "LastIP"
#define KEY_CASH "Cash"
#define KEY_BANKCASH "BankCash"
#define KEY_XP "XP"
#define KEY_GANG "Gang"
#define KEY_KILLS "Kills"
#define KEY_DEATHS "Deaths"
#define KEY_SUICIDES "Suicides"
#define KEY_SKIN "Skin"
#define KEY_SPECIALTY "Specialty"
#define KEY_WEAPONS "Weapons"
#define KEY_ACTIVEWEAPONS "ActiveWeapons"
#define KEY_X "PosX"
#define KEY_Y "PosY"
#define KEY_Z "PosZ"
#define KEY_ROT "RotZ"
#define KEY_SIDECHAT "SideChat"
#define KEY_SIDECHATLINES "SideChatLines"
#define KEY_SIDECHATTIME "SideChatTime"
#define KEY_LOGINMSG "LoginMsg"
#define KEY_BOUNTY "Bounty"
#define KEY_JOINSTYLE "JoinStyle"
#define KEY_DISABLEAUTOLOGIN "DisableAutoLogin"

enum playerDataInfo {
	pFirstConnect,
	pLastHash,
	
	pIsRegistered,
	pIsLoggedIn,
	pLoginName[MAX_NAME],
	pPassword[MAX_HASH],
	pLastLoginTime,
	
	pBankCash,
	pLevel,
	pXP,
	pGang[MAX_GANG_NAME],
	pWeapons[MAX_WEAPON_ID],
	pActiveWeapons[MAX_WEAPON_ID],
	pKills,
	pDeaths,
	pSuicides,
	pSkin,
	pSpecialty,
	
	pSideChat,
	pSideChatLines,
	pSideChatTime,
	
	pDisableAutoLogin,
	pPasswordReset,
	pLoginMsg[MAX_LOGIN_MSG],
	pJoinStyle,
	pBounty
}
new pData[MAX_PLAYERS][playerDataInfo];

PlayersInit() {
	// Start autosaving timer
	kEnableTimer(15); // SetTimer("fdm_playersSave", 1500, true);
	return 1;
}

players_OnPlayerRequestClass(playerid, gamemodestartup = 0) {
	if (!pData[playerid][pFirstConnect]) {
		ResetPlayerAccountData(playerid);
		pData[playerid][pFirstConnect] = 1;
		
		new filename[MAX_FILENAME];
		strcpy(filename, GetPlayerIDFilePath(playerid));
		
		// Perform auto login
		if (!INI_Exist(filename)) { // Acount must exist
			SendClientMessage(playerid, COLOUR_HELP, "You must register (/register [password]) to have your stats saved.");
			SendClientMessage(playerid, COLOUR_HELP, "Registering also gives you basic starting spawn weapons and some money.");
			return 1;
		}
		
		new playerip[MAX_IP]; GetPlayerIp(playerid, playerip, sizeof(playerip));
		new fileplayerip[MAX_IP];
		
		if (INI_Exist(filename)) {
			pData[playerid][pIsRegistered] = 1;
			
			if (INI_Open(filename)) {
				if (INI_ReadString(fileplayerip, KEY_LASTIP)) {
					if (!gamemodestartup && INI_ReadInt(KEY_DISABLEAUTOLOGIN)) { // Autologin is disabled
						INI_Close();
						SendClientMessage(playerid, COLOUR_HELP, "Autologin is disabled for this account. Use /login [password] to login.");
						return 1;
					}
					
					if (!strcmp(fileplayerip, playerip)) {
						INI_Close();
						if (LoadPlayerAccount(playerid)) {
							SendClientMessage(playerid, COLOUR_SUCCESSFUL, "AUTOLOGIN SUCCESSFUL: You are logged in.");
							if (!gamemodestartup) SendLoginMessageToAll(playerid, LOGIN_AUTO);
						} else {
							SendClientMessage(playerid, COLOUR_ERROR, "AUTOLOGIN WARNING: Failed to load existing account.");
						}
						return 1;
					}
				}
			}
			
			pData[playerid][pLastLoginTime] = INI_ReadInt(KEY_LASTLOGIN);
			INI_ReadString(pData[playerid][pPassword], KEY_PASSWORD, MAX_PASS_LEN);
			
			// Player must use /login
			//if (gettime() - pData[playerid][pLastLoginTime] >  MIN_LASTLOGIN_DELETE_TIME) {
			SendClientMessage(playerid, COLOUR_HELP, "An account exists on this name. Use /login [password] to login.");
			//} else {
			//	SendClientMessage(playerid, COLOUR_HELP, "An account exists on this name but has expired.");
			//	SendClientMessage(playerid, COLOUR_HELP, "Use /login [password] to login, or /register [password] to create a new account.");
			//}
			
			INI_Close();
		}
		return 1;
	}
	return 0;
}

players_OnPlayerConnect(playerid, gamemodestartup = 0) {
	ResetPlayerAccountData(playerid);
	
	if (gamemodestartup) {
		players_OnPlayerRequestClass(playerid, gamemodestartup);
	}
	
	return 1;
}

players_OnPlayerDisconnect(playerid, reason) {
	if (pData[playerid][pIsLoggedIn]) {
		new msg[MAX_INPUT];
		
		switch (reason) {
			case -1: {}
			case 0: {
				format(msg, sizeof(msg), "*** %s(%i) has left the server (Timeout)", pData[playerid][pLoginName], playerid);
			}
			case 1: {
				format(msg, sizeof(msg), "*** %s(%i) has left the server (Left)", pData[playerid][pLoginName], playerid);
			}
			case 2: {
				//format(msg, sizeof(msg), "*** %s(%i) has left the server (Kicked)", pData[playerid][pLoginName], playerid);
			}
		}
		if (msg[0]) {
			//SendClientMessageToAll(COLOUR_LOGOUT, msg);
			LoopPlayers(i) {
				if (!pData[i][pIsLoggedIn] || pData[i][pJoinStyle] == JOINSTYLE_ALL) {
					SendClientMessage(i, COLOUR_LOGOUT, msg);
				}
			}
		}
		
		SavePlayerAccount(playerid);
	}
	ResetPlayerAccountData(playerid);
	return 1;
}

players_OnPlayerDeath(playerid, killerid) {
	if (killerid == INVALID_PLAYER_ID) {
		// Player suicided
		pData[playerid][pSuicides]++;
	} else {
		// Player got kill
		pData[killerid][pKills]++;
		pData[playerid][pDeaths]++;
	}
	return 1;
}

new psaveid;
forward fdm_playersSave();
public fdm_playersSave() { // This will save one player account
	
	new maxplayers = GetMaxPlayers();
	if (psaveid >= MAX_PLAYERS) psaveid = 0;
	
	for (new i; i < (maxplayers); i++) {
		psaveid = psaveid + 1;
		if (psaveid >= maxplayers) psaveid = 0;
		
		if (!IsPlayerConnected(psaveid)) continue;
		if (!pData[psaveid][pIsLoggedIn]) continue;
		
		if (pData[psaveid][pLastHash] != GetAccountHash(psaveid)) {
			SavePlayerAccount(psaveid);
		}
		break;
	}
	
	return 1;
}

SendLoginMessageToAll(playerid, logintype = 0) {
	if (!pData[playerid][pIsLoggedIn]) return 0;
	
	new msg[MAX_INPUT];
	
	switch (logintype) {
		case LOGIN_NORMAL, LOGIN_FORCED: format(msg, sizeof(msg), "*** %s(%i) has logged in", pData[playerid][pLoginName], playerid);
		case LOGIN_AUTO: format(msg, sizeof(msg), "*** %s(%i) has auto-logged in", pData[playerid][pLoginName], playerid);
		case LOGIN_REGISTER: format(msg, sizeof(msg), "*** %s(%i) has registered", pData[playerid][pLoginName], playerid);
	}
	
	// Countries
	new countryCode[4];
	new countryName[48];
	
	if (GetPlayerCountryCode(playerid, countryCode)) {
		GetCountryFromCode(countryCode, countryName);
		format(msg, sizeof(msg), "%s | Country: %s", msg, countryName);
	}
	
	if (pData[playerid][pBounty]) format(msg, sizeof(msg), "%s | Bounty: $%s", msg, kvalstr(pData[playerid][pBounty]));
	if (pData[playerid][pLoginMsg][0]) format(msg, sizeof(msg), "%s | %s", msg, pData[playerid][pLoginMsg]);
	
	LoopPlayers(i) {
		if (!pData[i][pIsLoggedIn] || pData[i][pJoinStyle] == JOINSTYLE_ALL || pData[i][pJoinStyle] == JOINSTYLE_LOGINS) {
			SendClientMessage(i, COLOUR_LOGOUT, msg);
		}
	}
	return 1;
}

CheckPlayerPassword(playerid, password[]) {
	/*new hashpassword[MAX_HASH];
	new filename[MAX_FILENAME];
	strcpy(filename, GetPlayerIDFilePath(playerid));
	
	if (INI_Exist(filename)) {
		if (INI_Open(filename)) {
			if (INI_ReadString(hashpassword, KEY_PASSWORD)) {
				if (!strcmp(hashpassword, MD5_Hash(password))) {
					INI_Close();
					return 1;
				}
			}
			INI_Close();
		}
		return 0;
	}*/
	
	if (!pData[playerid][pPassword][0]) {
		return 0;
	} else {
		if (!strcmp(pData[playerid][pPassword], MD5_Hash(password), true)) {
			return 1;
		} else {
			return 0;
		}
	}
}

LoadPlayerAccount(playerid) {
	if (pData[playerid][pIsLoggedIn]) return 0; // Player must not be logged in
	
	// Check if name is valid
	new loginname[MAX_NAME];
	
	GetPlayerName(playerid, loginname, MAX_NAME);
	LoopPlayers(i) {
		if (pData[i][pIsLoggedIn] &&
			!strcmp(loginname, pData[i][pLoginName], true)) {
			
			return 0;
		}
	}
	
	new time = GetTickCount();
	new filename[MAX_FILENAME];
	strcpy(filename, GetPlayerIDFilePath(playerid));
	
	if (!INI_Exist(filename)) return 0;
	if (!INI_Open(filename)) return 0;
	
	// Fill in known data
	pData[playerid][pIsLoggedIn] = 1;
	strcpy(pData[playerid][pLoginName], loginname, MAX_NAME);
	
	// Starting reading data needed
	INI_ReadString(pData[playerid][pPassword], KEY_PASSWORD, MAX_HASH);
	pData[playerid][pLastLoginTime] = INI_ReadInt(KEY_LASTLOGIN);
	
	pData[playerid][pBankCash] = INI_ReadInt(KEY_BANKCASH);
	pData[playerid][pXP] = INI_ReadInt(KEY_XP);
	INI_ReadString(pData[playerid][pGang], KEY_GANG, MAX_GANG_NAME);
	pData[playerid][pKills] = INI_ReadInt(KEY_KILLS);
	pData[playerid][pDeaths] = INI_ReadInt(KEY_DEATHS);
	pData[playerid][pSuicides] = INI_ReadInt(KEY_SUICIDES);
	pData[playerid][pSkin] = INI_ReadInt(KEY_SKIN);
	pData[playerid][pSpecialty] = INI_ReadInt(KEY_SPECIALTY);
	
	pData[playerid][pSideChat] = INI_ReadInt(KEY_SIDECHAT);
	pData[playerid][pSideChatLines] = INI_ReadInt(KEY_SIDECHATLINES);
	pData[playerid][pSideChatTime] = INI_ReadInt(KEY_SIDECHATTIME);
	
	INI_ReadString(pData[playerid][pLoginMsg], KEY_LOGINMSG, MAX_LOGIN_MSG);
	pData[playerid][pDisableAutoLogin] = INI_ReadInt(KEY_DISABLEAUTOLOGIN);
	pData[playerid][pBounty] = INI_ReadInt(KEY_BOUNTY);
	pData[playerid][pJoinStyle] = INI_ReadInt(KEY_JOINSTYLE);
	pData[playerid][pPasswordReset] = 0;
	
	new wpnstr[MAX_STRING];
	new wpnactstr[MAX_STRING];
	INI_ReadString(wpnstr, KEY_WEAPONS);
	INI_ReadString(wpnactstr, KEY_ACTIVEWEAPONS);
	
	// Data loaded, load/calculate other data
	GivePlayerMoney(playerid, INI_ReadInt(KEY_CASH));
	
	new Float:x = INI_ReadFloat(KEY_X);
	new Float:y = INI_ReadFloat(KEY_Y);
	new Float:z = INI_ReadFloat(KEY_Z);
	new Float:rot = INI_ReadFloat(KEY_ROT);
	
	// Get player last point
	if (z < 200.0 && x != 0.0 && y != 0.0 && z != 0.0) { // Check if spawn position is valid
		SkinsSetNextSpawnPos(playerid, x, y, z, rot);
	} else {
		ClearSpawnPoint(playerid);
	}
	
	// Load player weapons
	new idx1;
	new idx2;
	for (new weaponid; weaponid < MAX_WEAPON_ID; weaponid++) {
		pData[playerid][pWeapons][weaponid] = strval(split(wpnstr, idx1, '|'));
		if (strval(split(wpnactstr, idx2, '|'))) {
			pData[playerid][pActiveWeapons][weaponid] = 1;
		} else {
			pData[playerid][pActiveWeapons][weaponid] = 0;
		}
	}
	
	// Save login IP address
	new playerip[MAX_IP]; GetPlayerIp(playerid, playerip, sizeof(playerip));
	INI_WriteString(KEY_LASTIP, playerip);
	INI_Save();
	INI_Close();
	
	printf("[FDM] LOGIN: %s(%i) has logged in (%s)", pData[playerid][pLoginName], playerid, playerip, GetTickCount() - time);
	
	OnPlayerLogin(playerid);
	
	pData[playerid][pLastHash] = GetAccountHash(psaveid);
	return 1;
}

SavePlayerAccount(playerid) {
	if (!pData[playerid][pIsLoggedIn]) return 0; // Player must be logged in
	
	new filename[MAX_FILENAME];
	//format(filename, sizeof(filename), "%s%s.txt", FILE_PLAYERS, pData[playerid][pLoginName]);
	strcpy(filename, GetPlayerFilePath(pData[playerid][pLoginName]));
	//filename = GetValidFilenameFromString(filename);
	
	if (!INI_Open(filename)) return 0;
	
	// Write nessasary data
	INI_WriteString(KEY_PASSWORD, pData[playerid][pPassword]);
	INI_WriteInt(KEY_LASTLOGIN, gettime());
	
	INI_WriteInt(KEY_CASH, GetPlayerMoney(playerid));
	INI_WriteInt(KEY_BANKCASH, pData[playerid][pBankCash]);
	INI_WriteInt(KEY_XP, pData[playerid][pXP]);
	INI_WriteString(KEY_GANG, pData[playerid][pGang]);
	INI_WriteInt(KEY_KILLS, pData[playerid][pKills]);
	INI_WriteInt(KEY_DEATHS, pData[playerid][pDeaths]);
	INI_WriteInt(KEY_SUICIDES, pData[playerid][pSuicides]);
	INI_WriteInt(KEY_SKIN, pData[playerid][pSkin]);
	INI_WriteInt(KEY_SPECIALTY, pData[playerid][pSpecialty]);
	
	INI_WriteInt(KEY_SIDECHAT, pData[playerid][pSideChat]);
	INI_WriteInt(KEY_SIDECHATLINES, pData[playerid][pSideChatLines]);
	INI_WriteInt(KEY_SIDECHATTIME, pData[playerid][pSideChatTime]);
	
	INI_WriteString(KEY_LOGINMSG, pData[playerid][pLoginMsg]);
	INI_WriteInt(KEY_BOUNTY, pData[playerid][pBounty]);
	INI_WriteInt(KEY_JOINSTYLE, pData[playerid][pJoinStyle]);
	INI_WriteInt(KEY_DISABLEAUTOLOGIN, pData[playerid][pDisableAutoLogin]);
	
	// Player positions
	new Float:x;
	new Float:y;
	new Float:z;
	new Float:rot;
	new vehicleid;
	
	// If xyz is 0.0, player will just respawn upon joining
	if (IsPlayerSpawned(playerid) &&
		!GetPlayerInterior(playerid) &&
		!IsPlayerNearAnyZone(playerid) &&
		!IsPlayerInMinimode(playerid)) {
		
		vehicleid = GetPlayerVehicleID(playerid);
		if (vehicleid) {
			switch (GetVehicleModel(vehicleid)) {
				case 592,577,511,512,593,520,553,476,519,460,513,548,425,417,487,488,497,563,447,469: {} // Airplanes
				default: {
					GetVehiclePos(vehicleid, x, y, z);
					GetVehicleZAngle(vehicleid, rot);
				}
			}
		} else {
			GetPlayerPos(playerid, x, y, z);
			GetPlayerFacingAngle(playerid, rot);
		}
		
		if (
			x < -3000.0 || x > 3000.0 || // Out of the map
			y < -3000.0 || y > 3000.0 ||
			z < 2.0 || z > 530.0) { // In water or flying in some way
			
			x = 0.0;
			y = 0.0;
			z = 0.0;
			rot = 0.0;
		}
	}
	
	INI_WriteFloat(KEY_X, x);
	INI_WriteFloat(KEY_Y, y);
	INI_WriteFloat(KEY_Z, z);
	INI_WriteFloat(KEY_ROT, rot);
	
	// Get weapon data
	new wpnstr[MAX_STRING];
	new wpnactstr[MAX_STRING];
	new tmpstr[MAX_REAL];
	
	for (new weaponid; weaponid < MAX_WEAPON_ID; weaponid++) {
		valstr(tmpstr, pData[playerid][pWeapons][weaponid]);
		strcat(wpnstr, tmpstr);
		strcat(wpnstr, "|");
		
		valstr(tmpstr, pData[playerid][pActiveWeapons][weaponid]);
		strcat(wpnactstr, tmpstr);
		strcat(wpnactstr, "|");
	}
	
	// Write weapon data
	INI_WriteString(KEY_WEAPONS, wpnstr);
	INI_WriteString(KEY_ACTIVEWEAPONS, wpnactstr);
	
	pData[playerid][pLastHash] = GetAccountHash(psaveid);
	
	INI_Save();
	INI_Close();
	return 1;
}

CreatePlayerAccount(playerid, password[]) { // Creates a new player account
	new filename[MAX_FILENAME];
	new passwordhash[MAX_HASH];
	strcpy(filename, GetPlayerIDFilePath(playerid));
	strcpy(passwordhash, MD5_Hash(password));
	
	if (!INI_Open(filename)) return 0;
	/*
	INI_WriteString(KEY_PASSWORD, passwordhash);
	INI_WriteInt(KEY_LASTLOGIN, gettime());
	new playerip[MAX_IP]; GetPlayerIp(playerid, playerip, sizeof(playerip));
	INI_WriteString(KEY_LASTIP, playerip);
	
	INI_WriteInt(KEY_CASH, 0);
	INI_WriteInt(KEY_BANKCASH, 0);
	INI_WriteInt(KEY_XP, 0);
	INI_WriteString(KEY_GANG, "");
	INI_WriteInt(KEY_KILLS, 0);
	INI_WriteInt(KEY_DEATHS, 0);
	INI_WriteInt(KEY_SUICIDES, 0);
	INI_WriteInt(KEY_SKIN, 0);
	INI_WriteInt(KEY_SIDECHAT, 1);
	INI_WriteInt(KEY_SIDECHATLINES, 5);
	INI_WriteInt(KEY_SIDECHATTIME, 0);
	INI_WriteInt(KEY_BOUNTY, 0);
	
	// Player positions
	new Float:x;
	new Float:y;
	new Float:z;
	new Float:rot;
	new vehicleid;
	
	if (IsPlayerSpawned(playerid)) { // If xyz is 0.0, player will just respawn upon joining
		vehicleid = GetPlayerVehicleID(playerid);
		if (vehicleid) {
			GetVehiclePos(vehicleid, x, y, z);
			GetVehicleZAngle(vehicleid, rot);
		} else {
			GetPlayerPos(playerid, x, y, z);
			GetPlayerFacingAngle(playerid, rot);
		}
	}
	
	INI_WriteFloat(KEY_X, x);
	INI_WriteFloat(KEY_Y, y);
	INI_WriteFloat(KEY_Z, z);
	INI_WriteFloat(KEY_ROT, rot);
	
	INI_Save();
	INI_Close();
	
	// Get weapon data
	new wpnstr[MAX_STRING];
	new wpnactstr[MAX_STRING];
	new tmpstr[MAX_STRING];
	for (new weaponid; weaponid < MAX_WEAPON_ID; weaponid++) {
		valstr(tmpstr, pData[playerid][pWeapons][weaponid]);
		strins(wpnstr, tmpstr, strlen(wpnstr));
		strins(wpnstr, "|", strlen(wpnstr));
		
		valstr(tmpstr, pData[playerid][pActiveWeapons][weaponid]);
		strins(wpnactstr, tmpstr, strlen(wpnactstr));
		strins(wpnactstr, "|", strlen(wpnactstr));
	}
	*/
	
	// Create player data
	pData[playerid][pIsLoggedIn] = 1;
	GetPlayerName(playerid, pData[playerid][pLoginName], MAX_NAME);
	strcpy(pData[playerid][pPassword], passwordhash, MAX_HASH);
	pData[playerid][pLastLoginTime] = gettime();
	
	pData[playerid][pBankCash] = 0;
	pData[playerid][pXP] = 0;
	pData[playerid][pLevel] = 0;
	strclr(pData[playerid][pGang]);
	pData[playerid][pKills] = 0;
	pData[playerid][pDeaths] = 0;
	pData[playerid][pSuicides] = 0;
	pData[playerid][pSkin] = 0;
	pData[playerid][pSideChat] = 1;
	pData[playerid][pSideChatLines] = SIDECHAT_DEFAULT_LINES;
	pData[playerid][pSideChatTime] = SIDECHAT_DEFAULT_TIME;
	if (GetAlarVersion()) {
		pData[playerid][pJoinStyle] = JOINSTYLE_LOGINS;
	} else {
		pData[playerid][pJoinStyle] = JOINSTYLE_ALL;
	}
	pData[playerid][pBounty] = 0;
	
	new playerip[MAX_IP]; GetPlayerIp(playerid, playerip, sizeof(playerip));
	INI_WriteString(KEY_LASTIP, playerip);
	SavePlayerAccount(playerid);
	
	printf("[FDM] REGISTER: %s(%i) has registered (%s)", pData[playerid][pLoginName], playerid, playerip);
	
	OnPlayerRegister(playerid);
	return 1;
}

GetAccountHash(playerid) {
	new key[64] = "92uGz5eN7vNoe&7Vx,lgXzUvrRsXh&Lp+6/v0Z9GKV%zQ1JD-%uVtDFaf,+B/I#";
	new hash;
	new k;
	
	HashString(pData[playerid][pLoginName], key, k, hash);
	HashString(pData[playerid][pPassword], key, k, hash);
	
	HashInt(GetPlayerMoney(playerid), key, k, hash);
	HashInt(pData[playerid][pBankCash], key, k, hash);
	HashInt(pData[playerid][pXP], key, k, hash);
	HashString(pData[playerid][pGang], key, k, hash);
	HashInt(pData[playerid][pKills], key, k, hash);
	HashInt(pData[playerid][pDeaths], key, k, hash);
	HashInt(pData[playerid][pSuicides], key, k, hash);
	HashInt(pData[playerid][pSkin], key, k, hash);
	HashInt(pData[playerid][pSpecialty], key, k, hash);
	
	HashInt(pData[playerid][pSideChat], key, k, hash);
	HashInt(pData[playerid][pSideChatLines], key, k, hash);
	HashInt(pData[playerid][pSideChatTime], key, k, hash);
	
	HashString(pData[playerid][pLoginMsg], key, k, hash);
	HashInt(pData[playerid][pBounty], key, k, hash);
	HashInt(pData[playerid][pJoinStyle], key, k, hash);
	HashInt(pData[playerid][pDisableAutoLogin], key, k, hash);
	
	for (new weaponid; weaponid < MAX_WEAPON_ID; weaponid++) {
		HashInt(pData[playerid][pWeapons][weaponid], key, k, hash);
		HashInt(pData[playerid][pActiveWeapons][weaponid], key, k, hash);
	}
	
	return hash;
}

HashString(str[], key[64], &k, &hash) {
	new len;
	for (new i; str[i] != 0; i++) {
		HashInt(str[i], key, k, hash);
		len = i;
	}
	for (new i = len; i >= 0; i--) {
		HashInt(str[i], key, k, hash);
	}
	HashInt(len, key, k, hash);
	return 1;
}

HashInt(number, key[64], &k, &hash) {
	hash += (number | key[k]) + (number & key[k]) - (number << (key[k] % 32)) + (number << (key[k] % 4));
	key[k] += hash;
	if (key[k] > 255) key[k] = key[k] % 255;
	k++;
	if (k >= 64) k = 0;
	return 1;
}

/*DeletePlayerAccount(playerid) { //TODO
	ResetPlayerAccountData(playerid);
	INIDestroy(GetPlayerIDFilePath(playerid));
	return 1;
}*/

ResetPlayerAccountData(playerid) {
	new tmpdata[playerDataInfo];
	pData[playerid] = tmpdata;
	return 1;
}

stock GetPlayerIDFilePath(playerid) {
	new filename[MAX_FILENAME];
	if (pData[playerid][pIsLoggedIn]) {
		format(filename, sizeof(filename), "%s%s.txt", FILE_PLAYERS, GetValidFilenameFromString(pData[playerid][pLoginName], 0));
	} else {
		new playername[MAX_NAME];
		GetPlayerName(playerid, playername, sizeof(playername));
		strcpy(playername, GetValidFilenameFromString(playername, 0));
		format(filename, sizeof(filename), "%s%s.txt", FILE_PLAYERS, playername);
	}
	return filename;
}

stock GetPlayerFilePath(playername[]) {
	new filename[MAX_FILENAME];
	format(filename, sizeof(filename), "%s%s.txt", FILE_PLAYERS, GetValidFilenameFromString(playername, 0));
	return filename;
}

stock kKickPlayer(playerid, reason[], colour = COLOUR_KICK) {
	new playerName[MAX_PLAYER_NAME];
	new message[MAX_INPUT];
	
	if (pData[playerid][pIsLoggedIn]) {
		strcpy(playerName, pData[playerid][pLoginName]);
	} else {
		GetPlayerName(playerid, playerName, sizeof(playerName));
	}
	
	format(message, sizeof(message), "%s(%i) has left the server (%s)", playerName, playerid, reason);
	
	if (AddJoinLine(playerid, colour, message)) {
		printf("[FDM] KICKED: *** %s", message);
	} else {
		format(message, sizeof(message), "*** %s(%i) has left the server (%s)", playerName, playerid, reason);
		SendClientMessageToAll(colour, message);
		printf("[FDM] KICKED: %s", message);
	}
	
	return Kick(playerid);
}

//-----------------------------------------------------------------------------------------------------

/*players_OnPlayerSpawn(playerid) {
	if (pData[playerid][pIsRegistered] && !pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "An account exists on this name. You must login before spawning. (/login [password])");
		StartSkinSelection(playerid);
		return 1;
	}
	return 0;
}*/

player_OnPlayerStopSelection(playerid) {
	if (pData[playerid][pIsRegistered] && !pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "An account exists on this name. You must login before spawning. (/login [password])");
		return 0;
	}
	pData[playerid][pSkin] = skinList[skinPlayerData[playerid][pCurrentClass]];
	return 1;
}

players_OnPlayerText(playerid) {
	if (!pData[playerid][pFirstConnect]) {
		return 0;
	}
	
	if (pData[playerid][pIsRegistered] && !pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "An account exists on this name. You must login before chatting. (/login [password])");
		return 0;
	}
	
	return 1;
}

//	/login [password]
kcmd:login(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /login [password]");
		return 1;
	}
	
	if (pData[playerid][pIsLoggedIn]) { // Is player logged in
		SendClientMessage(playerid, COLOUR_ERROR, "LOGIN: You are already logged in.");
		return 1;
	}
	
	if (!pData[playerid][pIsRegistered]) { // Check if account exists
		SendClientMessage(playerid, COLOUR_ERROR, "LOGIN: The account on your name does not exist.");
		return 1;
	}
	
	if (!CheckPlayerPassword(playerid, text)) { // Is the password correct
		SendClientMessage(playerid, COLOUR_ERROR, "LOGIN: Incorrect Password.");
		kKickPlayer(playerid, "Failed login attempt", COLOUR_ATTENTION);
		return 1;
	}
	
	LoadPlayerAccount(playerid);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, "LOGIN: You are logged in.");
	SendLoginMessageToAll(playerid, LOGIN_NORMAL);
	return 1;
}
	
//	/logout
kcmd:logout(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) { // Player must be logged in, to log out.
		SendClientMessage(playerid, COLOUR_ERROR, "LOGOUT: You must be logged in to logout.");
		return 1;
	}
	
	if (INI_Open(GetPlayerIDFilePath(playerid))) {
		INI_WriteString(KEY_LASTIP, "LOGGED_OUT");
		//INI_Save();
		//INI_Close();
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "LOGOUT: You have logged out and will be kicked.");
		kKickPlayer(playerid, "Logout", COLOUR_LOGOUT);
	} else {
		SendClientMessage(playerid, COLOUR_ERROR, "LOGOUT: Failed for unknown reasons.");
	}
	return 1;
}
	
//	/register [password]
kcmd:register(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /register [password]");
		return 1;
	}
	
	if (pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "REGISTER: You are already logged in.");
		return 1;
	}
	
	if (pData[playerid][pIsRegistered]) { // Check if account exists
		//if (gettime() - pData[playerid][pLastLoginTime] > MIN_LASTLOGIN_DELETE_TIME || !pData[playerid][pLastLoginTime]) {
			SendClientMessage(playerid, COLOUR_ERROR, "REGISTER: The account already exists. Use /login [password]");
			return 1;
		//}
	}
	
	new password[MAX_PASS_LEN];
	new idx;
	splitcpy(password, text, idx, ' '); // No spaces in password
	
	CreatePlayerAccount(playerid, password);
	OnPlayerRegister(playerid);
	
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, "REGISTER: Account created. You are logged in.");
	SendLoginMessageToAll(playerid, LOGIN_REGISTER);
	
	return 1;
}
	
//	/changepass [oldpassword] [newpassword]
kcmd:changepass(playerid, text[]) {
	
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "CHANGE PASSWORD: You must be registered to use /changepass. (/register [password])");
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /changepass [oldpassword] [newpassword]");
		return 1;
	}
	
	new oldpassword[MAX_PASS_LEN];
	new newpassword[MAX_PASS_LEN];
	new idx;
	
	splitcpy(oldpassword, text, idx, ' ');
	splitcpy(newpassword, text, idx, ' ');
	
	if (!oldpassword[0] || !newpassword[0]) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /changepass [oldpassword] [newpassword]");
		return 1;
	}
	
	if (!strcmp(MD5_Hash(oldpassword), pData[playerid][pPassword])) {
		// Password match
		strcpy(pData[playerid][pPassword], MD5_Hash(newpassword), MAX_HASH);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "CHANGE PASSWORD: Password changed.");
	} else {
		SendClientMessage(playerid, COLOUR_ERROR, "CHANGE PASSWORD: Your 'old password' does not match your password.");
	}
	return 1;
}
	
//	/recover [player] [password]
kcmd:recover(playerid, text[]) {
	
	if (pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "RECOVER: You cannot use /recover while logged in.");
		return 1;
	}
	
	new msg[MAX_INPUT],
		rname[MAX_NAME],
		password[MAX_INPUT],
		recoverid,
		idx;
	
	recoverid = FindPlayer(split(text, idx, ' '));
	splitcpy(password, text, idx, ' ');
	
	if (recoverid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "RECOVER: Invalid Player ID/Name.");
		return 1;
	}
	
	if (pData[recoverid][pIsLoggedIn]) {
		format(msg, sizeof(msg), "RECOVER: %s is already logged in.", pData[recoverid][pLoginMsg]);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	if (!CheckPlayerPassword(recoverid, password)) {
		// Incorrect password
		SendClientMessage(playerid, COLOUR_ERROR, "RECOVER: Incorrect password.");
		kKickPlayer(playerid, "Failed recovery attempt", COLOUR_ATTENTION);
		return 1;
	}
	
	// Good to go
	SendClientMessage(recoverid, COLOUR_ERROR, "RECOVER: You were kicked - another person recovered their account on the name you used.");
	kKickPlayer(recoverid, "Imposter", COLOUR_ATTENTION);
	
	SetPlayerName(playerid, rname);
	if (LoadPlayerAccount(playerid)) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "RECOVER SUCCESSFUL: You have recovered your account and logged in.");
		SendLoginMessageToAll(playerid, LOGIN_NORMAL);
	} else {
		SendClientMessage(playerid, COLOUR_ERROR, "RECOVER WARNING: Failed to load existing account.");
	}
	
	return 1;
}
	
//	/joinstyle [all/logins/none]
kcmd:joinstyle(playerid, text[]) {
	
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "JOIN STYLE: You must be registered to use /joinstyle. (/register [password])");
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /joinstyle [all/logins/none]");
		return 1;
	}
	
	if (!strcmp(text, "all", true)) {
		pData[playerid][pJoinStyle] = JOINSTYLE_ALL;
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "JOIN STYLE: Login and leave messages for registered players will be displayed.");
	} else if (!strcmp(text, "logins", true)) {
		pData[playerid][pJoinStyle] = JOINSTYLE_LOGINS;
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "JOIN STYLE: Only login messages will be displayed.");
	} else if (!strcmp(text, "none", true)) {
		pData[playerid][pJoinStyle] = JOINSTYLE_NONE;
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "JOIN STYLE: No login or leave messages will be displayed.");
	} else {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /joinstyle [all/logins/none]");
	}
	
	return 1;
}
	
//	/loginmsg [text]
kcmd:loginmsg(playerid, text[]) {
	
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "LOGIN MSG: You must be registered to use /loginmsg. (/register [password])");
		return 1;
	}
	
	new msg[MAX_INPUT];
	
	if (isnull(text)) {
		if (pData[playerid][pLoginMsg][0]) {
			format(msg, sizeof(msg), "LOGIN MSG: Your login message has been cleared. (Was '%s').", pData[playerid][pLoginMsg]);
			strclr(pData[playerid][pLoginMsg]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		} else {
			SendClientMessage(playerid, COLOUR_HELP, "USAGE: /loginmsg [text]");
		}
	} else {
		if (strlen(text) >= MAX_LOGIN_MSG) {
			SendClientMessage(playerid, COLOUR_ERROR, "LOGIN MSG: Your login message is too long.");
			return 1;
		}
		
		strcpy(pData[playerid][pLoginMsg], text, MAX_LOGIN_MSG);
		format(msg, sizeof(msg), "LOGIN MSG: Your login message has been changed to '%s'.", pData[playerid][pLoginMsg]);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);	
	}
	return 1;
}
	
//	/aloginmsg [player] [text]
kcmd:aloginmsg(playerid, text[]) {
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MODERATOR) {
		new message[MAX_INPUT];
		format(message, sizeof(message), "LOGIN MSG: You must be at least admin level %i to use /aloginmsg.", ADMIN_LEVEL_MODERATOR);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /aloginmsg [player] (text)");
		return 1;
	}
	
	new msgpid,
		msgtext[MAX_INPUT],
		msg[MAX_INPUT],
		idx;
	
	msgpid = FindPlayer(split(text, idx, ' '));
	if (msgpid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "LOGIN MSG: Invalid player ID/Name.");
		return 1;
	}
	if (!pData[msgpid][pIsLoggedIn]) {
		GetPlayerName(msgpid, msg, sizeof(msg));
		format(msg, sizeof(msg), "LOGIN MSG: %s must be registered.", msg);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	splitcpy(msgtext, text, idx, 0);
	
	if (!msgtext[0]) {
		if (pData[msgpid][pLoginMsg][0]) {
			format(msg, sizeof(msg), "LOGIN MSG: %s's login message has been cleared. (was '%s').", pData[msgpid][pLoginName], pData[msgpid][pLoginMsg]);
			strclr(pData[msgpid][pLoginMsg]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
			
			GetPlayerName(playerid, msg, sizeof(msg));
			format(msg, sizeof(msg), "%s has cleared %s's login message", msg, pData[msgpid][pLoginName]);
			printf("[FDM] ADMIN: %s", msg);
			AddAdminLogLine(COLOUR_LOG, msg);
		} else {
			format(msg, sizeof(msg), "LOGIN MSG: %s's login message is already cleared.", pData[msgpid][pLoginName]);
			SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		}
	} else {
		if (strlen(text) >= MAX_LOGIN_MSG) {
			SendClientMessage(playerid, COLOUR_ERROR, "LOGIN MSG: The login message is too long.");
			return 1;
		}
		
		strcpy(pData[msgpid][pLoginMsg], msgtext, MAX_LOGIN_MSG);
		format(msg, sizeof(msg), "LOGIN MSG: %s's login message has been changed to '%s'.", pData[msgpid][pLoginName], pData[msgpid][pLoginMsg]);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, msg);
		
		GetPlayerName(playerid, msg, sizeof(msg));
		format(msg, sizeof(msg), "%s has changed %s's login message to '%s'", msg, pData[msgpid][pLoginName], pData[msgpid][pLoginMsg]);
		printf("[FDM] ADMIN: %s", msg);
		AddAdminLogLine(COLOUR_LOG, msg);
	}
	return 1;
}
	
//	/aresetpass [player]
kcmd:aresetpass(playerid, text[]) {
	
	new message[MAX_INPUT];
	
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MODERATOR) {
		format(message, sizeof(message), "RESET PASS: You must be at least admin level %i to use /aresetpass.", ADMIN_LEVEL_MODERATOR);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /aresetpass [player]");
		return 1;
	}
	
	new pid = FindPlayer(text);
	if (pid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "RESET PASS: Invalid Player ID/Name.");
		return 1;
	}
	
	if (pData[pid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "RESET PASS: This command can only be used on an unregistered player.");
		return 1;
	}
	
	new filepath[MAX_FILENAME],
		//passwordtext[16],
		pname[MAX_NAME];
	
	GetPlayerName(pid, pname, sizeof(pname));
	filepath = GetPlayerFilePath(pname);
	
	if (!pData[playerid][pIsRegistered]) {
		format(message, sizeof(message), "RESET PASS: %s's account does not exist.", message);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	/*for (new i; i < sizeof(passwordtext) - 1; i++) {
		passwordtext[i] = random('z' - 'a')  + 'a';
	}*/
	
	pData[pid][pPasswordReset] = 1;
	SendClientMessage(pid, COLOUR_SUCCESSFUL, "RESET PASS: Your password was reset. Create a new password NOW with /newpass [password]");
	
	format(message, sizeof(message), "RESET PASS: You have reset %s's password.", pname);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	
	GetPlayerName(playerid, message, sizeof(message));
	format(message, sizeof(message), "%s has reset %s's password", message, pname);
	printf("[FDM] ADMIN: %s", message);
	AddAdminLogLine(COLOUR_LOG, message);
	
	return 1;
}
	
//	/newpass [password]
kcmd:newpass(playerid, text[]) {
	if (!pData[playerid][pPasswordReset]) {
		SendClientMessage(playerid, COLOUR_ERROR, "NEW PASSWORD: This command can only be used after your password is reset.");
		return 1;
	}
	
	if (pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "NEW PASSWORD: This command cannot be used if you are registered.");
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /newpass [password]");
		return 1;
	}
	
	LoadPlayerAccount(playerid);
	SendLoginMessageToAll(playerid, LOGIN_NORMAL);
	
	strcpy(pData[playerid][pPassword], MD5_Hash(text), MAX_HASH);
	SavePlayerAccount(playerid);
	
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, "NEW PASSWORD: Password changed. Remember your password this time.");
	
	return 1;
}

kcmd:autologin(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "AUTOLOGIN: You must be registered to use /autologin. (/register [password])");
		return 1;
	}
	
	pData[playerid][pDisableAutoLogin] = !pData[playerid][pDisableAutoLogin];
	if (pData[playerid][pDisableAutoLogin]) {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "AUTOLOGIN: Auto-login is disabled.");
	} else {
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, "AUTOLOGIN: Auto-login is enabled.");
	}
	
	return 1;
}
