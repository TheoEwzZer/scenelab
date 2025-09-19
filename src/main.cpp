
#include <iostream>

#include "App.hpp"


int main(void)
{
    try {
        App app;
        app.run();
        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}