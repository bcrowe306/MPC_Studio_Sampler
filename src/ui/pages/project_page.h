#pragma once
#include "page_widget.h"
#include "widgets/ui_helpers.h"
#include <memory>
#include <string>
#include "core/project_manager.h"
#include "ui/router.h"

class ProjectPage : public PageWidget {
    // Device-specific UI elements and layout
public:
    shared_ptr<ProjectManager> projectManager;
    ProjectPage(shared_ptr<MPCSampler> mpcSampler, shared_ptr<ProjectManager> projectManager, unsigned int x, unsigned int y, unsigned int width, unsigned int height,
               const std::string &title = "Project Page")
        : PageWidget(mpcSampler, x, y, width, height), _title(title), projectManager(projectManager) {
    }

    void draw(Vector offset) override {
        auto projects = projectManager->getVisibleItems();
        auto _itemsCount = projectManager->getItemsCount();
        auto _pageSize = projectManager->getPageSize();
        auto _scrollIndex = projectManager->getScrollIndex();
        auto _viewRange = projectManager->getViewRange();
        auto selectedProjectIndex = _scrollIndex - _viewRange.start;

        // Draw header
        cairo_draw_horizontal_line(cr, 12.0, 0.0, width);

        for (size_t i = 0; i < projects.size(); ++i) {
            auto &project = projects[i];
            auto isSelected = (i == selectedProjectIndex);
            double start_x = 0.0;
            double start_y = i * 12 + 13; // Vertical offset for text
            if (isSelected) {
              cairo_draw_rectangle(cr, 1, start_y + 1, width - 10, 12, isSelected);
            }
            cairo_draw_text(
                cr, project.fileName,
                0, start_y + 11, 11, !isSelected);
        }
        cairo_draw_vertical_scrollbar(cr, width - 8, 13, 8, height - 25,
                                      _itemsCount, _pageSize, _scrollIndex);
    }

    void onActivated() override {
        functionWidgets[0]->setLabel("New");
        functionWidgets[1]->setLabel("Refresh");
        functionWidgets[2]->setLabel("Duplicate");
        functionWidgets[3]->setLabel("Delete");
        functionWidgets[4]->setLabel("Rename");
        functionWidgets[5]->setLabel("Load");
        // Connect signals and set up the page when activated
        projectManager->getProjects(); // Load projects
        // Connect signals and set up the page when activated
        signalConnections.push_back(controlSurface->jogWheel->onOffset.connect(
            [this](int offset) { projectManager->scroll(offset); }));
        signalConnections.push_back(controlSurface->upButton->onPressed.connect(
            [this]() { projectManager->scroll(-1); }));
        signalConnections.push_back(
            controlSurface->downButton->onPressed.connect(
                [this]() { projectManager->scroll(1); }));
        addConnection(projectManager->onScroll.connect(
            [this](int newOffset) { render(); }));

        addConnection(controlSurface->functionButtons->onPressed.connect(
            [this](int index) {
                switch (index) {
                    case 0: // New Project
                        projectManager->createProjectDirectories();
                        break;
                    case 1: // Refresh Projects
                        projectManager->getProjects();
                        break;
                    case 2: // Duplicate Project
                        if (projectManager->getSelectedProject().fileName.empty()) {
                            std::cerr << "No project selected to duplicate." << std::endl;
                        } else {
                            projectManager->duplicateProject(projectManager->getSelectedProject().fileName);
                        }
                        break;
                    case 3: // Delete Project
                        if (projectManager->getSelectedProject().fileName.empty()) {
                            std::cerr << "No project selected to delete." << std::endl;
                        } else {
                            projectManager->deleteProject(projectManager->getSelectedProject().fileName);
                        }
                        break;
                    case 4: // Rename Project
                        if (projectManager->getSelectedProject().fileName.empty()) {
                            std::cerr << "No project selected to rename." << std::endl;
                        } else {
                            // projectManager->renameProject(projectManager->getSelectedProject().fileName);
                        }
                        break;
                    case 5: // Load Project
                        if (projectManager->getSelectedProject().fileName.empty()) {
                            std::cerr << "No project selected to load." << std::endl;
                        } else {
                            projectManager->loadProjectByName(projectManager->getSelectedProject().fileName);
                        }
                        break;
                }
            }));

        

    }
protected:
    std::string _title;
    int _fontSize = 10; // Font size for text rendering
    
};