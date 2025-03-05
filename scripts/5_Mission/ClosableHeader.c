 modded class ClosableHeader
 {
     // UI Components
     protected Widget m_widget;
     protected ButtonWidget m_Re;
     protected ButtonWidget m_Add;
     protected PlayerBase localPlayer;
     protected ref TStringArray TitleTextArray = {
         "Fuel", "Accessories", "Kindling", "Body",
         "Chassis", "Engine", "Materials", "Containers"
     };
 
     int nowTime = 0, lastTime = 0;
     int SortLimitTime;
 
     void ClosableHeader(LayoutHolder parent, string function_name)
     {
         localPlayer = PlayerBase.Cast(GetGame().GetPlayer());
         if (localPlayer)
         {
             SortLimitTime = localPlayer.SortLimitTime;
         }
         ImageWidget imgwidget = ImageWidget.Cast(m_RootWidget.FindAnyWidget("PanelWidget"));
         if (!imgwidget) return;
         m_widget = GetGame().GetWorkspace().CreateWidgets("CargoSorting/scripts/GUI/sort.layout", imgwidget);
         if (!m_widget) return;
         m_Re = ButtonWidget.Cast(m_widget.FindAnyWidget("ButtonWidget0"));
         m_Add = ButtonWidget.Cast(m_widget.FindAnyWidget("ButtonWidget1"));
         if (m_Re && m_Add)
         {
             WidgetEventHandler.GetInstance().RegisterOnMouseButtonDown(m_Re, this, "ReCargo");
             WidgetEventHandler.GetInstance().RegisterOnMouseButtonDown(m_Add, this, "AddCargo");
         }
     }
     
     override void SetItemPreview(EntityAI entity_ai)
     {
         super.SetItemPreview(entity_ai);
         CargoBase cargo = entity_ai ? entity_ai.GetInventory().GetCargo() : null;
          
         // Hide buttons if:
         // - Entity is null
         // - Cargo is invalid or empty
         // - Entity is not a container
         if ((!cargo || (cargo.GetWidth() <= 0 && cargo.GetHeight() <= 0)) || 
             !(entity_ai && (entity_ai.IsInherited(Container_Base) || entity_ai.IsKindOf("Container_Base"))))
         {
             if (m_widget) m_widget.Show(false);
         }
         else
         {
             if (m_widget) m_widget.Show(true);
         }
     }
 
     override void SetName(string name)
     {
         super.SetName(name);
         if (TitleTextArray.Find(name) != -1)
         {
             if (m_widget) m_widget.Show(false);
         }
     }
     
     void ReCargo()
     {
         nowTime = GetGame().GetTime();
         if (lastTime > 0 && nowTime - lastTime < SortLimitTime * 1000)
             return;
         if (!m_Entity) return;
         Object entityIB;
         if (!Class.CastTo(entityIB, m_Entity)) return;
         if (localPlayer)
         {
             localPlayer.RPCSingleParam(BACKPACKSORT.RE_CARGO, new Param1<Object>(entityIB), true, null);
             lastTime = nowTime;
         }
     }
 
     void AddCargo()
     {    
         nowTime = GetGame().GetTime();
         if (lastTime > 0 && nowTime - lastTime < SortLimitTime * 1000)
             return;
         if (!m_Entity) return;
         Object entityIB;
         if (!Class.CastTo(entityIB, m_Entity)) return;
         if (localPlayer)
         {
             localPlayer.RPCSingleParam(BACKPACKSORT.ADD_CARGO, new Param1<Object>(entityIB), true, null);
             lastTime = nowTime;
         }
     }
 };