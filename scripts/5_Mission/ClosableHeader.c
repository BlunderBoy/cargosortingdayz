modded class ClosableHeader
{
    protected Widget m_widget;
    protected ButtonWidget m_Re;
    protected ButtonWidget m_Add;
    
    protected PlayerBase localPlayer;

    ref TStringArray TitleTextArray = {
        "Fuel", "Accessories", "Kindling", "Body",
        "Chassis", "Engine", "Materials", "Containers"
    };

    int nowTime = 0, lastTime = 0;
    int SortLimitTime;

    void ClosableHeader( LayoutHolder parent, string function_name )
    {
        localPlayer = PlayerBase.Cast( GetGame().GetPlayer() );
        SortLimitTime = localPlayer.SortLimitTime;

        ImageWidget imgwidget = ImageWidget.Cast( m_RootWidget.FindAnyWidget( "PanelWidget" ) );
        if ( !imgwidget ) return;
        
        m_widget     = GetGame().GetWorkspace().CreateWidgets( "CargoSorting/scripts/GUI/sort.layout", imgwidget );
        m_Re         = ButtonWidget.Cast( m_widget.FindAnyWidget( "ButtonWidget0" ) );
        m_Add          = ButtonWidget.Cast( m_widget.FindAnyWidget( "ButtonWidget1" ) );
        
        WidgetEventHandler.GetInstance().RegisterOnMouseButtonDown( m_Re,  this, "ReCargo" );
        WidgetEventHandler.GetInstance().RegisterOnMouseButtonDown( m_Add, this, "AddCargo" );
    }
    
    override void SetItemPreview( EntityAI entity_ai )
    {
        super.SetItemPreview( entity_ai );

        CargoBase cargo = entity_ai.GetInventory().GetCargo();
         
        if (  (!cargo || ( cargo.GetWidth() <= 0 && cargo.GetHeight() <= 0 )) || !(entity_ai.IsInherited(Container_Base) || entity_ai.IsKindOf("Container_Base")))
            m_widget.Show( false );
    }

    override void SetName( string name )
    {
        super.SetName( name );
        
        if ( TitleTextArray.Find( name ) != -1 )
            m_widget.Show( false );
    }
    
    void ReCargo()
    {
        nowTime = GetGame().GetTime();
        if ( lastTime > 0 && nowTime - lastTime < SortLimitTime * 1000 )
            return;

        if ( !m_Entity ) return;

        Object entityIB;
        if (!Class.CastTo(entityIB, m_Entity)) return;

        localPlayer.RPCSingleParam( BACKPACKSORT.RE_CARGO, new Param1<Object>( entityIB ), true, null );

        lastTime = nowTime;
    }

    void AddCargo()
    {    
        nowTime = GetGame().GetTime();
        if ( lastTime > 0 && nowTime - lastTime < SortLimitTime * 1000 )
            return;

        if ( !m_Entity ) return;

        Object entityIB;
        if ( !Class.CastTo( entityIB, m_Entity ) ) return;

        localPlayer.RPCSingleParam( BACKPACKSORT.ADD_CARGO, new Param1<Object>( entityIB ), true, null );

        lastTime = nowTime;
    }
};