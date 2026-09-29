#include "raylib.h"
#include <cstdio>

#define COLS 5
#define ROWS 5
#define MAX_ITEMS 1244

// ITEM CACHING AND CLEARANCE
Texture2D itemCache[MAX_ITEMS + 1] = {0};

// CONST VARIABLES
const int ScrHeight = 500;
const int ScrWidth = 800;

const int GameScrHeight = 500;
const int GameScrWidth = 500;

const int CellWidth = GameScrWidth / COLS;
const int CellHeight = GameScrHeight / ROWS;

// OBJECT-STRUCTS
struct Item{
    int stack = 10;
    int price = GetRandomValue(1, 1000);
    int count = 0;
    int ItemID = 0;
};             
struct Cell{
    int col;
    int row;
    Item item;
};

// DEFINING GLOBAL VARIABLES

Item IsDragged; // isDragged, a copy struct of item to use the copy instead of original item when dragging it into different cell.
int dragSourceSlot; // Source cell of where we drug Items from.
bool showToolTip = false; // Whether we show the tooltip or not
int tooltipItemCol = 0; // Col and Row of item that shows the tooltip
int tooltipItemRow = 0;
float btnAddCooldown = 0.0f; // Cooldowns for buttons, to implement clicking animation
float btnSellCooldown = 0.0f;
int gold = 0; // Amount of gold we receive from selling items


// FUNCTION DECLARATION
void AddRandomItem(Cell inventory[5][5], Sound fxAddItem); // Adds a random item to the nearest available slot with an index between 0 and 24
void DrawInventory(Cell inventory[5][5], Texture2D& frame); // Draws inventory and UI
Texture2D GetItemTexture(int itemID); // Gets Item Texture by using lazy loading (when we need it)

// ENTRY POINT
int main(){
    InitWindow(ScrWidth, ScrHeight, "Advanced_Inventory_System 1.0.0");
    InitAudioDevice();
    
    // Define Button Bounds and Rotation Origin to detect clicks
    Rectangle btnAddBounds = {500, 0, 300, 100};
    Vector2 btnAddOrigin = {0.0f, 0.0f};
    Rectangle boxSellBounds = {500, 400, 100, 100};
    Rectangle btnSellAllBounds = {600, 400, 200, 100};
    Vector2 btnDeleteOrigin = {0.0f, 0.0f};

    // UI TEXTURES
    // Loading Texture2D to use them later in while(!WindowShouldClose())
    Texture2D frame = LoadTexture("../resources/ui/ItemFrame.png");
    Texture2D btnAddInactive = LoadTexture("../resources/ui/btnAddItemInactive.png");
    Texture2D btnAddPressed = LoadTexture("../resources/ui/btnAddItemPressed.png");
    Texture2D SellZone = LoadTexture("../resources/ui/SellZone.png");
    Texture2D GoldenCoin = LoadTexture("../resources/ui/GoldenCoin.png");
    Texture2D MouseCursor = LoadTexture("../resources/ui/RedCursor.png");
    Texture2D btnSellAllInactive = LoadTexture("../resources/ui/btnSellAllInactive.png");
    Texture2D btnSellAllPressed = LoadTexture("../resources/ui/btnSellAllPressed.png");
    
    // SOUND CUES
    Sound fxAddItem = LoadSound("../resources/sounds/fxButtonAddItem.wav");
    Sound fxButtonPress = LoadSound("../resources/sounds/fxButtonAddItemPressed.wav");
    Sound fxSell = LoadSound("../resources/sounds/cha-ching.wav");
    
    Cell inventory[5][5] = {0}; // Assign twenty-five elements with 0 at the start,
                                // to make cells for items
    // Setting FPS
    int currentFPS(144);
    SetTargetFPS(currentFPS);

    // [LOG] Window Initialization
    printf("[LOG] Window initialized!\n");
    while (!WindowShouldClose())
    {
        // Get Mouse position and hide cursor.
        Vector2 mousePos = GetMousePosition(); 
        HideCursor();

        // Get Columns and Rows of where we clicked
        int Col = mousePos.x / CellWidth; 
        int Row = mousePos.y / CellHeight;
        
        // Two different cooldowns for buttons that we will apply later on.
        if(btnAddCooldown > 0) btnAddCooldown -= GetFrameTime();
        if(btnSellCooldown > 0) btnSellCooldown -= GetFrameTime();

        Rectangle destRect = {              // Offset by half cell size to center the texture on the mouse cursor                   
            mousePos.x - CellWidth / 2,         
            mousePos.y - CellHeight / 2,        
            (float)(CellWidth),                 
            (float)(CellHeight)
        };

        // Nested Loops to get 25 indexes for Drag&Drop Systems
        for(int col = 0; col < COLS; ++col){
            for(int row = 0; row < ROWS; ++row){

                Rectangle slotRect = {          // Item Slot, rectangle struct to define 
                    (float)(col * CellWidth),   // which cell we are editing
                    (float)(row * CellHeight),
                    (float)(CellWidth),
                    (float)(CellHeight)
                };

                // Checking which slot we are hitting with mouse
                if(CheckCollisionPointRec(mousePos, slotRect)){
                    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ 
                        if(inventory[col][row].item.ItemID != 0){
                            IsDragged = inventory[col][row].item; // Write copy with the original item
                            dragSourceSlot = col * ROWS + row; // Source Index to determine where would we put item back in the future
                            inventory[col][row].item = {}; // Reset the original item to 0, to not make two identical copies.
                            showToolTip = false; // Do not show a tooltip if Item is picked up
                        }
                    }
                    // If we hold right-click, we can show tooltip
                    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
                        if(inventory[col][row].item.ItemID != 0){
                            showToolTip = true;
                        }
                    } // On release, we stop showing it
                    if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
                        showToolTip = false;
                    }
                    // If we release dragged item on an empty slot, we assign the copied item to that slot
                    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
                        if(inventory[col][row].item.ItemID == 0){ // Empty Slot
                            inventory[col][row].item = IsDragged;
                            IsDragged = {};
                            dragSourceSlot = -1; // Set DragSourceSlot to -1, to make sure we don't have identical Source Slot on the next instance
                        }
                        // If we try to drop items in occupied slots, we swap them
                        else if(inventory[col][row].item.ItemID > 0){ // Slot isn't Empty
                            Item Temp = inventory[col][row].item; // We make a temporary Item copy, so we can swap items.
                            inventory[col][row].item = IsDragged; // Items are being swapped
                            IsDragged = Temp;
                        }
                    }
                }
            }
        }

        // BEGIN TO DRAW
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawInventory(inventory, frame); // Draw 5x5 Inventory grid with frame texture
        
        // Draw appropriate decorative-ui textures
        DrawTexture(SellZone, 500, 400, WHITE);
        DrawTexture(btnSellAllInactive, 600, 400, WHITE);
        DrawTexture(GoldenCoin, 520, 110, WHITE);
        
        // Write useful text on in-game ui for players
        DrawText(TextFormat("Total Gold Amount: %i", gold), 575, 130, 19, WHITE);
        DrawText("Hold LMB to drag Items", 510, 180, 19, LIGHTGRAY);
        DrawText("Hold RMB on Items to\nshow tooltips", 510, 210, 19, LIGHTGRAY);
        DrawText("Drag Items individually to\ndollar sign or click sell all", 510, 255, 19, LIGHTGRAY);
        
        // Set button collisions to "Add Random Item" button
        if (CheckCollisionPointRec(mousePos, btnAddBounds)){
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                // Here we actually setting cooldown for button, when While-Loop ends, it will reset the cooldown
                btnAddCooldown = 0.2f;
                // Calling Function that adds items for us
                AddRandomItem(inventory, fxAddItem);
                // Play "Button pressing" sound
                PlaySound(fxButtonPress);
            }
        }
        // If Cooldown is more than zero we re-draw "Pressed" version of the texture
        // to imitate button press, and make smooth click animation
        if(btnAddCooldown > 0){
            DrawTexture(btnAddPressed, 500, 0, WHITE);
        }
        else {
            DrawTexture(btnAddInactive, 500, 0, WHITE);
        }
        // If we drag an item to a sell slot we sell this individual item
        if (CheckCollisionPointRec(mousePos, boxSellBounds)){
            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                if(IsDragged.ItemID != 0){
                    gold += IsDragged.count * IsDragged.price; // gold = gold + copied item count * by its price, 
                    PlaySound(fxSell);                          // this way, if items are stacked we always receive full money for each of them
                    IsDragged = {}; // Reset the Drag Copy of the item
                    dragSourceSlot = -1;
                }
            }
        }
        // Sells all items for us
        if(CheckCollisionPointRec(mousePos, btnSellAllBounds)){
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                for(int i = 0; i < COLS; ++i){
                    for(int j = 0; j < ROWS; ++j){
                        if(inventory[i][j].item.ItemID != 0)
                        {
                            // Loops through every index, and if items are not equal to zero
                            // We sell them
                            btnSellCooldown = 0.2f;
                            gold += inventory[i][j].item.count * inventory[i][j].item.price;
                            PlaySound(fxSell);
                            PlaySound(fxButtonPress);
                            inventory[i][j].item = {};
                        }
                    }
                }
            }
        }
        // Another cooldown but for "Sell All" button
        if(btnSellCooldown > 0){
            DrawTexture(btnSellAllPressed, 600, 400, WHITE);
        }
        else {
            DrawTexture(btnSellAllInactive, 600, 400, WHITE);
        }

        // Scale up item's Texture to make it look more neat
        if(IsDragged.ItemID != 0){

            Texture2D itemTex = GetItemTexture(IsDragged.ItemID);                               // itemTex stores the item's id, this way we 
            Rectangle sourceRec = {0.0f, 0.0f, (float)itemTex.width, (float)itemTex.height};    // know what texture we should draw when we grab an item from a slot
            // sourceRec gets Textures original size
            DrawTexturePro(itemTex, sourceRec, destRect, {0, 0}, 0.0f, WHITE); // Draw the Item texture in our Mouse cursor
        }
        // If we drop an Item outside of game's UI, item will get back to its original position
        // This is where dragSourceSlot variable is getting actually used
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            if(IsDragged.ItemID != 0){
                // Convert flat dragSourceSlot index back to 2D col/row coordinates
                // e.g. index 6 -> col = 6/5 = 1, row 6%5 = 1
                int col = dragSourceSlot / ROWS;
                int row = dragSourceSlot % ROWS;

                inventory[col][row].item = IsDragged;
                IsDragged = {};
                dragSourceSlot = -1;
            }
        }
        
        // ToolTip position
        Rectangle tooltipSlot = {
            (float)(tooltipItemCol * CellWidth),
            (float)(tooltipItemRow * CellHeight),
            (float)CellWidth,
            (float)CellHeight
        };
        // Custom Mouse size, source texture
        Rectangle sourceMouseSize = {
            0.0f,
            0.0f, 
            (float)MouseCursor.width, 
            (float)MouseCursor.height
        }; // Actual resized mouse texture
        Rectangle destMouse = {
            mousePos.x,
            mousePos.y,
            75, // width
            75 // height
        };
        // Draw Custom Mouse texture
        // Wrote it here so Mouse Cursor would be the last to be rendered, so it doesn't overlap with other in-game UI e.g. buttons, text and images etc.
        DrawTexturePro(MouseCursor, sourceMouseSize, destMouse, {0.0f, 0.0f}, 0.0f, WHITE);
        // Define the position of where we draw Tooltip box with item's information
        if(!CheckCollisionPointRec(mousePos, tooltipSlot))
        {
            if(inventory[Col][Row].item.ItemID != 0 && showToolTip){
                tooltipItemCol = Col;
                tooltipItemRow = Row;
            } else {
                showToolTip = false;
            }
        }
        // When showing Tooltip, write this information in the blue box
        if(showToolTip){
            DrawRectangle(mousePos.x, mousePos.y, 200, 400, BLUE);
            DrawText(TextFormat("Item ID: %i", inventory[tooltipItemCol][tooltipItemRow].item.ItemID), mousePos.x+10, mousePos.y+10, 20, WHITE);
            DrawText(TextFormat("Item Count: %i", inventory[tooltipItemCol][tooltipItemRow].item.count), mousePos.x+10, mousePos.y+30, 20, WHITE);
            DrawText(TextFormat("Item Price: %i", inventory[tooltipItemCol][tooltipItemRow].item.price), mousePos.x+10, mousePos.y+50, 20, WHITE);
        }

        EndDrawing();
    }

    // De-Initialization
    // Unloading all game sounds and textures to free space quicker.
    UnloadSound(fxAddItem);
    UnloadSound(fxButtonPress);
    UnloadSound(fxSell);
    UnloadTexture(frame);
    UnloadTexture(btnAddInactive);
    UnloadTexture(btnAddPressed);
    UnloadTexture(SellZone);
    UnloadTexture(GoldenCoin);
    UnloadTexture(MouseCursor);
    UnloadTexture(btnSellAllInactive);
    UnloadTexture(btnSellAllPressed);
    CloseAudioDevice();

    // Clear all Item Textures that we used
    // over the course of time using the application
    for(int i = 1; i <= MAX_ITEMS; ++i){
        if(itemCache[i].id != 0){
            UnloadTexture(itemCache[i]);
        }
    }
    // Closing the application
    CloseWindow();
    return 0;
}


// FUNCTION DEFINITIONS
void DrawInventory(Cell inventory[5][5], Texture2D& frame){
    // Get 25 indexes, each represents a cell
    for(int col = 0; col < COLS; ++col)
    {
        for(int row = 0; row < ROWS; ++row)
        {
            // Position X = current column * Cell Width
            // Position Y = current row * Cell Height
            // This gives us perfect position (top-left corner of the cell)
            int posX = col * CellWidth;
            int posY = row * CellHeight;
            DrawTexture(frame, posX, posY, WHITE); // For each cell draw it's own frame texture

            // local variable id, represents Item's id
            int id = inventory[col][row].item.ItemID;
            if (id > 0)
            {
                Texture2D itemTex = GetItemTexture(id);
                // get original size of the texture
                Rectangle sourceRec = {0.0f, 0.0f, (float)itemTex.width, (float)itemTex.height};
                // Padding for both x and y axis, to make it look more neat
                int padding = 10;

                Rectangle destRec = {           // Destination of where we should draw these textures
                    (float)(posX + padding),
                    (float)(posY + padding),
                    (float)(CellWidth - padding * 2),
                    (float)(CellHeight - padding * 2)
                };

                Vector2 origin = {0.0f, 0.0f};
                DrawTexturePro(itemTex, sourceRec, destRec, origin, 0.0f, WHITE);
            }
            // Draw a small number in bottom right corner, that represents if items are stacked or not
            // Will work only if Items of the same type are more than one
            if(inventory[col][row].item.ItemID > 0 && inventory[col][row].item.count > 1){
                DrawText(TextFormat("%i", inventory[col][row].item.count), posX+85, posY+80, 20, RAYWHITE);
            }
        }
    }
}

Texture2D GetItemTexture(int ItemID){
    // id == 0 means texture not yet loaded (raylib default for empty Texture2D)
    if(itemCache[ItemID].id == 0){
        char path[64]; // Allocating 64 bytes that will represent path to textures
        // snprintf, gets the path and assigns ItemID to each texture
        snprintf(path, sizeof(path), "../resources/items/item%d.png", ItemID);
        // Load texture into itemCache, assigning it a unique GPU ID
        // that will help us to identify them
        itemCache[ItemID] = LoadTexture(path);

        // Filters Textures, so we can resize them without blur (due to them being PixelArt)
        SetTextureFilter(itemCache[ItemID], TEXTURE_FILTER_POINT);
        // [LOG] Loaded Textures into VRAM 
        printf("[LOG] Loaded texture for ItemID %d into cache\n", ItemID);
    }

    // Return the texture with unique id
    return itemCache[ItemID];
}

void AddRandomItem(Cell inventory[5][5], Sound fxAddItem)
{
    // Set Max and Min amounts of items that can randomly be added
    int itemMax = 1244;
    int itemMin = 1;

    // Random Item variable
    int newItemID = GetRandomValue(itemMin, itemMax);

    // [DEBUG] Item IDs that were chosen 
    printf("[DEBUG] Rerolled Item ID: %d\n", newItemID);
    // Nested-Loop 1
    // Check whether items could be stacked up
    for(int row = 0; row < ROWS; ++row){
        for(int col = 0; col < COLS; ++col){
            if(inventory[col][row].item.ItemID == newItemID) // Check whether chosen slot has the same item type
            {
                if(inventory[col][row].item.count == inventory[col][row].item.stack){ // If stack is full, find next available index
                    continue;
                }
                // If not, stack the items
                inventory[col][row].item.count++;
                PlaySound(fxAddItem);
                // [LOG] Items are stacked, show item IDs and its new count
                printf("[LOG] Item Added\n"); 
                printf("[LOG] Stacked ItemID %d (Count: %d)\n", newItemID, inventory[col][row].item.count); 
                return;
            }
        }
    }
    // Nested-Loop 2
    // If items could not be stacked, find next available slot/index
    for(int row = 0; row < ROWS; ++row){
        for (int col = 0; col < COLS; ++col)
        {
            if(inventory[col][row].item.ItemID == 0){
                inventory[col][row].item.ItemID = newItemID; // Add Item into chosen slot
                inventory[col][row].item.count = 1; // Avoid undefined behaviour, if in our Item struct we didn't set the count to 0
                PlaySound(fxAddItem);
                // [LOG] New Item was added into the inventory.
                printf("[LOG] New Item Added to empty slot!\n");
                return;
            }
        }
    }
}