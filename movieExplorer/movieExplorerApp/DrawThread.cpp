#include "DrawThread.h"
#include "GuiMain.h"
#include "../3rd_party/ImGuiSource/imgui.h"
#include <stdlib.h>
#include <thread>
#include <string>
#include <iostream>
#include <unordered_set>
#include <fstream>
#include <windows.h> // Include Windows API for PostQuitMessage function
#include <algorithm>

// A global set to store favorite movie titles.
std::unordered_set<std::string> favorites;

// Function to load favorite movies from a file into the favorites set.
void loadFavoritesFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        return;
    std::string currentLine;
    while (std::getline(file, currentLine))
        favorites.insert(currentLine);
    file.close();
}

// Function to save favorite movies from the favorites set to a file.
void saveFavoritesToFile(const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& title : favorites)
        file << title << std::endl;
    file.close();
}

// The main function to draw the application window using ImGui.
void DrawAppWindow(void* common_ptr)
{
    static char searchBuffer[128] = "";
    static bool showFavorites = false;
    static int currentGenreIndex = -1; // Start with -1 to indicate no selection
    static int selectedMovieIndex = -1;
    auto commonObj = (common*)common_ptr;

    ImGui::Begin("Movie Explorer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

    // Set the window position and size
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

    // Setup a menu bar with a centered title and a close button.
    if (ImGui::BeginMenuBar())
    {
        // Center the title text
        float windowWidth = ImGui::GetWindowWidth();
        float textWidth = ImGui::CalcTextSize("Movie Explorer").x;
        float centerPos = (windowWidth - textWidth) * 0.5f;
        ImGui::SetCursorPosX(centerPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.843f, 0.0f, 1.0f)); // Gold color
        ImGui::Text("Movie Explorer");
        ImGui::PopStyleColor();

        // Right-side position for the close button
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 67);
        if (ImGui::Button("Close"))
        {
            ::PostQuitMessage(0);
        }
        ImGui::EndMenuBar();
    }

    // Setup a search bar and buttons for searching and clearing the table.
    ImGui::Separator();
    ImGui::SetNextItemWidth(500);
    bool searchTriggered = ImGui::InputTextWithHint("##SearchBar", "Search Movie", searchBuffer, IM_ARRAYSIZE(searchBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("Search") || searchTriggered) {
        if (searchBuffer[0] != '\0') {
            std::lock_guard<std::mutex> lock(commonObj->mtx);
            commonObj->sharedInput = searchBuffer;
            commonObj->newSearchAvailable = true;
            commonObj->cv.notify_one();
            searchBuffer[0] = '\0';
            currentGenreIndex = -1;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear Table")) {
        commonObj->newTableData = false;
        currentGenreIndex = -1;
    }

    ImGui::Separator();

    // Toggle display of favorites list.
    if (ImGui::Button("Show Favorites")) {
        showFavorites = !showFavorites;
    }

    ImGui::SameLine();

    // Dropdown menu for genre selection.
    const char* currentGenre = (currentGenreIndex >= 0) ? commonObj->genreList[currentGenreIndex].c_str() : "Select a genre";

    ImGui::SetNextItemWidth(170);
    if (ImGui::BeginCombo("##GenreCombo", currentGenre))
    {
        for (int i = 0; i < commonObj->genreList.size(); i++)
        {
            bool isSelected = (currentGenreIndex == i);
            if (ImGui::Selectable(commonObj->genreList[i].c_str(), isSelected))
            {
                currentGenreIndex = i;
                std::lock_guard<std::mutex> lock(commonObj->mtx);
                commonObj->sharedInput = commonObj->genreList[i];
                commonObj->newGenreAvailable = true;
                commonObj->cv.notify_one();
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Display favorites if the toggle is enabled.
    if (showFavorites) {
        ImGui::Separator();

        // Styling for the "Favorites" header.
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.8f, 1.0f));
        float originalFontSize = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale *= 1.3f; // Increase font size by 30%
        ImGui::PushFont(ImGui::GetFont());

        ImGui::Text("Favorites");

        ImGui::GetFont()->Scale = originalFontSize;
        ImGui::PopFont();
        ImGui::PopStyleColor();

        ImGui::Separator();

        for (const auto& title : favorites) {
            ImGui::BulletText("%s", title.c_str());
        }

        ImGui::Separator();
    }

    // Movie table section
    if (commonObj->newTableData && !commonObj->movieList.empty()) {
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));  // Header row background
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(0.17f, 0.17f, 0.17f, 1.00f));  // Even row background
        ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f));     // Odd row background

        int sortColumn = -1;
        bool sortAscending = true;

        if (ImGui::BeginTable("Movies", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Sortable))
        {
            // Setup columns
            ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthFixed, 1200.0f);
            ImGui::TableSetupColumn("Language", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Release Date", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Popularity", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort);
            ImGui::TableSetupColumn("Favorite", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort);

            // Customize header row
            float cellPaddingY = ImGui::GetStyle().CellPadding.y;
            ImGui::GetStyle().CellPadding.y = 6.0f;
            ImGui::TableHeadersRow();
            ImGui::GetStyle().CellPadding.y = cellPaddingY;

            // Handle sorting
            ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs();
            if (sorts_specs && sorts_specs->SpecsDirty)
            {
                sortColumn = sorts_specs->Specs->ColumnIndex;
                sortAscending = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending;

                std::sort(commonObj->movieList.begin(), commonObj->movieList.end(),
                    [&](const auto& a, const auto& b)
                    {
                        switch (sortColumn)
                        {
                        case 0: return sortAscending ? a.title < b.title : a.title > b.title;
                        case 1: return sortAscending ? a.original_language < b.original_language : a.original_language > b.original_language;
                        case 2: return sortAscending ? a.release_date < b.release_date : a.release_date > b.release_date;
                        case 3: return sortAscending ? a.popularity < b.popularity : a.popularity > b.popularity;
                        default: return false;
                        }
                    });

                sorts_specs->SpecsDirty = false;
            }

            // Render table rows
            for (int i = 0; i < commonObj->movieList.size(); ++i)
            {
                auto& movie = commonObj->movieList[i];
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", movie.title.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", movie.original_language.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", movie.release_date.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.1f", movie.popularity);
                ImGui::TableSetColumnIndex(4);
                if (ImGui::Button(("Details##" + std::to_string(i)).c_str())) {
                    selectedMovieIndex = (selectedMovieIndex == i) ? -1 : i;
                }
                ImGui::TableSetColumnIndex(5);
                bool isFavorite = favorites.find(movie.title) != favorites.end();
                if (ImGui::Checkbox(("##Fav" + std::to_string(i)).c_str(), &isFavorite)) {
                    if (isFavorite) {
                        favorites.insert(movie.title);
                    }
                    else {
                        favorites.erase(movie.title);
                    }
                }

                if (selectedMovieIndex == i) {	// Show details if the "Details" button is pressed
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    float originalFontSize = ImGui::GetFont()->Scale;
                    ImGui::GetFont()->Scale *= 1.1f;
                    ImGui::PushFont(ImGui::GetFont());
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
                    ImGui::Text("Overview");
                    ImGui::GetFont()->Scale = originalFontSize;
                    ImGui::PopFont();

                    ImGui::SameLine();
                    ImGui::TextDisabled("(Click 'Details' again to hide)");
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    ImGui::TextWrapped("%s", movie.overview.c_str());
                    ImGui::Spacing();
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::TableSetColumnIndex(5);
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleColor(3);
    }
    else
        ImGui::Text("No data available");

    ImGui::End();
}

// Main function to initialize and start the GUI.
void DrawThread::call_Gui_main(common& common)
{
    // Initialize the genre list and load favorites.
    initializeGenreList(common);
    std::string filename = "favoritesLog.txt";
    loadFavoritesFromFile(filename);

    // Start the main GUI loop.
    GuiMain(DrawAppWindow, &common);

    // Save favorites when the application exits.
    saveFavoritesToFile(filename);

    // Signal other threads to exit.
    common.exit_flag = true;
    common.cv.notify_one();
}

// Function to initialize the genre list in the common object.
void DrawThread::initializeGenreList(common& common_ptr) {
    common_ptr.genreList = {
        "Action", "Adventure", "Animation", "Comedy", "Crime", "Documentary",
        "Drama", "Romance", "Family", "Fantasy", "History", "Horror"
    };
}
