#pragma once
#include <string>

struct WeatherConfig {
    std::string api_key;
    std::string name;   // display name for the location (e.g. "Seoul")
    int nx = 60;        // KMA grid X (default: Seoul)
    int ny = 127;       // KMA grid Y
    bool valid = false;
};

struct WeatherData {
    std::string temp;   // T1H: temperature (degrees C)
    std::string pty;    // PTY: precipitation type code
    std::string reh;    // REH: humidity (%)
    std::string pm10;   // PM10: fine dust (not yet fetched)
    bool valid = false;
};

WeatherConfig loadConfig(const std::string& path);
WeatherData   fetchWeather(const WeatherConfig& config);
