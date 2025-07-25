#pragma once

#include "core/mpc_sampler.h"
#include "core/project.h"
#include "util.h"
#include "yaml-cpp/yaml.h"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "core/item_lister.h"
// Project listing information struct
struct ProjectListing {
    std::string fileName;          // The project file name (without extension)
    std::string filePath;          // Full path to the project file
    std::chrono::system_clock::time_point lastModifiedDate;  // Last modification time

    ProjectListing() = default;
    ProjectListing(const std::string& name, const std::string& path, const std::chrono::system_clock::time_point& modTime)
        : fileName(name), filePath(path), lastModifiedDate(modTime) {}
    
    // Utility method to get formatted date string
    std::string getFormattedModifiedDate() const;
};

class ProjectManager : public ItemLister {
public:
    sigslot::signal<> onProjectLoaded; // Signal emitted when a project is loaded
    sigslot::signal<> onProjectSaved; // Signal emitted when a project is saved
    sigslot::signal<> onProjectDeleted; // Signal emitted when a project is deleted
    sigslot::signal<> onProjectListingUpdated; // Signal emitted when the project listing is updated

    shared_ptr<MPCSampler> mpcSampler; // Sampler instance for the project manager
    shared_ptr<Project> currentProject; // Current project being managed

    // Constructor
  ProjectManager(shared_ptr<MPCSampler> mpcSampler) : ItemLister(5, 0), mpcSampler(mpcSampler)
  {
    _visibleProjects.reserve(_pageSize);
    _projects.reserve(100);
    setPageSize(_pageSize);
    getProjects(); // Load initial project listings
    scrollToIndex(0); // Scroll to the first project by default
  };
  ~ProjectManager() = default;

  // Create the Traxe folder structure
  bool createProjectDirectories();

  // Get directory paths
  std::string getApplicationDirectory();
  std::string getProjectsDirectory();

  shared_ptr<Project> newProject(const std::string &projectName);

  // Save a project to file
  bool saveProject();

  // Load a project from file path
  std::shared_ptr<Project> loadProject( const std::string &filePath );

  // Load a project by name from the projects directory
  std::shared_ptr<Project> loadProjectByName( const std::string &projectName );

  // Duplicate a project with a new name
  bool duplicateProject(const std::string &sourceProjectName,
                        const std::string &newProjectName = "");

  // Duplicate a project and return the generated name
  std::string duplicateProjectWithName(const std::string &sourceProjectName,
                                       const std::string &newProjectName = "");

  // Delete a project
  bool deleteProject(const std::string &projectName);

  // List all available projects
  std::vector<ProjectListing>& getProjects();

  vector<ProjectListing>& getVisibleItems();

  ProjectListing getSelectedProject() const;

  // Check if a project exists
  bool projectExists(const std::string &projectName);

private:
    bool createTraxeFolder();
    bool createProjectsFolder();
    vector<ProjectListing> _projects;
    vector<ProjectListing> _visibleProjects;
};
