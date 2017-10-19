#include <a_samp>

#define VOLUME_RESET_TIME 3000

enum playerStealthInfo {
	Float:psVolume,
	Float:psWeaponVolume,
	psWeaponMelee,
	psTimer
}
new psData[MAX_PLAYERS][playerStealthInfo];

stealth_OnPlayerConnect(playerid) {
	stealth_ResetData(playerid);
	return 1;
}

stealth_OnPlayerDisonnect(playerid) {
	stealth_ResetData(playerid);
	return 1;
}

stealth_OnPlayerStateChange(playerid, newstate, oldstate) {
	if (((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED) &&
		!((oldstate >= PLAYER_STATE_ONFOOT && oldstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || oldstate == PLAYER_STATE_SPAWNED) ||
		
		((oldstate >= PLAYER_STATE_ONFOOT && oldstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || oldstate == PLAYER_STATE_SPAWNED) &&
		!((newstate >= PLAYER_STATE_ONFOOT && newstate <= PLAYER_STATE_ENTER_VEHICLE_PASSENGER) || newstate == PLAYER_STATE_SPAWNED)){
		
		// Player spawn/unspawn
		stealth_ResetData(playerid);
			
	} else if ((newstate == PLAYER_STATE_DRIVER || newstate == PLAYER_STATE_PASSENGER) &&
		(oldstate != PLAYER_STATE_DRIVER && oldstate != PLAYER_STATE_PASSENGER)) {
		// Player enter vehicle
		
		new vehicleid = GetPlayerVehicleID(playerid);
		if (vehicleid) {
			psData[playerid][psWeaponVolume] = 0.0;
			
			if (psData[playerid][psTimer]) {
				KillTimer(psData[playerid][psTimer]);
				psData[playerid][psTimer] = 0;
			}
			
			new Float:vehiclevolume = GetVehicleVolume(GetVehicleModel(vehicleid));
			if (vehiclevolume != psData[playerid][psVolume]) {
				new Float:old = psData[playerid][psVolume];
				psData[playerid][psVolume] = vehiclevolume;
				OnPlayerVolumeChange(playerid, vehiclevolume, old);
			}
		}
		
	} else if ((oldstate == PLAYER_STATE_DRIVER || oldstate == PLAYER_STATE_PASSENGER) &&
		(newstate != PLAYER_STATE_DRIVER && newstate != PLAYER_STATE_PASSENGER)) {
		// Player exit vehicle
		
		if (psData[playerid][psTimer]) {
			KillTimer(psData[playerid][psTimer]);
			psData[playerid][psTimer] = SetTimerEx("steatlh_ResetVolume", VOLUME_RESET_TIME, 0, "i", playerid);
		}
	}
	return 1;
}

stealth_OnPlayerKeyStateChange(playerid, newkeys, oldkeys) {
	new playerstate = GetPlayerState(playerid);
	if (playerstate == PLAYER_STATE_DRIVER ||
		playerstate == PLAYER_STATE_PASSENGER ||
		playerstate == PLAYER_STATE_NONE ||
		playerstate == PLAYER_STATE_WASTED ||
		playerstate == PLAYER_STATE_SPECTATING ||
		skinPlayerData[playerid][pSelecting]) return 1;
	
	new playerweapon = GetPlayerWeapon(playerid);
	
	if (IsKeyJustDown(KEY_FIRE, newkeys, oldkeys) || // Normal firing
		(IsKeyJustDown(KEY_SECONDARY_ATTACK | KEY_HANDBRAKE, newkeys, oldkeys) && IsWeaponMelee(playerweapon))) { // Fighting style
		// Player started shooting
		
		if (psData[playerid][psTimer]) {
			KillTimer(psData[playerid][psTimer]);
			psData[playerid][psTimer] = 0;
		}
		
		new Float:weaponvolume = GetWeaponVolume(playerweapon);
		if (weaponvolume > psData[playerid][psWeaponVolume]) {
			psData[playerid][psWeaponVolume] = weaponvolume;
			
			if (IsWeaponMelee(playerweapon)) {
				psData[playerid][psWeaponMelee] = 1;
			} else {
				psData[playerid][psWeaponMelee] = 0;
			}
			
			if (weaponvolume != psData[playerid][psVolume]) {
				new Float:old = psData[playerid][psVolume];
				psData[playerid][psVolume] = weaponvolume;
				OnPlayerVolumeChange(playerid, weaponvolume, old);
			}
		}
	}
	
	if (IsKeyJustUp(KEY_FIRE, newkeys, oldkeys) ||
		(psData[playerid][psWeaponMelee] && IsKeyJustUp(KEY_SECONDARY_ATTACK | KEY_HANDBRAKE, newkeys, oldkeys))) {
		// Player stopped shooting
		
		if (psData[playerid][psTimer]) KillTimer(psData[playerid][psTimer]);
		psData[playerid][psTimer] = SetTimerEx("steatlh_ResetVolume", VOLUME_RESET_TIME, 0, "i", playerid);
	}
	return 1;
}

forward steatlh_ResetVolume(playerid);
public steatlh_ResetVolume(playerid) {
	/*new playerstate = GetPlayerState(playerid);
	if (playerstate == PLAYER_STATE_DRIVER || playerstate == PLAYER_STATE_PASSENGER) {
		new vehicleid = GetPlayerVehicleID(playerid);
		if (vehicleid) {
			psData[playerid][psWeaponVolume] = 0.0;
			
			new vehiclevolume = GetVehicleVolume(GetVehicleModel(vehicleid));
			if (vehiclevolume != psData[playerid][psVolume]) {
				psData[playerid][psVolume] = vehiclevolume;
				OnPlayerVolumeChange(playerid, vehiclevolume, psData[playerid][psVolume]);
			}
			return 1;
		}
	} else if (
		!(playerstate == PLAYER_STATE_DRIVER ||
		playerstate == PLAYER_STATE_PASSENGER ||
		playerstate == PLAYER_STATE_NONE ||
		playerstate == PLAYER_STATE_WASTED ||
		playerstate == PLAYER_STATE_SPECTATING ||
		skinPlayerData[playerid][pSelecting]) &&
		GetPlayerKeys(playerid) & KEY_FIRE
		) {
		
		new Float:weaponvolume = GetWeaponVolume(GetPlayerWeapon(playerid));
		if (weaponvolume > psData[playerid][psWeaponVolume]) {
			psData[playerid][psWeaponVolume] = weaponvolume;
			
			if (weaponvolume != psData[playerid][psVolume]) {
				OnPlayerVolumeChange(playerid, weaponvolume, psData[playerid][psVolume]);
				psData[playerid][psVolume] = weaponvolume;
			}
		}
		return 1;
	}*/
	
	if (psData[playerid][psVolume] != 0.0) {
		new Float:old = psData[playerid][psVolume];
		psData[playerid][psVolume] = 0.0;
		OnPlayerVolumeChange(playerid, 0.0, old);
	}
	psData[playerid][psWeaponVolume] = 0.0;
	return 1;
}

stealth_ResetData(playerid) {
	psData[playerid][psVolume] = 0.0;
	if (psData[playerid][psTimer]) {
		KillTimer(psData[playerid][psTimer]);
		psData[playerid][psTimer] = 0;
	}
	new tmp[playerStealthInfo];
	psData[playerid] = tmp;
	return 1;
}

stock GetPlayerVolumeToPlayerByte(playerid, hearerid) {
	new Float:tmp = GetPlayerVolumeToPlayer(playerid, hearerid),
		volume = floatround(tmp, floatround_floor);
	
	if (volume < 0) {
		volume = 0;
	} else if (volume > 255) {
		volume = 255;
	}
	return volume;
}

forward Float:GetPlayerVolumeToPlayer(playerid, hearerid);
stock Float:GetPlayerVolumeToPlayer(playerid, hearerid) {
	if (!IsPlayerSpawned(playerid) || !IsPlayerSpawned(hearerid)) return 0.0;
	if (psData[playerid][psVolume] == 0.0) return 0.0;
	
	new Float:x1, Float:y1, Float:z1,
		Float:x2, Float:y2, Float:z2,
		Float:distance;
	
	GetPlayerPos(playerid, x1, y1, z1);
	GetPlayerPos(hearerid, x2, y2, z2);
	distance = FindDistance(x1, y1, z1, x2, y2, z2);
	
	if (distance >= psData[playerid][psVolume]) {
		return 0.0;
	} else {
		return 255.0 - ((255.0 / psData[playerid][psVolume]) * distance);
	}
}

stock GetPlayerVolumeByte(playerid) {
	if (playerid < 0 || playerid > MAX_PLAYERS) return 0;
	new volume = floatround(psData[playerid][psVolume], floatround_floor);
	if (volume < 0) {
		volume = 0;
	} else if (volume > 255) {
		volume = 255;
	}
	return volume;
}

forward Float:GetPlayerVolume(playerid);
stock Float:GetPlayerVolume(playerid) {
	if (playerid < 0 || playerid > MAX_PLAYERS) return 0.0;
	return psData[playerid][psVolume];
}

forward Float:GetWeaponVolume(weaponid);
stock Float:GetWeaponVolume(weaponid) {
	if (weaponid < 0 || weaponid > 46) return 0.0;
	new Float:wpnvolume[47] = {
			1.5, 2.0, 5.0, 5.0, 3.5, 5.0, 5.0, 3.0, 5.0, 30.0,
			4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 200.0, 10.0, 10.0, 50.0,
			150.0, 50.0, 40.0, 10.0, 55.0, 100.0, 110.0, 110.0, 50.0, 50.0,
			80.0, 80.0, 50.0, 120.0, 200.0, 255.0, 255.0, 40.0, 0.0, 10.0,
			1.5, 15.0, 25.0, 1.5, 1.5, 1.5, 20.0
		};
	return wpnvolume[weaponid];
}

forward Float:GetVehicleVolume(vehicletype);
stock Float:GetVehicleVolume(vehicletype) {
	if (vehicletype < 400 || vehicletype > 611) return 0.0;
	new Float:vehiclevolume[212] = {
			100.0,//{"Landstalker"},
			80.0,//{"Bravura"},
			100.0,//{"Buffalo"},
			140.0,//{"Linerunner"},
			80.0,//{"Perrenial"},
			80.0,//{"Sentinel"},
			150.0,//{"Dumper"},
			150.0,//{"Firetruck"},
			150.0,//{"Trashmaster"},
			90.0,//{"Stretch"},
			80.0,//{"Manana"},
			110.0,//{"Infernus"},
			90.0,//{"Voodoo"},
			90.0,//{"Pony"},
			110.0,//{"Mule"},
			110.0,//{"Cheetah"},
			120.0,//{"Ambulance"},
			200.0,//{"Leviathan"},
			90.0,//{"Moonbeam"},
			85.0,//{"Esperanto"},
			85.0,//{"Taxi"},
			90.0,//{"Washington"},
			100.0,//{"Bobcat"},
			100.0,//{"Mr Whoopee"},
			95.0,//{"BF Injection"},
			250.0,//{"Hunter"},
			85.0,//{"Premier"},
			100.0,//{"Enforcer"},
			100.0,//{"Securicar"},
			100.0,//{"Banshee"},
			100.0,//{"Predator"},
			120.0,//{"Bus"},
			150.0,//{"Rhino"},
			120.0,//{"Barracks"},
			100.0,//{"Hotknife"},
			50.0,//{"Trailer 1"}, //artict1
			80.0,//{"Previon"},
			125.0,//{"Coach"},
			85.0,//{"Cabbie"},
			95.0,//{"Stallion"},
			100.0,//{"Rumpo"},
			15.0,//{"RC Bandit"},
			85.0,//{"Romero"},
			100.0,//{"Packer"},
			110.0,//{"Monster"},
			80.0,//{"Admiral"},
			120.0,//{"Squalo"},
			200.0,//{"Seasparrow"},
			40.0,//{"Pizzaboy"},
			110.0,//{"Tram"},
			50.0,//{"Trailer 2"}, //artict2
			110.0,//{"Turismo"},
			120.0,//{"Speeder"},
			80.0,//{"Reefer"},
			80.0,//{"Tropic"},
			110.0,//{"Flatbed"},
			100.0,//{"Yankee"},
			40.0,//{"Caddy"},
			95.0,//{"Solair"},
			95.0,//{"Berkley's RC Van"},
			180.0,//{"Skimmer"},
			80.0,//{"PCJ-600"},
			40.0,//{"Faggio"},
			100.0,//{"Freeway"},
			60.0,//{"RC Baron"},
			40.0,//{"RC Raider"},
			85.0,//{"Glendale"},
			85.0,//{"Oceanic"},
			70.0,//{"Sanchez"},
			180.0,//{"Sparrow"},
			95.0,//{"Patriot"},
			50.0,//{"Quad"},
			100.0,//{"Coastguard"},
			50.0,//{"Dinghy"},
			95.0,//{"Hermes"},
			100.0,//{"Sabre"},
			210.0,//{"Rustler"},
			95.0,//{"ZR-350"},
			80.0,//{"Walton"},
			80.0,//{"Regina"},
			90.0,//{"Comet"},
			25.0,//{"BMX"},
			80.0,//{"Burrito"},
			75.0,//{"Camper"},
			80.0,//{"Marquis"},
			60.0,//{"Baggage"},
			85.0,//{"Dozer"},
			200.0,//{"Maverick"},
			200.0,//{"News Chopper"},
			90.0,//{"Rancher"},
			90.0,//{"FBI Rancher"},
			90.0,//{"Virgo"},
			80.0,//{"Greenwood"},
			125.0,//{"Jetmax"},
			110.0,//{"Hotring"},
			95.0,//{"Sandking"},
			75.0,//{"Blista Compact"},
			200.0,//{"Police Maverick"},
			100.0,//{"Boxville"},
			100.0,//{"Benson"},
			70.0,//{"Mesa"},
			40.0,//{"RC Goblin"},
			110.0,//{"Hotring Racer A"}, //hotrina
			110.0,//{"Hotring Racer B"}, //hotrinb
			90.0,//{"Bloodring Banger"},
			90.0,//{"Rancher"},
			110.0,//{"Super GT"},
			85.0,//{"Elegant"},
			100.0,//{"Journey"},
			20.0,//{"Bike"},
			30.0,//{"Mountain Bike"},
			180.0,//{"Beagle"},
			180.0,//{"Cropdust"},
			180.0,//{"Stunt"},
			150.0,//{"Tanker"}, //petro
			135.0,//{"Roadtrain"},
			80.0,//{"Nebula"},
			90.0,//{"Majestic"},
			90.0,//{"Buccaneer"},
			240.0,//{"Shamal"},
			250.0,//{"Hydra"},
			100.0,//{"FCR-900"},
			110.0,//{"NRG-500"},
			95.0,//{"HPV1000"},
			100.0,//{"Cement Truck"},
			90.0,//{"Tow Truck"},
			85.0,//{"Fortune"},
			80.0,//{"Cadrona"},
			95.0,//{"FBI Truck"},
			85.0,//{"Willard"},
			40.0,//{"Forklift"},
			70.0,//{"Tractor"},
			120.0,//{"Combine"},
			80.0,//{"Feltzer"},
			95.0,//{"Remington"},
			95.0,//{"Slamvan"},
			85.0,//{"Blade"},
			200.0,//{"Freight"},
			200.0,//{"Streak"},
			55.0,//{"Vortex"},
			85.0,//{"Vincent"},
			105.0,//{"Bullet"},
			105.0,//{"Clover"},
			85.0,//{"Sadler"},
			120.0,//{"Firetruck LA"}, //firela
			85.0,//{"Hustler"},
			85.0,//{"Intruder"},
			90.0,//{"Primo"},
			210.0,//{"Cargobob"},
			85.0,//{"Tampa"},
			85.0,//{"Sunrise"},
			85.0,//{"Merit"},
			95.0,//{"Utility"},
			255.0,//{"Nevada"},
			95.0,//{"Yosemite"},
			85.0,//{"Windsor"},
			110.0,//{"Monster A"}, //monstera
			110.0,//{"Monster B"}, //monsterb
			85.0,//{"Uranus"},
			90.0,//{"Jester"},
			90.0,//{"Sultan"},
			85.0,//{"Stratum"},
			95.0,//{"Elegy"},
			210.0,//{"Raindance"},
			35.0,//{"RC Tiger"},
			90.0,//{"Flash"},
			85.0,//{"Tahoma"},
			90.0,//{"Savanna"},
			100.0,//{"Bandito"},
			100.0,//{"Freight Flat"}, //freiflat
			100.0,//{"Streak Carriage"}, //streakc
			60.0,//{"Kart"},
			60.0,//{"Mower"},
			110.0,//{"Duneride"},
			65.0,//{"Sweeper"},
			90.0,//{"Broadway"},
			85.0,//{"Tornado"},
			255.0,//{"AT-400"},
			110.0,//{"DFT-30"},
			90.0,//{"Huntley"},
			90.0,//{"Stafford"},
			75.0,//{"BF-400"},
			95.0,//{"Newsvan"},
			55.0,//{"Tug"},
			50.0,//{"Trailer 3"}, //petrotr
			100.0,//{"Emperor"},
			90.0,//{"Wayfarer"},
			90.0,//{"Euros"},
			95.0,//{"Hotdog"},
			80.0,//{"Club"},
			50.0,//{"Freight Carriage"}, //freibox
			50.0,//{"Trailer 3"}, //artict3
			300.0,//{"Andromada"},
			180.0,//{"Dodo"},
			15.0,//{"RC Cam"},
			100.0,//{"Launch"},
			85.0,//{"Police Car (LSPD)"},
			85.0,//{"Police Car (SFPD)"},
			85.0,//{"Police Car (LVPD)"},
			90.0,//{"Police Ranger"},
			85.0,//{"Picador"},
			100.0,//{"S.W.A.T. Van"},
			90.0,//{"Alpha"},
			110.0,//{"Phoenix"},
			85.0,//{"Glendale"},
			85.0,//{"Sadler"},
			35.0,//{"Luggage Trailer A"}, //bagboxa
			35.0,//{"Luggage Trailer B"}, //bagboxb
			35.0,//{"Stair Trailer"}, //tugstair
			100.0,//{"Boxville"},
			40.0,//{"Farm Plow"}, //farmtr1
			40.0,//{"Utility Trailer"} //utiltr1
		};
	return vehiclevolume[vehicletype - 400];
}