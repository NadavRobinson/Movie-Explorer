#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "DownloadThread.h"
#include <iostream>

// The main download function which handles fetching movie data based on user input.
// It uses the common object for synchronization and shared data between threads.
void DownloadThread::download(common& common_ptr) {
    // API key for The Movie Database (TMDb)
    std::string api_key = "783d7e8d1b46831e05f602fb0da3311e";

    // Retrieve genre IDs for the genre names stored in the common object.
    get_genre_ids(api_key, common_ptr);

    std::string previousInput = ""; // Stores the previous input for comparison.
    std::string API;                // API endpoint string.

    while (true) {
        // Lock the mutex and wait for a signal that new data is available or to exit.
        std::unique_lock<std::mutex> lock(common_ptr.mtx);
        common_ptr.cv.wait(lock, [&] {
            return common_ptr.newGenreAvailable.load() ||
                common_ptr.newSearchAvailable.load() ||
                common_ptr.exit_flag;
            });

        // Exit the loop if the exit flag is set.
        if (common_ptr.exit_flag)
            break;

        // Get the shared input and unlock the mutex.
        std::string input = common_ptr.sharedInput;
        lock.unlock();

        // Clear the movie list if the input has changed.
        if (input != previousInput) {
            previousInput = input;
            common_ptr.movieList.clear();
        }
        else {
            // If the same input is provided, mark newTableData and continue.
            common_ptr.newTableData = true;
            continue;
        }

        // Build the API request based on whether a new genre or search is available.
        if (common_ptr.newGenreAvailable) {
            int genre_id = genreIDs[input];
            API = "/3/discover/movie?api_key=" + api_key + "&with_genres=" + std::to_string(genre_id);
            common_ptr.newGenreAvailable = false;
        }
        else if (common_ptr.newSearchAvailable) {
            API = "/3/search/movie?query=" + input + "&api_key=" + api_key;
            common_ptr.newSearchAvailable = false;
        }

        // If input is not empty, send the API request and process the response.
        if (!input.empty()) {
            httplib::SSLClient cli("api.themoviedb.org");
            auto res = cli.Get(API);
            if (res && res->status == 200) {
                auto json_result = nlohmann::json::parse(res->body);

                // Parse the JSON response and populate the movie list.
                for (const auto& movie : json_result["results"]) {
                    Movie_Details m;
                    m.title = movie["title"];
                    m.release_date = movie["release_date"];
                    m.original_language = movie["original_language"];
                    m.popularity = movie["popularity"];
                    m.overview = movie["overview"];
                    common_ptr.movieList.push_back(m);
                }
                common_ptr.newTableData = true; // Indicate that new table data is available.
            }
            else {
                // Log an error message if the API request failed.
                std::cout << "Error: " << (res ? res->status : -1) << std::endl;
            }
        }
    }
}

// Function to retrieve genre IDs from the TMDb API and map them to genre names.
void DownloadThread::get_genre_ids(const std::string& api_key, common& common_obj) {
    httplib::SSLClient cli("api.themoviedb.org");
    auto res = cli.Get(("/3/genre/movie/list?api_key=" + api_key + "&language=en-US").c_str());

    // If the response is successful, parse the genre data.
    if (res && res->status == 200) {
        auto genres = nlohmann::json::parse(res->body)["genres"];
        for (std::string ourGenre : common_obj.genreList) {
            for (const auto& genre : genres) {
                // Map genre names to their corresponding IDs.
                if (genre["name"] == ourGenre) {
                    genreIDs[ourGenre] = genre["id"];
                    break;
                }
            }
        }
    }
}
