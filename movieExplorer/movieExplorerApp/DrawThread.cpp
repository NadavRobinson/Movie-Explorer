#include "DrawThread.h"
#include "GuiMain.h"
#include "../3rd_party/ImGuiSource/imgui.h"
#include <stdlib.h>
#include <thread>
#include <string>
#include <iostream>

/*
void DrawAppWindow(void* common_ptr)
{
	static bool p;
	static int selectedMovieIndex = -1;
	auto commonObj = (common*)common_ptr;

	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Movie Explorer", nullptr, ImGuiWindowFlags_NoCollapse);
	//ImGui::Begin("App");

	if (ImGui::Button("Show Favorites")) {
		p = !p;
	}

	ImGui::SameLine();

	if (ImGui::TreeNode("Genres"))
	{
		for (std::string genre : commonObj->genreList)
		{
			if (ImGui::Button(genre.c_str()))
			{
				std::lock_guard<std::mutex> lock(commonObj->mtx);
				commonObj->sharedInput = genre;
				commonObj->newInputAvailable = true;
			}
			commonObj->cv.notify_one();
		}
		ImGui::TreePop();
	}

	// Display favorites if showFavorites is true
	if (p) {
		ImGui::Text("Favorites:");
		for (const auto& title : commonObj->genreList) {
			ImGui::BulletText("%s", title.c_str());
		}
	}

	if (commonObj->newTableData) {

		if (ImGui::BeginTable("Movies", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH))  //, flags))
		{
			ImGui::TableSetupColumn("Title");
			ImGui::TableSetupColumn("Language");
			ImGui::TableSetupColumn("Release Date");
			ImGui::TableSetupColumn("Popularity");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			//for (auto& movie : commonObj->movieList)
			for (int i = 0; i < commonObj->movieList.size(); ++i)
			{
				auto& movie = commonObj->movieList[i];

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(movie.title.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(movie.original_language.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(movie.release_date.c_str());
				ImGui::TableSetColumnIndex(3);
				ImGui::Text(std::to_string(movie.popularity).c_str());

				ImGui::TableSetColumnIndex(4);
				if (ImGui::Button(("Details##" + std::to_string(i)).c_str())) {
					// Toggle selection state
					selectedMovieIndex = (selectedMovieIndex == i) ? -1 : i;
				}

				ImGui::TableSetColumnIndex(5);
				if (ImGui::Button(("Add To Favorites##" + std::to_string(i)).c_str()))
				{
					//
				}

				// Insert an extra row for displaying the overview if this is the selected movie
				if (selectedMovieIndex == i) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.6f, 0.5f, 1.0f));
					ImGui::Text("Overview");
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::TextDisabled("(Click 'Details' again to hide)");
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// Calculate the width for the overview text
					float wrapWidth = ImGui::GetContentRegionAvail().x;

					// Display the actual overview with custom wrapping
					ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrapWidth);
					ImGui::TextUnformatted(movie.overview.c_str());
					ImGui::PopTextWrapPos();

					// Add some extra spacing after the overview
					ImGui::Spacing();
					ImGui::Spacing();

					// Span the overview across all columns
					ImGui::TableSetColumnIndex(1);
					ImGui::TableSetColumnIndex(2);
					ImGui::TableSetColumnIndex(3);
					ImGui::TableSetColumnIndex(4);
					ImGui::TableSetColumnIndex(5);
				}
			}
			ImGui::EndTable();
		}
	}


	ImGui::End();
}
*/


void DrawAppWindow(void* common_ptr)
{
	static bool p = false;
	static int selectedMovieIndex = -1;
	auto commonObj = (common*)common_ptr;

	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Movie Explorer", nullptr, ImGuiWindowFlags_NoCollapse);

	// Add some padding and style
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

	if (ImGui::Button("Show Favorites")) {
		p = !p;
	}

	ImGui::SameLine();

	// Genre section
	if (ImGui::CollapsingHeader("Genres", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent(10);
		int buttonsPerRow = 4;
		int buttonCount = 0;
		for (const std::string& genre : commonObj->genreList)
		{
			if (buttonCount > 0 && buttonCount % buttonsPerRow != 0)
				ImGui::SameLine();

			if (ImGui::Button(genre.c_str(), ImVec2(150, 30)))
			{
				std::lock_guard<std::mutex> lock(commonObj->mtx);
				commonObj->sharedInput = genre;
				commonObj->newInputAvailable = true;
				commonObj->cv.notify_one();
			}
			buttonCount++;
		}
		ImGui::Unindent(10);
		ImGui::Spacing();
	}

	// Display favorites if showFavorites is true
	if (p) {
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

		for (const auto& title : commonObj->genreList) {
			ImGui::BulletText("%s", title.c_str());
		}

		ImGui::Separator();
	}

	// Movie list section
	if (commonObj->newTableData) {
		ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));	// Change color for header row background
		ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));		// Change color for even row background
		ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));	// Change color for odd row background

		if (ImGui::BeginTable("Movies", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH |
			ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX))
		{
			// Setup columns with specific sizing behaviors
			ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch, 0.4f);
			ImGui::TableSetupColumn("Language", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("Release Date", ImGuiTableColumnFlags_WidthFixed, 100.0f);
			ImGui::TableSetupColumn("Popularity", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthFixed, 60.0f);
			ImGui::TableSetupColumn("Favorite", ImGuiTableColumnFlags_WidthFixed, 60.0f);
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
				if (ImGui::Button(("Fav##" + std::to_string(i)).c_str())) {
					// Add to favorites logic here
				}

				if (selectedMovieIndex == i) {	// if current row has "Details" button pressed
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.8f, 1.0f));
					ImGui::TextWrapped("Overview: %s", movie.overview.c_str());
					ImGui::PopStyleColor();
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

	ImGui::PopStyleVar(2);
	ImGui::End();
}

void DrawThread::call_Gui_main(common& common)
{
	initializeGenreList(common);
	GuiMain(DrawAppWindow, &common);
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
	common_ptr.genreList.push_back("Family");
}