#pragma once
#include <string>

// sdmc:/config/weather.json 에서 읽는 설정 구조체
struct WeatherConfig {
    std::string api_key;
    std::string name;   // 지역 표시 이름 (예: "서울")
    int nx = 60;        // 격자 X (기본값: 서울)
    int ny = 127;       // 격자 Y
    bool valid = false;
};

// 기상청 초단기실황 파싱 결과
struct WeatherData {
    std::string temp;   // T1H: 기온 (°C)
    std::string pty;    // PTY: 강수형태 코드
    std::string reh;    // REH: 습도 (%)
    std::string pm10;   // 미세먼지 농도 (미구현 시 빈 문자열)
    bool valid = false;
};

WeatherConfig loadConfig(const std::string& path);
WeatherData   fetchWeather(const WeatherConfig& config);
