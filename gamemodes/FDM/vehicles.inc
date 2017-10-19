#include <a_samp>

#define FILE_VEHICLES "FDM/Settings/Vehicles.ini"

new VehicleNames[212][] = { // Vehicle names by BetaMaster
	{"Landstalker"},
	{"Bravura"},
	{"Buffalo"},
	{"Linerunner"},
	{"Perrenial"},
	{"Sentinel"},
	{"Dumper"},
	{"Firetruck"},
	{"Trashmaster"},
	{"Stretch"},
	{"Manana"},
	{"Infernus"},
	{"Voodoo"},
	{"Pony"},
	{"Mule"},
	{"Cheetah"},
	{"Ambulance"},
	{"Leviathan"},
	{"Moonbeam"},
	{"Esperanto"},
	{"Taxi"},
	{"Washington"},
	{"Bobcat"},
	{"Mr Whoopee"},
	{"BF Injection"},
	{"Hunter"},
	{"Premier"},
	{"Enforcer"},
	{"Securicar"},
	{"Banshee"},
	{"Predator"},
	{"Bus"},
	{"Rhino"},
	{"Barracks"},
	{"Hotknife"},
	{"Trailer 1"}, //artict1
	{"Previon"},
	{"Coach"},
	{"Cabbie"},
	{"Stallion"},
	{"Rumpo"},
	{"RC Bandit"},
	{"Romero"},
	{"Packer"},
	{"Monster"},
	{"Admiral"},
	{"Squalo"},
	{"Seasparrow"},
	{"Pizzaboy"},
	{"Tram"},
	{"Trailer 2"}, //artict2
	{"Turismo"},
	{"Speeder"},
	{"Reefer"},
	{"Tropic"},
	{"Flatbed"},
	{"Yankee"},
	{"Caddy"},
	{"Solair"},
	{"Berkley's RC Van"},
	{"Skimmer"},
	{"PCJ-600"},
	{"Faggio"},
	{"Freeway"},
	{"RC Baron"},
	{"RC Raider"},
	{"Glendale"},
	{"Oceanic"},
	{"Sanchez"},
	{"Sparrow"},
	{"Patriot"},
	{"Quad"},
	{"Coastguard"},
	{"Dinghy"},
	{"Hermes"},
	{"Sabre"},
	{"Rustler"},
	{"ZR-350"},
	{"Walton"},
	{"Regina"},
	{"Comet"},
	{"BMX"},
	{"Burrito"},
	{"Camper"},
	{"Marquis"},
	{"Baggage"},
	{"Dozer"},
	{"Maverick"},
	{"News Chopper"},
	{"Rancher"},
	{"FBI Rancher"},
	{"Virgo"},
	{"Greenwood"},
	{"Jetmax"},
	{"Hotring"},
	{"Sandking"},
	{"Blista Compact"},
	{"Police Maverick"},
	{"Boxville"},
	{"Benson"},
	{"Mesa"},
	{"RC Goblin"},
	{"Hotring Racer A"}, //hotrina
	{"Hotring Racer B"}, //hotrinb
	{"Bloodring Banger"},
	{"Rancher"},
	{"Super GT"},
	{"Elegant"},
	{"Journey"},
	{"Bike"},
	{"Mountain Bike"},
	{"Beagle"},
	{"Cropdust"},
	{"Stunt"},
	{"Tanker"}, //petro
	{"Roadtrain"},
	{"Nebula"},
	{"Majestic"},
	{"Buccaneer"},
	{"Shamal"},
	{"Hydra"},
	{"FCR-900"},
	{"NRG-500"},
	{"HPV1000"},
	{"Cement Truck"},
	{"Tow Truck"},
	{"Fortune"},
	{"Cadrona"},
	{"FBI Truck"},
	{"Willard"},
	{"Forklift"},
	{"Tractor"},
	{"Combine"},
	{"Feltzer"},
	{"Remington"},
	{"Slamvan"},
	{"Blade"},
	{"Freight"},
	{"Streak"},
	{"Vortex"},
	{"Vincent"},
	{"Bullet"},
	{"Clover"},
	{"Sadler"},
	{"Firetruck LA"}, //firela
	{"Hustler"},
	{"Intruder"},
	{"Primo"},
	{"Cargobob"},
	{"Tampa"},
	{"Sunrise"},
	{"Merit"},
	{"Utility"},
	{"Nevada"},
	{"Yosemite"},
	{"Windsor"},
	{"Monster A"}, //monstera
	{"Monster B"}, //monsterb
	{"Uranus"},
	{"Jester"},
	{"Sultan"},
	{"Stratum"},
	{"Elegy"},
	{"Raindance"},
	{"RC Tiger"},
	{"Flash"},
	{"Tahoma"},
	{"Savanna"},
	{"Bandito"},
	{"Freight Flat"}, //freiflat
	{"Streak Carriage"}, //streakc
	{"Kart"},
	{"Mower"},
	{"Duneride"},
	{"Sweeper"},
	{"Broadway"},
	{"Tornado"},
	{"AT-400"},
	{"DFT-30"},
	{"Huntley"},
	{"Stafford"},
	{"BF-400"},
	{"Newsvan"},
	{"Tug"},
	{"Trailer 3"}, //petrotr
	{"Emperor"},
	{"Wayfarer"},
	{"Euros"},
	{"Hotdog"},
	{"Club"},
	{"Freight Carriage"}, //freibox
	{"Trailer 3"}, //artict3
	{"Andromada"},
	{"Dodo"},
	{"RC Cam"},
	{"Launch"},
	{"Police Car (LSPD)"},
	{"Police Car (SFPD)"},
	{"Police Car (LVPD)"},
	{"Police Ranger"},
	{"Picador"},
	{"S.W.A.T. Van"},
	{"Alpha"},
	{"Phoenix"},
	{"Glendale"},
	{"Sadler"},
	{"Luggage Trailer A"}, //bagboxa
	{"Luggage Trailer B"}, //bagboxb
	{"Stair Trailer"}, //tugstair
	{"Boxville"},
	{"Farm Plow"}, //farmtr1
	{"Utility Trailer"} //utiltr1
};

new mapVehicles[MAX_GMVEHICLES];

VehiclesInit() {
	new File:fhd;
	fhd = fopen(FILE_VEHICLES, filemode:io_read);
	if (!fhd) return 0;
	
	new linestr[MAX_STRING],
		vtype,
		Float:vx,
		Float:vy,
		Float:vz,
		Float:vrotz,
		vcolour1,
		vcolour2,
		vrespawndelay;
	
	new i;
	while (i < sizeof(mapVehicles) && fread(fhd, linestr, sizeof(linestr)) > 0) {
		
		if (!linestr[0] || linestr[0] == ';') continue;
		new idx;
		
		vtype = strval(split(linestr, idx, ','));
		
		if (vtype < 400 || vtype > 611) continue;
		
		vx = floatstr(split(linestr, idx, ','));
		vy = floatstr(split(linestr, idx, ','));
		vz = floatstr(split(linestr, idx, ','));
		vrotz = floatstr(split(linestr, idx, ','));
		vcolour1 = strval(split(linestr, idx, ','));
		vcolour2 = strval(split(linestr, idx, ','));
		vrespawndelay = strval(split(linestr, idx, ','));
		if (!vrespawndelay) vrespawndelay = DEFAULT_RESPAWN_DELAY;
		
		mapVehicles[i] = CreateVehicle(vtype, vx, vy, vz, vrotz, vcolour1, vcolour2, vrespawndelay);
		i++;
	}
	
	fclose(fhd);
	
	return 1;
}

VehiclesUnload() {
	for (new i; i < sizeof(mapVehicles); i++) {
		if (mapVehicles[i]) DestroyVehicle(mapVehicles[i]);
		mapVehicles[i] = 0;
	}
	return 1;
}

vehicles_OnVehicleSpawn(vehicleid) {
	for (new i; i < sizeof(mapVehicles); i++) {
		if (mapVehicles[i] == vehicleid) {
			SetVehicleVirtualWorld(vehicleid, 0);
			LinkVehicleToInterior(vehicleid, 0);
		}
	}
	return 1;
}

stock GetVehicleType(vehiclename[]) {
	if (!vehiclename[0]) return 0;
	new vnamelength = strlen(vehiclename);
	new vehicletype;
	
	new tmp = strval(vehiclename);
	if (tmp >= 400 && tmp <= 611) {
		return tmp;
	}
	
	for (new i; i < sizeof(VehicleNames); i++) {
		
		new fpos = strfind(VehicleNames[i], vehiclename, true);
		if (fpos == 0 && strlen(VehicleNames[i]) == vnamelength) return i + 400;
		if (fpos != -1) vehicletype = i + 400;
		
	}
	
	return vehicletype;
}

stock GetVehicleName(vehicletype) {
	new vehiclename[25];
	if (vehicletype < 400 || vehicletype > 611) {
		strcpy(vehiclename, "None");
		return vehiclename;
	} else {
		strcpy(vehiclename, VehicleNames[vehicletype - 400]);
		return vehiclename;
	}
}
