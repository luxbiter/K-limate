# K-limate (INDEV!!!)

Nintendo Switch Tesla overlay that displays real-time weather information using the Korea Meteorological Administration (KMA) API.

## Features

- Real-time temperature, weather condition, and air quality display
- Animated skeleton loading screen while fetching data
- Sidebar UI layout optimized for Tesla overlay
- Configured via a simple JSON file on the SD card

## Requirements

- Nintendo Switch with custom firmware (Atmosphere)
- [Tesla Menu](https://github.com/WerWolv/Tesla-Menu) installed
- Korea Meteorological Administration API key ([data.go.kr](https://www.data.go.kr))

## SD Card Layout

```
SD:/
├── switch/
│   └── .overlays/
│       └── k_limate.ovl        <-- overlay file here
└── config/
    └── weather.json            <-- config file here
```

## Installation

**1. Download the overlay**

Download `k_limate.ovl` from [Releases](../../releases) and copy it to:
```
SD:/switch/.overlays/k_limate.ovl
```

**2. Create the config file**

Create `SD:/config/weather.json` with the following content:
```json
{
  "api_key": "YOUR_KMA_API_KEY",
  "name": "Seoul",
  "nx": 60,
  "ny": 127
}
```

> Get your API key from [data.go.kr](https://www.data.go.kr) by signing up and requesting access to the **Korea Meteorological Administration Forecast API** (기상청_단기예보 ((구)_동네예보) 조회서비스).

**3. Find your grid coordinates (nx, ny)**

Use the [KMA coordinate finder](https://www.weather.go.kr/w/obs-climate/land/surface/aws.do) to look up the grid coordinates for your city. Default values `(60, 127)` correspond to Seoul.

**4. Launch**

Boot your Switch, open **Tesla Menu**, and select **K-limate** from the overlay list.

## Building from Source

Docker is required.

```bash
docker run --rm -v "${PWD}:/project" devkitpro/devkita64 bash /project/docker-build.sh
```

The output file `k_limate.ovl` will be created in the project root.

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

# K-limate (한국어) (개발중!!!)

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

## SD카드 배치

```
SD:/
├── switch/
│   └── .overlays/
│       └── k_limate.ovl        <-- 오버레이 파일
└── config/
    └── weather.json            <-- 설정 파일
```

## 설치 방법

**1. 오버레이 파일 복사**

[Releases](../../releases) 에서 `k_limate.ovl` 을 다운로드한 후 아래 경로에 복사:
```
SD:/switch/.overlays/k_limate.ovl
```

**2. 설정 파일 생성**

`SD:/config/weather.json` 파일을 아래 내용으로 생성:
```json
{
  "api_key": "기상청_API_키",
  "name": "서울",
  "nx": 60,
  "ny": 127
}
```

> API 키는 [data.go.kr](https://www.data.go.kr) 에 가입 후 **기상청_단기예보 조회서비스** 를 신청하면 발급됩니다.

**3. 격자 좌표 (nx, ny) 확인**

[기상청 관측소 페이지](https://www.weather.go.kr/w/obs-climate/land/surface/aws.do) 에서 지역별 격자 좌표를 확인하세요. 기본값 `(60, 127)` 은 서울입니다.

**4. 실행**

Switch 부팅 후 **Tesla Menu** 를 열고 오버레이 목록에서 **K-limate** 를 선택합니다.

## 빌드 방법

Docker가 필요합니다.

```bash
docker run --rm -v "${PWD}:/project" devkitpro/devkita64 bash /project/docker-build.sh
```

빌드가 완료되면 프로젝트 폴더에 `k_limate.ovl` 파일이 생성됩니다.

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
