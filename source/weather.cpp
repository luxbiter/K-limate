#include "weather.hpp"
#include <json.hpp>
#include <curl/curl.h>
#include <fstream>
#include <ctime>
#include <cstdio>

using json = nlohmann::json;

static size_t curlWrite(void* ptr, size_t size, size_t nmemb, std::string* out) {
    out->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

// Load config from sdmc:/config/weather.json
WeatherConfig loadConfig(const std::string& path) {
    WeatherConfig cfg;

    std::ifstream f(path);
    if (!f.is_open()) return cfg;

    json j = json::parse(f, nullptr, /*exceptions=*/false);
    if (j.is_discarded()) return cfg;

    if (j.contains("api_key") && j["api_key"].is_string())
        cfg.api_key = j["api_key"].get<std::string>();
    if (j.contains("name") && j["name"].is_string())
        cfg.name = j["name"].get<std::string>();
    if (j.contains("nx") && j["nx"].is_number())
        cfg.nx = j["nx"].get<int>();
    if (j.contains("ny") && j["ny"].is_number())
        cfg.ny = j["ny"].get<int>();

    cfg.valid = !cfg.api_key.empty();
    return cfg;
}

// KMA ultra-short-term observation reports on the hour; available after xx:40.
// Use the previous hour's data if current minute < 40.
static void calcBaseTime(char date_out[9], char time_out[5]) {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);

    if (t.tm_min < 40) {
        t.tm_hour -= 1;
        if (t.tm_hour < 0) {
            t.tm_hour = 23;
            t.tm_mday -= 1;
            mktime(&t);
        }
    }

    snprintf(date_out, 9, "%04d%02d%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    snprintf(time_out, 5, "%02d00", t.tm_hour);
}

WeatherData fetchWeather(const WeatherConfig& cfg) {
    WeatherData data;

    char base_date[9], base_time[5];
    calcBaseTime(base_date, base_time);

    std::string url =
        "https://apis.data.go.kr/1360000/VilageFcstInfoService_2.0/getUltraSrtNcst"
        "?ServiceKey=" + cfg.api_key +
        "&pageNo=1&numOfRows=10&dataType=JSON"
        "&base_date=" + std::string(base_date) +
        "&base_time=" + std::string(base_time) +
        "&nx=" + std::to_string(cfg.nx) +
        "&ny=" + std::to_string(cfg.ny);

    CURL* curl = curl_easy_init();
    if (!curl) return data;

    std::string response;
    response.reserve(4096);

    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  curlWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return data;

    json j = json::parse(response, nullptr, /*exceptions=*/false);
    if (j.is_discarded()) return data;

    if (!j.contains("response")) return data;
    auto& hdr = j["response"]["header"];
    if (!hdr.contains("resultCode")) return data;
    if (hdr["resultCode"].get<std::string>() != "00") return data;

    auto& items = j["response"]["body"]["items"]["item"];
    if (!items.is_array()) return data;

    for (auto& item : items) {
        if (!item.contains("category") || !item.contains("obsrValue")) continue;
        const auto cat = item["category"].get<std::string>();
        const auto val = item["obsrValue"].get<std::string>();
        if      (cat == "T1H") data.temp = val;
        else if (cat == "PTY") data.pty  = val;
        else if (cat == "REH") data.reh  = val;
    }

    data.valid = !data.temp.empty();
    return data;
}
