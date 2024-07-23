#pragma once

#include "commonObject.h"
#include <unordered_map>

class DownloadThread
{
public:
	void download(common& common_ptr);
private:
	//std::string _download_url;
	void get_genre_ids(const std::string& api_key, common& common_obj);
	std::unordered_map<std::string, int> genreIDs;

};
