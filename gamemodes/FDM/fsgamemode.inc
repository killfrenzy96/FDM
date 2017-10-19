/*

By KillFrenzy.

This will help in creating a gamemode as a filterscript.
It provides a replacement of certain API's.

*/

#include <a_samp>

new gmVehicles[MAX_GMVEHICLES] = INVALID_VEHICLE_ID;
//new gmVehicleTypes[MAX_GMVEHICLE_TYPES];

//-----------------------------------------------------------------------------------------------------
// UNLOAD FILTERSCRIPT ALTERNATIVE
//-----------------------------------------------------------------------------------------------------
FsGamemodeInit() {
	for (new i; i < MAX_GMVEHICLES; i++) {
		gmVehicles[i] = INVALID_VEHICLE_ID;
	}
	return 1;
}

FsGamemodeExit() {
	kDestroyAllVehicles();
	kDestroyAllTimers();
	
	for (new i; i < MAX_PLAYERS; i++) {
		ResetPlayerWeapons(i);
		ResetPlayerMoney(i);
		SetPlayerArmour(i, 0.0);
		SetPlayerHealth(i, 100.0);
	}
	return 1;
}

#if ENABLE_VEHICLE_WORLD_FIX
	new gmpFirstConnect[MAX_PLAYERS];
	
	fsgm_OnPlayerConnect(playerid) {
		gmpFirstConnect[playerid] = 1;
		return 1;
	}
	
	fsgm_OnPlayerRequestClass(playerid) {
		if (gmpFirstConnect[playerid]) {
			gmpFirstConnect[playerid] = 0;
			for (new i; i < MAX_VEHICLES; i++) {
				if (GetVehicleModel(i)) {
					new worldid = GetVehicleVirtualWorld(i);
					if (worldid) {
						SetVehicleVirtualWorld(i, worldid);
					}
				}
			}
		}
		return 1;
	}
#endif

//-----------------------------------------------------------------------------------------------------
// VEHICLES
//-----------------------------------------------------------------------------------------------------
// TODO: Prevent more than 50 vehicle types from being spawned

stock xCreateVehicle(vehicletype, Float:X, Float:Y, Float:Z, Float:rotation, color1, color2, respawn_delay) {
	new i = FindFreeVehicleInArray();
	if (i == -1) return 0;
	
	gmVehicles[i] = CreateVehicle(vehicletype, X, Y, Z, rotation, color1, color2, respawn_delay);
	
	if (gmVehicles[i]) {
		return gmVehicles[i];
	} else {
		gmVehicles[i] = INVALID_VEHICLE_ID;
		return 0;
	}
}

stock xDestroyVehicle(vehicleid) {
	kDestroyVehicle(vehicleid);
	for (new i; i < MAX_GMVEHICLES; i++) {
		if (gmVehicles[i] == vehicleid) {
			gmVehicles[i] = INVALID_VEHICLE_ID;
		}
	}
	return 1;
}

stock kDestroyAllVehicles() {
	for (new i; i < MAX_GMVEHICLES; i++) {
		if (gmVehicles[i] != INVALID_VEHICLE_ID) {
			DestroyVehicle(gmVehicles[i]);
			gmVehicles[i] = INVALID_VEHICLE_ID;
		}
	}
	return 1;
}

stock FindFreeVehicleInArray() {
	for (new i; i < MAX_GMVEHICLES; i++) {
		if (gmVehicles[i] == INVALID_VEHICLE_ID) {
			return i;
		}
	}
	return -1;
}

/*stock FindFreeVehicleTypeInArray() {
	for (new i; i < MAX_GMVEHICLE_TYPES; i++) {
		if (!gmVehicleTypes[i]) {
			return i;
		}
	}
	return -1;
}*/


//-----------------------------------------------------------------------------------------------------
// TIMERS
//-----------------------------------------------------------------------------------------------------
new gmTimers[MAX_TIMERS];

stock xSetTimer(funcname[], interval, repeating) {
	new i = FindFreeTimerInArray();
	if (i == -1) return 0;
	
	gmTimers[i] = SetTimer(funcname, interval, repeating);
	return gmTimers[i];
}

/*stock xSetTimerEx(funcname[], interval, repeating, const format[], {Float,_}:...) {
	if (!repeating) {
		return SetTimerEx(funcname, interval, repeating, format, {Float,_}:...);
	}
	new i = FindFreeTimerInArray();
	if (i == -1) return 0;
	
	gmTimers[i] = SetTimerEx(funcname, interval, repeating, format, {Float,_}:...);
	return gmTimers[i];
}*/

stock xKillTimer(timerid) {
	KillTimer(timerid)
	for (new i; i < MAX_TIMERS; i++) {
		if (gmTimers[i] = timerid) {
			gmTimers[i] = 0;
		}
	}
	return 1;
}

stock kDestroyAllTimers() {
	for (new i; i < MAX_TIMERS; i++) {
		if (!gmTimers[i]) {
			KillTimer(gmTimers[i]);
			gmTimers[i] = 0;
		}
	}
	return 1;
}

stock FindFreeTimerInArray() {
	for (new i; i < MAX_TIMERS; i++) {
		if (!gmTimers[i]) {
			return i;
		}
	}
	return -1;
}

// Functions replacer
#define AddStaticVehicle(%1,%2,%3,%4,%5,%6,%7) xCreateVehicle(%1,%2,%3,%4,%5,%6,%7,DEFAULT_RESPAWN_DELAY)
#define xAddStaticVehicle(%1,%2,%3,%4,%5,%6,%7) xCreateVehicle(%1,%2,%3,%4,%5,%6,%7,DEFAULT_RESPAWN_DELAY)
#define AddStaticVehicleEx xCreateVehicle
#define CreateVehicle xCreateVehicle
#define DestroyVehicle xDestroyVehicle
#define SetTimer xSetTimer
//#define SetTimerEx xSetTimerEx