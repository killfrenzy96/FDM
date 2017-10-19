#include <a_samp>

#define BOUNTY_PLAYERS_PER_LINE 4
#define MIN_BOUNTY 1000
#define MAX_BOUNTY 100000000

/*bounty_OnPlayerLogin(playerid) {
	if (pData[playerid][pBounty]) {
		new msg[MAX_INPUT];
		format(msg, sizeof(msg), "*** There is a bounty of $%s on %s!", kvalstr(pData[playerid][pBounty]), pData[playerid][pLoginName]);
		SendClientMessageToAll(COLOUR_ATTENTION, msg);
	}
	return 1;
}*/

bounty_OnPlayerDeath(playerid, killerid) {
	if (killerid == INVALID_PLAYER_ID) return 1;
	if (!pData[playerid][pIsLoggedIn]) return 1;
	if (!pData[killerid][pIsLoggedIn] && pData[playerid][pBounty]) {
		SendClientMessage(killerid, COLOUR_ERROR, "BOUNTIES: You must be registered to recieve bounties. (/register [password])");
		return 1;
	}
	
	if (pData[playerid][pBounty]) {
		
		if (cData[playerid][cGangID] != INVALID_GANG_ID &&
			cData[playerid][cGangID] == cData[killerid][cGangID]) {
			
			SendClientMessage(killerid, COLOUR_ERROR, "BOUNTIES: You must not be in the same gang to recieve the bounty.");
			return 1;
		}
		
		new msg[MAX_INPUT];
		format(msg, sizeof(msg), "*** %s recieved $%s off a bounty from %s!",
			pData[killerid][pLoginName], kvalstr(pData[playerid][pBounty]), pData[playerid][pLoginName]);
		SendClientMessageToAll(COLOUR_SUCCESSFUL, msg);
		GivePlayerMoney(killerid, pData[playerid][pBounty]);
		SendMoneyMessageToPlayer(killerid, pData[playerid][pBounty]);
		pData[playerid][pBounty] = 0;
		return 1;
	}
	
	return 1;
}

kcmd:hitman(playerid, text[]) {
	if (!pData[playerid][pIsLoggedIn]) {
		SendClientMessage(playerid, COLOUR_ERROR, "HITMAN: You must be regitered to use /hitman. (/register [password])");
		return 1;
	}
	
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /hitman [player] [money]");
		return 1;
	}
	
	new targetid, money, idx;
	targetid = FindPlayer(split(text, idx, ' '));
	money = strval(split(text, idx, ' '));
	
	if (targetid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "HITMAN: Invalid player ID/Name.");
		return 1;
	}
	
	if (targetid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "HITMAN: You cannot place a bounty on yourself.");
		return 1;
	}
	
	new msg[MAX_INPUT];
	
	if (!pData[targetid][pIsLoggedIn]) {
		GetPlayerName(targetid, msg, sizeof(msg));
		format(msg, sizeof(msg), "HITMAN: %s must be registered.", msg);
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	if (money < MIN_BOUNTY) {
		format(msg, sizeof(msg), "HITMAN: You cannot place a bounty less than $%s.", kvalstr(MIN_BOUNTY));
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	if (money > MAX_BOUNTY) {
		format(msg, sizeof(msg), "HITMAN: You cannot place a bounty greater than $%s.", kvalstr(MAX_BOUNTY));
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	if (GetPlayerMoney(playerid) < money) {
		format(msg, sizeof(msg), "HITMAN: You don't have enough money to place a bounty of $%s.", kvalstr(money));
		SendClientMessage(playerid, COLOUR_ERROR, msg);
		return 1;
	}
	
	if (pData[targetid][pBounty] + money > MAX_BOUNTY) {
		money = MAX_BOUNTY - pData[targetid][pBounty];
		if (!money) {
			format(msg, sizeof(msg), "HITMAN: Maximum bounty for %s reached.", pData[targetid][pLoginName]);
			SendClientMessage(playerid, COLOUR_ERROR, msg);
			return 1;
		}
	}
	
	// Place bounty
	pData[targetid][pBounty] = pData[targetid][pBounty] + money;
	GivePlayerMoney(playerid, -money);
	
	if (pData[targetid][pBounty] == money) {
		format(msg, sizeof(msg), "*** %s has placed a bounty of $%s on %s!", pData[playerid][pLoginName], kvalstr(money), pData[targetid][pLoginName]);
	} else {
		format(msg, sizeof(msg), "*** %s has placed a bounty of $%s on %s! (Total: $%s)", pData[playerid][pLoginName], kvalstr(money), pData[targetid][pLoginName], kvalstr(pData[targetid][pBounty]));
	}
	SendClientMessageToAll(COLOUR_ATTENTION, msg);
	return 1;
}

kcmd:bounties(playerid, text[]) {
	SendClientMessage(playerid, COLOUR_TITLE, "Bounties:");
	
	new linestr[MAX_INPUT];
	new linecount;
	new totallines;
	
	LoopPlayers(i) {
		if (!pData[i][pIsLoggedIn]) continue;
		if (!pData[i][pBounty]) continue;
		
		if (linecount >= BOUNTY_PLAYERS_PER_LINE) {
			SendClientMessage(playerid, COLOUR_HELP, linestr);
			strclr(linestr);
			linecount = 0;
			totallines++;
		}
		
		if (!linestr[0]) {
			format(linestr, sizeof(linestr), "%s ($%s),", pData[i][pLoginName], kvalstr(pData[i][pBounty]));
		} else {
			format(linestr, sizeof(linestr), "%s  %s ($%s),", linestr, pData[i][pLoginName], kvalstr(pData[i][pBounty]));
		}
		linecount++;
		
	}
	
	if (linestr[0]) {
		linestr[strlen(linestr) - 1] = 0;
		SendClientMessage(playerid, COLOUR_HELP, linestr);
		totallines++;
		
		if (totallines > 8) {
			SendClientMessage(playerid, COLOUR_TITLE, "Use PGUP and PGDN to scroll the chatbox.");
		}
		
	} else {
		SendClientMessage(playerid, COLOUR_HELP, "No bounties avaliable.");
	}
	
	return 1;
}

