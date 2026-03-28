// TESLA_INIT_IMPL must be defined in exactly one translation unit.
#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <switch.h>
#include <curl/curl.h>

#include "weather.hpp"

#include <future>
#include <string>
#include <cmath>

// Colors: tsl::Color uses 4-bit RGBA (0x0-0xF per channel)
static constexpr tsl::Color COL_WHITE = {0xF, 0xF, 0xF, 0xF};
static constexpr tsl::Color COL_GRAY  = {0x8, 0x8, 0x8, 0xF};

class WeatherGui : public tsl::Gui {
public:
    explicit WeatherGui(WeatherConfig cfg)
        : m_config(std::move(cfg))
    {
        if (m_config.valid) {
            m_future = std::async(std::launch::async, [this]() -> WeatherData {
                return fetchWeather(m_config);
            });
        }
    }

    tsl::elm::Element* createUI() override {
        auto* frame = new tsl::elm::OverlayFrame("K-limate", "");

        if (!m_config.valid) {
            auto* list = new tsl::elm::List();
            list->addItem(new tsl::elm::ListItem("Config not found."));
            list->addItem(new tsl::elm::ListItem("sdmc:/config/weather.json"));
            frame->setContent(list);
            return frame;
        }

        auto* drawer = new tsl::elm::CustomDrawer(
            [this](tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) {
                draw(r, x, y, w, h);
            });
        frame->setContent(drawer);
        return frame;
    }

    void update() override {
        m_tick++;
        if (m_fetched || !m_future.valid()) return;
        if (m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            m_data    = m_future.get();
            m_fetched = true;
        }
    }

    bool handleInput(u64 keysDown, u64, const HidTouchState&,
                     HidAnalogStickState, HidAnalogStickState) override
    {
        if (keysDown & HidNpadButton_B) { tsl::goBack(); return true; }
        return false;
    }

private:
    void draw(tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) {
        r->drawRect(x, y, w, h, COL_GRAY);

        const s32 lx  = x + 18;
        s32       ly  = y + 18;
        const s32 gap = 120;

        // Location
        r->drawString("Location", false, lx, ly, 20, COL_WHITE);
        ly += 32;
        if (m_fetched)
            r->drawString(m_config.name.empty() ? "-" : m_config.name.c_str(),
                          false, lx, ly, 18, COL_WHITE);
        else
            drawSkeleton(r, lx, ly, w - 36);
        ly += gap;

        // Temperature
        r->drawString("Temp", false, lx, ly, 20, COL_WHITE);
        ly += 32;
        if (m_fetched && m_data.valid) {
            std::string t = m_data.temp + "\xc2\xb0\x43"; // degrees C (UTF-8)
            r->drawString(t.c_str(), false, lx, ly, 18, COL_WHITE);
        } else {
            drawSkeleton(r, lx, ly, w - 36);
        }
        ly += gap;

        // Weather
        r->drawString("Weather", false, lx, ly, 20, COL_WHITE);
        ly += 32;
        if (m_fetched && m_data.valid)
            r->drawString(ptyText().c_str(), false, lx, ly, 18, COL_WHITE);
        else
            drawSkeleton(r, lx, ly, w - 36);
        ly += gap;

        // Fine dust (PM10)
        r->drawString("PM10", false, lx, ly, 20, COL_WHITE);
        ly += 32;
        if (m_fetched && !m_data.pm10.empty())
            r->drawString(m_data.pm10.c_str(), false, lx, ly, 18, COL_WHITE);
        else
            drawSkeleton(r, lx, ly, w - 36);
    }

    // Animated sine-wave skeleton line
    void drawSkeleton(tsl::gfx::Renderer* r, s32 x, s32 y, s32 maxW) {
        const float amp   = 4.0f;
        const float freq  = 0.28f;
        const float phase = static_cast<float>(m_tick) * 0.07f;

        for (s32 i = 0; i < maxW - 2; i += 2) {
            float y0 = amp * std::sinf(freq * i + phase);
            float y1 = amp * std::sinf(freq * (i + 2) + phase);
            s32 iy0 = y + static_cast<s32>(y0);
            s32 iy1 = y + static_cast<s32>(y1);
            s32 top = iy0 < iy1 ? iy0 : iy1;
            s32 ht  = (iy0 < iy1 ? iy1 - iy0 : iy0 - iy1) + 2;
            r->drawRect(x + i, top, 2, ht, COL_WHITE);
        }
    }

    // PTY code -> weather string
    std::string ptyText() const {
        if (m_data.pty.empty() || m_data.pty == "0") return "Clear";
        int code = 0;
        for (char c : m_data.pty)
            if (c >= '0' && c <= '9') code = code * 10 + (c - '0');
        switch (code) {
            case 1: return "Rain";
            case 2: return "Rain/Snow";
            case 3: return "Snow";
            case 5: return "Drizzle";
            case 6: return "Drizzle/Snow";
            case 7: return "Snowflakes";
            default: return "Precipitation";
        }
    }

    WeatherConfig            m_config;
    WeatherData              m_data;
    std::future<WeatherData> m_future;
    bool                     m_fetched = false;
    u32                      m_tick    = 0;
};

class WeatherOverlay : public tsl::Overlay {
public:
    void initServices() override {
        socketInitializeDefault();
        nifmInitialize(NifmServiceType_User);
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    void exitServices() override {
        curl_global_cleanup();
        nifmExit();
        socketExit();
    }

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        WeatherConfig cfg = loadConfig("sdmc:/config/weather.json");
        return initially<WeatherGui>(std::move(cfg));
    }
};

int main(int argc, char* argv[]) {
    return tsl::loop<WeatherOverlay>(argc, argv);
}
