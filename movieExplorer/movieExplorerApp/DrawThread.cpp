#include "DrawThread.h"
#include "GuiMain.h"
#include "../3rd_party/ImGuiSource/imgui.h"
#include <stdlib.h>
#include <thread>
#include <string>
#include <iostream>
#include <unordered_set>
#include <fstream>
#include <windows.h> // // Include Windows API for PostQuitMessage function

std::unordered_set<std::string> favorites;

void loadFavoritesFromFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open())
		return;
	std::string currentLine;
	while (std::getline(file, currentLine)) 
		favorites.insert(currentLine);
	file.close();
}

void saveFavoritesToFile(const std::string& filename) {
	std::ofstream file(filename);
	for (const auto& title : favorites)
		file << title << std::endl;
	file.close();
}

void DrawAppWindow(void* common_ptr)
{
	static char searchBuffer[128] = "";
	static bool showFavorites = false;
	static int currentGenreIndex = -1; // Start with -1 to indicate no selection
	static int selectedMovieIndex = -1;
	auto commonObj = (common*)common_ptr;
	
	ImGui::Begin("Movie Explorer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);
	// Set the window position to the top-left corner
	ImGui::SetWindowPos(ImVec2(0, 0));
	// Set the window size to the full display size
	ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

	// Setup a menu bar for a close button in the top right of the screen
	if (ImGui::BeginMenuBar())
	{

		// Center title
		float windowWidth = ImGui::GetWindowWidth();
		float textWidth = ImGui::CalcTextSize("Movie Explorer").x;
		float centerPos = (windowWidth - textWidth) * 0.5f;

		ImGui::SetCursorPosX(centerPos);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.843f, 0.0f, 1.0f)); // Gold color
		ImGui::Text("Movie Explorer");
		ImGui::PopStyleColor();

		// Right-side position
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 67);

		// Add the button to the menu bar
		if (ImGui::Button("Close"))
		{
			::PostQuitMessage(0);
		}
		ImGui::EndMenuBar();
	}

	ImGui::Separator();
	ImGui::SetNextItemWidth(500);
	bool searchTriggered = ImGui::InputTextWithHint("##SearchBar", "Search Movie", searchBuffer, IM_ARRAYSIZE(searchBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::SameLine();
	if (ImGui::Button("Search") || searchTriggered) {
		
	}

	ImGui::SameLine();
	if (ImGui::Button("Clear Table")) {
		commonObj->newTableData = false;
		currentGenreIndex = -1;
	}

	ImGui::Separator();

	// Show Favorites button
	if (ImGui::Button("Show Favorites")) {
		showFavorites = !showFavorites;
	}

	ImGui::SameLine();

	// Genre selection
	const char* currentGenre = (currentGenreIndex >= 0) ? commonObj->genreList[currentGenreIndex].c_str() : "Select a genre";

	ImGui::SetNextItemWidth(170); // Set the width of the combo to 150 pixels
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
				// Focus on the current selected genre when clicked on the combo
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// Display favorites if showFavorites is true
	if (showFavorites) {
		ImGui::Separator();

		// Header styling
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.8f, 1.0f));

		// Increase text size for header
		float originalFontSize = ImGui::GetFont()->Scale;
		ImGui::GetFont()->Scale *= 1.3f; // Increase font size by 30%
		ImGui::PushFont(ImGui::GetFont());

		ImGui::Text("Favorites");

		// Restore original font size
		ImGui::GetFont()->Scale = originalFontSize;
		ImGui::PopFont();

		ImGui::PopStyleColor();

		ImGui::Separator();

		for (const auto& title : favorites) {
			ImGui::BulletText("%s", title.c_str());
		}

		ImGui::Separator();
	}
	
	// Movie list section
	if (commonObj->newTableData) {
		ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));  //Change color for header row background. Darker Charcoal (#1E1E1E)
		ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.17f, 0.17f, 0.17f, 1.00f));		// Change color for even row background. Very Dark Charcoal (#2C2C2C)
		ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(0.23f, 0.23f, 0.23f, 1.00f));	// Change color for odd row background. Slightly Lighter Charcoal (#3A3A3A)

		if (ImGui::BeginTable("Movies", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH |
			ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX))
		{
			// Setup columns with specific sizing behaviors
			ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch, 0.4f);
			ImGui::TableSetupColumn("Language", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("Release Date", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Popularity", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthFixed, 70.0f);
			ImGui::TableSetupColumn("Favorite", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableHeadersRow();

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
				if (ImGui::Checkbox(("Fav##" + std::to_string(i)).c_str(), &isFavorite)) {
					if (isFavorite) {
						favorites.insert(movie.title);
					}
					else {
						favorites.erase(movie.title);
					}
				}

				if (selectedMovieIndex == i) {	// if current row has "Details" button pressed
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

	ImGui::End();
}

void DrawThread::call_Gui_main(common& common)
{
	initializeGenreList(common);
	std::string filename = "favoritesLog.txt";
	loadFavoritesFromFile(filename);
	GuiMain(DrawAppWindow, &common);
	saveFavoritesToFile(filename);
	common.exit_flag = true;
	common.cv.notify_one();
}

void DrawThread::initializeGenreList(common& common_ptr) {
	common_ptr.genreList.push_back("Action");
	common_ptr.genreList.push_back("Adventure");
	common_ptr.genreList.push_back("Animation");
	common_ptr.genreList.push_back("Comedy");
	common_ptr.genreList.push_back("Crime");
	common_ptr.genreList.push_back("Documentary");
	common_ptr.genreList.push_back("Drama");
	common_ptr.genreList.push_back("Romance");
	common_ptr.genreList.push_back("Family");
	common_ptr.genreList.push_back("Fantasy");
	common_ptr.genreList.push_back("History");
	common_ptr.genreList.push_back("Horror");
}