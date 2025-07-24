#pragma once
#include "router.h"
Router::Router() {
        // Initialize the router with a display
        _pages = std::make_shared<unordered_map<string, shared_ptr<PageWidget>>>();
    }

void Router::push(const string &pageName) {
        // Push a page onto the navigation stack
        _stack.push_back(pageName);
        _route(pageName);
    }
void Router::pop() {
        // Pop the last page from the navigation stack
        if (!_stack.empty()) {
            _stack.pop_back();
            if (!_stack.empty()) {
                _route(_stack.back());
            } else {
                _route("devicePage"); // Default to device page if stack is empty
            }
        }
    }

void Router::add_page(const string &name, shared_ptr<PageWidget> page) {
        if (page) {
            _pages->emplace(name, page);
        }
    }

PageCollection Router::get_pages() { return _pages; }

shared_ptr<PageWidget> Router::get_page(const string &name) {
        auto it = _pages->find(name);
        if (it != _pages->end()) {
            return it->second;
        }
        return nullptr;
}

void Router::showPage(const string &pageName) { push(pageName); }
void Router::printCurrentPage() {
        // Print the current page name
        if (!_current_page.empty()) {
            std::cout << "Current page: " << _current_page << std::endl;
        } else {
            std::cout << "No current page." << std::endl;
        }
    }
void Router::_route(const string &pageName) {
        // show page by name and activating it. only one page can be active at a
        // time
        if (_current_page == pageName) {

            return; // Already showing this page
        }
        beforePageChanged(); // Emit signal before the page changes
        for (auto &[page_name, page] : *_pages) {
            if (page_name == pageName) {

                // Connect onFrame signal to the page's onFrame method
                _pageConnections[page_name] = onFrame.connect(
                    std::bind(&PageWidget::onFrame, page.get()));
                page->activate();
                _current_page = page_name;
                page->render();
            } 
            else {
                _pageConnections[page_name].disconnect();
                page->deactivate();
            }
        }
        onPageChanged(pageName); // Emit signal when the page changes
        afterPageChanged(); // Emit signal after the page has changed
}
