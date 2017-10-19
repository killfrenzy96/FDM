#include <a_samp>

enum playerCombatInfo {
	pcEndCombatTick,
	pcCombatPlayer,
	pcUpdated
}
new pCombatData[MAX_PLAYERS][playerCombatInfo];
new combatBlockTeleport;
//new combatTimer;


CombatInit() {
	combatBlockTeleport = INI_ReadInt("BlockTeleportInCombat");
	
	for (new i; i < MAX_PLAYERS; i++) {
		pCombatData[i][pcCombatPlayer] = INVALID_PLAYER_ID;
		pCombatData[i][pcEndCombatTick] = 0;
		pCombatData[i][pcUpdated] = 0;
	}
	
	kEnableTimer(2); // combatTimer = SetTimer("fdm_CombatUpdate", 100, true);
	return 1;
}

CombatExit() {
	kDisableTimer(2); // KillTimer(combatTimer);
	return 1;
}

combat_OnPlayerDisconnect(playerid) {
	CombatResetPlayer(playerid);
	return 1;
}

combat_OnPlayerDeath(playerid) {
	CombatResetPlayer(playerid);
	return 1;
}

combat_OnPlayerUpdate(playerid) {
	pCombatData[playerid][pcUpdated] = 1;
	return 1;
}

CombatResetPlayer(playerid) {
	LoopPlayers(i) {
		if (pCombatData[i][pcCombatPlayer] == playerid) {
			pCombatData[i][pcCombatPlayer]= INVALID_PLAYER_ID;
			pCombatData[i][pcEndCombatTick] = 0;
			pCombatData[i][pcUpdated] = 0;
		}
	}
	
	pCombatData[playerid][pcCombatPlayer] = INVALID_PLAYER_ID;
	pCombatData[playerid][pcEndCombatTick] = 0;
	pCombatData[playerid][pcUpdated] = 0;
	return 1;
}

GetPlayerCombatPlayer(playerid) {
	if (pCombatData[playerid][pcCombatPlayer] == INVALID_PLAYER_ID || pGodData[playerid][pgIsOnGodmode]) return INVALID_PLAYER_ID;
	
	if (pCombatData[playerid][pcEndCombatTick] && pCombatData[playerid][pcEndCombatTick] > GetTickCount()) {
		return pCombatData[playerid][pcCombatPlayer];
	} else {
		return INVALID_PLAYER_ID;
	}
}

forward fdm_CombatUpdate();
public fdm_CombatUpdate() {
	LoopPlayers(playerid) {
		if (!pCombatData[playerid][pcUpdated] || !IsPlayerSpawned(playerid)) continue;
		
		new keys, updown, leftright, pstate;
		GetPlayerKeys(playerid, keys, updown, leftright);
		pstate = GetPlayerState(playerid);
		
		if (pstate == PLAYER_STATE_ONFOOT) {
			if (keys & KEY_FIRE) {
				new combatid = GetPlayerNearestCombatPlayer(playerid);
				if (combatid != INVALID_PLAYER_ID) UpdateCombatTick(playerid, combatid, 10000);
			} else if (keys & KEY_HANDBRAKE) {
				new combatid = GetPlayerNearestCombatPlayer(playerid);
				if (combatid != INVALID_PLAYER_ID) UpdateCombatTick(playerid, combatid, 5000);
			}
		} else if (pstate == PLAYER_STATE_DRIVER || pstate == PLAYER_STATE_PASSENGER) {
			if (keys & KEY_FIRE || keys & KEY_ACTION) {
				new combatid = GetPlayerNearestCombatPlayer(playerid);
				if (combatid != INVALID_PLAYER_ID) UpdateCombatTick(playerid, combatid, 10000);
			}
		}
		
		pCombatData[playerid][pcUpdated] = 0;
	}
	return 1;
}

UpdateCombatTick(playerid, combatid, time) {
	new combattick = GetTickCount() + time;
	if (!pCombatData[combatid][pcEndCombatTick] || combattick > pCombatData[combatid][pcEndCombatTick]) {
		pCombatData[combatid][pcEndCombatTick] = combattick;
	}
	if (!pCombatData[playerid][pcEndCombatTick] || combattick > pCombatData[playerid][pcEndCombatTick]) {
		pCombatData[playerid][pcEndCombatTick] = combattick;
	}
	pCombatData[combatid][pcCombatPlayer] = playerid;
	pCombatData[playerid][pcCombatPlayer] = combatid;
	
	return 1;
}

// ------------------------------------------------------------------------------------------------

static Float:weaponrange[] = {
	1.6,
	1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6,
	1.6, 1.6, 1.6, 1.6, 1.6, 40.0, 40.0, 40.0, 0.0, 0.0,
	0.0, 35.0, 35.0, 35.0, 40.0, 35.0, 40.0, 35.0, 45.0, 70.0,
	90.0, 35.0, 100.0, 100.0, 55.0, 55.0, 5.1, 75.0, 40.0, 0.0,
	6.1, 10.1, 0.0, 0.0, 0.0, 1.6
};

IsRunningWeapon(weaponid) {
	switch (weaponid) {
		case 22,26,28,32: return 1;
	}
	return 0;
}

GetPlayerNearestCombatPlayer(playerid) {
	
	new pstate = GetPlayerState(playerid);
	
	if (pstate == PLAYER_STATE_ONFOOT) { // Weaponry
		
		new Float:px, Float:py, Float:pz, Float:protz, pworld, Float:pweaponrange, pweaponid, pisrunningwpn,
			Float:tx, Float:ty, Float:tz,
			Float:distance;
		
		GetPlayerPos(playerid, px, py, pz);
		GetPlayerFacingAngle(playerid, protz);
		pworld = GetPlayerVirtualWorld(playerid);
		pweaponid = GetPlayerWeapon(playerid);
		pweaponrange = weaponrange[pweaponid] * 2;
		
		if (pweaponrange == 0.0) {
			return INVALID_PLAYER_ID;
		} else {
			pweaponrange = pweaponrange + 5.0;
		}
		
		pisrunningwpn = IsRunningWeapon(pweaponid);
		
		new closestplayer = INVALID_PLAYER_ID;
		new Float:closest;
		
		LoopPlayers(i) {
			if (playerid != i && IsPlayerSpawned(i) && GetPlayerVirtualWorld(i) == pworld && !pGodData[i][pgIsOnGodmode] && (cData[playerid][cGangID] == INVALID_GANG_ID || cData[playerid][cGangID] != cData[i][cGangID])) {
				GetPlayerPos(i, tx, ty, tz);
				distance = FindDistance(px, py, pz, tx, ty, tz);
				
				if (pisrunningwpn) {
					// Distance check
					if (distance < pweaponrange ||
						(IsPlayerInAnyVehicle(playerid) && distance < pweaponrange * 2.0) &&
						(closestplayer == INVALID_PLAYER_ID || distance < closest)) {
						
						closest = distance;
						closestplayer = i;
					}
				} else {
					// Angle check
					new Float:angle = ReturnAngle(px, py, tx, ty) - 90.0,
						Float:angledifference = AngDif(protz, angle);
					
					if (angledifference < 0.0) angledifference = angledifference * -1;
					if (angledifference == 0.0) angledifference = 0.00001;
					
					new Float:multiplier = 300.0;
					if (IsPlayerInAnyVehicle(i)) multiplier = 500.0;
					
					if (distance < pweaponrange &&
						(distance < (1.0 / angledifference) * multiplier) &&
						(closestplayer == INVALID_PLAYER_ID || angledifference < closest)) {
						
						closest = angledifference;
						closestplayer = i;
					}
				}
			}
		}
		
		return closestplayer;
		
	} else if (pstate == PLAYER_STATE_DRIVER) { // Vehicle weapons / driveby
		
		new Float:px, Float:py, Float:pz, Float:protz, pworld, pvehicleid, pvehiclemodel, pvehiclefiretype,
			Float:tx, Float:ty, Float:tz,
			Float:distance;
		
		pworld = GetPlayerVirtualWorld(playerid);
		pvehicleid = GetPlayerVehicleID(playerid);
		pvehiclemodel = GetVehicleModel(pvehicleid);
		//GetPlayerPos(playerid, px, py, pz);
		GetVehiclePos(pvehicleid, px, py, pz);
		GetVehicleZAngle(pvehicleid, protz);
		
		switch (pvehiclemodel) {
			case
				400..416,418..424,426..429,431,433..434,436..440,
				442..446,451,452,455..459,466,467,470..475,477..480,
				482,483,485,486,489..496,498..500,502..508,514..518,
				524..531,533..536,539..547,549..552,554..562,
				565..568,571..576,579,580,582,583,585,587,588,589,
				596..605,609
				: { // Side attack
				
				pvehiclefiretype = 1;
			}
				
			case 425,447,464,476,520
				: { // Forward attack
				
				pvehiclefiretype = 2;
			}
			
			case 430,461..463,468,481,509,510,521..523,581,586
				: { // Forward/side attack
				
				pvehiclefiretype = 3;
			}
			case 432
				: { // 360 attack
				
				pvehiclefiretype = 4;
			}
			default: { // No attack
				return INVALID_PLAYER_ID;
			}
		}
		
		new closestplayer = INVALID_PLAYER_ID;
		new Float:closest;
		
		LoopPlayers(i) {
			if (playerid != i && IsPlayerSpawned(i) && GetPlayerVirtualWorld(i) == pworld && GetPlayerVehicleID(i) != pvehicleid && !pGodData[i][pgIsOnGodmode] && (cData[playerid][cGangID] == INVALID_GANG_ID || cData[playerid][cGangID] != cData[i][cGangID])) {
				GetPlayerPos(i, tx, ty, tz);
				distance = FindDistance(px, py, pz, tx, ty, tz);
				
				if (distance < 120.0) {
					
					switch (pvehiclefiretype) {
						case 1: { // Side attack
							new Float:angle,
								Float:angledifference1,
								Float:angledifference2;
							
							angle = ReturnAngle(px, py, tx, ty) - 180.0;
							angledifference1 = AngDif(protz, angle);
							
							angle = ReturnAngle(px, py, tx, ty);
							angledifference2 = AngDif(protz, angle);
							
							if (angledifference1 < 0.0) angledifference1 = angledifference1 * -1;
							if (angledifference2 < 0.0) angledifference2 = angledifference2 * -1;
							if (angledifference1 == 0.0) angledifference1 = 0.00001;
							if (angledifference2 == 0.0) angledifference2 = 0.00001;
							
							if (((distance < (1.0 / angledifference1) * 500.0) || (distance < (1.0 / angledifference2) * 500.0)) &&
								(closestplayer == INVALID_PLAYER_ID || distance < closest)) {
									
									closest = distance;
									closestplayer = i;
							}
						}
						case 2: { // Forward attack
							new Float:angle = ReturnAngle(px, py, tx, ty) - 90.0,
								Float:angledifference = AngDif(protz, angle);
							
							if (angledifference < 0.0) angledifference = angledifference * -1;
							if (angledifference == 0.0) angledifference = 0.00001;
							
							if ((distance < (1.0 / angledifference) * 500.0) &&
								(closestplayer == INVALID_PLAYER_ID || distance < closest)) {
									
									closest = distance;
									closestplayer = i;
							}
						}
						case 3: { // Forward/side attack
							new Float:angle,
								Float:angledifference1,
								Float:angledifference2,
								Float:angledifference3;
							
							angle = ReturnAngle(px, py, tx, ty);
							
							angledifference1 = AngDif(protz, angle - 180.0);
							angledifference2 = AngDif(protz, angle);
							angledifference3 = AngDif(protz, angle - 90.0);
							
							if (angledifference1 < 0.0) angledifference1 = angledifference1 * -1;
							if (angledifference2 < 0.0) angledifference2 = angledifference2 * -1;
							if (angledifference3 < 0.0) angledifference3 = angledifference3 * -1;
							if (angledifference1 == 0.0) angledifference1 = 0.00001;
							if (angledifference2 == 0.0) angledifference2 = 0.00001;
							if (angledifference3 == 0.0) angledifference3 = 0.00001;
							
							if (((distance < (1.0 / angledifference1) * 500.0) || (distance < (1.0 / (angledifference2 + 90.0)) * 500.0) || (distance < (1.0 / (angledifference3 + 90.0)) * 500.0)) &&
								(closestplayer == INVALID_PLAYER_ID || distance < closest)) {
									
									closest = distance;
									closestplayer = i;
							}
						}
						case 4: { // 360 attack
							if (closestplayer == INVALID_PLAYER_ID || distance < closest) {
								closest = distance;
								closestplayer = i;
							}
						}
					}
				}
			}
		}
		
		return closestplayer;
		
	} else if (pstate == PLAYER_STATE_PASSENGER) { // Passenger driveby
		
		new Float:px, Float:py, Float:pz, pworld, pvehicleid,
			Float:tx, Float:ty, Float:tz,
			Float:distance;
		
		GetPlayerPos(playerid, px, py, pz);
		pworld = GetPlayerVirtualWorld(playerid);
		pvehicleid = GetPlayerVehicleID(playerid);
		
		new closestplayer = INVALID_PLAYER_ID;
		new Float:closest;
		
		LoopPlayers(i) {
			if (playerid != i && IsPlayerSpawned(i) && GetPlayerVirtualWorld(i) == pworld && GetPlayerVehicleID(i) != pvehicleid && !pGodData[i][pgIsOnGodmode] && (cData[playerid][cGangID] == INVALID_GANG_ID || cData[playerid][cGangID] != cData[i][cGangID])) {
				GetPlayerPos(i, tx, ty, tz);
				distance = FindDistance(px, py, pz, tx, ty, tz);
				
				if (distance < 40.0 &&
					(closestplayer == INVALID_PLAYER_ID || distance < closest)) {
					
					closest = distance;
					closestplayer = i;
				}
			}
		}
		
		return closestplayer;
	}
	
	return INVALID_PLAYER_ID;
}
