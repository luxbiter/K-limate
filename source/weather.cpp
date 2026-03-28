#include "weather.hpp"
#include <json.hpp>
#include <curl/curl.h>
#include <fstream>
#include <ctime>
#include <cstdio>

using json = nlohmann::json;

// ──────────────────────────────────────────────
// libcurl 수신 콜백
// ──────────────────────────────────────────────
static size_t curlWrite(void* ptr, size_t size, size_t nmemb, std::string* out) {
    out->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

// ──────────────────────────────────────────────
// 설정 파일 로드  (sdmc:/config/weather.json)
// {
//   "api_key": "YOUR_ENCODING_KEY",
//   "nx": 60,
//   "ny": 127
// }
// ──────────────────────────────────────────────
WeatherConfig loadConfig(const std::string& path) {
    WeatherConfig cfg;

    std::ifstream f(path);
    if (!f.is_open()) return cfg;           // 파일 없음

    try {
        json j = json::parse(f);
        cfg.api_key = j.at("api_key").get<std::string>();
        if (j.contains("name")) cfg.name = j["name"].get<std::string>();
        if (j.contains("nx"))   cfg.nx   = j["nx"].get<int>();
        if (j.contains("ny"))   cfg.ny   = j["ny"].get<int>();
        cfg.valid = !cfg.api_key.empty();
    } catch (...) {
        // 파싱 실패 → cfg.valid = false 유지
    }

    return cfg;
}

// ──────────────────────────────────────────────
// base_date / base_time 계산
// 초단기실황은 매 정시 발표, xx:40 이후 제공
// ──────────────────────────────────────────────
static void calcBaseTime(char date_out[9], char time_out[5]) {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);          // libnx: localtime_r 사용 가능

    // 아직 40분이 안 됐으면 1시간 이전 발표분 사용
    if (t.tm_min < 40) {
        t.tm_hour -= 1;
        if (t.tm_hour < 0) {
            t.tm_hour = 23;
            t.tm_mday -= 1;         // 날짜 감소 (mktime 재계산)
            mktime(&t);
        }
    }

    snprintf(date_out, 9,  "%04d%02d%02d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    snprintf(time_out, 5,  "%02d00", t.tm_hour);
}

// ──────────────────────────────────────────────
// 기상청 초단기실황 조회 (getUltraSrtNcst)
// ──────────────────────────────────────────────
WeatherData fetchWeather(const WeatherConfig& cfg) {
    WeatherData data;

    char base_date[9], base_time[5];
    calcBaseTime(base_date, base_time);

    // URL 조립 – ServiceKey는 이미 URL인코딩된 상태로 발급됨
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
    // 스위치 환경: 내장 CA 번들이 없으므로 검증 비활성화
    // (프로덕션에서는 적절한 CA 번들 제공 권장)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return data;

    // ── JSON 파싱 ──────────────────────────────
    try {
        auto j     = json::parse(response);
        auto& body = j.at("response").at("body");

        // 결과 코드 확인
        std::string resultCode =
            j["response"]["header"]["resultCode"].get<std::string>();
        if (resultCode != "00") return data;

        auto& items = body.at("items").at("item");
        for (auto& item : items) {
            const auto& cat = item.at("category").get<std::string>();
            const auto& val = item.at("obsrValue").get<std::string>();
            if      (cat == "T1H") data.temp = val;
            else if (cat == "PTY") data.pty  = val;
            else if (cat == "REH") data.reh  = val;
        }
        data.valid = !data.temp.empty();
    } catch (...) {
        // JSON 구조가 예상과 다를 경우 valid=false 유지
    }

    return data;
}

