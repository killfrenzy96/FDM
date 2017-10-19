#include <a_samp>

#define MAX_INTERIOR_ID 255

#if ENABLE_INTERIOR_WORLD_SYNC
	#define INTERIOR_WORLD_ID 1001
	//new interiorWorldID[MAX_INTERIOR_ID];
#endif
new pDisableChange[MAX_PLAYERS];

InteriorsInit() {
	LoopPlayers(i) {
		new interior = GetPlayerInterior(i);
		
		#if ENABLE_INTERIOR_WORLD_SYNC
		if (interior && !GetPlayerVirtualWorld(i)) {
		#else
		if (interior) {
		#endif
			
			#if !ENABLE_INTERIOR_WEAPONS
				DisableGodmode(i, 1);
			#endif
			
			#if ENABLE_INTERIOR_WORLD_SYNC
				/*if (!interiorWorldID[interior]) {
					interiorWorldID[interior] = GetFreeVirtualWorldID();
					ToggleVirtualWorldUsed(interiorWorldID[interior], 1);
				}
				SetPlayerVirtualWorld(i, interiorWorldID[interior]);*/
				SetPlayerVirtualWorld(i, INTERIOR_WORLD_ID);
			#endif
		}
		
		#if ENABLE_INTERIOR_WORLD_SYNC
		else {
			SetPlayerVirtualWorld(i, 0);
		}
		#endif
	}
	return 1;
}

InteriorsExit() {
	LoopPlayers(i) {
		if (pDisableChange[i]) {
			pDisableChange[i] = 0;
		}
		#if ENABLE_INTERIOR_WORLD_SYNC
		else {
			/*new interior = GetPlayerInterior(i);
			if (interior && !IsPlayerInMinimode(i) && GetPlayerVirtualWorld(i) == interiorWorldID[interior]) {
				SetPlayerVirtualWorld(i, 0);
			}*/
			if (GetPlayerInterior(i) && !IsPlayerInMinimode(i) && GetPlayerVirtualWorld(i) == INTERIOR_WORLD_ID) {
				SetPlayerVirtualWorld(i, 0);
			}
		}
		#endif
	}
	
	#if ENABLE_INTERIOR_WORLD_SYNC
		/*for (new i; i < MAX_INTERIOR_ID; i++) {
			if (interiorWorldID[i]) {
				ToggleVirtualWorldUsed(interiorWorldID[i], 0);
				interiorWorldID[i] = 0;
			}
		}*/
	#endif
	return 1;
}

DisableNextInteriorChange(playerid) {
	if (IsPlayerConnected(playerid)) {
		pDisableChange[playerid] = 1;
		return 1;
	} else {
		return 0;
	}
}

interior_OnPlayerDisconnect(playerid) {
	pDisableChange[playerid] = 0;
	
	#if ENABLE_INTERIOR_WORLD_SYNC
		/*new pinterior = GetPlayerInterior(playerid);
		if (pinterior && interiorWorldID[pinterior]) {
			new interiorused;
			LoopPlayers(i) {
				if (playerid == i) continue;
				
				new interior = GetPlayerInterior(i);
				if (interior == pinterior) {
					interiorused = 1;
					break;
				}
			}
			if (!interiorused) {
				ToggleVirtualWorldUsed(interiorWorldID[pinterior], 0);
				interiorWorldID[pinterior] = 0;
			}
		}*/
	#endif
	
	return 1;
}

interior_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid) {
	SetTimerEx("fdmOnPlayerInteriorChange", 0, false, "iii", playerid, newinteriorid, oldinteriorid);
	return 1;
}

forward fdmOnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
public fdmOnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid) {
	if (!IsPlayerConnected(playerid)) return 1;
	
	#if ENABLE_INTERIOR_WORLD_SYNC
		//new oldworld = interiorWorldID[oldinteriorid];
	#endif
	
	/*if (interiorWorldID[oldinteriorid]) {
		new interiorused;
		LoopPlayers(i) {
			if (playerid == i) continue;
			
			new interior = GetPlayerInterior(i);
			if (interior == oldinteriorid) {
				interiorused = 1;
				break;
			}
		}
		if (!interiorused) {
			ToggleVirtualWorldUsed(interiorWorldID[oldinteriorid], 0);
			interiorWorldID[oldinteriorid] = 0;
		}
	}*/
	
	if (IsPlayerInMinimode(playerid)) return 1;
	if (!IsPlayerSpawned(playerid)) return 1;
	
	if (newinteriorid) {
		// Player entered interior
		
		if (pDisableChange[playerid]) {
			pDisableChange[playerid] = 0;
			return 1;
		}
		
		#if !ENABLE_INTERIOR_WEAPONS
			DisableGodmode(playerid, 1);
		#endif
		
		#if ENABLE_INTERIOR_WORLD_SYNC
			/*if (GetPlayerVirtualWorld(playerid) != oldworld) {
				return 1;
			}*/
			if (GetPlayerVirtualWorld(playerid)) {
				return 1;
			}
			
			/*if (!interiorWorldID[newinteriorid]) {
				interiorWorldID[newinteriorid] = GetFreeVirtualWorldID();
				ToggleVirtualWorldUsed(interiorWorldID[newinteriorid], 1);
			}*/
			
			//SetPlayerVirtualWorld(playerid, interiorWorldID[newinteriorid]);
			SetPlayerVirtualWorld(playerid, INTERIOR_WORLD_ID);
			
			if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER) {
				new vid = GetPlayerVehicleID(playerid);
				if (vid) {
					//SetVehicleVirtualWorld(vid, interiorWorldID[newinteriorid]);
					SetVehicleVirtualWorld(vid, INTERIOR_WORLD_ID);
				}
			}
		#endif
		
	} else {
		// Player left interior
		
		// Maintain virtual world streaming with interiors
		#if ENABLE_INTERIOR_WORLD_SYNC
			/*new interiorused;
			LoopPlayers(i) {
				if (playerid == i) continue;
				
				new interior = GetPlayerInterior(i);
				if (interior == oldinteriorid) {
					interiorused = 1;
					break;
				}
			}
			if (!interiorused) {
				ToggleVirtualWorldUsed(interiorWorldID[oldinteriorid], 0);
				interiorWorldID[oldinteriorid] = 0;
			}*/
		#endif
		
		if (pDisableChange[playerid]) {
			pDisableChange[playerid] = 0;
			return 1;
		}
		
		#if !ENABLE_INTERIOR_WEAPONS
			DisableGodmode(playerid, 0);
		#endif
		
		#if ENABLE_INTERIOR_WORLD_SYNC
			/*if (oldworld == GetPlayerVirtualWorld(playerid)) {
				SetPlayerVirtualWorld(playerid, 0);
				
				if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER) {
					new vid = GetPlayerVehicleID(playerid);
					if (vid) {
						SetVehicleVirtualWorld(vid, 0);
					}
				}
			}*/
			if (GetPlayerVirtualWorld(playerid) == INTERIOR_WORLD_ID) {
				SetPlayerVirtualWorld(playerid, 0);
				
				if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER) {
					new vid = GetPlayerVehicleID(playerid);
					if (vid) {
						SetVehicleVirtualWorld(vid, 0);
					}
				}
			}
		#endif
	}
	
	return 1;
}

