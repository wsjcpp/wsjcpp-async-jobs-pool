
#include <string.h>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include <wsjcpp_async_jobs_pool.h>

int main(int argc, const char* argv[]) {
    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_APP_NAME);
    std::string appVersion = std::string(WSJCPP_APP_VERSION);

    if (!WsjcppCore::dirExists(".wsjcpp")) {
        WsjcppCore::makeDir(".wsjcpp");
    }
    WsjcppLog::setPrefixLogFile("wsjcpp");
    WsjcppLog::setLogDirectory(".wsjcpp/logs");

    /*std::vector<std::string> vArgs;

    for (int i = 0; i < argc; i++) {
        vArgs.push_back(std::string(argv[i]));
    }*/

    // TODO example

    return 0;
}