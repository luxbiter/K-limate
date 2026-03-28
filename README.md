# K-limate

Nintendo Switch Tesla overlay(Uberhands) that displays real-time weather information using the Korea Meteorological Administration (KMA) API.

## Features

- Real-time temperature, weather condition, and air quality display
- Animated skeleton loading screen while fetching data
- Sidebar UI layout optimized for Tesla overlay
- Configured via a simple JSON file on the SD card

## Requirements

- Nintendo Switch with custom firmware (Atmosphere)
- [Tesla Menu](https://github.com/WerWolv/Tesla-Menu) installed
- Korea Meteorological Administration API key ([data.go.kr](https://www.data.go.kr))

## Installation

1. Download `weather_overlay.ovl` from [Releases](../../releases)
2. Copy to your SD card:
   ```
   SD:/switch/.overlays/weather_overlay.ovl
   ```
3. Create the config file at `SD:/config/weather.json`:
   ```json
   {
     "api_key": "YOUR_KMA_API_KEY",
     "name": "Seoul",
     "nx": 60,
     "ny": 127
   }
   ```
4. Launch Tesla Menu on your Switch and select **Klimate**

### Grid coordinates (nx, ny)

Find your coordinates at the [KMA grid search page](https://www.weather.go.kr/w/obs-climate/land/surface/aws.do).
Default values (60, 127) correspond to Seoul.

## Building from Source

Docker is required.

```bash
docker run --rm -v "${PWD}:/project" devkitpro/devkita64 bash /project/docker-build.sh
```

The output file `weather_overlay.ovl` will be created in the project root.

## Project Structure

```
klimate/
├── source/
│   ├── main.cpp          # UI layout and overlay logic
│   └── weather.cpp       # KMA API fetch and JSON parsing
├── include/
│   └── weather.hpp       # Structs and function declarations
├── docker-build.sh       # One-command Docker build script
├── weather.json.example  # Config file template
└── Makefile
```

## License

MIT

---

# K-limate (한국어)

**대단하다! K-날씨!**
기상청 API를 이용해 실시간 날씨 정보를 표시하는 Nintendo Switch Tesla 오버레이입니다.

## 기능

- 실시간 기온, 날씨, 미세먼지 표시
- 데이터 로딩 중 물결 스켈레톤 애니메이션
- Tesla 오버레이에 최적화된 사이드바 UI
- SD카드의 JSON 파일로 간단하게 설정

## 필요 사항

- 커스텀 펌웨어가 설치된 Nintendo Switch (Atmosphere)
- [Tesla Menu](https://github.com/WerWolv/Tesla-Menu) 설치
- 기상청 API 키 ([data.go.kr](https://www.data.go.kr) 에서 발급)

## 설치 방법

1. [Releases](../../releases) 에서 `weather_overlay.ovl` 다운로드
2. SD카드에 복사:
   ```
   SD:/switch/.overlays/weather_overlay.ovl
   ```
3. `SD:/config/weather.json` 파일 생성:
   ```json
   {
     "api_key": "기상청_API_키",
     "name": "서울",
     "nx": 60,
     "ny": 127
   }
   ```
4. Switch에서 Tesla Menu 실행 후 **Klimate** 선택

### 격자 좌표 (nx, ny)

[기상청 격자 조회](https://www.weather.go.kr/w/obs-climate/land/surface/aws.do) 에서 지역별 좌표를 확인하세요.
기본값 (60, 127) 은 서울입니다.

## 빌드 방법

Docker가 필요합니다.

```bash
docker run --rm -v "${PWD}:/project" devkitpro/devkita64 bash /project/docker-build.sh
```

빌드가 완료되면 프로젝트 폴더에 `weather_overlay.ovl` 파일이 생성됩니다.

## 프로젝트 구조

```
klimate/
├── source/
│   ├── main.cpp          # UI 레이아웃 및 오버레이 로직
│   └── weather.cpp       # 기상청 API 요청 및 JSON 파싱
├── include/
│   └── weather.hpp       # 구조체 및 함수 선언
├── docker-build.sh       # 한 줄 Docker 빌드 스크립트
├── weather.json.example  # 설정 파일 예시
└── Makefile
```

## 라이선스

MIT
