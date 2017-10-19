#include <a_samp>

#define COLOUR_PM 0xFFDD00AA

enum commandsListInfo {
	cmdSet,
	cmdAdminLevel,
	cmdIsVariant,
	cmdCommand[24],
	cmdParams[96],
	cmdDescription[MAX_INPUT]
}
new cmdList[][commandsListInfo] = {
	// Player login set
	{CMD_SET_LOGIN, 0, 0, "register", "[password]", "register an account on your name. To login instead, use /login."},
	{CMD_SET_LOGIN, 0, 0, "login", "[password]", "Loads your account. Use if you did not auto-login."},
	{CMD_SET_LOGIN, 0, 0, "logout", "", "Forces a login next time you join."},
	{CMD_SET_LOGIN, 0, 0, "recover", "[player] [password]", "Kicks a player using your name and login."},
	{CMD_SET_LOGIN, 0, 0, "joinstyle", "[all/logins/none]", "Changes if login or leave messages are displayed."},
	{CMD_SET_LOGIN, 0, 0, "loginmsg", "[text]", "Changes the message displayed when you login."},
	{CMD_SET_LOGIN, 0, 0, "autologin", "", "Toggles the autologin for your account."},
	{CMD_SET_LOGIN, 0, 0, "objective", "", "View the point of this gamemode."},
	
	// Gang set
	{CMD_SET_GANG, 0, 0, "gangcreate", "[colour] [gangname]", "Create a gang. Use /gangcolours to view colours."},
	{CMD_SET_GANG, 0, 0, "ganginvite", "[player]", "Invite a player into your gang (leader only)."},
	{CMD_SET_GANG, 0, 0, "gangkick", "[player]", "Kick a player from your gang (leader only)."},
	{CMD_SET_GANG, 0, 0, "gangleadergive", "[player]", "Promote a gang member to leader (leader only)."},
	{CMD_SET_GANG, 0, 0, "gangleadertake", "[player]", "Demote a gang member to member (leader only)."},
	{CMD_SET_GANG, 0, 0, "gangchangecolour", "[colour]", "Change the colour of your gang (leader only)."},
	{CMD_SET_GANG, 0, 0, "gangchatcolour", "", "Toggle using the gang colour for gang chat (leader only)."},
	{CMD_SET_GANG, 0, 0, "gangmembers", "", "A list of the current gang members in your gang."},
	{CMD_SET_GANG, 0, 0, "gangcolours", "", "List of avaliable gang colours."},
	{CMD_SET_GANG, 0, 0, "gangjoin", "", "Join the gang that invited you last."},
	{CMD_SET_GANG, 0, 0, "gangquit", "", "Quit your gang."},
	
	// Bank set
	{CMD_SET_MONEY, 0, 0, "balance", "", "Check how much money you have in the bank."},
	{CMD_SET_MONEY, 0, 0, "bank", "[amount]", "Bank a given amount of money."},
	{CMD_SET_MONEY, 0, 0, "withdraw", "[amount]", "Withdraw a given amount of money from the bank."},
	{CMD_SET_MONEY, 0, 0, "givecash", "[player] [amount]", "Give another player a given amount of money."},
	
	// Weapons set
	{CMD_SET_WEAPONS, 0, 0, "weaponlist", "", "A list of weapons you can buy (Short: /wl)."},
	{CMD_SET_WEAPONS, 0, 0, "weaponbuy", "[weapon] [ammo]", "Buy a spawn weapon (Short: /wb)."},
	{CMD_SET_WEAPONS, 0, 0, "weaponenable", "[weapon]", "Enable a spawn weapon (Short: /we)."},
	{CMD_SET_WEAPONS, 0, 0, "weapondisable", "[weapon]", "Disable a spawn weapon (Short: /wd)."},
	{CMD_SET_WEAPONS, 0, 0, "weaponlevels", "", "A list all weapons and their level requirement (Short: /wlevels)."},
	{CMD_SET_WEAPONS, 0, 0, "weapons", "", "A list of spawn weapons you own."},
	
	// Minimodes set
	{CMD_SET_MINIMODES, 0, 0, "start", "[mode]", "Start a minimode (Short: /s)"},
	{CMD_SET_MINIMODES, 0, 0, "dstart", "[mode]", "Start a minimode with a 'drunk' flag (Short: /s)"},
	{CMD_SET_MINIMODES, 0, 0, "join", "[mode]", "Teleport to a minimode (Short: /j)"},
	{CMD_SET_MINIMODES, 0, 0, "leave", "", "Leave a minimode (Short: /l)"},
	{CMD_SET_MINIMODES, 0, 0, "spec", "[modeid]", "Spectate a minimode."},
	{CMD_SET_MINIMODES, 0, 0, "modes", "", "Shows a list of minimodes that are waiting for players or in progress."},
	{CMD_SET_MINIMODES, 0, 1, "modes", "[page]", "Shows a list of all minimodes, on a given page."},
	{CMD_SET_MINIMODES, 0, 1, "modes", "[text]", "Searches for a minimode."},
	{CMD_SET_MINIMODES, 0, 0, "modeinfo", "[mode]", "Displays detailed information on a minimode."},
	{CMD_SET_MINIMODES, 0, 0, "records", "[mode]", "Show the records for a given race."},
	{CMD_SET_MINIMODES, 0, 0, "myrecords", "", "Shows all of your records."},
	{CMD_SET_MINIMODES, 0, 1, "myrecords", "[position]", "Shows all of your records in a position."},
	{CMD_SET_MINIMODES, 0, 0, "playerrecords", "(player) (position)", "Shows another player's records."},
	
	// Specialties set
	{CMD_SET_SPECIALTY, 0, 0, "specialty", "[type]", "Select your specialty type."},
	{CMD_SET_SPECIALTY, 0, 0, "specialtylist", "", "A list of specialties and their commands."},
	{CMD_SET_SPECIALTY, 0, 0, "specialtyinfo", "", "Detailed information on a specific specialty."},
	{CMD_SET_SPECIALTY, 0, 0, "heal", "(player)", "Heals a player. (Short: /h)"},
	{CMD_SET_SPECIALTY, 0, 0, "givelife", "(player)", "Gives your health to another player. (Short: /gl)"},
	{CMD_SET_SPECIALTY, 0, 0, "repair", "", "Repairs your vehicle. (Short: /rv)"},
	{CMD_SET_SPECIALTY, 0, 0, "rapidrepair", "", "Rapidly repairs your vehicle - chance of death. (Short: /rr)"},
	{CMD_SET_SPECIALTY, 0, 0, "target", "[player]", "Marks a player with an X on your screen. (Short: /tg)"},
	{CMD_SET_SPECIALTY, 0, 0, "stealth", "", "Stealths yourself against others. (Short: /sl)"},
	
	// Misc set
	{CMD_SET_MISC, 0, 0, "stext", "", "Toggles the text displayed at the bottom of your screen."},
	{CMD_SET_MISC, 0, 0, "stextlines", "[lines]", "Sets how many lines of text there are at the bottom of the screen."},
	{CMD_SET_MISC, 0, 0, "stexttime", "(seconds)", "Sets how long each textdraw is shown for. Leave empty for unlimited time."},
	{CMD_SET_MISC, 0, 0, "hitman", "[player] [money]", "Place a bounty on a player."},
	{CMD_SET_MISC, 0, 0, "bounties", "", "Shows all the bounties of the online players."},
	{CMD_SET_MISC, 0, 0, "stats", "", "View your own stats."},
	{CMD_SET_MISC, 0, 1, "stats", "[player]", "View another player's stats."},
	{CMD_SET_MISC, 0, 0, "kill", "", "Kill yourself."},
	{CMD_SET_MISC, 0, 0, "emo", "", "Cut yourself."},
	{CMD_SET_MISC, 0, 0, "pm", "[player] [message]", "Send a private message to a player (Short: /m)."},
	{CMD_SET_MISC, 0, 0, "r", "[message]", "Send a private message to the last player that sent you a message."},
	
	{CMD_SET_MISC, 0, 0, "commands", "", "Shows a list of all commands (Short: /cmds)."},
	{CMD_SET_MISC, 0, 1, "commands", "[login/gang/money/weapon/minimodes/misc/admin]", "Shows help on a set of commands (Short: /cmds)."},
	{CMD_SET_MISC, 0, 0, "commandhelp", "[command]", "Shows help on a command (Short: /cmdhelp)."},
	
	// Admin set
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "astart", "[mode]", "Starts a minimode without any restrictions."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "adstart", "[mode]", "Starts a minimode without any restrictions with a 'drunk' flag."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MODERATOR, 0, "astop", "[mode]", "Stops a minimode."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MODERATOR, 0, "amodekick", "[player] (reason)", "Kicks a player from their current minimode."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "amodelock", "", "Prevents any minimodes from starting."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "amodeunlock", "", "Allows minimodes to start again."},
	
	{CMD_SET_ADMIN, ADMIN_LEVEL_MODERATOR, 0, "adeleterecord", "[playername] [racemode id/name]", "Delete a player's record in a race."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "adeleteracerecords", "", "Delete all records in a race."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MODERATOR, 0, "adeleteplayerrecords", "", "Delete a player's record across all races."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "amergerecords", "[foldername]", "Merges race records with the current race records."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "aresetracestats", "[racemode id/name]", "Resets a race's statistics."},
	
	{CMD_SET_ADMIN, ADMIN_LEVEL_MODERATOR, 0, "aresetpass", "[player]", "Reset a player's password."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MODERATOR, 0, "atransfer", "[player] [name]", "Change a player's name and transfers all gamemode data to it."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "asetlevel", "", "Set a player's level."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "askinselect", "[player]", "Force a player into skin selection (Without killing them)."},
	
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "aloginmsg", "[player] (text)", "Set's a player's login message. Leave the 'text' empty to remove."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "agivexp", "", "Give a player experience."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "agangcolour", "[player] [r] [g] [b]", "Change a gang's colour."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MODERATOR, 0, "acresist", "[player] (enable)", "Toggles the anti-cheat's resistance against a player."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "acban", "[player] (reason)", "Bans a player that is suspected by the anti-cheat."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "acblock", "[player]", "Blocks a player using the anti-cheat temporarily."},
	{CMD_SET_ADMIN, ADMIN_LEVEL_MISC, 0, "acunblock", "[ip]", "Unblocks an IP that was suspended by the anti-cheat."},
	
	{CMD_SET_ADMIN, ADMIN_LEVEL_MASTER, 0, "reload", "[all/levels/zones/weapons/vehicles/spawns/minimodes/specialty/other]", "Reload specific server data."}
};
new pLastPM[MAX_PLAYERS];

misccmdInit() {
	for (new i; i < MAX_PLAYERS; i++) {
		pLastPM[i] = INVALID_PLAYER_ID;
	}
}

/*misccmd_OnPlayerPrivmsg(playerid, recieverid) {
	pLastPM[playerid] = recieverid;
	pLastPM[recieverid] = playerid;
	return 1;
}*/

misccmd_OnPlayerDisconnect(playerid) {
	pLastPM[playerid] = INVALID_PLAYER_ID;
	LoopPlayers(i) {
		if (pLastPM[i] == playerid) {
			pLastPM[i] = INVALID_PLAYER_ID;
		}
	}
	return 1;
}

stock SendCommandSetToPlayer(playerid, cmdset) {
	new msg[MAX_INPUT],
		count,
		adminlevel = GetAdminLevel(playerid);
	
	for (new i; i < sizeof(cmdList); i++) {
		if (cmdset == cmdList[i][cmdSet] && adminlevel >= cmdList[i][cmdAdminLevel]) {
			format(msg, sizeof(msg), "/%s %s - %s", cmdList[i][cmdCommand], cmdList[i][cmdParams], cmdList[i][cmdDescription]);
			SendClientMessage(playerid, COLOUR_HELP, msg);
			count++;
		}
	}
	
	if (!count) {
		SendClientMessage(playerid, COLOUR_HELP, "No commands avaliable.");
	} else if (count > 8) {
		SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	}
}

stock PlayerSendPlayerPM(playerid, sendid, msg[]) {
	if (playerid == INVALID_PLAYER_ID || sendid == INVALID_PLAYER_ID || playerid == sendid || !msg[0] || !IsPlayerConnected(playerid) || !IsPlayerConnected(sendid)) return 0;
	
	pLastPM[playerid] = sendid;
	pLastPM[sendid] = playerid;
	
	new playername[MAX_NAME];
	new sendname[MAX_NAME];
	new tmpstr[MAX_STRING];
	
	GetPlayerName(playerid, playername, sizeof(playername));
	GetPlayerName(sendid, sendname, sizeof(sendname));
	
	format(tmpstr, sizeof(tmpstr), "PM sent to %s(%i): %s", sendname, sendid, msg);
	SendWrappedMessageToPlayer(playerid, COLOUR_PM, tmpstr);
	
	format(tmpstr, sizeof(tmpstr), "PM from %s(%i): %s", playername, playerid, msg);
	SendWrappedMessageToPlayer(sendid, COLOUR_PM, tmpstr);
	
	printf("[pm] [%s to %s]: %s", playername, sendname, msg);
	return 1;
}

// Suicide
kcmd:kill(playerid, text[]) {
	if (!IsPlayerSpawned(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "KILL: You must be spawned.");
		return 1;
	}
	
	SendClientMessage(playerid, COLOUR_ATTENTION, "KILL: You have killed youself.");
	KillPlayer(playerid, WEAPON_BOMB);
	return 1;
}

kcmd:r(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /r [message]");
		return 1;
	}
	
	if (pLastPM[playerid] == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "PM: You have not sent/recieved any PM's yet.");
		return 1;
	}
	
	if (!PlayerSendPlayerPM(playerid, pLastPM[playerid], text)) {
		SendClientMessage(playerid, COLOUR_ERROR, "PM: Cannot send PM - Unknown error.");
		return 1;
	}
	return 1;
}

kcmd:m(playerid, text[]) return kcmd_pm(playerid, text);
kcmd:pm(playerid, text[]) {
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /pm [player] [message]");
		return 1;
	}
	
	new idx, sendid, msg[MAX_INPUT];
	sendid = FindPlayer(split(text, idx, ' '));
	splitcpy(msg, text, idx, 0);
	
	if (sendid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "PM: Invalid player ID.");
		return 1;
	}
	
	if (playerid == sendid) {
		SendClientMessage(playerid, COLOUR_ERROR, "PM: You cannot message yourself.");
		return 1;
	}
	
	if (!msg[0]) {
		SendClientMessage(playerid, COLOUR_ERROR, "PM: You must enter a message.");
		return 1;
	}
	
	if (!PlayerSendPlayerPM(playerid, sendid, msg)) {
		SendClientMessage(playerid, COLOUR_ERROR, "PM: Cannot send PM - Unknown error.");
		return 1;
	}
	return 1;
}
	
// Stats
kcmd:stats(playerid, text[]) {
	new statsid;
	if (isnull(text)) {
		statsid = playerid;
	} else {
		statsid = FindPlayer(text);
		/*if (statsid == INVALID_PLAYER_ID) {
			SendClientMessage(playerid, COLOUR_ERROR, "STATS: Invalid player ID/Name.");
			return 1;
		}*/
	}
	
	new loggedIn, playerName[MAX_PLAYER_NAME], level, bounty, xp, gang[MAX_GANG_NAME], kills, deaths, suicides;
	
	if (statsid == INVALID_PLAYER_ID) {
		// Offline mode
		new fileName[MAX_FILENAME];
		
		if (text[0] == '"' && text[strlen(text) - 1] == '"') {
			strcpy(playerName, text[1], strlen(text) - 1);
			format(fileName, sizeof(fileName), "%s%s.txt", FILE_PLAYERS, GetValidFilenameFromString(playerName, 0));
		} else {
			format(fileName, sizeof(fileName), "%s%s.txt", FILE_PLAYERS, GetValidFilenameFromString(text, 0));
			strcpy(playerName, text);
		}
		
		if (!INI_Exist(fileName) || !INI_Open(fileName)) {
			SendClientMessage(playerid, COLOUR_ERROR, "STATS: Player not found.");
			return 1;
		}
		
		loggedIn = 0;
		xp = INI_ReadInt(KEY_XP);
		bounty = INI_ReadInt(KEY_BOUNTY);
		kills = INI_ReadInt(KEY_KILLS);
		deaths = INI_ReadInt(KEY_DEATHS);
		suicides = INI_ReadInt(KEY_SUICIDES);
		INI_ReadString(gang, KEY_GANG, MAX_GANG_NAME);
		
		if (xp >= lData[lList][lData[lMax]]) {
			level = lData[lMax];
		} else {
			for (new i = 1; i < lData[lMax] + 1; i++) {
				if (xp >= lData[lList][i]) continue;
				level = i - 1;
				break;
			}
		}
		
		INI_Close();
	} else {
		// Online mode
		if (pData[statsid][pIsLoggedIn]) {
			loggedIn = 1;
			strcpy(playerName, pData[statsid][pLoginName]);
		} else {
			loggedIn = 0;
			GetPlayerName(statsid, playerName, sizeof(playerName));
		}
		
		level = pData[statsid][pLevel];
		xp = pData[statsid][pXP];
		bounty = pData[statsid][pBounty];
		kills = pData[statsid][pKills];
		deaths = pData[statsid][pDeaths];
		suicides = pData[statsid][pSuicides];
		strcpy(gang, pData[statsid][pGang]);
	}
	
	new tmpstr[MAX_INPUT];
	
	format(tmpstr, sizeof(tmpstr), "Player Stats for '%s'", playerName);
	SendClientMessage(playerid, COLOUR_TITLE, tmpstr);
	
	// Is logged in
	if (loggedIn || statsid == INVALID_PLAYER_ID) {
		// Level
		if (statsid == INVALID_PLAYER_ID) {
			format(tmpstr, sizeof(tmpstr), "Logged In: Offline / Level: %i / Bounty: $%s", level, kvalstr(bounty));
		} else {
			format(tmpstr, sizeof(tmpstr), "Logged In: Yes / Level: %i / Bounty: $%s", level, kvalstr(bounty));
		}
		SendClientMessage(playerid, COLOUR_HELP, tmpstr);
		
		// XP
		if (level >= lData[lMax]) {
			format(tmpstr, sizeof(tmpstr), "XP: %s", kvalstr(xp));
			SendClientMessage(playerid, COLOUR_HELP, tmpstr);
		} else {
			format(tmpstr, sizeof(tmpstr), "XP: %s / Next level XP needed: %s", kvalstr(xp), kvalstr(lData[lList][level + 1] - xp));
			SendClientMessage(playerid, COLOUR_HELP, tmpstr);
		}
		
		// Gang
		if (gang[0]) {
			if (statsid == INVALID_PLAYER_ID || sppData[statsid][spType] == SP_TYPE_INVALID) {
				format(tmpstr, sizeof(tmpstr), "Gang: %s", gang);
			} else {
				format(tmpstr, sizeof(tmpstr), "Gang: %s / Specialty: %s", gang, sTypes[sppData[statsid][spType]][stName]);
			}
			SendClientMessage(playerid, COLOUR_HELP, tmpstr);
		} else {
			if (statsid != INVALID_PLAYER_ID && sppData[statsid][spType] != SP_TYPE_INVALID) {
				format(tmpstr, sizeof(tmpstr), "Specialty: %s", sTypes[sppData[statsid][spType]][stName]);
				SendClientMessage(playerid, COLOUR_HELP, tmpstr);
			}
		}
	} else {
		SendClientMessage(playerid, COLOUR_HELP, "Logged In: No");
	}
	
	// Kills / Deaths / Suicides
	new Float:ratio;
	if (deaths) {
		ratio = float(kills) / float(deaths);
	} else {
		ratio = float(kills);
	}
	
	format(tmpstr, sizeof(tmpstr), "Kills: %i / Deaths: %i / Suicides: %i / Ratio: %0.3f",
		kills, deaths, suicides, ratio);
	SendClientMessage(playerid, COLOUR_HELP, tmpstr);
	
	if (loggedIn || statsid == INVALID_PLAYER_ID) {
		new recordcountpos[3];
		new recordcountall;
		new tmp;
		for (new a; a < MAX_RACE_MAPS; a++) {
			if (raceMaps[a][rcModeID] == INVALID_MODE_ID) continue; // No race there
			tmp = 0;
			for (new b; b < MAX_RACE_RECORDS; b++) {
				if (!raceMapRecords[a][b][rcTime]) continue; // Invalid record
				
				if (!strcmp(raceMapRecords[a][b][rcPlayer], playerName, true)) { // Record
					if (tmp < sizeof(recordcountpos)) {
						recordcountpos[tmp]++;
					}
					recordcountall++;
				}
				
				tmp++; // Record position counting
			}
		}
		if (recordcountall) {
			format(tmpstr, sizeof(tmpstr), "Race Records: 1st: %i / 2nd: %i / 3rd: %i / All: %i",
				recordcountpos[0], recordcountpos[1], recordcountpos[2], recordcountall);
			SendClientMessage(playerid, COLOUR_HELP, tmpstr);
		}
	}
	
	if (statsid != INVALID_PLAYER_ID && mpData[statsid][mpModeID] != INVALID_MODE_ID) {
		format(tmpstr, sizeof(tmpstr), "Minimode: %s", modes[mpData[statsid][mpModeID]][mName]);
		SendClientMessage(playerid, COLOUR_HELP, tmpstr);
	}
	
	return 1;
}

// Admin give xp
kcmd:agivexp(playerid, text[]) {
	new message[MAX_INPUT];
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) {
		format(message, sizeof(message), "GIVE XP: You must be at least admin level %i to use /agivexp.", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /agivexp [player] [xp]");
		return 1;
	}
	
	new giveid;
	new xp;
	new idx;
	
	giveid = FindPlayer(split(text, idx, ' '));
	xp = strval(split(text, idx, ' '));
	
	if (giveid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "GIVE XP: Invalid player id/name.");
		return 1;
	}
	if (!xp) {
		SendClientMessage(playerid, COLOUR_ERROR, "GIVE XP: Invalid XP amount.");
		return 1;
	}
	if (!pData[giveid][pIsLoggedIn]) {
		new givename[MAX_NAME];
		GetPlayerName(giveid, givename, sizeof(givename));
		format(message, sizeof(message), "GIVE XP: %s is not registered.", givename);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	new playername[MAX_NAME];
	GetPlayerName(playerid, playername, sizeof(playername));
	
	xp = GivePlayerXP(giveid, xp, 0);
	
	if (!xp) {
		format(message, sizeof(message), "GIVE XP: %s cannot get/lose any more XP.", pData[giveid][pLoginName]);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (playerid == giveid) {
		format(message, sizeof(message), "GIVE XP: You have given yourself %s XP.", kvalstr(xp));
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s has given themself %s XP", playername, kvalstr(xp));
	} else {
		format(message, sizeof(message), "GIVE XP: You have given %s %s XP.", pData[giveid][pLoginName], kvalstr(xp));
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		format(message, sizeof(message), "GIVE XP: You have been given %s XP by %s.", kvalstr(xp), playername);
		SendClientMessage(giveid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s has given %s %s XP", playername, pData[giveid][pLoginName], kvalstr(xp));
	}
	
	// Admin logs
	printf("[FDM] ADMIN: %s", message);
	AddAdminLogLine(COLOUR_LOG, message);
	return 1;
}

// Admin set player level
kcmd:asetlevel(playerid, text[]) {
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) {
		new message[MAX_INPUT];
		format(message, sizeof(message), "SET LEVEL: You must be at least admin level %i to use /asetlevel.", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /asetlevel [player] [level]");
		return 1;
	}
	
	new giveid;
	new level;
	new idx;
	
	giveid = FindPlayer(split(text, idx, ' '));
	level = strval(split(text, idx, ' '));
	new message[MAX_INPUT];
	
	if (giveid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "SET LEVEL: Invalid player id/name.");
		return 1;
	}
	if (!pData[giveid][pIsLoggedIn]) {
		new givename[MAX_NAME];
		GetPlayerName(giveid, givename, sizeof(givename));
		format(message, sizeof(message), "SET LEVEL: %s is not registered.", givename);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (!SetPlayerLevel(giveid, level)) {
		SendClientMessage(playerid, COLOUR_ERROR, "SET LEVEL: Invalid level.");
		return 1;
	}
	
	new playername[MAX_NAME];
	GetPlayerName(playerid, playername, sizeof(playername));
	
	if (playerid == giveid) {
		format(message, sizeof(message), "SET LEVEL: You have set your to level %i.", level);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s has set their FDM level to %i", playername, level);
	} else {
		format(message, sizeof(message), "SET LEVEL: You have set %s to level %i.", pData[giveid][pLoginName], level);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		format(message, sizeof(message), "SET LEVEL: You have been set to level %i by %s.", level, playername);
		SendClientMessage(giveid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s has set %s's FDM level to %i", playername, pData[giveid][pLoginName], level);
	}
	
	// Admin logs
	printf("[FDM] ADMIN: %s", message);
	AddAdminLogLine(COLOUR_LOG, message);
	return 1;
}

// Admin transfer accounts
kcmd:atransfer(playerid, text[]) {
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) {
		new message[MAX_INPUT];
		format(message, sizeof(message), "TRANSFER: You must be at least admin level %i to use /atransfer.", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /atransfer [player] [new playername]");
		return 1;
	}
	new tmpstr[MAX_INPUT];
	new idx;
	new transferid;
	
	splitcpy(tmpstr, text, idx, ' ');
	transferid = FindPlayer(tmpstr);
	
	if (transferid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "TRANSFER ACCOUNT: Invalid player ID.");
		return 1;
	}
	if (!IsPlayerConnected(transferid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "TRANSFER ACCOUNT: Invalid player ID.");
		return 1;
	}
	
	new oldname[MAX_NAME];
	new transfername[MAX_NAME];
	splitcpy(transfername, text, idx, ' ');
	strcpy(oldname, pData[transferid][pLoginName]);
	
	new message[MAX_INPUT];
	if (!pData[transferid][pIsLoggedIn]) {
		format(message, sizeof(message), "TRANSFER ACCOUNT: %s is not registered.", oldname);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	new len = strlen(transfername);
	if (len < 3 || len > 20) {
		SendClientMessage(playerid, COLOUR_ERROR, "TRANSFER ACCOUNT: The name must be between 3 and 20 characters long.");
		return 1;
	}
	
	new playername[MAX_NAME];
	GetPlayerName(playerid, playername, sizeof(playername));
	
	SetPlayerName(transferid, transfername);
	GetPlayerName(transferid, tmpstr, sizeof(tmpstr));
	
	if (!strcmp(oldname, tmpstr)) { // Player name still the same
		SendClientMessage(playerid, COLOUR_ERROR, "TRANSFER ACCOUNT: Invalid transfer name.");
		return 1;
	}
	
	INI_Remove(GetPlayerFilePath(oldname));
	strcpy(pData[transferid][pLoginName], transfername, MAX_NAME);
	SavePlayerAccount(transferid);
	
	// Save login IP address
	if (INI_Open(GetPlayerFilePath(pData[transferid][pLoginName]))) {
		new playerip[MAX_IP]; GetPlayerIp(playerid, playerip, sizeof(playerip));
		INI_WriteString(KEY_LASTIP, playerip);
		INI_Save();
		INI_Close();
	}
	
	// Transfer race records
	for (new a; a < MAX_RACE_MAPS; a++) {
		for (new b; b < MAX_RACE_RECORDS; b++) { // Find equal names first
			if (raceMapRecords[a][b][rcTime] && !strcmp(raceMapRecords[a][b][rcPlayer], oldname, true)) { // Find same name
				strcpy(raceMapRecords[a][b][rcPlayer], pData[transferid][pLoginName], MAX_NAME);
				SaveRaceMapStats(a);
				continue;
			}
		}
	}
	
	if (cData[transferid][cGangID] != INVALID_GANG_ID) { // Player is in gang
		
		strcpy(cData[transferid][cName], pData[transferid][pLoginName], MAX_NAME);
		
		new gangid = cData[transferid][cGangID];
		for (new i; i < MAX_GANG_MEMBERS; i++) {
			if (!gMembers[gangid][i][mName][0]) continue;
			if (!strcmp(gMembers[gangid][i][mName], oldname)) {
				strcpy(gMembers[gangid][i][mName], pData[transferid][pLoginName], MAX_NAME);
				SaveGang(gangid);
				break;
			}
		}
	}
	
	if (playerid == transferid) {
		format(message, sizeof(message), "TRANSFER ACCOUNT: You have transfered your account to %s.", pData[transferid][pLoginName]);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s has transfered their FDM account to %s", playername, pData[transferid][pLoginName]);
	} else {
		format(message, sizeof(message), "TRANSFER ACCOUNT: You have transfered %s's account to %s.", oldname, pData[transferid][pLoginName]);
		SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
		format(message, sizeof(message), "TRANSFER ACCOUNT: Your account has been transferred from %s to %s.", oldname, pData[transferid][pLoginName]);
		SendClientMessage(transferid, COLOUR_SUCCESSFUL, message);
		
		format(message, sizeof(message), "%s has transfered %s's FDM account to %s", playername, oldname, pData[transferid][pLoginName]);
	}
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdm_OnPlayerNameTransfer", "iss", transferid, oldname, pData[transferid][pLoginName]);
	#endif
	
	// Admin logs
	printf("[FDM] ADMIN: %s", message);
	AddAdminLogLine(COLOUR_LOG, message);
	return 1;
}

kcmd:agangcolour(playerid, text[]) {
	new message[MAX_INPUT];
	if (GetAdminLevel(playerid) < ADMIN_LEVEL_MISC) {
		format(message, sizeof(message), "GANG COLOUR: You must be at least admin level %i to use /agangcolour.", ADMIN_LEVEL_MISC);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	new tmpplayer[MAX_INPUT];
	new r, g, b, a, idx;
	
	splitcpy(tmpplayer, text, idx, ' ');
	splitcpy(message, text, idx, 0);
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /agangcolour [player] [r] [g] [b]");
		return 1;
	} else {
		if (ishex(message)) {
			new colour = hexstr(message);
			
			r = (colour & 0xFF0000) >> 16;
			g = (colour & 0x00FF00) >> 8;
			b = (colour & 0x0000FF);
			a = 0x60;
			
		} else {
			idx = 0;
			
			r = strval(split(message, idx, ' '));
			g = strval(split(message, idx, ' '));
			b = strval(split(message, idx, ' '));
			a = 0x60;
			
			if (!tmpplayer[0] ||
				r < 0 || r > 255 ||
				g < 0 || g > 255 ||
				b < 0 || b > 255 ||
				a < 0 || a > 255) {
				SendClientMessage(playerid, COLOUR_HELP, "USAGE: /agangcolour [player] [r] [g] [b]");
				return 1;
			}
		}
	}
	
	new changeid = FindPlayer(tmpplayer);
	
	if (changeid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "GANG COLOUR: Invalid player ID.");
		return 1;
	}
	
	new changename[MAX_NAME];
	GetPlayerName(changeid, changename, sizeof(changename));
	
	if (!pData[changeid][pIsLoggedIn]) {
		format(message, sizeof(message), "GANG COLOUR: %s must be registered to have a custom colour.", changename);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	if (cData[changeid][cGangID] == INVALID_GANG_ID) {
		format(message, sizeof(message), "GANG COLOUR: %s must be in a gang to have a custom colour.", changename);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	new colour = RGBAToHex(r, g, b, a);//(r * 0xF000000) + (g * 0xF0000) + (b * 0xF00) + (a);//IntHex(tmphex);
	
	// Change the gang colour
	gData[cData[changeid][cGangID]][gColour] = colour;
	LoopPlayers(p) { // Update every gang member's colour
		if (cData[p][cGangID] == cData[changeid][cGangID]) {
			SetPlayerColor(p, colour);
		}
	}
	SaveGang(cData[changeid][cGangID]);
	
	// Send player message of success
	new tmpstr[MAX_INPUT];
	format(tmpstr, sizeof(tmpstr), "GANG COLOUR: Gang colour changed for '%s'.", gData[cData[changeid][cGangID]][gName]);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, tmpstr);
	printf("[FDM] ADMIN: %s to %i %i %i", tmpstr, r, g, b);
	
	GetPlayerName(playerid, message, sizeof(message));
	format(message, sizeof(message), "%s changed the gang colour of '%s' (%s) to %i %i %i",
		message, gData[cData[changeid][cGangID]][gName], pData[changeid][pLoginName], r, g, b);
	AddAdminLogLine(COLOUR_LOG, message);
	return 1;
}

// Reload server settings
kcmd:reload(playerid, text[]) {
	new message[MAX_INPUT];
	
	if (!IsPlayerAdmin(playerid) && GetAdminLevel(playerid) < ADMIN_LEVEL_MASTER) {
		format(message, sizeof(message), "MINIMODES: You must be at least admin level %i or RCON to use /reload", ADMIN_LEVEL_MASTER);
		SendClientMessage(playerid, COLOUR_ERROR, message);
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /reload [all/levels/zones/weapons/vehicles/spawns/minimodes/specialty/other]");
		return 1;
	}
	
	new reloadall;
	if (!strcmp("all", text)) {
		reloadall = 1;
	} else {
		if (strcmp("levels", text) &&
			strcmp("zones", text) &&
			strcmp("weapons", text) &&
			strcmp("vehicles", text) &&
			strcmp("spawns", text) &&
			strcmp("minimodes", text) &&
			strcmp("specialty", text) &&
			strcmp("other", text)
			) {
			
			SendClientMessage(playerid, COLOUR_HELP, "USAGE: /reload [all/levels/zones/weapons/vehicles/spawns/minimodes/specialty/other]");
			return 1;
		}
	}
	
	GetPlayerName(playerid, message, sizeof(message));
	format(message, sizeof(message), "%s reloaded the server FDM settings (%s)", message, text);
	AddAdminLogLine(COLOUR_LOG, message);
	
	format(message, sizeof(message), "[FDM] Server settings (%s) reloading....", text);
	print(message);
	SendClientMessageToAll(COLOUR_HELP, message);
	new time = GetTickCount();
	
	// Temporarily hold anti-cheat - it can lag players enough to make them suspected
	//if (anticheatTimer) KillTimer(anticheatTimer);
	//SetTimer("fdm_AntiCheatEnableSync", 10000, false); // Compensates for time taken to start up
	kEnableTimer(1);
	
	// Reload player levels
	if (reloadall || !strcmp("levels", text)) {
		print("[FDM] Reloading levels list..");
		UnloadLevelsList();
		LoadLevelsList();
		LoopPlayersEx(i, bb) {
			if (!pData[i][pIsLoggedIn]) continue;
			UpdatePlayerLevel(i);
		}
	}
	
	// Reload XP zones
	if (reloadall || !strcmp("zones", text)) {
		print("[FDM] Reloading zones..");
		UnloadXPZones();
		LoadXPZones();
	}
	
	// Reload weapon levels
	if (reloadall || !strcmp("weapons", text)) {
		print("[FDM] Reloading weapons..");
		UnloadWeaponSettings();
		LoadWeaponSettings();
	}
	
	// Reload map vehicles
	if (reloadall || !strcmp("vehicles", text)) {
		print("[FDM] Reloading map vehicles..");
		VehiclesUnload();
		VehiclesInit();
	}
	
	// Reload spawn points
	if (reloadall || !strcmp("spawns", text)) {
		print("[FDM] Reloading spawn points..");
		SpawnsInit();
		SpawnsUnload();
	}
	
	// Reload minimodes
	if (reloadall || !strcmp("minimodes", text)) {
		print("[FDM] Reloading minimodes..");
		//SidechatExit();
		//SpecExit();
		MinimodesUnload();
		MinimodesInit();
		INI_Open(FILE_OTHER_SETTINGS);
		MinimodesSettingsInit();
		//SpecInit();
		//SidechatInit();
		LoopPlayers(p) {
			if (!pData[playerid][pIsLoggedIn]) continue;
			sidechat_OnPlayerLogin(p);
		}
	}
	
	// Reload autologins
	/*
	if (reloadall || !strcmp("logins", text)) {
		print("[FDM] Reloading autologins..");
		LoopPlayers(p) {
			if (pData[playerid][pIsLoggedIn]) continue;
			players_OnPlayerRequestClass(p, 1);
		}
	}
	*/
	
	// Reload specialties
	if (reloadall || !strcmp("specialty", text)) {
		print("[FDM] Reloading specialties..");
		SpecialtyExit();
		SpecialtyInit();
	}
	
	// Reload others
	if (reloadall || !strcmp("other", text)) {
		print("[FDM] Reloading other..");
		INI_Open(FILE_OTHER_SETTINGS);
		DriveByInit();
		PassengerGlitchExit();
		PassengerGlitchInit();
		CombatExit();
		CombatInit();
		MinimodesSettingsInit();
		AntiCheatSettingsInit();
	}
	
	#if ENABLE_CALLBACKS
		CallRemoteFunction("fdmOnReloadData", "s", text);
	#endif
	
	printf("[FDM] Server settings reload successful. Time Taken: %ims.", GetTickCount() - time);
	SendClientMessageToAll(COLOUR_SUCCESSFUL, "[FDM] Server settings reload successful.");
	return 1;
}
	
// Emo
kcmd:emo(playerid, text[]) {
	if (!IsPlayerSpawned(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "EMO: You must be spawned.");
		return 1;
	}
	
	new Float:hp;
	GetPlayerHealth(playerid, hp);
	hp = hp - 15.0;
	if (hp <= 0.0) {
		KillPlayer(playerid, WEAPON_PARACHUTE);
		SendClientMessage(playerid, COLOUR_HELP, "EMO: You emo'd yourself to death.");
	} else {
		SetPlayerHealth(playerid, hp);
		SendClientMessage(playerid, COLOUR_HELP, "EMO: You have cut yourself, it hurts.");
	}
	return 1;
}

// Me
kcmd:me(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /me [text]");
		return 1;
	}
	
	new tmpstr[MAX_INPUT];
	new playername[MAX_NAME];
	GetPlayerName(playerid, playername, sizeof(playername));
	format(tmpstr, sizeof(tmpstr), "%s %s", playername, text);
	SendWrappedMessageToAll(0xFF99CCAA, tmpstr);
	HistoryAddLine(0xFF99CCAA, tmpstr);
	return 1;
}

// Help
kcmd:help(playerid, text[]) {
	SendClientMessage(playerid, COLOUR_TITLE, "Help:");
	SendClientMessage(playerid, COLOUR_HELP, "To have your stats saved, you must register - /register [password]");
	SendClientMessage(playerid, COLOUR_HELP, "To view your objective - /objective.");
	SendClientMessage(playerid, COLOUR_HELP, "To view your stats and level - /stats.");
	SendClientMessage(playerid, COLOUR_HELP, "To all other commands on the server - /commands.");
	return 1;
}

// Objective
kcmd:objective(playerid, text[]) {
	SendClientMessage(playerid, COLOUR_TITLE, "Objective:");
	
	SendClientMessage(playerid, COLOUR_HELP, "Your objective is to gain experience to attain higher levels.");
	
	SendClientMessage(playerid, COLOUR_HELP, "To gain levels, you can:");
	SendClientMessage(playerid, COLOUR_HELP, " - Kill other people who have experience.");
	SendClientMessage(playerid, COLOUR_HELP, " - Do minimodes such as races and deathmatches.");
	SendClientMessage(playerid, COLOUR_HELP, " - Hold experience zone areas.");
	
	SendClientMessage(playerid, COLOUR_HELP, "The rewards of gaining levels is:");
	SendClientMessage(playerid, COLOUR_HELP, " - More access to minimodes.");
	SendClientMessage(playerid, COLOUR_HELP, " - Better spawn weapons.");
	SendClientMessage(playerid, COLOUR_HELP, " - More spawn armour.");
	SendClientMessage(playerid, COLOUR_HELP, " - Improved specialties.");
	SendClientMessage(playerid, COLOUR_HELP, " - And a few other fancy stuff.");
	
	SendClientMessage(playerid, COLOUR_HELP, "To have your stats saved, you must register - /register [password]");
	
	SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	return 1;
}

// Commands
kcmd:cmds(playerid, text[]) return kcmd_commands(playerid, text);
kcmd:commands(playerid, text[]) {
	if (isnull(text)) {
		// List all commands
		SendClientMessage(playerid, COLOUR_TITLE, "Commands: (For help on any command, use /cmdhelp [command])");
		
		new msg[1024] = WRAPPED_MESSAGE_PREFIX,
			tmpstr[MAX_INPUT],
			adminlevel = GetAdminLevel(playerid);
		
		for (new i; i < sizeof(cmdList); i++) {
			if (!cmdList[i][cmdIsVariant] && adminlevel >= cmdList[i][cmdAdminLevel]) {
				format(tmpstr, sizeof(tmpstr), "/%s ", cmdList[i][cmdCommand]);
				strcat(msg, tmpstr);
			}
		}
		
		new len = strlen(msg);
		if (len) {
			msg[len-1] = 0;
			SendWrappedMessageToPlayer(playerid, COLOUR_HELP, msg, 128);
		}
		
	} else {
		// List a command set
		new cmdset;
		if (!strcmp("login", text, true)) {
			cmdset = CMD_SET_LOGIN;
		} else if (!strcmp("gang", text, true)) {
			cmdset = CMD_SET_GANG;
		} else if (!strcmp("money", text, true)) {
			cmdset = CMD_SET_MONEY;
		} else if (!strcmp("weapon", text, true)) {
			cmdset = CMD_SET_WEAPONS;
		} else if (!strcmp("minimodes", text, true)) {
			cmdset = CMD_SET_MINIMODES;
		} else if (!strcmp("specialty", text, true)) {
			cmdset = CMD_SET_SPECIALTY;
		} else if (!strcmp("misc", text, true)) {
			cmdset = CMD_SET_MISC;
		} else if (!strcmp("admin", text, true)) {
			cmdset = CMD_SET_ADMIN;
		} else {
			SendClientMessage(playerid, COLOUR_HELP, "USAGE: /commands (login/gang/money/weapon/minimodes/specialty/misc/admin)");
			return 1;
		}
		
		new msg[MAX_INPUT];
		
		format(msg, sizeof(msg), "Commands List (%s):", text);
		SendClientMessage(playerid, COLOUR_TITLE, msg);
		
		SendCommandSetToPlayer(playerid, cmdset);
	}
	return 1;
}

kcmd:cmdhelp(playerid, text[]) return kcmd_commandhelp(playerid, text);
kcmd:commandhelp(playerid, text[]) {
	//{CMD_SET_MISC, 0, 0, "commandhelp", "[command]", "Shows help on a command (Short: /cmdhelp)."},
	if (isnull(text) || (text[0] == '/' && !text[1])) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /commandhelp [command]");
		return 1;
	}
	
	new msg[MAX_INPUT],
		count,
		adminlevel = GetAdminLevel(playerid),
		alternate;
	
	if (text[0] == '/') {
		alternate = 1;
	}
	
	SendClientMessage(playerid, COLOUR_TITLE, "Commands Found:");
	
	for (new i; i < sizeof(cmdList); i++) {
		if (adminlevel >= cmdList[i][cmdAdminLevel] &&
			(!alternate && strfind(cmdList[i][cmdCommand], text, true) != -1) ||
			(alternate && !strcmp(cmdList[i][cmdCommand], text[1], true, strlen(text) - 1))
			) {
			
			if (!cmdList[i][cmdParams][0]) {
				format(msg, sizeof(msg), "/%s - %s", cmdList[i][cmdCommand], cmdList[i][cmdDescription]);
			} else {
				format(msg, sizeof(msg), "/%s %s - %s", cmdList[i][cmdCommand], cmdList[i][cmdParams], cmdList[i][cmdDescription]);
			}
			
			SendClientMessage(playerid, COLOUR_HELP, msg);
			count++;
		}
	}
	
	if (!count) {
		SendClientMessage(playerid, COLOUR_HELP, "No matching commands found.");
	} else if (count >= 8) {
		SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
	}
	return 1;
}
