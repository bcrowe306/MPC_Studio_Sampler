#pragma once
#include "Widget.h"
#include <unordered_map>
#include <string>
#include <memory>

using std::shared_ptr;
using std::unordered_map;
using std::string;

class Display {
public:
    unordered_map<string, shared_ptr<Widget>> pages;
    EncodeSurfaceCallback encode_surface_callback;
    string current_page;
    Display(EncodeSurfaceCallback encode_surface_callback) 
        : encode_surface_callback(encode_surface_callback), current_page("") {
        // Initialize the display with an empty set of pages

    }
    ~Display() {
        
    }

    void initialize() {
        // Initialize all pages
        for (auto &[name, page] : pages) {
            if (page) {
                page->initialize(encode_surface_callback);
            }
        }
    }

    void add_page(const string &name, shared_ptr<Widget> page) {
        if (page) {
            pages[name] = page;
        }
    }
    shared_ptr<Widget> get_page(const string &name) {
        auto it = pages.find(name);
        if (it != pages.end()) {
            return it->second;
        }
        return nullptr;
    }

    void show_page(const string &name) {
        // show page by name and activating it. only one page can be active at a time
        for(auto &[page_name, page] : pages) {
            if(page_name == name) {
                page->activate();
                current_page = name;
                page->render();
            } else {
                page->deactivate();
            }
        }
    }
};