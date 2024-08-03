#pragma once

#include "commonObject.h"
#include <unordered_map>

// The DownloadThread class is responsible for managing the download process.
// It uses a common object to perform various download-related tasks.
class DownloadThread
{
public:
    // Public method to start the download process. It takes a reference to a common object.
    void download(common& common_ptr);

private:
    // Private method to retrieve genre IDs using an API key and a common object.
    // It populates the genreIDs unordered_map.
    void get_genre_ids(const std::string& api_key, common& common_obj);

    // A container to store genre IDs mapped to their corresponding genre names.
    std::unordered_map<std::string, int> genreIDs;
};
