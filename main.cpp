#include<iostream>
#include "App.hpp"
#include "gtkmm/application.h"


int main(int argc, char **argv) {

    auto app = Gtk::Application::create("org.klewy.app");

    
    return app->make_window_and_run<Main>(argc, argv);
}