modded class MissionServer extends MissionBase
{
    override void OnMissionStart()
    {
        super.OnMissionStart();

        #ifdef SERVER
            string limitTime;
            if ( GetCLIParam( "sortlimt", limitTime ) )
            {
                int t = limitTime.ToInt();
                G_SortTimeLimit = t;
            }
        #endif
    }
}