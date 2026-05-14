#include "AppConfig.h"

AppConfig& AppConfig::instance() {
    static AppConfig cfg;
    return cfg;
}
