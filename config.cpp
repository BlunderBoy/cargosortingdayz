////////////////////////////////////////////////////////////////////
//DeRap: config.bin
//Produced from mikero's Dos Tools Dll version 9.44
//https://mikero.bytex.digital/Downloads
//'now' is Fri Nov 22 10:46:38 2024 : 'file' last modified on Tue Oct 22 04:05:51 2024
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgPatches
{
	class CargoSorting
	{
		requiredAddons[] = {"DZ_Data","DZ_Animals","DZ_Characters","DZ_AI","DZ_Scripts"};
	};
};
class CfgMods
{
	class CargoSorting
	{
		dir = "CargoSorting";
		picture = "";
		action = "";
		hideName = 0;
		hidePicture = 1;
		name = "CargoSorting";
		credits = "FUCK_521";
		author = "FUCK_521";
		authorID = "0";
		version = "0.1";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"CargoSorting/scripts/3_Game","CargoSorting/scripts/Common"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"CargoSorting/scripts/4_World","CargoSorting/scripts/Common"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"CargoSorting/scripts/5_Mission","CargoSorting/scripts/Common"};
			};
		};
	};
};
