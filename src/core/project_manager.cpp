#include "project_manager.h"
#include "fmt/format.h"
#include <memory>

using std::make_shared;
using std::shared_ptr;

std::string ProjectListing::getFormattedModifiedDate() const {
    auto time_t = std::chrono::system_clock::to_time_t(lastModifiedDate);
    auto tm = *std::localtime(&time_t);
    
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buffer);
}

bool ProjectManager::createProjectDirectories() {
    return createTraxeFolder() && createProjectsFolder();
}

std::string ProjectManager::getApplicationDirectory() {
    try {
        std::filesystem::path documentsPath = getDocumentsFolder();
        if (documentsPath.empty()) {
            return "";
        }
        std::filesystem::path traxePath = documentsPath / "Traxe";
        return traxePath.string();
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error getting application directory: " << ex.what() << std::endl;
        return "";
    }
}

std::string ProjectManager::getProjectsDirectory() {
    try {
        std::filesystem::path documentsPath = getDocumentsFolder();
        if (documentsPath.empty()) {
            return "";
        }
        std::filesystem::path projectsPath = documentsPath / "Traxe" / "Projects";
        return projectsPath.string();
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error getting projects directory: " << ex.what() << std::endl;
        return "";
    }
}

shared_ptr<Project> ProjectManager::newProject(const std::string &projectName) {
    std::string newName = projectName;
    try {
        if (projectName.empty()) {
            newName = fmt::format("Untitled Project {}", _projects.size() + 1);
        }

        // Create a new project instance
        currentProject.reset(); // Reset current project if it exists
        currentProject = make_shared<Project>(mpcSampler);
        currentProject->projectName->setValue(newName); // Set the project name
        onProjectLoaded(); // Emit the project loaded signal
        return currentProject;
        // Save the new project to the default location
    } catch (const std::exception &ex) {
        std::cerr << "Error creating new project: " << ex.what() << std::endl;
        return nullptr;
    }
}

bool ProjectManager::saveProject() {
    try {
        

        std::string projectsDir = getProjectsDirectory();
        if (projectsDir.empty()) {
            std::cerr << "Could not get projects directory" << std::endl;
            return false;
        }

        // Create the full file path with .yaml extension
        std::filesystem::path filePath = std::filesystem::path(projectsDir) / (currentProject->projectName->getValue() + ".yaml");

        // Create YAML emitter and serialize the project
        YAML::Emitter out;
        currentProject->serialize(out);

        // Write to file (this will overwrite if the file exists)
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return false;
        }

        file << out.c_str();
        file.close();

        if (file.good()) {
            std::cout << "Project saved successfully to: " << filePath << std::endl;
            onProjectSaved(); // Emit the project saved signal
            onProjectListingUpdated(); // Emit the project listing updated signal
            return true;
        } else {
            std::cerr << "Error occurred while writing to file: " << filePath << std::endl;
            return false;
        }

    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error saving project: " << ex.what() << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cerr << "Error saving project: " << ex.what() << std::endl;
        return false;
    }
}

shared_ptr<Project> ProjectManager::loadProject(const std::string& filePath) {
    try {
        if (filePath.empty()) {
            std::cerr << "File path cannot be empty" << std::endl;
            return nullptr;
        }

        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            std::cerr << "Project file does not exist: " << filePath << std::endl;
            return nullptr;
        }

        // Load YAML from file
        YAML::Node yamlData;
        try {
            yamlData = YAML::LoadFile(filePath);
        } catch (const YAML::Exception& ex) {
            std::cerr << "Error parsing YAML file: " << ex.what() << std::endl;
            return nullptr;
        }

        // get filename from path without extension
        std::filesystem::path path(filePath);
        std::string fileName = path.filename().stem().string();


        std::cout << "Project loaded successfully from: " << filePath << std::endl;
        currentProject.reset(); // Reset current project if it exists
        currentProject = make_shared<Project>(mpcSampler); 
        currentProject->deserialize(yamlData);
        currentProject->projectName->setValue(fileName); // Set the project name from the file name
        onProjectLoaded(); // Emit the project loaded signal
        return currentProject;

    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error loading project: " << ex.what() << std::endl;
        return nullptr;
    } catch (const std::exception& ex) {
        std::cerr << "Error loading project: " << ex.what() << std::endl;
        return nullptr;
    }
}

shared_ptr<Project> ProjectManager::loadProjectByName( const std::string& projectName ) {
    try {
        if (projectName.empty()) {
            std::cerr << "Project name cannot be empty" << std::endl;
            return nullptr;
        }

        std::string projectsDir = getProjectsDirectory();
        if (projectsDir.empty()) {
            std::cerr << "Could not get projects directory" << std::endl;
            return nullptr;
        }

        // Build the full file path
        std::filesystem::path filePath = std::filesystem::path(projectsDir) / (projectName + ".yaml");
        auto loadedProject = loadProject(filePath.string());
        return loadedProject;

    } catch (const std::exception& ex) {
        std::cerr << "Error loading project by name: " << ex.what() << std::endl;
        return nullptr;
    }
}

bool ProjectManager::duplicateProject(const std::string& sourceProjectName, const std::string& newProjectName) {
    try {
        if (sourceProjectName.empty()) {
            std::cerr << "Source project name cannot be empty" << std::endl;
            return false;
        }

        std::string projectsDir = getProjectsDirectory();
        if (projectsDir.empty()) {
            std::cerr << "Could not get projects directory" << std::endl;
            return false;
        }

        std::filesystem::path sourcePath = std::filesystem::path(projectsDir) / (sourceProjectName + ".yaml");

        if (!std::filesystem::exists(sourcePath)) {
            std::cerr << "Source project does not exist: " << sourcePath << std::endl;
            return false;
        }

        // Generate a unique name for the duplicate
        std::string finalNewName;
        if (newProjectName.empty()) {
            // Auto-generate name: "ProjectName copy", "ProjectName copy 2", etc.
            finalNewName = sourceProjectName + " copy";
            int copyNumber = 2;
            
            while (projectExists(finalNewName)) {
                finalNewName = sourceProjectName + " copy " + std::to_string(copyNumber);
                copyNumber++;
            }
        } else {
            // Use provided name, but ensure it's unique
            finalNewName = newProjectName;
            int copyNumber = 2;
            
            while (projectExists(finalNewName)) {
                finalNewName = newProjectName + " copy " + std::to_string(copyNumber);
                copyNumber++;
            }
        }

        std::filesystem::path destPath = std::filesystem::path(projectsDir) / (finalNewName + ".yaml");

        std::filesystem::copy_file(sourcePath, destPath);
        std::cout << "Project duplicated successfully from " << sourcePath << " to " << destPath << std::endl;
        return true;

    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error duplicating project: " << ex.what() << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cerr << "Error duplicating project: " << ex.what() << std::endl;
        return false;
    }
}

std::string ProjectManager::duplicateProjectWithName(const std::string& sourceProjectName, const std::string& newProjectName) {
    try {
        if (sourceProjectName.empty()) {
            std::cerr << "Source project name cannot be empty" << std::endl;
            return "";
        }

        std::string projectsDir = getProjectsDirectory();
        if (projectsDir.empty()) {
            std::cerr << "Could not get projects directory" << std::endl;
            return "";
        }

        std::filesystem::path sourcePath = std::filesystem::path(projectsDir) / (sourceProjectName + ".yaml");

        if (!std::filesystem::exists(sourcePath)) {
            std::cerr << "Source project does not exist: " << sourcePath << std::endl;
            return "";
        }

        // Generate a unique name for the duplicate
        std::string finalNewName;
        if (newProjectName.empty()) {
            // Auto-generate name: "ProjectName copy", "ProjectName copy 2", etc.
            finalNewName = sourceProjectName + " copy";
            int copyNumber = 2;
            
            while (projectExists(finalNewName)) {
                finalNewName = sourceProjectName + " copy " + std::to_string(copyNumber);
                copyNumber++;
            }
        } else {
            // Use provided name, but ensure it's unique
            finalNewName = newProjectName;
            int copyNumber = 2;
            
            while (projectExists(finalNewName)) {
                finalNewName = newProjectName + " copy " + std::to_string(copyNumber);
                copyNumber++;
            }
        }

        std::filesystem::path destPath = std::filesystem::path(projectsDir) / (finalNewName + ".yaml");

        std::filesystem::copy_file(sourcePath, destPath);
        std::cout << "Project duplicated successfully from " << sourcePath << " to " << destPath << std::endl;
        onProjectListingUpdated(); // Emit the project listing updated signal
        return finalNewName;

    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error duplicating project: " << ex.what() << std::endl;
        return "";
    } catch (const std::exception& ex) {
        std::cerr << "Error duplicating project: " << ex.what() << std::endl;
        return "";
    }
}

bool ProjectManager::deleteProject(const std::string& projectName) {
    try {
        if (projectName.empty()) {
            std::cerr << "Project name cannot be empty" << std::endl;
            return false;
        }

        std::string projectsDir = getProjectsDirectory();
        if (projectsDir.empty()) {
            std::cerr << "Could not get projects directory" << std::endl;
            return false;
        }

        std::filesystem::path filePath = std::filesystem::path(projectsDir) / (projectName + ".yaml");

        if (!std::filesystem::exists(filePath)) {
            std::cerr << "Project does not exist: " << filePath << std::endl;
            return false;
        }

        std::filesystem::remove(filePath);
        std::cout << "Project deleted successfully: " << filePath << std::endl;
        onProjectDeleted(); // Emit the project deleted signal
        onProjectListingUpdated(); // Emit the project listing updated signal
        getProjects(); // Refresh the project listing
        return true;

    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error deleting project: " << ex.what() << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cerr << "Error deleting project: " << ex.what() << std::endl;
        return false;
    }
}

std::vector<ProjectListing> &ProjectManager::getProjects() {
    _projects.clear(); // Clear previous listings
    
    try {
        std::string projectsDir = getProjectsDirectory();
        if (projectsDir.empty()) {
            std::cerr << "Could not get projects directory" << std::endl;
            return _projects;
        }

        if (!std::filesystem::exists(projectsDir)) {
            std::cerr << "Projects directory does not exist: " << projectsDir << std::endl;
            return _projects;
        }

        for (const auto& entry : std::filesystem::directory_iterator(projectsDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
                std::string fileName = entry.path().filename().stem().string();
                std::string filePath = entry.path().string();
                
                std::cout << "Found project file: " << fileName << std::endl; // Debug output --- IGNORE ---

                // Get the last modification time
                auto fileTime = std::filesystem::last_write_time(entry.path());
                auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    fileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                
                _projects.emplace_back(fileName, filePath, systemTime);
            }
        }
        setItemCount((int)_projects.size());
        setPageSize(_pageSize);
        _viewRange = {0, _pageSize, (int)_projects.size()};

    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error listing projects: " << ex.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error listing projects: " << ex.what() << std::endl;
    }

    return _projects;
}


vector<ProjectListing> &ProjectManager::getVisibleItems() {
    int start = _viewRange.start;
    int end = std::min(start + _viewRange.pageSize, (int)_projects.size());
    _visibleProjects.clear(); // Clear the previous visible items
    for (int i = start; i < end; ++i) {
        _visibleProjects.push_back(_projects[i]);
    }
    return _visibleProjects; // Return the current list of visible projects
}

ProjectListing ProjectManager::getSelectedProject() const {
    if (_scrollIndex >= 0 && _scrollIndex < (int)_projects.size()) {
        return _projects[_scrollIndex]; // Return the currently selected project
    }
    return ProjectListing(); // Return an empty ProjectListing if no project is selected
}

bool ProjectManager::projectExists(const std::string& projectName) {
    try {
        if (projectName.empty()) {
            return false;
        }

        std::string projectsDir = getProjectsDirectory();
        if (projectsDir.empty()) {
            return false;
        }

        std::filesystem::path filePath = std::filesystem::path(projectsDir) / (projectName + ".yaml");
        return std::filesystem::exists(filePath);

    } catch (const std::exception& ex) {
        std::cerr << "Error checking if project exists: " << ex.what() << std::endl;
        return false;
    }
}

bool ProjectManager::createTraxeFolder() {
    try {
        std::filesystem::path documentsPath = getDocumentsFolder();
        if (documentsPath.empty()) {
            std::cerr << "Could not get documents folder path" << std::endl;
            return false;
        }

        std::filesystem::path traxePath = documentsPath / "Traxe";
        
        if (std::filesystem::exists(traxePath)) {
            std::cout << "Traxe folder already exists at: " << traxePath << std::endl;
            return true;
        }

        if (std::filesystem::create_directories(traxePath)) {
            std::cout << "Traxe folder created at: " << traxePath << std::endl;
            return true;
        } else {
            std::cerr << "Failed to create Traxe folder at: " << traxePath << std::endl;
            return false;
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
        return false;
    }
}

bool ProjectManager::createProjectsFolder() {
    try {
        std::filesystem::path documentsPath = getDocumentsFolder();
        if (documentsPath.empty()) {
            std::cerr << "Could not get documents folder path" << std::endl;
            return false;
        }

        std::filesystem::path projectsPath = documentsPath / "Traxe" / "Projects";
        
        if (std::filesystem::exists(projectsPath)) {
            std::cout << "Projects folder already exists at: " << projectsPath << std::endl;
            return true;
        }

        if (std::filesystem::create_directories(projectsPath)) {
            std::cout << "Projects folder created at: " << projectsPath << std::endl;
            return true;
        } else {
            std::cerr << "Failed to create Projects folder at: " << projectsPath << std::endl;
            return false;
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Filesystem error: " << ex.what() << std::endl;
        return false;
    }
}
