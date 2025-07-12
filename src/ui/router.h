#pragma once
#include "sigslot/signal.hpp"
#include "pages/page_widget.h"
#include <memory>
#include <string>
#include <unordered_map>
using std::shared_ptr;
using std::string;
using std::unordered_map;

typedef shared_ptr<unordered_map<string, shared_ptr<PageWidget>>>
    PageCollection;

class Router {
public:
    sigslot::signal<> onFrame;
    Router();

    void push(const string &pageName);
    void pop();
    void add_page(const string &name, shared_ptr<PageWidget> page);
    PageCollection get_pages();
    shared_ptr<PageWidget> get_page(const string &name);
    void showPage(const string &pageName);
    void printCurrentPage();

protected:

    PageCollection _pages;
    string _current_page;
    vector<string> _stack; // Stack for page navigation
    unordered_map<string, sigslot::scoped_connection> _pageConnections; // Connections for each page's onFrame signal
    void _route(const string &pageName);
};