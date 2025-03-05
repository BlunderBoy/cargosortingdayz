#ifdef SERVER
    static int CARGO_SORT_DEFAULT_TIME_LIMIT = 2;
    static int g_CargoSortTimeLimit = CARGO_SORT_DEFAULT_TIME_LIMIT;
#endif

enum BACKPACKSORT
{
    RE_CARGO = 5452188,  // Reorganize cargo items by size and type
    ADD_CARGO = 5452199  // Combine stackable items within cargo
};