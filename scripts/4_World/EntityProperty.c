 class EntityProperty
 {
     EntityAI entity;
     int width;
     int height;
     int area;       // Precalculated area (width * height) for sorting optimization
     bool isFlipped; // Whether item is rotated in cargo grid
     string typeName; // Item type name for stable sorting
     
     void EntityProperty(EntityAI _entity, int _width, int _height, bool _isFlipped)
     {
         entity = _entity;
         width = _width;
         height = _height;
         area = _width * _height;
         isFlipped = _isFlipped;
         
         if (entity)
         {
             typeName = entity.GetType();
         }
     }
 }