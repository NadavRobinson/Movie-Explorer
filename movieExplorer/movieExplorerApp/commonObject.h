#pragma once
#include <string>
#include <mutex>
#include <atomic>

struct Movie_Details {
	std::string title;
	std::string original_language;
	std::string release_date;
	std::string overview;
	float popularity;
};

struct common {
	std::vector<Movie_Details> movieList;
	std::vector<std::string> genreList;
	std::condition_variable cv;
	std::mutex mtx;
	std::string sharedInput;
	std::atomic_bool newGenreAvailable = false;
	std::atomic_bool newSearchAvailable = false;
	std::atomic_bool newTableData = false;
	std::atomic_bool exit_flag = false;
};