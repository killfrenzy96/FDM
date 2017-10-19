#include <a_samp>

#define COLOUR_MONEYGAIN 0xDDFF11AA
#define COLOUR_MONEYLOSS 0xFF1111AA

#define MONEY_STEAL_DIVISOR 2
#define MONEY_REGISTER 15000
#define MAX_BANK 500000000

enum bankLocationInfo {
	bCheckpointID,
	Float:bX,
	Float:bY,
	Float:bZ
}
new bankLocations[][bankLocationInfo] = {
	{INVALID_CHECKPOINT_ID, 368.0807, -66.4260, 1001.5078}, // Burger shot
	{INVALID_CHECKPOINT_ID, 379.5497,-119.5294, 1001.4922}, // Pizza
	{INVALID_CHECKPOINT_ID, 379.3677, -8.6247, 1001.8516}, // Clukin bell
	{INVALID_CHECKPOINT_ID, -23.1598, -55.5494, 1003.5469}, // 24/7
	{INVALID_CHECKPOINT_ID, -30.8613, -28.7209, 1003.5573}, // 24/7
	{INVALID_CHECKPOINT_ID, -27.9776, -89.9459, 1003.5469} // 24/7
};

MoneyInit() {
	for (new i; i < sizeof(bankLocations); i++) { // Create bank checkpoints
		bankLocations[i][bCheckpointID] = AddCheckpoint(bankLocations[i][bX], bankLocations[i][bY], bankLocations[i][bZ], 10.0);
	}
	return 1;
}

money_OnPlayerRegister(playerid) {
	GivePlayerMoney(playerid, MONEY_REGISTER);
	return 1;
}

money_OnPlayerDeath(playerid, killerid) {
	// Transfer cash
	if (GetPlayerMoney(playerid) <= 0) {
		// Negative player money
		return 1;
	}
	if (killerid == INVALID_PLAYER_ID) return 1;
	
	// Transfer the money
	new transfermoney;
	transfermoney = GetPlayerMoney(playerid) / MONEY_STEAL_DIVISOR;
	
	GivePlayerMoney(playerid, -transfermoney);
	GivePlayerMoney(killerid, transfermoney);
	SendMoneyMessageToPlayer(playerid, -transfermoney);
	SendMoneyMessageToPlayer(killerid, transfermoney);
	return 1;
}

money_OnPlayerEnterCheckpoint(playerid, checkpointid) { //TODO
	for (new i; i < sizeof(bankLocations); i++) { // Check if it was a bank
		if (bankLocations[i][bCheckpointID] == checkpointid) {
			// Send the player bank information
			new message[MAX_INPUT];
			SendClientMessage(playerid, COLOUR_TITLE, "Bank:");
			format(message, sizeof(message), "There is $%s in your bank. The bank commands are:", kvalstr(pData[playerid][pBankCash]));
			SendClientMessage(playerid, COLOUR_HELP, message);
			//SendClientMessage(playerid, COLOUR_HELP, "/balance - Check the amount of cash you have.");
			SendClientMessage(playerid, COLOUR_HELP, "/bank [money] - Bank a given amount of money.");
			SendClientMessage(playerid, COLOUR_HELP, "/withdraw [money] - Withdraw a given amount of money from the bank.");
			return 0;
		}
	}
	return 1;
}

IsPlayerAtBank(playerid) {
	for (new i; i < sizeof(bankLocations); i++) {
		if (IsPlayerInCheckpointID(playerid, bankLocations[i][bCheckpointID])) return 1;
	}
	return 0;
}

stock SendMoneyMessageToPlayer(playerid, money, type = 0) {
	// type = 0 = text message
	// type = 1 = big text
	new message[MAX_INPUT];
	
	if (!money) return 0;
	if (money > 0) {
		if (!type) {
			format(message, sizeof(message), "You have gained $%s", kvalstr(money));
			SendClientMessage(playerid, COLOUR_MONEYGAIN, message);
		} else {
			format(message, sizeof(message), "+$%s", kvalstr(money));
			GameTextForPlayer(playerid, message, 1, 1);
		}
	} else {
		if (!type) {
			format(message, sizeof(message), "You have lost $%s", kvalstr(-money));
			SendClientMessage(playerid, COLOUR_MONEYLOSS, message);
		} else {
			format(message, sizeof(message), "-$%s", kvalstr(-money));
			GameTextForPlayer(playerid, message, 1, 1);
		}
	}
	return 1;
}

kcmd:balance(playerid, text[]) {
	new message[MAX_INPUT];
	format(message, sizeof(message), "You have $%s in your bank.", kvalstr(pData[playerid][pBankCash]));
	SendClientMessage(playerid, COLOUR_HELP, message);
	return 1;
}

kcmd:bank(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /bank [money]");
		return 1;
	}
	if (!IsPlayerAtBank(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "BANKS: You must be at a bank to use this command.");
		return 1;
	}
	new amount = strval(text);
	if (amount <= 0) {
		if (!strcmp(text, "all", true)) {
			amount = GetPlayerMoney(playerid);
		} else {
			SendClientMessage(playerid, COLOUR_ERROR, "BANKS: Invalid bank cash amount.");
			return 1;
		}
	}
	
	new playermoney = GetPlayerMoney(playerid);
	if (playermoney <= 0) {
		SendClientMessage(playerid, COLOUR_ERROR, "BANKS: You don't have any cash on you.");
		return 1;
	}
	
	if (amount > playermoney) amount = playermoney; // Do not allow them to bank more than their cash on hand
	if (amount + pData[playerid][pBankCash] > MAX_BANK) amount = MAX_BANK - pData[playerid][pBankCash];
	
	if (amount == 0) {
		SendClientMessage(playerid, COLOUR_ERROR, "BANKS: Your bank is full.");
		return 1;
	}
	
	// Perform transaction
	pData[playerid][pBankCash] = pData[playerid][pBankCash] + amount;
	GivePlayerMoney(playerid, -amount);
	
	new message[MAX_INPUT];
	format(message, sizeof(message), "BANKS: $%s banked. There is $%s in your bank.", kvalstr(amount), kvalstr(pData[playerid][pBankCash]));
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	
	return 1;
}

kcmd:withdraw(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /withdraw [money]");
		return 1;
	}
	if (!IsPlayerAtBank(playerid)) {
		SendClientMessage(playerid, COLOUR_ERROR, "BANKS: You must be at a bank to use this command.");
		return 1;
	}
	new amount = strval(text);
	if (amount <= 0) {
		if (!strcmp(text, "all", true)) {
			amount = pData[playerid][pBankCash];
		} else {
			SendClientMessage(playerid, COLOUR_ERROR, "BANKS: Invalid withdraw cash amount.");
			return 1;
		}
	}
	
	if (amount > pData[playerid][pBankCash]) amount = pData[playerid][pBankCash]; // Do not allow them to withdraw more than they have in bank
	
	if (!amount) {
		SendClientMessage(playerid, COLOUR_ERROR, "BANKS: You have no cash in your bank.");
		return 1;
	}
	
	// Perform transaction
	pData[playerid][pBankCash] = pData[playerid][pBankCash] - amount;
	GivePlayerMoney(playerid, amount);
	
	new message[MAX_INPUT];
	format(message, sizeof(message), "BANKS: $%s withdrawn. There is $%s in your bank", kvalstr(amount), kvalstr(pData[playerid][pBankCash]));
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	
	return 1;
}

kcmd:givecash(playerid, text[]) {
	if (isnull(text)) {
		SendClientMessage(playerid, COLOUR_HELP, "USAGE: /givecash [player] [money]");
		return 1;
	}
	new giveid;
	new money;
	new idx;
	giveid = FindPlayer(split(text, idx, ' '));
	money = strval(split(text, idx, ' '));
	
	if (giveid == INVALID_PLAYER_ID) {
		SendClientMessage(playerid, COLOUR_ERROR, "GIVE MONEY: Player cannot be found.");
		return 1;
	}
	if (giveid == playerid) {
		SendClientMessage(playerid, COLOUR_ERROR, "GIVE MONEY: You cannot give money to yourself.");
		return 1;
	}
	if (money <= 0) {
		SendClientMessage(playerid, COLOUR_ERROR, "GIVE MONEY: Invalid cash amount.");
		return 1;
	}
	
	new playermoney = GetPlayerMoney(playerid);
	if (playermoney <= 0) {
		SendClientMessage(playerid, COLOUR_ERROR, "GIVE MONEY: You don't have any cash on you.");
		return 1;
	}
	
	if (money > playermoney) money = playermoney;
	
	// Perform transaction
	GivePlayerMoney(giveid, money);
	GivePlayerMoney(playerid, -money);
	
	new message[MAX_INPUT];
	new playername[MAX_NAME];
	new givename[MAX_NAME];
	GetPlayerName(playerid, playername, sizeof(playername));
	GetPlayerName(giveid, givename, sizeof(givename));
	format(message, sizeof(message), "GIVE MONEY: You have given $%s to %s.", kvalstr(money), givename);
	SendClientMessage(playerid, COLOUR_SUCCESSFUL, message);
	format(message, sizeof(message), "GIVE MONEY: You have recieved $%s from %s.", kvalstr(money), playername);
	SendClientMessage(giveid, COLOUR_SUCCESSFUL, message);
	return 1;
}