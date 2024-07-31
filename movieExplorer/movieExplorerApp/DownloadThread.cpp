#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "DownloadThread.h"
#include <iostream>

void DownloadThread::download(common& common_ptr) {
	std::string api_key = "783d7e8d1b46831e05f602fb0da3311e";
	get_genre_ids(api_key, common_ptr);
	std::string check = "";
	std::string API;
	while (true) {
		std::unique_lock<std::mutex> lock(common_ptr.mtx);
		common_ptr.cv.wait(lock, [&] { return common_ptr.newGenreAvailable.load() || common_ptr.newSearchAvailable.load() || common_ptr.exit_flag; });

		if (common_ptr.exit_flag)
			break;

		std::string input = common_ptr.sharedInput;
		
		lock.unlock();

		if (input != check) {
			// Another genre or movie was entered
			check = input;
			common_ptr.movieList.clear();
		}
		else {
			// Same genre or movie was entered twice
			common_ptr.newTableData = true;
			continue;
		}
		
		if (common_ptr.newGenreAvailable) {
			int genre_id = genreIDs[input];
			API = "/3/discover/movie?api_key=" + api_key + "&with_genres=" + std::to_string(genre_id);
			common_ptr.newGenreAvailable = false;

		}

		else if(common_ptr.newSearchAvailable) {
			API = "/3/search/movie?query=" + input + "&api_key=" + api_key;
			common_ptr.newSearchAvailable = false;
		}

		if (!input.empty()) {
			httplib::SSLClient cli("api.themoviedb.org");
			auto res = cli.Get(API);
			if (res && res->status == 200) {
				auto json_result = nlohmann::json::parse(res->body);

				for (const auto& movie : json_result["results"]) {
					Movie_Details m;
					m.title = movie["title"];
					m.release_date = movie["release_date"];
					m.original_language = movie["original_language"];
					m.popularity = movie["popularity"];
					m.overview = movie["overview"];
					common_ptr.movieList.push_back(m);
				}
				common_ptr.newTableData = true;
			}
			else {
				std::cout << "Error: " << (res ? res->status : -1) << std::endl;
			}
		}
	}
}

// Function to get the genre IDs 
void DownloadThread::get_genre_ids(const std::string& api_key, common& common_obj) {
	httplib::SSLClient cli("api.themoviedb.org");
	auto res = cli.Get(("/3/genre/movie/list?api_key=" + api_key + "&language=en-US").c_str());

	if (res && res->status == 200) {
		auto genres = nlohmann::json::parse(res->body)["genres"];
		for (std::string ourGenre : common_obj.genreList) {
			for (const auto& genre : genres) {
				if (genre["name"] == ourGenre) {
					genreIDs[ourGenre] = genre["id"];
					break;
				}
			}
		}
	}
}

