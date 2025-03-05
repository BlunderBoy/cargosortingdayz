class EntityProperty
{
    EntityAI    entity;
    int         area;
    int         width, height;
    bool        flip;

    void EntityProperty(EntityAI _entity, int _width, int _height, bool _flip)
    {
        entity  = _entity;
        width   = _width;
        height  = _height;
        area    = _width * _height;
        flip    = _flip;
    }
}

modded class PlayerBase extends ManBase
{
    int nowTime = 0, lastTime = 0;
    int SortLimitTime;

    void PlayerBase()
    {
        RegisterNetSyncVariableInt("SortLimitTime", 1, 99);

#ifdef SERVER
        SortLimitTime = G_SortTimeLimit;
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
                EntityCargoSort(entityIB, true);
            }
        }

        if (rpc_type == BACKPACKSORT.ADD_CARGO)
        {
            if (ctx.Read(param))
            {
                entityIB = param.param1;
                EntityCargoSort(entityIB);
            }
        }
    }

    void EntityCargoSort(Object obj, bool isReCargo = false)
    {
        nowTime = GetGame().GetTime();
        if (lastTime > 0 && nowTime - lastTime < SortLimitTime * 1000)
        {
            return;
        }

        EntityAI entity = EntityAI.Cast(obj);
        if (!entity)
        {
            return;
        }

        CargoBase cargo = entity.GetInventory().GetCargo();
        int cargoWidth = cargo.GetWidth();
        int cargoHeight = cargo.GetHeight();
        int itemCount = cargo.GetItemCount();

        ref array<ref EntityProperty> entityProperties = new array<ref EntityProperty>;
        ref array<ItemBase> cargoItems = new array<ItemBase>;

        CollectCargoItems(cargo, itemCount, cargoItems, entityProperties);

        // Declare variables once at function scope
        EntityAI item;
        InventoryLocation src = new InventoryLocation();
        InventoryLocation dst = new InventoryLocation();

        if (isReCargo)
        {

            // Step 1: Drop all items on the ground
            vector dropPos = entity.GetPosition() + Vector(1, 0, 1); // Offset slightly from entity
            for (int i = 0; i < entityProperties.Count(); i++)
            {
                item = entityProperties[i].entity;
                item.GetInventory().GetCurrentInventoryLocation(src);
                dst.SetGround(item, dropPos);
                GameInventory.LocationSyncMoveEntity(src, dst);
                InventoryInputUserData.SendServerMove(null, InventoryCommandType.SYNC_MOVE, src, dst); // Added for client sync
            }

            // Step 2: Sort the items
            SortEntityPropertiesByAreaAndName(entityProperties);

            // Step 3: Place items back into cargo
            array<ref array<int>> cargoGrid = InitializeCargoGrid(cargoWidth, cargoHeight);
            foreach (ref EntityProperty entityProp : entityProperties)
            {
                item = entityProp.entity;
                string itemName = item.GetType();

                item.GetInventory().GetCurrentInventoryLocation(src);
                bool placed = false;

                for (int y = 0; y <= cargoHeight - entityProp.height; y++)
                {
                    for (int x = 0; x <= cargoWidth - entityProp.width; x++)
                    {
                        if (CanPlaceItem(cargoGrid, x, y, entityProp.width, entityProp.height))
                        {
                            PlaceItem(cargoGrid, x, y, entityProp.width, entityProp.height);
                            dst.SetCargoAuto(cargo, ItemBase.Cast(item), y, x, entityProp.flip);
                            GameInventory.LocationSyncMoveEntity(src, dst);
                            InventoryInputUserData.SendServerMove(null, InventoryCommandType.SYNC_MOVE, src, dst); // Added for client sync
                            placed = true;
                            break;
                        }
                    }
                    if (placed)
                        break;
                }
            }
        }
        else
        {
            CombineStackableItems(cargoItems);
        }

        lastTime = nowTime;
    }

    void CollectCargoItems(CargoBase cargo, int itemCount, ref array<ItemBase> cargoItems, ref array<ref EntityProperty> entityProperties)
    {
        for (int i = 0; i < itemCount; i++)
        {
            EntityAI item = cargo.GetItem(i);
            if (!item)
            {
                continue;
            }

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
            else
            {
            }

            ref EntityProperty entityProperty = new EntityProperty(item, w, h, flip);
            entityProperties.Insert(entityProperty);
        }
    }

    void SortEntityPropertiesByAreaAndName(ref array<ref EntityProperty> entityProperties)
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
                bool shouldBreak = entityProperties[j].area > key.area || (entityProperties[j].area == key.area && currentName <= keyName);

                if (shouldBreak)
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

    void PlaceEntitiesInCargo(ref array<ref EntityProperty> entityProperties, CargoBase cargo, ref array<ref array<int>> cargoGrid, int cargoWidth, int cargoHeight)
    {
        foreach (ref EntityProperty entityProp : entityProperties)
        {
            ItemBase eAI = ItemBase.Cast(entityProp.entity);
            string itemName = eAI.GetType();

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
                placed = TryPlaceItem(cargoGrid, entityProp, cargo, src, dst, cargoWidth, cargoHeight, true);
            }
            if (!placed)
            {
            }
        }
    }

    bool TryPlaceItem(ref array<ref array<int>> cargoGrid, ref EntityProperty entityProp, CargoBase cargo, InventoryLocation src, InventoryLocation dst, int cargoWidth, int cargoHeight, bool flip = false)
    {
        string itemName = entityProp.entity.GetType();

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
                        dst.SetCargoAuto(cargo, ItemBase.Cast(entityProp.entity), y, x, entityProp.flip);
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
            if (!itemA)
            {
                continue;
            }

            for (int j = i + 1; j < items.Count(); j++)
            {
                ItemBase itemB = items[j];
                if (!itemB || !itemA.CanBeCombined(itemB, false))
                {
                    continue;
                }

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
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (x + j >= grid[0].Count() || y + i >= grid.Count() || grid[y + i][x + j] != 0)
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