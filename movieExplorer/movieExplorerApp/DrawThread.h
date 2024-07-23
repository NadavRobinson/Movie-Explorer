#pragma once
#include <vector>
#include "commonObject.h"

class DrawThread
{
public:
	//void operator()(CommonObjects& common);
	//void SetUrl(std::string_view new_url);

	void call_Gui_main(common& common);
private:
	//std::string _download_url;
	void initializeGenreList(common& common);
};
