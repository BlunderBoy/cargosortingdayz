 modded class MissionServer extends MissionBase
 {
     override void OnMissionStart()
     {
         super.OnMissionStart();
         #ifdef SERVER
             string limitTimeParam;
             if (GetCLIParam("sortlimit", limitTimeParam))
             {
                 int timeValue = limitTimeParam.ToInt();
                 if (timeValue >= 1 && timeValue <= 10)
                 {
                     g_CargoSortTimeLimit = timeValue;
                     // Print("[CargoSorting] Setting sort time limit to: " + timeValue + " seconds");
                 }
                 else
                 {
                     // Print("[CargoSorting] Invalid sort time limit: " + timeValue + ". Using default: " + CARGO_SORT_DEFAULT_TIME_LIMIT);
                 }
             }
             else
             {
                 // Print("[CargoSorting] Using default sort time limit: " + CARGO_SORT_DEFAULT_TIME_LIMIT + " seconds");
             }
         #endif
     }
 }