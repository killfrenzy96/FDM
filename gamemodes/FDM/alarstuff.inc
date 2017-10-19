
new pAdminLevel[MAX_PLAYERS];

alarInit() {
	LoopPlayers(i) {
		pAdminLevel[i] = GetAdminLevel(i);
	}
	SetAlarLoginCheck("fdm_IsPlayerLoggedIn");
	SetSpawnType(SPAWN_ALAR_SPAWN, INFOTYPE_POSITION | INFOTYPE_WORLD | INFOTYPE_WEAPONS | INFOTYPE_HEALTH | INFOTYPE_ARMOUR | INFOTYPE_VEHICLE);
	return 1;
}

public OnAlarInit() {
	SetAlarLoginCheck("fdm_IsPlayerLoggedIn");
	SetSpawnType(SPAWN_ALAR_SPAWN, INFOTYPE_POSITION | INFOTYPE_WORLD | INFOTYPE_WEAPONS | INFOTYPE_HEALTH | INFOTYPE_ARMOUR | INFOTYPE_VEHICLE);
	return 1;
}

alarOnPlayerDisconnect(playerid) {
	pAdminLevel[playerid] = 0;
	return 1;
}

alarOnPlayerUpdate(playerid)
{
	if (kGetAdminState(playerid) & ADMIN_STATE_DESYNCED) {
		return 0;
	} else {
		return 1;
	}
}

public OnAdminLogin(playerid, level) {
	pAdminLevel[playerid] = level;
	return 1;
}

public OnAdminLogout(playerid) {
	pAdminLevel[playerid] = 0;
	return 1;
}

/*#if defined SetPlayerVirtualWorld
	#undef SetPlayerVirtualWorld
#endif*/
#if defined GetAdminLevel
	#undef GetAdminLevel
#endif

stock GetAdminLevel(playerid) return pAdminLevel[playerid];
