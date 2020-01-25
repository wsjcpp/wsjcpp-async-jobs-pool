
#include <string.h>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include <wsjcpp_async_jobs_pool.h>

int main(int argc, const char* argv[]) {
    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_NAME);
    std::string appVersion = std::string(WSJCPP_VERSION);

    // TODO move to WSJCppCore::init
    if (!WSJCppCore::dirExists(".wsjcpp")) {
        WSJCppCore::makeDir(".wsjcpp");
    }
    std::string appLogPath = ".wsjcpp/logs";
    if (!WSJCppCore::dirExists(appLogPath)) {
        WSJCppCore::makeDir(appLogPath);
    }
    WSJCppLog::setPrefixLogFile("wsjcpp");
    WSJCppLog::setLogDirectory(".wsjcpp/logs");

    std::vector<std::string> vArgs;

    for (int i = 0; i < argc; i++) {
        vArgs.push_back(std::string(argv[i]));
    }

    // TODO example

    return 0;
}