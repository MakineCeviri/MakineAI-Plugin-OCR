/**
 * Live plugin core — initialization, shutdown, and pipeline coordination.
 * Uses RapidOcrOnnx prebuilt DLL for native OCR (no Python, no scripts).
 */

#include "capture.h"
#include "ocr_rapid.h"
#include <memory>
#include <string>

namespace live {

static std::unique_ptr<ScreenCapture> s_capture;
static std::unique_ptr<OcrEngine> s_ocr;
static std::string s_dataPath;

bool init(const char* dataPath)
{
    s_dataPath = dataPath;

    s_capture = std::make_unique<ScreenCapture>();
    s_capture->init(CaptureMethod::Auto);

    s_ocr = std::make_unique<RapidOcrEngine>();
    if (!s_ocr->init(s_dataPath)) {
        // Non-fatal — plugin loads but OCR unavailable
        return true;
    }

    return true;
}

void shutdown()
{
    if (s_ocr) { s_ocr->shutdown(); s_ocr.reset(); }
    if (s_capture) { s_capture->shutdown(); s_capture.reset(); }
}

bool ready()
{
    return s_capture != nullptr;
}

std::string captureAndRecognize(void* windowHandle, int x, int y, int w, int h)
{
    if (!s_capture || !s_ocr || !s_ocr->isReady()) return "";

    CapturedFrame frame;
    CaptureRegion region{x, y, w, h};

    if (!s_capture->captureRegion(windowHandle, region, frame))
        return "CAPTURE_FAILED: " + s_capture->lastError();

    auto results = s_ocr->recognize(frame.pixels.data(), frame.width, frame.height);

    std::string text;
    for (const auto& box : results) {
        if (!text.empty()) text += "\n";
        text += box.text;
    }
    return text;
}

} // namespace live
