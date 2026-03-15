#pragma once

#include "ocr.h"
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace live {

/**
 * RapidOCR engine — uses prebuilt RapidOcrOnnx.dll via C API.
 *
 * RapidOcrOnnx bundles ONNX Runtime + OpenCV + PaddleOCR pipeline
 * in a single DLL with a simple C API (OcrInit/OcrDetect/OcrGetResult).
 *
 * This approach avoids OpenCV/ONNX build complexity — we just call the
 * prebuilt DLL via LoadLibrary + GetProcAddress (cross-compiler safe).
 */
class RapidOcrEngine : public OcrEngine {
public:
    bool init(const std::string& dataPath) override;
    void shutdown() override;
    bool isReady() const override { return m_ready; }

    std::vector<OcrBox> recognize(
        const uint8_t* pixels, int width, int height) override;

    std::string name() const override { return "RapidOCR"; }
    std::string lastError() const override { return m_error; }

private:
    bool saveBmp(const uint8_t* bgra, int w, int h, const std::string& path);
    std::string parseOcrResult(const std::string& jsonResult);

    // RapidOcrOnnx C API types
    using OcrInitFn    = void* (*)(const char*, const char*, const char*, const char*, int);
    using OcrDetectFn  = char (*)(void*, const char*, const char*, void*);
    using OcrGetLenFn  = int (*)(void*);
    using OcrGetResultFn = char (*)(void*, char*, int);
    using OcrDestroyFn = void (*)(void*);

    HMODULE m_hDll = nullptr;
    void* m_handle = nullptr; // OCR_HANDLE

    OcrInitFn    m_fnInit = nullptr;
    OcrDetectFn  m_fnDetect = nullptr;
    OcrGetLenFn  m_fnGetLen = nullptr;
    OcrGetResultFn m_fnGetResult = nullptr;
    OcrDestroyFn m_fnDestroy = nullptr;

    bool m_ready = false;
    std::string m_error;
    std::string m_dataPath;
};

} // namespace live
