
// Implements cargo sorting functionality for player inventories


 modded class PlayerBase extends ManBase
 {
     private int m_LastSortTime = 0;
     int SortLimitTime;
     private ref array<ref EntityProperty> m_CachedEntityProperties;
     private ref array<ItemBase> m_CachedCargoItems;
     void PlayerBase()
     {
         RegisterNetSyncVariableInt("SortLimitTime", 1, 10);
         m_CachedEntityProperties = new array<ref EntityProperty>;
         m_CachedCargoItems = new array<ItemBase>;
         #ifdef SERVER
             SortLimitTime = g_CargoSortTimeLimit;
             SetSynchDirty();
         #endif
     }
 
     override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
     {
         super.OnRPC(sender, rpc_type, ctx);
         Object entityIB;
         Param1<Object> param;
         if (rpc_type == BACKPACKSORT.RE_CARGO)
         {
             if (ctx.Read(param))
             {
                 entityIB = param.param1;
                 if (entityIB && entityIB.IsEntityAI())
                 {
                     EntityCargoSort(entityIB, true);
                 }
             }
         }
         if (rpc_type == BACKPACKSORT.ADD_CARGO)
         {
             if (ctx.Read(param))
             {
                 entityIB = param.param1;
                 if (entityIB && entityIB.IsEntityAI())
                 {
                     EntityCargoSort(entityIB);
                 }
             }
         }
     }
 
     void EntityCargoSort(Object obj, bool isReCargo = false)
     {
         int nowTime = GetGame().GetTime();
         if (m_LastSortTime > 0 && nowTime - m_LastSortTime < SortLimitTime * 1000)
         {
             return;
         }
         EntityAI entity = EntityAI.Cast(obj);
         if (!entity) return;
         CargoBase cargo = entity.GetInventory().GetCargo();
         if (!cargo) return;
         int cargoWidth = cargo.GetWidth();
         int cargoHeight = cargo.GetHeight();
         int itemCount = cargo.GetItemCount();
         m_CachedEntityProperties.Clear();
         m_CachedCargoItems.Clear();
         CollectCargoItems(cargo, itemCount, m_CachedCargoItems, m_CachedEntityProperties);
         if (isReCargo)
         {
             SortEntityPropertiesByAreaAndName(m_CachedEntityProperties);
             array<ref array<int>> cargoGrid = InitializeCargoGrid(cargoWidth, cargoHeight);
             PlaceEntitiesInCargo(m_CachedEntityProperties, cargo, cargoGrid, cargoWidth, cargoHeight);
         }
         else
         {
             CombineStackableItems(m_CachedCargoItems);
         }
         m_LastSortTime = nowTime;
     }

     private void CollectCargoItems(CargoBase cargo, int itemCount, ref array<ItemBase> cargoItems, ref array<ref EntityProperty> entityProperties)
     {
         for (int i = 0; i < itemCount; i++)
         {
             EntityAI item = cargo.GetItem(i);
             if (!item) continue;
             cargoItems.Insert(ItemBase.Cast(item));
             ref InventoryLocation itemsrc = new InventoryLocation();
             item.GetInventory().GetCurrentInventoryLocation(itemsrc);
             bool flip = itemsrc.GetFlip();
             int w, h;
             cargo.GetItemSize(i, w, h);
             if (flip)
             {
                 int _temp = w;
                 w = h;
                 h = _temp;
             }
             ref EntityProperty entityProperty = new EntityProperty(item, w, h, flip);
             entityProperties.Insert(entityProperty);
         }
     }
 
     private void SortEntityPropertiesByAreaAndName(ref array<ref EntityProperty> entityProperties)
     {
         for (int i = 1; i < entityProperties.Count(); i++)
         {
             ref EntityProperty key = entityProperties[i];
             int j = i - 1;
             EntityAI keyEntity = key.entity;
             string keyName = keyEntity.GetType();
             while (j >= 0)
             {
                 EntityAI currentEntity = entityProperties[j].entity;
                 string currentName = currentEntity.GetType();
                 if (entityProperties[j].area > key.area || (entityProperties[j].area == key.area && currentName <= keyName))
                     break;
                 entityProperties[j + 1] = entityProperties[j];
                 j = j - 1;
             }
             entityProperties[j + 1] = key;
         }
     }
 
     array<ref array<int>> InitializeCargoGrid(int cargoWidth, int cargoHeight)
     {
         array<ref array<int>> cargoGrid = new array<ref array<int>>;
         for (int y = 0; y < cargoHeight; y++)
         {
             array<int> row = new array<int>;
             for (int x = 0; x < cargoWidth; x++)
             {
                 row.Insert(0);
             }
             cargoGrid.Insert(row);
         }
         return cargoGrid;
     }
 
     private void PlaceEntitiesInCargo(ref array<ref EntityProperty> entityProperties, CargoBase cargo, 
         ref array<ref array<int>> cargoGrid, int cargoWidth, int cargoHeight)
     {
         foreach (ref EntityProperty entityProp : entityProperties)
         {
             ItemBase eAI = ItemBase.Cast(entityProp.entity);
             if (!eAI) continue;
             InventoryLocation src = new InventoryLocation();
             eAI.GetInventory().GetCurrentInventoryLocation(src);
             InventoryLocation dst = new InventoryLocation();
             bool placed = false;
             placed = TryPlaceItem(cargoGrid, entityProp, cargo, src, dst, cargoWidth, cargoHeight);
             if (!placed)
             {
                 int temp = entityProp.width;
                 entityProp.width = entityProp.height;
                 entityProp.height = temp;
                 TryPlaceItem(cargoGrid, entityProp, cargo, src, dst, cargoWidth, cargoHeight, true);
             }
         }
     }
 
     bool TryPlaceItem(ref array<ref array<int>> cargoGrid, ref EntityProperty entityProp, CargoBase cargo, 
         InventoryLocation src, InventoryLocation dst, int cargoWidth, int cargoHeight, bool flip = false)
     {
         for (int y = 0; y <= cargoHeight - entityProp.height; y++)
         {
             for (int x = 0; x <= cargoWidth - entityProp.width; x++)
             {
                 if (CanPlaceItem(cargoGrid, x, y, entityProp.width, entityProp.height))
                 {
                     PlaceItem(cargoGrid, x, y, entityProp.width, entityProp.height);
                     if (flip)
                     {
                         dst.SetCargoAuto(cargo, ItemBase.Cast(entityProp.entity), y, x, true);
                     }
                     else
                     {
                         dst.SetCargoAuto(cargo, ItemBase.Cast(entityProp.entity), y, x, entityProp.isFlipped);
                     }
                     InventoryInputUserData.SendServerMove(null, InventoryCommandType.SYNC_MOVE, src, dst);
                     GameInventory.LocationSyncMoveEntity(src, dst);
                     return true;
                 }
             }
         }
         return false;
     }
 
     void CombineStackableItems(ref array<ItemBase> items)
     {
         for (int i = 0; i < items.Count(); i++)
         {
             ItemBase itemA = items[i];
             if (!itemA) continue;
             for (int j = i + 1; j < items.Count(); j++)
             {
                 ItemBase itemB = items[j];
                 if (!itemB || !itemA.CanBeCombined(itemB, false))
                     continue;
                 if (itemA.IsInherited(Ammunition_Base))
                 {
                     Magazine magA = Magazine.Cast(itemA);
                     if (magA && magA.GetAmmoCount() > 0)
                     {
                         magA.CombineItems(itemB);
                     }
                 }
                 else
                 {
                     itemA.CombineItems(itemB);
                 }
                 if (itemB.GetQuantity() <= 0)
                 {
                     items.Remove(j);
                     j--;
                 }
             }
         }
     }
 
     bool CanPlaceItem(array<ref array<int>> grid, int x, int y, int width, int height)
     {
         // Added boundary checks to prevent array index out-of-bounds errors
         if (x + width > grid[0].Count() || y + height > grid.Count()) 
             return false;
         for (int i = 0; i < height; i++)
         {
             for (int j = 0; j < width; j++)
             {
                 if (grid[y + i][x + j] != 0)
                 {
                     return false;
                 }
             }
         }
         return true;
     }
 
     void PlaceItem(array<ref array<int>> grid, int x, int y, int width, int height)
     {
         for (int i = 0; i < height; i++)
         {
             for (int j = 0; j < width; j++)
             {
                 grid[y + i][x + j] = 1;
             }
         }
     }
 }