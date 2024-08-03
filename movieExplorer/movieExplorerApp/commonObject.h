#pragma once

#include <string>
#include <mutex>
#include <atomic>
#include <vector>

// Structure to store details about a movie.
struct Movie_Details {
    std::string title;              // Title of the movie.
    std::string original_language;  // Original language of the movie.
    std::string release_date;       // Release date of the movie.
    std::string overview;           // Brief overview/description of the movie.
    float popularity;               // Popularity score of the movie.
};

// Structure to share common resources and data between threads.
struct common {
    std::vector<Movie_Details> movieList;  // List of movie details.
    std::vector<std::string> genreList;    // List of movie genres.

    std::condition_variable cv;            // Condition variable for thread synchronization.
    std::mutex mtx;                        // Mutex for protecting shared resources.

    std::string sharedInput;               // Shared input string used by threads.

    std::atomic_bool newGenreAvailable = false;  // Flag indicating if new genre data is available.
    std::atomic_bool newSearchAvailable = false; // Flag indicating if new search data is available.
    std::atomic_bool newTableData = false;       // Flag indicating if new table data is available.
    std::atomic_bool exit_flag = false;          // Flag to signal exit to threads.
};
