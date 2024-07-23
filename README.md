# Movie Explorer

Movie Explorer is a Windows desktop application that allows users to browse and discover films across various genres. 
Built with C++, it features an intuitive interface for filtering movies, managing favorite genres, and viewing film details.
The app demonstrates effective GUI design and internet data retrieval, making it an ideal tool for movie enthusiasts and a showcase of modern Windows application development.

## Installation

Download the setup.exe file from the [Releases](https://github.com/NadavRobinson/movie-explorer/releases) section.
Run the setup.exe file and follow the installation wizard.

## Usage

1. Install the application using the provided setup.msi file in the releases folder.
2. Launch the application from your Start menu or desktop shortcut.
3. Use the graphical interface to interact with the system:
   - Browse and explore movies across various genres
   - View movie details including title, language, release date, and popularity
   - Filter movies by genre categories
   - Maintain a list of favorite genres
4. The application will fetch and display data from the internet based on your inputs.
5. 
Note: Ensure you have an active internet connection for the application to function properly.

## Project Structure

- `Gui_Project.sln`: Main solution file
- `3rd_party/`: Third-party dependencies
- `winapp/`: Windows application source files
- `releases/`: Contains setup.msi for easy installation

## Dependencies

nlohmann json
openssl
httplib
Dear Imgui
