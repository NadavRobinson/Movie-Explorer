#pragma once

#include <vector>
#include "commonObject.h"

// The DrawThread class handles the graphical user interface (GUI) operations.
// It interacts with the common object to perform GUI-related tasks.
class DrawThread
{
public:
    // Public method to initiate the main GUI operations.
    // It takes a reference to a common object.
    void call_Gui_main(common& common);

private:
    // Private method to initialize the genre list.
    // It takes a reference to a common object and sets up the genre data for GUI display.
    void initializeGenreList(common& common);
};
