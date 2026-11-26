#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

inline float SmoothstepCpp(float e0, float e1, float x) {
    float denom = (e1 - e0);
    float t = Clamp((fabsf(denom) > 1e-6f) ? ((x - e0) / denom) : 0.0f, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

namespace TextUtil {

    inline const char* Tr(int lang, const char* ru, const char* en) {
        return (lang == 0) ? ru : en;
    }

    inline std::vector<int> BuildUnicodeCodepoints() {
        std::vector<int> cps;
        cps.reserve(400);
        for (int c = 32; c <= 126; c++) cps.push_back(c);
        for (int c = 0x0400; c <= 0x04FF; c++) cps.push_back(c);
        const int extra[] = { 0x2013, 0x2014, 0x00AB, 0x00BB, 0x2022, 0x2190, 0x2192, 0x2191, 0x2193, 0x00B0, 0x221E };
        for (int c : extra) cps.push_back(c);
        return cps;
    }

    inline Font LoadRussianFont(int fontSize) {
        const char* candidates[] = {
            "assets/fonts/Roboto-Regular.ttf",
            "assets/fonts/DejaVuSans.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf"
        };
        std::vector<int> cps = BuildUnicodeCodepoints();
        for (const char* path : candidates) {
            if (FileExists(path)) {
                Font f = LoadFontEx(path, fontSize, cps.data(), static_cast<int>(cps.size()));
                if (f.texture.id != 0) {
                    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
                    return f;
                }
            }
        }
        return GetFontDefault();
    }
}

namespace ConfigSys {

    struct GameSettings {
        int preset = 2;
        int resIndex = 1;
        bool fullscreen = false;
        bool vsync = true;
        int fpsLimit = 120;
        float drawDistance = 1100.0f;
        int texQuality = 2;
        int shadowQuality = 2;
        int reflectionQuality = 2;
        int fogQuality = 2;

        bool bloomEnabled = true;
        bool ssaoEnabled = true;
        bool hdrEnabled = true;
        bool godRaysEnabled = true;
        bool dofEnabled = true;
        bool chromaEnabled = true;
        bool grainEnabled = true;

        float masterVolume = 0.85f;
        float musicVolume = 0.65f;
        float sfxVolume = 0.85f;
        float ambientVolume = 0.80f;
        float uiVolume = 0.75f;

        float mouseSensitivity = 1.0f;
        bool invertY = false;
        int keyForward = KEY_W;
        int keyBackward = KEY_S;
        int keySprint = KEY_LEFT_SHIFT;
        int keyInteract = KEY_E;
        int keyFlashlight = KEY_F;
        int keyZoom = KEY_Z;

        float fov = 72.0f;
        float brightness = 1.02f;
        float contrast = 1.05f;
        int language = 0;
        bool subtitles = true;
        int difficulty = 1;
    };

    struct SaveProgress {
        bool hasSave = false;
        int sectorId = 0;
        float posX = 0.0f;
        float posY = 1.8f;
        float posZ = 19.0f;
        float yaw = 0.0f;
        float pitch = 0.0f;
        float anomalyFactor = 0.0f;
    };

    inline void SetDefaults(GameSettings& cfg) {
        cfg = GameSettings{};
    }

    inline void ApplyPreset(GameSettings& cfg, int preset) {
        cfg.preset = Clamp(preset, 0, 3);
        if (cfg.preset == 0) {
            cfg.drawDistance = 650.0f;
            cfg.texQuality = 0;
            cfg.shadowQuality = 0;
            cfg.reflectionQuality = 0;
            cfg.fogQuality = 1;
            cfg.bloomEnabled = false;
            cfg.ssaoEnabled = false;
            cfg.hdrEnabled = true;
            cfg.godRaysEnabled = false;
            cfg.dofEnabled = false;
            cfg.chromaEnabled = false;
            cfg.grainEnabled = false;
        } else if (cfg.preset == 1) {
            cfg.drawDistance = 850.0f;
            cfg.texQuality = 1;
            cfg.shadowQuality = 1;
            cfg.reflectionQuality = 1;
            cfg.fogQuality = 1;
            cfg.bloomEnabled = true;
            cfg.ssaoEnabled = false;
            cfg.hdrEnabled = true;
            cfg.godRaysEnabled = true;
            cfg.dofEnabled = false;
            cfg.chromaEnabled = false;
            cfg.grainEnabled = true;
        } else if (cfg.preset == 2) {
            cfg.drawDistance = 1150.0f;
            cfg.texQuality = 2;
            cfg.shadowQuality = 2;
            cfg.reflectionQuality = 2;
            cfg.fogQuality = 2;
            cfg.bloomEnabled = true;
            cfg.ssaoEnabled = true;
            cfg.hdrEnabled = true;
            cfg.godRaysEnabled = true;
            cfg.dofEnabled = true;
            cfg.chromaEnabled = true;
            cfg.grainEnabled = true;
        } else {
            cfg.drawDistance = 1500.0f;
            cfg.texQuality = 3;
            cfg.shadowQuality = 3;
            cfg.reflectionQuality = 3;
            cfg.fogQuality = 2;
            cfg.bloomEnabled = true;
            cfg.ssaoEnabled = true;
            cfg.hdrEnabled = true;
            cfg.godRaysEnabled = true;
            cfg.dofEnabled = true;
            cfg.chromaEnabled = true;
            cfg.grainEnabled = true;
        }
    }

    inline const char* KeyToName(int key) {
        switch (key) {
            case KEY_W: return "W";
            case KEY_A: return "A";
            case KEY_S: return "S";
            case KEY_D: return "D";
            case KEY_E: return "E";
            case KEY_F: return "F";
            case KEY_Q: return "Q";
            case KEY_R: return "R";
            case KEY_Z: return "Z";
            case KEY_X: return "X";
            case KEY_C: return "C";
            case KEY_V: return "V";
            case KEY_SPACE: return "SPACE";
            case KEY_LEFT_SHIFT: return "L-SHIFT";
            case KEY_RIGHT_SHIFT: return "R-SHIFT";
            case KEY_LEFT_CONTROL: return "L-CTRL";
            default: return TextFormat("KEY_%d", key);
        }
    }

    inline void SaveSettings(const GameSettings& cfg, const char* path = "settings.cfg") {
        std::ofstream out(path);
        if (!out.is_open()) return;
        out << "preset=" << cfg.preset << "\n";
        out << "resIndex=" << cfg.resIndex << "\n";
        out << "fullscreen=" << (cfg.fullscreen ? 1 : 0) << "\n";
        out << "vsync=" << (cfg.vsync ? 1 : 0) << "\n";
        out << "fpsLimit=" << cfg.fpsLimit << "\n";
        out << "drawDistance=" << cfg.drawDistance << "\n";
        out << "texQuality=" << cfg.texQuality << "\n";
        out << "shadowQuality=" << cfg.shadowQuality << "\n";
        out << "reflectionQuality=" << cfg.reflectionQuality << "\n";
        out << "fogQuality=" << cfg.fogQuality << "\n";
        out << "bloomEnabled=" << (cfg.bloomEnabled ? 1 : 0) << "\n";
        out << "ssaoEnabled=" << (cfg.ssaoEnabled ? 1 : 0) << "\n";
        out << "hdrEnabled=" << (cfg.hdrEnabled ? 1 : 0) << "\n";
        out << "godRaysEnabled=" << (cfg.godRaysEnabled ? 1 : 0) << "\n";
        out << "dofEnabled=" << (cfg.dofEnabled ? 1 : 0) << "\n";
        out << "chromaEnabled=" << (cfg.chromaEnabled ? 1 : 0) << "\n";
        out << "grainEnabled=" << (cfg.grainEnabled ? 1 : 0) << "\n";
        out << "masterVolume=" << cfg.masterVolume << "\n";
        out << "musicVolume=" << cfg.musicVolume << "\n";
        out << "sfxVolume=" << cfg.sfxVolume << "\n";
        out << "ambientVolume=" << cfg.ambientVolume << "\n";
        out << "uiVolume=" << cfg.uiVolume << "\n";
        out << "mouseSensitivity=" << cfg.mouseSensitivity << "\n";
        out << "invertY=" << (cfg.invertY ? 1 : 0) << "\n";
        out << "keyForward=" << cfg.keyForward << "\n";
        out << "keyBackward=" << cfg.keyBackward << "\n";
        out << "keySprint=" << cfg.keySprint << "\n";
        out << "keyInteract=" << cfg.keyInteract << "\n";
        out << "keyFlashlight=" << cfg.keyFlashlight << "\n";
        out << "keyZoom=" << cfg.keyZoom << "\n";
        out << "fov=" << cfg.fov << "\n";
        out << "brightness=" << cfg.brightness << "\n";
        out << "contrast=" << cfg.contrast << "\n";
        out << "language=" << cfg.language << "\n";
        out << "subtitles=" << (cfg.subtitles ? 1 : 0) << "\n";
        out << "difficulty=" << cfg.difficulty << "\n";
    }

    inline void LoadSettings(GameSettings& cfg, const char* path = "settings.cfg") {
        std::ifstream in(path);
        if (!in.is_open()) return;
        std::string line;
        while (std::getline(in, line)) {
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string k = line.substr(0, eq);
            std::string v = line.substr(eq + 1);
            if (k == "preset") cfg.preset = std::stoi(v);
            else if (k == "resIndex") cfg.resIndex = Clamp(std::stoi(v), 0, 2);
            else if (k == "fullscreen") cfg.fullscreen = (std::stoi(v) != 0);
            else if (k == "vsync") cfg.vsync = (std::stoi(v) != 0);
            else if (k == "fpsLimit") cfg.fpsLimit = std::stoi(v);
            else if (k == "drawDistance") cfg.drawDistance = std::stof(v);
            else if (k == "texQuality") cfg.texQuality = Clamp(std::stoi(v), 0, 3);
            else if (k == "shadowQuality") cfg.shadowQuality = Clamp(std::stoi(v), 0, 3);
            else if (k == "reflectionQuality") cfg.reflectionQuality = Clamp(std::stoi(v), 0, 3);
            else if (k == "fogQuality") cfg.fogQuality = Clamp(std::stoi(v), 1, 2);
            else if (k == "bloomEnabled") cfg.bloomEnabled = (std::stoi(v) != 0);
            else if (k == "ssaoEnabled") cfg.ssaoEnabled = (std::stoi(v) != 0);
            else if (k == "hdrEnabled") cfg.hdrEnabled = (std::stoi(v) != 0);
            else if (k == "godRaysEnabled") cfg.godRaysEnabled = (std::stoi(v) != 0);
            else if (k == "dofEnabled") cfg.dofEnabled = (std::stoi(v) != 0);
            else if (k == "chromaEnabled") cfg.chromaEnabled = (std::stoi(v) != 0);
            else if (k == "grainEnabled") cfg.grainEnabled = (std::stoi(v) != 0);
            else if (k == "masterVolume") cfg.masterVolume = std::stof(v);
            else if (k == "musicVolume") cfg.musicVolume = std::stof(v);
            else if (k == "sfxVolume") cfg.sfxVolume = std::stof(v);
            else if (k == "ambientVolume") cfg.ambientVolume = std::stof(v);
            else if (k == "uiVolume") cfg.uiVolume = std::stof(v);
            else if (k == "mouseSensitivity") cfg.mouseSensitivity = std::stof(v);
            else if (k == "invertY") cfg.invertY = (std::stoi(v) != 0);
            else if (k == "keyForward") cfg.keyForward = std::stoi(v);
            else if (k == "keyBackward") cfg.keyBackward = std::stoi(v);
            else if (k == "keySprint") cfg.keySprint = std::stoi(v);
            else if (k == "keyInteract") cfg.keyInteract = std::stoi(v);
            else if (k == "keyFlashlight") cfg.keyFlashlight = std::stoi(v);
            else if (k == "keyZoom") cfg.keyZoom = std::stoi(v);
            else if (k == "fov") cfg.fov = std::stof(v);
            else if (k == "brightness") cfg.brightness = std::stof(v);
            else if (k == "contrast") cfg.contrast = std::stof(v);
            else if (k == "language") cfg.language = Clamp(std::stoi(v), 0, 1);
            else if (k == "subtitles") cfg.subtitles = (std::stoi(v) != 0);
            else if (k == "difficulty") cfg.difficulty = Clamp(std::stoi(v), 0, 2);
        }
    }

    inline void SaveProgressFile(const SaveProgress& s, const char* path = "savegame.dat") {
        std::ofstream out(path);
        if (!out.is_open()) return;
        out << (s.hasSave ? 1 : 0) << " " << s.sectorId << " "
            << s.posX << " " << s.posY << " " << s.posZ << " "
            << s.yaw << " " << s.pitch << " " << s.anomalyFactor << "\n";
    }

    inline void LoadProgressFile(SaveProgress& s, const char* path = "savegame.dat") {
        std::ifstream in(path);
        if (!in.is_open()) return;
        int flag = 0;
        if (in >> flag >> s.sectorId >> s.posX >> s.posY >> s.posZ >> s.yaw >> s.pitch >> s.anomalyFactor) {
            s.hasSave = (flag != 0);
        }
    }
}

namespace AudioSys {

    struct EngineAudio {
        bool initialized = false;
        Sound uiHover{};
        Sound uiClick{};
        Sound clickSwitch{};
        Sound footstepMetal{};
        Sound ambientPulse{};
        Sound liftHum{};
    };

    inline Sound GenSynthSound(float durationSec, float freqStart, float freqEnd, float noiseMix, float decay) {
        const int sampleRate = 44100;
        unsigned int frameCount = static_cast<unsigned int>(durationSec * static_cast<float>(sampleRate));
        short* samples = static_cast<short*>(RL_MALLOC(frameCount * sizeof(short)));
        float phase = 0.0f;
        for (unsigned int i = 0; i < frameCount; i++) {
            float t = static_cast<float>(i) / static_cast<float>(std::max<unsigned int>(frameCount, 1u));
            float env = expf(-t * decay) * (1.0f - powf(t, 4.0f));
            float freq = Lerp(freqStart, freqEnd, t);
            phase += (2.0f * PI * freq) / static_cast<float>(sampleRate);
            float tone = sinf(phase);
            float rnd = static_cast<float>(GetRandomValue(-1000, 1000)) / 1000.0f;
            float val = Lerp(tone, rnd, noiseMix) * env * 0.45f;
            samples[i] = static_cast<short>(Clamp(val * 32767.0f, -32767.0f, 32767.0f));
        }
        Wave w{};
        w.frameCount = frameCount;
        w.sampleRate = static_cast<unsigned int>(sampleRate);
        w.sampleSize = 16;
        w.channels = 1;
        w.data = samples;
        Sound s = LoadSoundFromWave(w);
        RL_FREE(samples);
        return s;
    }

    inline EngineAudio InitEngineAudio() {
        EngineAudio a{};
        InitAudioDevice();
        if (!IsAudioDeviceReady()) return a;
        a.uiHover       = GenSynthSound(0.06f, 520.0f, 640.0f, 0.05f, 18.0f);
        a.uiClick       = GenSynthSound(0.11f, 380.0f, 180.0f, 0.15f, 14.0f);
        a.clickSwitch   = GenSynthSound(0.09f, 760.0f, 290.0f, 0.25f, 20.0f);
        a.footstepMetal = GenSynthSound(0.18f, 145.0f, 52.0f,  0.65f, 12.0f);
        a.ambientPulse  = GenSynthSound(2.40f, 58.0f,  41.0f,  0.30f, 1.8f);
        a.liftHum       = GenSynthSound(1.40f, 92.0f,  118.0f, 0.22f, 1.4f);
        a.initialized = true;
        return a;
    }

    inline void ApplyVolumes(EngineAudio& a, const ConfigSys::GameSettings& cfg, float anomalyFactor, bool zoomed) {
        if (!a.initialized) return;
        float m = Clamp(cfg.masterVolume, 0.0f, 1.0f);
        if (zoomed) m *= 0.85f;
        SetMasterVolume(m);
        SetSoundVolume(a.uiHover, Clamp(cfg.uiVolume * 0.7f, 0.0f, 1.0f));
        SetSoundVolume(a.uiClick, Clamp(cfg.uiVolume, 0.0f, 1.0f));
        SetSoundVolume(a.ambientPulse, Clamp(cfg.ambientVolume * (0.6f + anomalyFactor * 0.4f), 0.0f, 1.0f));
    }

    inline void UnloadEngineAudio(EngineAudio& a) {
        if (!a.initialized) return;
        UnloadSound(a.uiHover);
        UnloadSound(a.uiClick);
        UnloadSound(a.clickSwitch);
        UnloadSound(a.footstepMetal);
        UnloadSound(a.ambientPulse);
        UnloadSound(a.liftHum);
        CloseAudioDevice();
        a.initialized = false;
    }
}

namespace TextureSys {

    struct PBRMaterialSet {
        Texture2D albedo{};
        Texture2D normalMap{};
        Texture2D ormMap{};
    };

    struct MaterialBank {
        PBRMaterialSet concrete{};
        PBRMaterialSet steel{};
        PBRMaterialSet rust{};
        PBRMaterialSet grating{};
        PBRMaterialSet copper{};
        bool loaded = false;
    };

    inline float Hash2D(int x, int y, int seed) {
        int n = x * 374761393 + y * 668265263 + seed * 1442695041;
        n = (n ^ (n >> 13)) * 1274126177;
        return static_cast<float>((n ^ (n >> 16)) & 0x7fffffff) / static_cast<float>(0x7fffffff);
    }

    inline PBRMaterialSet GenerateSurfaceSet(int size, int style, int seed) {
        Image imgAlb = GenImageColor(size, size, WHITE);
        Image imgNrm = GenImageColor(size, size, { 128, 128, 255, 255 });
        Image imgOrm = GenImageColor(size, size, { 255, 180, 20, 255 });

        std::vector<float> heightField(static_cast<size_t>(size) * static_cast<size_t>(size), 0.5f);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                float u = static_cast<float>(x) / static_cast<float>(size);
                float v = static_cast<float>(y) / static_cast<float>(size);
                float nFine = Hash2D(x, y, seed);
                float nCoarse = Hash2D(x / 8, y / 8, seed + 17);
                float h = nFine * 0.35f + nCoarse * 0.65f;

                Color cAlb = { 130, 136, 145, 255 };
                unsigned char ao = 235;
                unsigned char rough = 190;
                unsigned char metal = 20;

                if (style == 0) {
                    int period = std::max(size / 4, 4);
                    bool seam = ((x % period) < 2) || ((y % period) < 2);
                    if (seam) h *= 0.25f;
                    unsigned char base = static_cast<unsigned char>(Clamp(108.0f + h * 46.0f, 65.0f, 175.0f));
                    cAlb = { base, static_cast<unsigned char>(Clamp(base + 4, 0, 255)), static_cast<unsigned char>(Clamp(base + 10, 0, 255)), 255 };
                    ao = seam ? 145 : static_cast<unsigned char>(Clamp(210.0f + h * 40.0f, 0.0f, 255.0f));
                    rough = seam ? 235 : 195;
                    metal = 12;
                } else if (style == 1) {
                    float streak = 0.5f + 0.5f * sinf(u * PI * 16.0f + nCoarse * 2.0f);
                    h = 0.45f + streak * 0.35f + nFine * 0.2f;
                    unsigned char base = static_cast<unsigned char>(Clamp(128.0f + h * 55.0f, 80.0f, 215.0f));
                    cAlb = { base, static_cast<unsigned char>(Clamp(base + 6, 0, 255)), static_cast<unsigned char>(Clamp(base + 16, 0, 255)), 255 };
                    ao = 230;
                    rough = 92;
                    metal = 215;
                } else if (style == 2) {
                    float patch = SmoothstepCpp(0.32f, 0.78f, nCoarse);
                    h = 0.3f + nFine * 0.5f + patch * 0.2f;
                    Color cClean = { 132, 138, 146, 255 };
                    Color cRust  = { 156, 92,  58,  255 };
                    cAlb = ColorLerp(cClean, cRust, patch);
                    ao = static_cast<unsigned char>(Clamp(Lerp(230.0f, 165.0f, patch), 0.0f, 255.0f));
                    rough = static_cast<unsigned char>(Clamp(Lerp(110.0f, 232.0f, patch), 0.0f, 255.0f));
                    metal = static_cast<unsigned char>(Clamp(Lerp(195.0f, 45.0f, patch), 0.0f, 255.0f));
                } else if (style == 3) {
                    int cell = std::max(size / 16, 4);
                    bool bar = ((x % cell) < 2) || ((y % cell) < 2);
                    h = bar ? 0.85f : 0.15f;
                    cAlb = bar ? Color{ 165, 174, 186, 255 } : Color{ 44, 52, 62, 255 };
                    ao = bar ? 245 : 110;
                    rough = bar ? 105 : 210;
                    metal = bar ? 210 : 60;
                } else {
                    float wave = 0.5f + 0.5f * sinf(v * PI * 24.0f);
                    h = 0.5f + wave * 0.25f + nFine * 0.15f;
                    cAlb = {
                        static_cast<unsigned char>(Clamp(185.0f + wave * 35.0f, 0.0f, 255.0f)),
                        static_cast<unsigned char>(Clamp(125.0f + wave * 25.0f, 0.0f, 255.0f)),
                        static_cast<unsigned char>(Clamp(74.0f + wave * 18.0f, 0.0f, 255.0f)),
                        255
                    };
                    ao = 240;
                    rough = 74;
                    metal = 235;
                }

                heightField[static_cast<size_t>(y) * static_cast<size_t>(size) + static_cast<size_t>(x)] = h;
                ImageDrawPixel(&imgAlb, x, y, cAlb);
                ImageDrawPixel(&imgOrm, x, y, { ao, rough, metal, 255 });
            }
        }

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int xm = (x - 1 + size) % size;
                int xp = (x + 1) % size;
                int ym = (y - 1 + size) % size;
                int yp = (y + 1) % size;
                float hL = heightField[static_cast<size_t>(y) * static_cast<size_t>(size) + static_cast<size_t>(xm)];
                float hR = heightField[static_cast<size_t>(y) * static_cast<size_t>(size) + static_cast<size_t>(xp)];
                float hU = heightField[static_cast<size_t>(ym) * static_cast<size_t>(size) + static_cast<size_t>(x)];
                float hD = heightField[static_cast<size_t>(yp) * static_cast<size_t>(size) + static_cast<size_t>(x)];
                float dx = (hR - hL) * 2.4f;
                float dy = (hD - hU) * 2.4f;
                Vector3 n = Vector3Normalize({ -dx, -dy, 1.0f });
                Color cn = {
                    static_cast<unsigned char>(Clamp((n.x * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f)),
                    static_cast<unsigned char>(Clamp((n.y * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f)),
                    static_cast<unsigned char>(Clamp((n.z * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f)),
                    255
                };
                ImageDrawPixel(&imgNrm, x, y, cn);
            }
        }

        PBRMaterialSet set{};
        set.albedo = LoadTextureFromImage(imgAlb);
        set.normalMap = LoadTextureFromImage(imgNrm);
        set.ormMap = LoadTextureFromImage(imgOrm);

        GenTextureMipmaps(&set.albedo);
        GenTextureMipmaps(&set.normalMap);
        GenTextureMipmaps(&set.ormMap);

        SetTextureWrap(set.albedo, TEXTURE_WRAP_REPEAT);
        SetTextureWrap(set.normalMap, TEXTURE_WRAP_REPEAT);
        SetTextureWrap(set.ormMap, TEXTURE_WRAP_REPEAT);
        SetTextureFilter(set.albedo, TEXTURE_FILTER_TRILINEAR);
        SetTextureFilter(set.normalMap, TEXTURE_FILTER_TRILINEAR);
        SetTextureFilter(set.ormMap, TEXTURE_FILTER_TRILINEAR);

        UnloadImage(imgAlb);
        UnloadImage(imgNrm);
        UnloadImage(imgOrm);
        return set;
    }

    inline MaterialBank CreateMaterials(int texQuality) {
        const int sizes[4] = { 128, 256, 512, 1024 };
        int qIdx = static_cast<int>(Clamp(static_cast<float>(texQuality), 0.0f, 3.0f));
        int sz = sizes[qIdx];
        MaterialBank bank{};
        bank.concrete = GenerateSurfaceSet(sz, 0, 101);
        bank.steel    = GenerateSurfaceSet(sz, 1, 202);
        bank.rust     = GenerateSurfaceSet(sz, 2, 303);
        bank.grating  = GenerateSurfaceSet(sz, 3, 404);
        bank.copper   = GenerateSurfaceSet(sz, 4, 505);
        bank.loaded = true;
        return bank;
    }

    inline void UnloadSurfaceSet(PBRMaterialSet& s) {
        if (s.albedo.id != 0) UnloadTexture(s.albedo);
        if (s.normalMap.id != 0) UnloadTexture(s.normalMap);
        if (s.ormMap.id != 0) UnloadTexture(s.ormMap);
    }

    inline void UnloadMaterials(MaterialBank& bank) {
        if (!bank.loaded) return;
        UnloadSurfaceSet(bank.concrete);
        UnloadSurfaceSet(bank.steel);
        UnloadSurfaceSet(bank.rust);
        UnloadSurfaceSet(bank.grating);
        UnloadSurfaceSet(bank.copper);
        bank.loaded = false;
    }
}

namespace ShaderSys {

    struct ShaderBundle {
        Shader pbrWorld{};
        Shader shadowDepth{};
        Shader postProcess{};
        RenderTexture2D shadowMap{};
        RenderTexture2D sceneColor{};
        RenderTexture2D sceneDepth{};
        int shadowRes = 2048;
        int sceneW = 0;
        int sceneH = 0;

        int locViewPos = -1, locLightDir = -1, locLightColor = -1, locLightVP = -1;
        int locShadowMap = -1, locNormalMap = -1, locOrmMap = -1;
        int locHasNormalMap = -1, locHasOrmMap = -1, locTime = -1;
        int locFogDensity = -1, locFogColor = -1, locAnomalyFactor = -1;
        int locWaterLevel = -1, locReflectStrength = -1;
        int locShadowRes = -1, locShadowEnabled = -1, locLightCount = -1;

        int locPostScene = -1, locPostDepth = -1, locPostTime = -1;
        int locPostBloom = -1, locPostSsao = -1, locPostHdr = -1;
        int locPostGodRays = -1, locPostDof = -1, locPostChroma = -1, locPostGrain = -1;
        int locPostBrightness = -1, locPostContrast = -1, locPostSunScreen = -1, locPostResolution = -1;

        bool valid = false;
    };

    static const char* kShadowVS = R"GLSL(
#version 330
in vec3 vertexPosition;
uniform mat4 mvp;
void main() {
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)GLSL";

    static const char* kShadowFS = R"GLSL(
#version 330
void main() {}
)GLSL";

    static const char* kPbrWorldVS = R"GLSL(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform mat4 lightVP;

out vec3 fragWorldPos;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec4 fragColor;
out vec4 fragLightPos;

void main() {
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize((matNormal * vec4(vertexNormal, 0.0)).xyz);
    fragColor = vertexColor;
    fragLightPos = lightVP * worldPos;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)GLSL";

    static const char* kPbrWorldFS = R"GLSL(
#version 330
#define MAX_POINT_LIGHTS 8

in vec3 fragWorldPos;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;
in vec4 fragLightPos;

uniform sampler2D texture0;
uniform sampler2D normalMap;
uniform sampler2D ormMap;
uniform sampler2D shadowMap;

uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float time;
uniform float fogDensity;
uniform vec3 fogColor;
uniform float anomalyFactor;
uniform float waterLevel;
uniform float reflectStrength;
uniform float shadowRes;
uniform int hasNormalMap;
uniform int hasOrmMap;
uniform int shadowEnabled;
uniform int pointLightCount;
uniform vec4 pointLightPosRadius[MAX_POINT_LIGHTS];
uniform vec4 pointLightColorInt[MAX_POINT_LIGHTS];

out vec4 finalColor;

const float PI = 3.14159265359;

mat3 ComputeTBN(vec3 N, vec3 p, vec2 uv) {
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invMax = inversesqrt(max(dot(T, T), dot(B, B)) + 1e-6);
    return mat3(T * invMax, B * invMax, N);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = PI * pow(NdotH * NdotH * (a2 - 1.0) + 1.0, 2.0);
    return a2 / max(denom, 1e-5);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 1e-5);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float CalcDirectionalShadow(vec4 lightPosSpace, vec3 N, vec3 L) {
    if (shadowEnabled == 0) return 1.0;
    vec3 proj = lightPosSpace.xyz / max(lightPosSpace.w, 1e-5);
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0) {
        return 1.0;
    }
    float bias = max(0.0022 * (1.0 - dot(N, L)), 0.00045);
    float texel = 1.0 / max(shadowRes, 512.0);
    float shadow = 0.0;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float d = texture(shadowMap, proj.xy + vec2(x, y) * texel).r;
            shadow += (proj.z - bias > d) ? 0.32 : 1.0;
        }
    }
    return shadow / 9.0;
}

vec3 EvalEnvironmentReflection(vec3 R, float roughness, bool isWater) {
    float skyGrad = clamp(R.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 horizon = mix(vec3(0.05, 0.08, 0.12), vec3(0.22, 0.07, 0.08), anomalyFactor * 0.65);
    vec3 zenith  = vec3(0.015, 0.025, 0.045);
    vec3 envCol  = mix(horizon, zenith, pow(skyGrad, 1.4));

    vec3 moonDir = normalize(vec3(-0.35, 0.42, -0.84));
    float moonSpec = pow(max(dot(R, moonDir), 0.0), mix(180.0, 16.0, roughness));
    envCol += vec3(0.75, 0.85, 1.0) * moonSpec * (isWater ? 1.6 : 0.75);

    float towerBand = smoothstep(0.02, 0.18, abs(R.y)) * (1.0 - smoothstep(0.18, 0.45, abs(R.y)));
    float pillarWave = 0.5 + 0.5 * sin(R.x * 26.0 + R.z * 18.0);
    envCol += vec3(0.06, 0.09, 0.13) * towerBand * pillarWave * (1.0 - roughness * 0.7);
    return envCol;
}

void main() {
    vec4 texAlbedo = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    if (texAlbedo.a < 0.05) discard;

    vec3 albedo = texAlbedo.rgb;
    float ao = 1.0;
    float roughness = 0.68;
    float metallic = 0.10;

    if (hasOrmMap != 0) {
        vec3 orm = texture(ormMap, fragTexCoord).rgb;
        ao = mix(1.0, orm.r, 0.85);
        roughness = clamp(orm.g, 0.06, 0.98);
        metallic = clamp(orm.b, 0.0, 0.96);
    }

    vec3 N = normalize(fragNormal);
    if (hasNormalMap != 0) {
        vec3 mapN = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;
        mapN.xy *= 1.15;
        N = normalize(ComputeTBN(N, fragWorldPos, fragTexCoord) * mapN);
    }

    bool isWaterSurface = (abs(fragWorldPos.y - waterLevel) < 0.25 && fragWorldPos.y < waterLevel + 0.35 && N.y > 0.75);
    float puddleMask = 0.0;
    if (!isWaterSurface && N.y > 0.82 && reflectStrength > 0.01) {
        float n1 = sin(fragWorldPos.x * 0.14 + time * 0.15) * cos(fragWorldPos.z * 0.14 - time * 0.12);
        float n2 = sin((fragWorldPos.x + fragWorldPos.z) * 0.37);
        puddleMask = smoothstep(0.32, 0.72, n1 * 0.6 + n2 * 0.4 + 0.35) * reflectStrength * 0.65;
        roughness = mix(roughness, 0.09, puddleMask);
        albedo = mix(albedo, albedo * 0.62, puddleMask * 0.6);
    }

    if (isWaterSurface) {
        float w1 = sin(fragWorldPos.x * 0.45 + time * 1.6) * cos(fragWorldPos.z * 0.42 + time * 1.2);
        float w2 = sin((fragWorldPos.x * 0.9 - fragWorldPos.z * 0.8) - time * 2.1);
        N = normalize(N + vec3(w1 * 0.08 + w2 * 0.04, 0.0, w2 * 0.08 - w1 * 0.04));
        roughness = 0.07;
        metallic = 0.15;
    }

    vec3 V = normalize(viewPos - fragWorldPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);

    vec3 specularDir = (DistributionGGX(N, H, roughness) * GeometrySmith(N, V, L, roughness) * FresnelSchlick(max(dot(H, V), 0.0), F0)) /
                       (4.0 * max(dot(N, V), 0.0) * NdotL + 1e-4);
    vec3 kD = (vec3(1.0) - FresnelSchlick(max(dot(H, V), 0.0), F0)) * (1.0 - metallic);

    float shadow = CalcDirectionalShadow(fragLightPos, N, L);
    float contactAO = clamp(0.65 + 0.35 * smoothstep(0.0, 2.5, abs(fragWorldPos.y - waterLevel)), 0.55, 1.0);
    vec3 Lo = (kD * albedo / PI + specularDir) * lightColor * NdotL * shadow * contactAO;

    int activeLights = min(pointLightCount, MAX_POINT_LIGHTS);
    for (int i = 0; i < activeLights; i++) {
        vec3 lPos = pointLightPosRadius[i].xyz;
        float lRad = max(pointLightPosRadius[i].w, 1.0);
        vec3 lCol = pointLightColorInt[i].rgb;
        float lInt = pointLightColorInt[i].a;

        vec3 toLight = lPos - fragWorldPos;
        float dist = length(toLight);
        if (dist < lRad) {
            vec3 pL = toLight / max(dist, 1e-4);
            vec3 pH = normalize(V + pL);
            float atten = pow(clamp(1.0 - dist / lRad, 0.0, 1.0), 2.0);
            float pNdotL = max(dot(N, pL), 0.0);
            vec3 pF = FresnelSchlick(max(dot(pH, V), 0.0), F0);
            vec3 pSpec = (DistributionGGX(N, pH, roughness) * GeometrySmith(N, V, pL, roughness) * pF) /
                         (4.0 * max(dot(N, V), 0.0) * pNdotL + 1e-4);
            vec3 pKD = (vec3(1.0) - pF) * (1.0 - metallic);
            Lo += (pKD * albedo / PI + pSpec) * lCol * lInt * atten * pNdotL;
        }
    }

    vec3 R = reflect(-V, N);
    vec3 envRefl = EvalEnvironmentReflection(R, roughness, isWaterSurface);
    float reflMix = isWaterSurface
        ? clamp(0.55 + 0.45 * pow(1.0 - max(dot(N, V), 0.0), 3.0), 0.0, 0.95)
        : clamp((puddleMask * 0.75 + (1.0 - roughness) * metallic * 0.45) * reflectStrength, 0.0, 0.85);

    vec3 ambient = albedo * vec3(0.11, 0.14, 0.19) * ao * (1.0 + anomalyFactor * 0.25);
    vec3 color = ambient + Lo + envRefl * reflMix;

    float camDist = length(viewPos - fragWorldPos);
    float heightFactor = exp(-max(fragWorldPos.y + 30.0, 0.0) * 0.012);
    float effDensity = fogDensity * (0.65 + 0.55 * heightFactor) * (1.0 + anomalyFactor * 0.35);
    float fogFactor = clamp(1.0 - exp(-pow(camDist * effDensity, 1.65)), 0.0, 0.96);

    vec3 dynamicFog = mix(fogColor, vec3(0.24, 0.07, 0.08), anomalyFactor * 0.65);
    color = mix(color, dynamicFog, fogFactor);
    finalColor = vec4(color, texAlbedo.a);
}
)GLSL";

    static const char* kPostProcessFS = R"GLSL(
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform sampler2D depthTex;
uniform float time;
uniform int bloomEnabled;
uniform int ssaoEnabled;
uniform int hdrEnabled;
uniform int godRaysEnabled;
uniform int dofEnabled;
uniform int chromaEnabled;
uniform int grainEnabled;
uniform float brightness;
uniform float contrast;
uniform vec2 sunScreenPos;
uniform vec2 resolution;

vec3 ACESFilm(vec3 x) {
    float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float Hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec2 uv = fragTexCoord;
    vec2 texel = 1.0 / max(resolution, vec2(1280.0, 720.0));

    vec3 col;
    if (chromaEnabled != 0) {
        vec2 offset = (uv - 0.5) * 0.0018;
        col = vec3(
            texture(texture0, uv + offset).r,
            texture(texture0, uv).g,
            texture(texture0, uv - offset).b
        );
    } else {
        col = texture(texture0, uv).rgb;
    }

    if (dofEnabled != 0) {
        float blurAmt = smoothstep(0.18, 0.55, length(uv - 0.5)) * 2.2;
        vec3 acc = col;
        acc += texture(texture0, uv + vec2( blurAmt, 0.0) * texel).rgb;
        acc += texture(texture0, uv + vec2(-blurAmt, 0.0) * texel).rgb;
        acc += texture(texture0, uv + vec2(0.0,  blurAmt) * texel).rgb;
        acc += texture(texture0, uv + vec2(0.0, -blurAmt) * texel).rgb;
        col = acc / 5.0;
    }

    if (ssaoEnabled != 0) {
        float dCenter = texture(depthTex, uv).r;
        float occ = 0.0;
        for (int i = 0; i < 8; i++) {
            float ang = float(i) * 0.785398;
            vec2 dir = vec2(cos(ang), sin(ang)) * texel * 5.0;
            float diff = texture(depthTex, clamp(uv + dir, 0.001, 0.999)).r - dCenter;
            if (diff > 0.0005 && diff < 0.015) occ += 0.14;
        }
        col *= clamp(1.0 - occ, 0.58, 1.0);
    }

    if (godRaysEnabled != 0) {
        vec2 delta = (sunScreenPos - uv) * 0.035;
        vec2 rayUV = uv;
        float rayAccum = 0.0, decay = 0.92, illum = 0.18;
        for (int s = 0; s < 10; s++) {
            rayUV += delta;
            vec3 sCol = texture(texture0, clamp(rayUV, 0.001, 0.999)).rgb;
            rayAccum += smoothstep(0.35, 1.0, dot(sCol, vec3(0.299, 0.587, 0.114))) * illum;
            illum *= decay;
        }
        col += vec3(0.55, 0.68, 0.85) * rayAccum;
    }

    if (bloomEnabled != 0) {
        vec3 bloomSum = vec3(0.0);
        float rads[3] = float[](2.0, 5.0, 9.0);
        for (int rIdx = 0; rIdx < 3; rIdx++) {
            float rad = rads[rIdx];
            vec2 o1 = vec2(rad, 0.0) * texel;
            vec2 o2 = vec2(0.0, rad) * texel;
            vec2 o3 = vec2(rad, rad) * texel * 0.7;
            vec2 o4 = vec2(rad, -rad) * texel * 0.7;
            vec3 samples[8] = vec3[](
                texture(texture0, uv + o1).rgb, texture(texture0, uv - o1).rgb,
                texture(texture0, uv + o2).rgb, texture(texture0, uv - o2).rgb,
                texture(texture0, uv + o3).rgb, texture(texture0, uv - o3).rgb,
                texture(texture0, uv + o4).rgb, texture(texture0, uv - o4).rgb
            );
            for (int k = 0; k < 8; k++) bloomSum += max(samples[k] - vec3(0.72), vec3(0.0));
        }
        col += (bloomSum / 24.0) * 0.55;
    }

    col = (hdrEnabled != 0) ? ACESFilm(col * 1.15) : clamp(col, 0.0, 1.0);
    col = ((col - 0.5) * contrast + 0.5) * brightness;

    if (grainEnabled != 0) {
        col += vec3((Hash12(uv * resolution + vec2(time * 60.0)) - 0.5) * 0.032);
    }
    vec2 vigUV = uv * (1.0 - uv.yx);
    col *= clamp(pow(vigUV.x * vigUV.y * 16.0, 0.22), 0.35, 1.0);
    finalColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
)GLSL";

    inline int ShadowSizeFromQuality(int q) {
        if (q <= 0) return 512;
        if (q == 1) return 1024;
        if (q == 2) return 2048;
        return 4096;
    }

    inline void CreateSceneTargets(ShaderBundle& b, int w, int h, int shadowRes) {
        w = std::max(w, 640);
        h = std::max(h, 360);
        if (b.sceneColor.id != 0) UnloadRenderTexture(b.sceneColor);
        if (b.sceneDepth.id != 0) UnloadRenderTexture(b.sceneDepth);
        if (b.shadowMap.id != 0) UnloadRenderTexture(b.shadowMap);

        b.sceneColor = LoadRenderTexture(w, h);
        SetTextureFilter(b.sceneColor.texture, TEXTURE_FILTER_BILINEAR);
        b.sceneDepth = LoadRenderTexture(w, h);
        SetTextureFilter(b.sceneDepth.texture, TEXTURE_FILTER_BILINEAR);
        b.shadowMap = LoadRenderTexture(shadowRes, shadowRes);
        SetTextureFilter(b.shadowMap.texture, TEXTURE_FILTER_BILINEAR);

        b.sceneW = w;
        b.sceneH = h;
        b.shadowRes = shadowRes;
    }

    inline ShaderBundle InitShaders(int screenW, int screenH, int shadowQuality) {
        ShaderBundle b{};
        b.shadowDepth = LoadShaderFromMemory(kShadowVS, kShadowFS);
        b.pbrWorld = LoadShaderFromMemory(kPbrWorldVS, kPbrWorldFS);
        b.postProcess = LoadShaderFromMemory(nullptr, kPostProcessFS);
        if (b.pbrWorld.id == 0 || b.postProcess.id == 0) return b;

        if (b.pbrWorld.locs != nullptr) {
            b.pbrWorld.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(b.pbrWorld, "texture0");
            b.pbrWorld.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(b.pbrWorld, "viewPos");
            b.pbrWorld.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(b.pbrWorld, "colDiffuse");
        }

        b.locViewPos = GetShaderLocation(b.pbrWorld, "viewPos");
        b.locLightDir = GetShaderLocation(b.pbrWorld, "lightDir");
        b.locLightColor = GetShaderLocation(b.pbrWorld, "lightColor");
        b.locLightVP = GetShaderLocation(b.pbrWorld, "lightVP");
        b.locShadowMap = GetShaderLocation(b.pbrWorld, "shadowMap");
        b.locNormalMap = GetShaderLocation(b.pbrWorld, "normalMap");
        b.locOrmMap = GetShaderLocation(b.pbrWorld, "ormMap");
        b.locHasNormalMap = GetShaderLocation(b.pbrWorld, "hasNormalMap");
        b.locHasOrmMap = GetShaderLocation(b.pbrWorld, "hasOrmMap");
        b.locTime = GetShaderLocation(b.pbrWorld, "time");
        b.locFogDensity = GetShaderLocation(b.pbrWorld, "fogDensity");
        b.locFogColor = GetShaderLocation(b.pbrWorld, "fogColor");
        b.locAnomalyFactor = GetShaderLocation(b.pbrWorld, "anomalyFactor");
        b.locWaterLevel = GetShaderLocation(b.pbrWorld, "waterLevel");
        b.locReflectStrength = GetShaderLocation(b.pbrWorld, "reflectStrength");
        b.locShadowRes = GetShaderLocation(b.pbrWorld, "shadowRes");
        b.locShadowEnabled = GetShaderLocation(b.pbrWorld, "shadowEnabled");
        b.locLightCount = GetShaderLocation(b.pbrWorld, "pointLightCount");

        b.locPostScene = GetShaderLocation(b.postProcess, "texture0");
        b.locPostDepth = GetShaderLocation(b.postProcess, "depthTex");
        b.locPostTime = GetShaderLocation(b.postProcess, "time");
        b.locPostBloom = GetShaderLocation(b.postProcess, "bloomEnabled");
        b.locPostSsao = GetShaderLocation(b.postProcess, "ssaoEnabled");
        b.locPostHdr = GetShaderLocation(b.postProcess, "hdrEnabled");
        b.locPostGodRays = GetShaderLocation(b.postProcess, "godRaysEnabled");
        b.locPostDof = GetShaderLocation(b.postProcess, "dofEnabled");
        b.locPostChroma = GetShaderLocation(b.postProcess, "chromaEnabled");
        b.locPostGrain = GetShaderLocation(b.postProcess, "grainEnabled");
        b.locPostBrightness = GetShaderLocation(b.postProcess, "brightness");
        b.locPostContrast = GetShaderLocation(b.postProcess, "contrast");
        b.locPostSunScreen = GetShaderLocation(b.postProcess, "sunScreenPos");
        b.locPostResolution = GetShaderLocation(b.postProcess, "resolution");

        CreateSceneTargets(b, screenW, screenH, ShadowSizeFromQuality(shadowQuality));
        b.valid = true;
        return b;
    }

    inline void UnloadShaders(ShaderBundle& b) {
        if (b.sceneColor.id != 0) UnloadRenderTexture(b.sceneColor);
        if (b.sceneDepth.id != 0) UnloadRenderTexture(b.sceneDepth);
        if (b.shadowMap.id != 0) UnloadRenderTexture(b.shadowMap);
        if (b.pbrWorld.id != 0) UnloadShader(b.pbrWorld);
        if (b.shadowDepth.id != 0) UnloadShader(b.shadowDepth);
        if (b.postProcess.id != 0) UnloadShader(b.postProcess);
        b.valid = false;
    }
}

inline void BindPBRToActiveMaterial(Texture2D albedo, Texture2D normalMap, Texture2D ormMap,
                                    bool useNormal, bool useOrm, Color tint) {
    (void)normalMap;
    (void)ormMap;
    (void)useNormal;
    (void)useOrm;
    (void)tint;
    rlSetTexture(albedo.id);
}

namespace WorldSys {

    struct SectorInfo {
        int id;
        const char* nameRu;
        const char* nameEn;
        Vector3 anchorPos;
        float radius;
        float waterLevel;
        bool hasElevator;
    };

    struct LoreLog {
        Vector3 pos;
        const char* titleRu;
        const char* titleEn;
        const char* textRu;
        const char* textEn;
        bool read = false;
    };

    struct SteamVent {
        Vector3 pos;
        float timer = 0.0f;
        float period = 3.2f;
    };

    struct WorldState {
        std::vector<SectorInfo> sectors;
        std::vector<LoreLog> logs;
        std::vector<SteamVent> vents;
        int activeSector = 0;
        float elevatorY = 0.0f;
        float elevatorTargetY = 0.0f;
        bool elevatorMoving = false;
        float rotorAngle = 0.0f;
        float turbineAngle = 0.0f;
    };

    inline bool IsSphereVisible(Vector3 center, float radius, Vector3 camPos, Vector3 camForward, float maxDist) {
        Vector3 toObj = Vector3Subtract(center, camPos);
        float dist = Vector3Length(toObj);
        if (dist - radius > maxDist) return false;
        if (dist <= radius * 1.4f) return true;
        Vector3 dir = Vector3Scale(toObj, 1.0f / std::max(dist, 1e-4f));
        return Vector3DotProduct(dir, camForward) >= -0.25f;
    }

    inline int ComputeLOD(float dist) {
        if (dist < 140.0f) return 0;
        if (dist < 420.0f) return 1;
        return 2;
    }

    inline WorldState CreateWorld() {
        WorldState w{};
        w.sectors = {
            { 0, "СЕКТОР 08: ВЫХОДНОЙ ШЛЮЗ", "SECTOR 08: EXIT AIRLOCK",   { 0.0f, 1.8f, 19.0f },      140.0f, -155.0f, false },
            { 1, "СЕКТОР 09: МОСТ И РЕАКТОР",  "SECTOR 09: BRIDGE & CORE",  { 0.0f, 1.8f, -125.0f },    260.0f, -155.0f, false },
            { 2, "СЕКТОР 10: ТУРБИННЫЙ ЗАЛ",   "SECTOR 10: TURBINE HALL",   { 0.0f, 1.8f, -315.0f },    220.0f, -148.0f, true  },
            { 3, "СЕКТОР 11: АРХИВ-БУНКЕР",    "SECTOR 11: ARCHIVE BUNKER", { 68.0f, -19.5f, -315.0f }, 150.0f, -148.0f, false }
        };

        w.logs = {
            {
                { 0.0f, 1.8f, 14.5f },
                "ЖУРНАЛ ШЛЮЗА №08 // ДОК-01", "AIRLOCK LOG #08 // DOC-01",
                "Давление в шахте 420 МПа. Мост над бездной рассчитан на ветровые нагрузки, но масштабы несущих балок вызывают у персонала когнитивный диссонанс.",
                "Shaft pressure 420 MPa. The chasm bridge handles wind loads, yet the sheer scale of structural girders induces severe scale disorientation."
            },
            {
                { -16.0f, 1.8f, -92.0f },
                "ТЕХНИЧЕСКИЙ ОТЧЁТ // РЕАКТОР-9", "TECH REPORT // REACTOR-9",
                "Сферический резонатор «Гелиос-9» удерживается наклонными опорами с кольцами жёсткости. При переходе на нижний подмосток используйте боковую лестницу.",
                "Spherical resonator Helios-9 is braced by ribbed struts. Use the side ladder to reach the lower catwalk beneath the span."
            },
            {
                { 0.0f, 1.8f, -268.0f },
                "ДИСПЕТЧЕРСКАЯ // ТУРБИНЫ 01–04", "DISPATCH // TURBINES 01–04",
                "Нижний ярус гидротурбинного зала приподнят над уровнем воды на отметке -148 м. Грузовой лифт связывает верхнюю галерею с нижней технической платформой.",
                "Lower turbine hall deck sits above water at -148m. Freight lift links the upper gallery with the lower industrial deck."
            },
            {
                { 66.0f, -19.5f, -315.0f },
                "СЕКРЕТНЫЙ АРХИВ // ПРОЕКТ ТИШИНА", "CLASSIFIED ARCHIVE // SILENCE",
                "Комплекс возводился не ради выработки энергии — массивные плотины и монолиты служили акустическим экраном для гашения инфразвуковых колебаний глубин.",
                "The complex was not built for power alone — massive dams and monoliths acted as an acoustic shield damping infrasonic depth resonance."
            }
        };

        w.vents = {
            { {  10.5f,   1.2f,  -45.0f }, 0.4f, 3.0f },
            { { -11.0f,   1.2f, -140.0f }, 1.5f, 3.5f },
            { {  18.0f,   1.5f, -285.0f }, 0.9f, 2.8f },
            { {  58.0f, -19.2f, -312.0f }, 2.1f, 3.4f }
        };
        return w;
    }

    inline int DetectSector(const WorldState& w, Vector3 pos) {
        (void)w;
        if (pos.x > 42.0f && pos.z < -265.0f && pos.z > -365.0f) return 3;
        if (pos.z < -215.0f) return 2;
        if (pos.z < 4.0f) return 1;
        return 0;
    }

    inline void DrawDetailedCryoTank(Vector3 pos, float yawDeg, int lod,
                                       TextureSys::PBRMaterialSet& steelMat,
                                       TextureSys::PBRMaterialSet& rustMat,
                                       TextureSys::PBRMaterialSet& copperMat) {
        rlPushMatrix();
        rlTranslatef(pos.x, pos.y, pos.z);
        rlRotatef(yawDeg, 0.0f, 1.0f, 0.0f);

        BindPBRToActiveMaterial(steelMat.albedo, steelMat.normalMap, steelMat.ormMap, true, true, WHITE);
        DrawCube({ 0.0f, -0.42f, -0.55f }, 0.92f, 0.22f, 0.26f, { 125, 132, 142, 255 });
        DrawCube({ 0.0f, -0.42f,  0.55f }, 0.92f, 0.22f, 0.26f, { 125, 132, 142, 255 });

        BindPBRToActiveMaterial(rustMat.albedo, rustMat.normalMap, rustMat.ormMap, true, true, WHITE);
        DrawCylinder({ 0.0f, 0.15f, 0.0f }, 0.62f, 0.62f, 2.1f, lod == 0 ? 28 : 16, { 155, 118, 92, 255 });

        BindPBRToActiveMaterial(steelMat.albedo, steelMat.normalMap, steelMat.ormMap, true, true, WHITE);
        DrawSphere({ 0.0f, 0.15f,  1.05f }, 0.61f, { 195, 204, 216, 255 });
        DrawSphere({ 0.0f, 0.15f, -1.05f }, 0.61f, { 195, 204, 216, 255 });
        DrawCylinderWires({ 0.0f, 0.15f,  0.52f }, 0.65f, 0.65f, 0.12f, 20, { 165, 175, 190, 255 });
        DrawCylinderWires({ 0.0f, 0.15f, -0.52f }, 0.65f, 0.65f, 0.12f, 20, { 165, 175, 190, 255 });

        if (lod == 0) {
            BindPBRToActiveMaterial(copperMat.albedo, copperMat.normalMap, copperMat.ormMap, true, true, WHITE);
            DrawCylinder({ 0.0f, 0.88f, 0.0f }, 0.08f, 0.08f, 0.28f, 12, { 225, 155, 95, 255 });
            DrawCylinder({ 0.0f, 1.02f, 0.0f }, 0.26f, 0.26f, 0.06f, 16, { 235, 110, 65, 255 });
        }

        rlPopMatrix();
    }

    inline void DrawMegaReactorAssembly(float rotorAngle, int lod,
                                        TextureSys::PBRMaterialSet& concreteMat,
                                        TextureSys::PBRMaterialSet& steelMat,
                                        TextureSys::PBRMaterialSet& rustMat) {
        Vector3 corePos = { -46.0f, 22.0f, -115.0f };

        BindPBRToActiveMaterial(steelMat.albedo, steelMat.normalMap, steelMat.ormMap, true, true, WHITE);
        DrawSphere(corePos, 24.0f, { 122, 134, 152, 255 });

        rlPushMatrix();
        rlTranslatef(corePos.x, corePos.y, corePos.z);
        rlRotatef(rotorAngle, 0.2f, 1.0f, 0.15f);
        DrawCylinder({ 0.0f, 0.0f, 0.0f }, 26.2f, 26.2f, 1.4f, lod == 0 ? 36 : 20, { 88, 98, 114, 255 });
        rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        DrawCylinder({ 0.0f, 0.0f, 0.0f }, 25.4f, 25.4f, 1.1f, lod == 0 ? 32 : 18, { 105, 116, 132, 255 });
        rlPopMatrix();

        BindPBRToActiveMaterial(rustMat.albedo, rustMat.normalMap, rustMat.ormMap, true, true, WHITE);
        DrawCylinderEx({ -16.0f, -155.0f, -72.0f }, { -34.0f, 10.0f, -98.0f }, 3.4f, 2.8f, 16, { 112, 92, 82, 255 });
        DrawCylinderEx({ -68.0f, -155.0f, -85.0f }, { -54.0f, 8.0f, -104.0f }, 3.2f, 2.6f, 16, { 112, 92, 82, 255 });

        BindPBRToActiveMaterial(concreteMat.albedo, concreteMat.normalMap, concreteMat.ormMap, true, true, WHITE);
        DrawCube({ -25.0f, -72.0f, -85.0f }, 8.5f, 3.2f, 8.5f, { 125, 132, 142, 255 });
        DrawCube({ -61.0f, -73.0f, -94.0f }, 8.0f, 3.2f, 8.0f, { 125, 132, 142, 255 });
    }

    inline void DrawTurbineHallAndArchive(const WorldState& w, Vector3 camPos, Vector3 camFwd, float maxDist,
                                         TextureSys::PBRMaterialSet& concreteMat,
                                         TextureSys::PBRMaterialSet& steelMat,
                                         TextureSys::PBRMaterialSet& rustMat,
                                         TextureSys::PBRMaterialSet& gratingMat,
                                         TextureSys::PBRMaterialSet& copperMat) {
        Vector3 hallCenter = { 0.0f, -8.0f, -315.0f };
        if (!IsSphereVisible(hallCenter, 190.0f, camPos, camFwd, maxDist)) return;

        int lod = ComputeLOD(Vector3Distance(camPos, hallCenter));

        BindPBRToActiveMaterial(concreteMat.albedo, concreteMat.normalMap, concreteMat.ormMap, true, true, WHITE);
        DrawCube({ -26.0f, -1.0f, -315.0f }, 42.0f, 2.0f, 110.0f, { 118, 126, 138, 255 });
        DrawCube({  26.0f, -1.0f, -315.0f }, 42.0f, 2.0f, 110.0f, { 118, 126, 138, 255 });
        DrawCube({   0.0f, -1.0f, -274.0f }, 14.0f, 2.0f,  28.0f, { 118, 126, 138, 255 });
        DrawCube({   0.0f, -1.0f, -354.0f }, 14.0f, 2.0f,  32.0f, { 118, 126, 138, 255 });
        DrawCube({  16.0f, -22.0f, -315.0f }, 126.0f, 2.0f, 110.0f, { 96, 106, 120, 255 });

        DrawCube({ -47.0f, 14.0f, -315.0f }, 4.0f, 76.0f, 110.0f, { 105, 114, 126, 255 });
        DrawCube({  47.0f, 14.0f, -315.0f }, 4.0f, 76.0f, 110.0f, { 105, 114, 126, 255 });
        DrawCube({   0.0f, 14.0f, -370.0f }, 98.0f, 76.0f, 4.0f,   { 105, 114, 126, 255 });
        DrawCube({   0.0f, 51.0f, -315.0f }, 98.0f, 4.0f, 110.0f,  { 92, 100, 112, 255 });

        const float zTurbines[3] = { -290.0f, -315.0f, -340.0f };
        for (int i = 0; i < 3; i++) {
            float zT = zTurbines[i];
            BindPBRToActiveMaterial(steelMat.albedo, steelMat.normalMap, steelMat.ormMap, true, true, WHITE);
            DrawCylinder({ -24.0f, -11.0f, zT }, 8.5f, 8.5f, 22.0f, lod == 0 ? 28 : 16, { 110, 122, 138, 255 });

            rlPushMatrix();
            rlTranslatef(-24.0f, -2.0f, zT);
            rlRotatef(w.turbineAngle * ((i % 2 == 0) ? 1.0f : -1.0f), 0.0f, 1.0f, 0.0f);
            BindPBRToActiveMaterial(copperMat.albedo, copperMat.normalMap, copperMat.ormMap, true, true, WHITE);
            DrawCylinder({ 0.0f, 0.0f, 0.0f }, 6.8f, 6.8f, 1.8f, lod == 0 ? 24 : 14, { 185, 130, 82, 255 });
            for (int b = 0; b < 8; b++) {
                rlRotatef(45.0f, 0.0f, 1.0f, 0.0f);
                DrawCube({ 3.8f, 1.2f, 0.0f }, 6.5f, 2.2f, 0.35f, { 205, 150, 95, 255 });
            }
            rlPopMatrix();
        }

        BindPBRToActiveMaterial(gratingMat.albedo, gratingMat.normalMap, gratingMat.ormMap, true, true, WHITE);
        DrawCube({ 0.0f, -0.5f + w.elevatorY, -315.0f }, 9.0f, 0.8f, 9.0f, { 165, 175, 190, 255 });
        BindPBRToActiveMaterial(steelMat.albedo, steelMat.normalMap, steelMat.ormMap, true, true, WHITE);
        DrawCube({ -4.3f, 1.2f + w.elevatorY, -315.0f }, 0.25f, 2.6f, 9.0f, { 215, 165, 45, 255 });
        DrawCube({  4.3f, 1.2f + w.elevatorY, -315.0f }, 0.25f, 2.6f, 9.0f, { 215, 165, 45, 255 });

        BindPBRToActiveMaterial(concreteMat.albedo, concreteMat.normalMap, concreteMat.ormMap, true, true, WHITE);
        DrawCube({ 64.0f, -22.0f, -315.0f }, 36.0f, 2.0f, 24.0f, { 112, 120, 132, 255 });
        DrawCube({ 64.0f, -14.5f, -327.0f }, 36.0f, 15.0f, 2.0f, { 102, 110, 122, 255 });
        DrawCube({ 64.0f, -14.5f, -303.0f }, 36.0f, 15.0f, 2.0f, { 102, 110, 122, 255 });
        DrawCube({ 82.0f, -14.5f, -315.0f }, 2.0f, 15.0f, 24.0f, { 102, 110, 122, 255 });

        BindPBRToActiveMaterial(rustMat.albedo, rustMat.normalMap, rustMat.ormMap, true, true, WHITE);
        for (int r = 0; r < 4; r++) {
            float zRack = -322.0f + static_cast<float>(r) * 4.5f;
            DrawCube({ 74.0f, -17.5f, zRack }, 2.2f, 7.0f, 2.4f, { 130, 112, 104, 255 });
        }
    }
}

namespace UISys {

    enum ScreenState {
        SCREEN_MAIN_MENU = 0,
        SCREEN_PLAYING,
        SCREEN_PAUSED,
        SCREEN_SETTINGS,
        SCREEN_LEVEL_SELECT,
        SCREEN_CONTROLS,
        SCREEN_CREDITS
    };

    enum SettingsTab {
        TAB_GRAPHICS = 0,
        TAB_AUDIO,
        TAB_CONTROLS,
        TAB_GAME
    };

    struct UIContext {
        ScreenState screen = SCREEN_MAIN_MENU;
        ScreenState prevScreen = SCREEN_MAIN_MENU;
        SettingsTab settingsTab = TAB_GRAPHICS;
        int hoveredId = -1;
        int waitingKeyFor = -1;
        bool showCodex = false;
        int activeLogIndex = -1;
        std::string statusBanner;
        float statusTimer = 0.0f;
    };

    inline bool DrawAAAButton(Font font, Rectangle rect, const char* label, bool active,
                             int id, int& hoveredId, AudioSys::EngineAudio& audio, float uiVol) {
        Vector2 mp = GetMousePosition();
        bool hover = CheckCollisionPointRec(mp, rect);
        if (hover) {
            if (hoveredId != id && audio.initialized) {
                SetSoundVolume(audio.uiHover, Clamp(uiVol * 0.75f, 0.0f, 1.0f));
                PlaySound(audio.uiHover);
            }
            hoveredId = id;
        }

        Color fill = active ? Color{ 46, 86, 128, 225 } : (hover ? Color{ 36, 48, 64, 215 } : Color{ 16, 21, 29, 195 });
        Color border = active ? Color{ 120, 195, 255, 255 } : (hover ? Color{ 165, 195, 225, 240 } : Color{ 70, 84, 102, 160 });

        DrawRectangleRec(rect, fill);
        DrawRectangleLinesEx(rect, active ? 2.0f : 1.2f, border);
        if (hover || active) {
            DrawRectangle(static_cast<int>(rect.x), static_cast<int>(rect.y), 4, static_cast<int>(rect.height), { 110, 190, 255, 255 });
        }

        float fSize = 20.0f;
        Vector2 ts = MeasureTextEx(font, label, fSize, 1.0f);
        DrawTextEx(font, label, { rect.x + 18.0f, rect.y + (rect.height - ts.y) * 0.5f }, fSize, 1.0f,
                   (hover || active) ? WHITE : Color{ 212, 222, 235, 255 });

        if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (audio.initialized) {
                SetSoundVolume(audio.uiClick, Clamp(uiVol, 0.0f, 1.0f));
                PlaySound(audio.uiClick);
            }
            return true;
        }
        return false;
    }

    inline bool DrawAAASlider(Font font, Rectangle barRect, const char* title, float& value,
                              float minV, float maxV, const char* fmt, int id, int& hoveredId) {
        Vector2 mp = GetMousePosition();
        Rectangle hit = { barRect.x - 4.0f, barRect.y - 24.0f, barRect.width + 8.0f, barRect.height + 30.0f };
        bool hover = CheckCollisionPointRec(mp, hit);
        if (hover) hoveredId = id;

        bool changed = false;
        if (hover && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float t = Clamp((mp.x - barRect.x) / std::max(barRect.width, 1.0f), 0.0f, 1.0f);
            float nv = Lerp(minV, maxV, t);
            if (fabsf(nv - value) > 1e-4f) {
                value = nv;
                changed = true;
            }
        }

        DrawTextEx(font, title, { barRect.x, barRect.y - 22.0f }, 17.0f, 0.8f, { 205, 216, 230, 255 });
        const char* valStr = TextFormat(fmt, value);
        Vector2 vs = MeasureTextEx(font, valStr, 17.0f, 0.8f);
        DrawTextEx(font, valStr, { barRect.x + barRect.width - vs.x, barRect.y - 22.0f }, 17.0f, 0.8f, { 130, 200, 255, 255 });

        DrawRectangleRec(barRect, { 22, 28, 36, 230 });
        float norm = Clamp((value - minV) / std::max(maxV - minV, 1e-5f), 0.0f, 1.0f);
        DrawRectangle(static_cast<int>(barRect.x), static_cast<int>(barRect.y),
                      static_cast<int>(barRect.width * norm), static_cast<int>(barRect.height), { 75, 148, 215, 245 });
        DrawRectangleLinesEx(barRect, 1.0f, hover ? Color{ 150, 210, 255, 255 } : Color{ 75, 90, 110, 180 });
        return changed;
    }

    inline void DrawMainMenu(Font font, int sw, int sh, UIContext& ui,
                             ConfigSys::SaveProgress& save, ConfigSys::GameSettings& cfg,
                             AudioSys::EngineAudio& audio, bool& startNewGame, bool& continueGame, bool& exitGame) {
        ui.hoveredId = -1;
        DrawRectangle(0, 0, sw, sh, { 6, 9, 14, 115 });
        DrawRectangleGradientH(0, 0, static_cast<int>(static_cast<float>(sw) * 0.48f), sh, { 8, 12, 18, 232 }, { 8, 12, 18, 40 });

        int lang = cfg.language;
        DrawTextEx(font, "MEGALOPHOBIA 3D", { 56.0f, 64.0f }, 44.0f, 2.0f, WHITE);
        DrawTextEx(font, TextUtil::Tr(lang, "АРХИТЕКТУРА ТИШИНЫ // AAA RTX EDITION", "ARCHITECTURE OF SILENCE // AAA RTX EDITION"),
                   { 58.0f, 114.0f }, 18.0f, 1.0f, { 125, 188, 245, 255 });

        const char* labels[7] = {
            TextUtil::Tr(lang, "НОВАЯ ИГРА", "NEW GAME"),
            TextUtil::Tr(lang, "ПРОДОЛЖИТЬ", "CONTINUE"),
            TextUtil::Tr(lang, "ВЫБОР УРОВНЯ", "LEVEL SELECT"),
            TextUtil::Tr(lang, "НАСТРОЙКИ", "SETTINGS"),
            TextUtil::Tr(lang, "УПРАВЛЕНИЕ", "CONTROLS"),
            TextUtil::Tr(lang, "АВТОРЫ", "CREDITS"),
            TextUtil::Tr(lang, "ВЫХОД", "EXIT")
        };

        float bx = 58.0f, by = 175.0f, bw = 310.0f, bh = 46.0f, gap = 10.0f;
        for (int i = 0; i < 7; i++) {
            float ry = by + static_cast<float>(i) * (bh + gap);
            if (i == 1 && !save.hasSave) {
                DrawRectangleRec({ bx, ry, bw, bh }, { 18, 22, 28, 130 });
                DrawRectangleLinesEx({ bx, ry, bw, bh }, 1.0f, { 55, 65, 78, 120 });
                DrawTextEx(font, labels[i], { bx + 18.0f, ry + 13.0f }, 20.0f, 1.0f, { 95, 106, 120, 160 });
                continue;
            }
            if (DrawAAAButton(font, { bx, ry, bw, bh }, labels[i], false, 10 + i, ui.hoveredId, audio, cfg.uiVolume)) {
                if (i == 0) startNewGame = true;
                else if (i == 1) continueGame = true;
                else if (i == 2) { ui.prevScreen = SCREEN_MAIN_MENU; ui.screen = SCREEN_LEVEL_SELECT; }
                else if (i == 3) { ui.prevScreen = SCREEN_MAIN_MENU; ui.screen = SCREEN_SETTINGS; }
                else if (i == 4) { ui.prevScreen = SCREEN_MAIN_MENU; ui.screen = SCREEN_CONTROLS; }
                else if (i == 5) { ui.prevScreen = SCREEN_MAIN_MENU; ui.screen = SCREEN_CREDITS; }
                else if (i == 6) exitGame = true;
            }
        }

        DrawTextEx(font, "C++17 // RAYLIB 5.x // GLSL 330 PBR + ACES", { 58.0f, static_cast<float>(sh) - 36.0f }, 15.0f, 0.8f, { 130, 148, 170, 190 });
    }

    inline void DrawSettingsScreen(Font font, int sw, int sh, UIContext& ui,
                                   ConfigSys::GameSettings& cfg, AudioSys::EngineAudio& audio,
                                   bool& graphicsDirty, bool& applyNow) {
        ui.hoveredId = -1;
        int lang = cfg.language;
        DrawRectangle(0, 0, sw, sh, { 9, 13, 20, 242 });

        DrawTextEx(font, TextUtil::Tr(lang, "НАСТРОЙКИ СИСТЕМЫ", "SYSTEM SETTINGS"), { 48.0f, 34.0f }, 32.0f, 1.5f, WHITE);

        const char* tabs[4] = {
            TextUtil::Tr(lang, "ГРАФИКА", "GRAPHICS"),
            TextUtil::Tr(lang, "ЗВУК", "AUDIO"),
            TextUtil::Tr(lang, "УПРАВЛЕНИЕ", "CONTROLS"),
            TextUtil::Tr(lang, "ИГРА", "GAMEPLAY")
        };

        for (int t = 0; t < 4; t++) {
            float tx = 48.0f + static_cast<float>(t) * 205.0f;
            if (DrawAAAButton(font, { tx, 82.0f, 192.0f, 40.0f }, tabs[t],
                              static_cast<int>(ui.settingsTab) == t, 50 + t, ui.hoveredId, audio, cfg.uiVolume)) {
                ui.settingsTab = static_cast<SettingsTab>(t);
            }
        }

        Rectangle panel = { 48.0f, 138.0f, static_cast<float>(sw) - 96.0f, static_cast<float>(sh) - 215.0f };
        DrawRectangleRec(panel, { 15, 20, 29, 235 });
        DrawRectangleLinesEx(panel, 1.2f, { 68, 84, 106, 180 });

        float lx = panel.x + 28.0f;
        float ly = panel.y + 34.0f;
        float rx = panel.x + panel.width * 0.52f;

        if (ui.settingsTab == TAB_GRAPHICS) {
            const char* presets[4] = {
                TextUtil::Tr(lang, "НИЗКИЕ", "LOW"),
                TextUtil::Tr(lang, "СРЕДНИЕ", "MEDIUM"),
                TextUtil::Tr(lang, "ВЫСОКИЕ", "HIGH"),
                TextUtil::Tr(lang, "УЛЬТРА (RTX)", "ULTRA (RTX)")
            };
            DrawTextEx(font, TextUtil::Tr(lang, "ПРЕСЕТ ГРАФИКИ:", "QUALITY PRESET:"), { lx, ly - 24.0f }, 17.0f, 0.8f, { 205, 216, 230, 255 });
            for (int p = 0; p < 4; p++) {
                float px = lx + static_cast<float>(p) * 145.0f;
                if (DrawAAAButton(font, { px, ly, 136.0f, 36.0f }, presets[p],
                                  cfg.preset == p, 70 + p, ui.hoveredId, audio, cfg.uiVolume)) {
                    ConfigSys::ApplyPreset(cfg, p);
                    graphicsDirty = true;
                }
            }

            const char* resNames[3] = { "1280 x 720", "1600 x 900", "1920 x 1080" };
            int rIdx = Clamp(cfg.resIndex, 0, 2);
            if (DrawAAAButton(font, { lx, ly + 56.0f, 360.0f, 36.0f },
                              TextFormat("%s: %s", TextUtil::Tr(lang, "РАЗРЕШЕНИЕ", "RESOLUTION"), resNames[rIdx]),
                              false, 75, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.resIndex = (cfg.resIndex + 1) % 3;
                graphicsDirty = true;
            }
            if (DrawAAAButton(font, { lx, ly + 102.0f, 360.0f, 36.0f },
                              TextFormat("%s: %s", TextUtil::Tr(lang, "РЕЖИМ ЭКРАНА", "DISPLAY MODE"),
                                         cfg.fullscreen ? TextUtil::Tr(lang, "ПОЛНОЭКРАННЫЙ", "FULLSCREEN") : TextUtil::Tr(lang, "ОКОННЫЙ", "WINDOWED")),
                              cfg.fullscreen, 76, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.fullscreen = !cfg.fullscreen;
                graphicsDirty = true;
            }
            if (DrawAAAButton(font, { lx, ly + 148.0f, 360.0f, 36.0f },
                              TextFormat("V-SYNC: %s", cfg.vsync ? "ON" : "OFF"),
                              cfg.vsync, 77, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.vsync = !cfg.vsync;
                graphicsDirty = true;
            }
            if (DrawAAAButton(font, { lx, ly + 194.0f, 360.0f, 36.0f },
                              TextFormat("%s: %s", TextUtil::Tr(lang, "ЛИМИТ FPS", "FPS CAP"),
                                         cfg.fpsLimit == 0 ? "UNLIMITED" : TextFormat("%d FPS", cfg.fpsLimit)),
                              false, 78, ui.hoveredId, audio, cfg.uiVolume)) {
                const int caps[4] = { 60, 120, 144, 0 };
                int idx = 0;
                for (int k = 0; k < 4; k++) if (caps[k] == cfg.fpsLimit) idx = (k + 1) % 4;
                cfg.fpsLimit = caps[idx];
                graphicsDirty = true;
            }

            if (DrawAAASlider(font, { rx, ly + 22.0f, 360.0f, 16.0f },
                              TextUtil::Tr(lang, "ДАЛЬНОСТЬ ПРОРИСОВКИ", "DRAW DISTANCE"), cfg.drawDistance, 600.0f, 1500.0f, "%.0f m", 80, ui.hoveredId)) {
                graphicsDirty = true;
            }
            if (DrawAAAButton(font, { rx, ly + 66.0f, 360.0f, 36.0f },
                              TextFormat("SSAO / BLOOM / HDR: %s", (cfg.ssaoEnabled && cfg.bloomEnabled && cfg.hdrEnabled) ? "ON" : "CUSTOM"),
                              cfg.bloomEnabled, 81, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.ssaoEnabled = !cfg.ssaoEnabled;
                cfg.bloomEnabled = cfg.ssaoEnabled;
                cfg.hdrEnabled = cfg.ssaoEnabled;
            }
            if (DrawAAAButton(font, { rx, ly + 112.0f, 360.0f, 36.0f },
                              TextFormat("%s: %d", TextUtil::Tr(lang, "КАЧЕСТВО ТЕКСТУР", "TEXTURE QUALITY"), cfg.texQuality),
                              false, 82, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.texQuality = (cfg.texQuality + 1) % 4;
                graphicsDirty = true;
            }
            if (DrawAAAButton(font, { rx, ly + 158.0f, 360.0f, 36.0f },
                              TextFormat("%s: %d", TextUtil::Tr(lang, "КАЧЕСТВО ТЕНЕЙ", "SHADOW QUALITY"), cfg.shadowQuality),
                              false, 83, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.shadowQuality = (cfg.shadowQuality + 1) % 4;
                graphicsDirty = true;
            }
            if (DrawAAAButton(font, { rx, ly + 204.0f, 360.0f, 36.0f },
                              TextFormat("%s: %d", TextUtil::Tr(lang, "ОТРАЖЕНИЯ / ТУМАН", "REFLECTIONS / FOG"), cfg.reflectionQuality),
                              false, 84, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.reflectionQuality = (cfg.reflectionQuality + 1) % 4;
                cfg.fogQuality = (cfg.reflectionQuality >= 2) ? 2 : 1;
            }
        } else if (ui.settingsTab == TAB_AUDIO) {
            DrawAAASlider(font, { lx, ly + 22.0f, 380.0f, 16.0f }, TextUtil::Tr(lang, "ОБЩАЯ ГРОМКОСТЬ", "MASTER VOLUME"), cfg.masterVolume, 0.0f, 1.0f, "%.0f%%", 90, ui.hoveredId);
            DrawAAASlider(font, { lx, ly + 86.0f, 380.0f, 16.0f }, TextUtil::Tr(lang, "МУЗЫКА", "MUSIC VOLUME"), cfg.musicVolume, 0.0f, 1.0f, "%.0f%%", 91, ui.hoveredId);
            DrawAAASlider(font, { lx, ly + 150.0f, 380.0f, 16.0f }, TextUtil::Tr(lang, "ЗВУКОВЫЕ ЭФФЕКТЫ", "SFX VOLUME"), cfg.sfxVolume, 0.0f, 1.0f, "%.0f%%", 92, ui.hoveredId);
            DrawAAASlider(font, { rx, ly + 22.0f, 380.0f, 16.0f }, TextUtil::Tr(lang, "ОКРУЖЕНИЕ", "AMBIENT VOLUME"), cfg.ambientVolume, 0.0f, 1.0f, "%.0f%%", 93, ui.hoveredId);
            DrawAAASlider(font, { rx, ly + 86.0f, 380.0f, 16.0f }, TextUtil::Tr(lang, "ИНТЕРФЕЙС", "UI VOLUME"), cfg.uiVolume, 0.0f, 1.0f, "%.0f%%", 94, ui.hoveredId);
            AudioSys::ApplyVolumes(audio, cfg, 0.0f, false);
        } else if (ui.settingsTab == TAB_CONTROLS) {
            DrawAAASlider(font, { lx, ly + 22.0f, 360.0f, 16.0f }, TextUtil::Tr(lang, "ЧУВСТВИТЕЛЬНОСТЬ МЫШИ", "MOUSE SENSITIVITY"), cfg.mouseSensitivity, 0.2f, 2.5f, "%.2fx", 100, ui.hoveredId);
            if (DrawAAAButton(font, { lx, ly + 72.0f, 360.0f, 36.0f },
                              TextFormat("%s: %s", TextUtil::Tr(lang, "ИНВЕРСИЯ Y", "INVERT Y"), cfg.invertY ? "ON" : "OFF"),
                              cfg.invertY, 101, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.invertY = !cfg.invertY;
            }

            struct KeySlot { int idx; const char* name; int* keyPtr; float x; float y; };
            KeySlot slots[6] = {
                { 0, TextUtil::Tr(lang, "ВПЕРЁД", "FORWARD"), &cfg.keyForward, rx, ly + 0.0f },
                { 1, TextUtil::Tr(lang, "НАЗАД", "BACKWARD"), &cfg.keyBackward, rx, ly + 44.0f },
                { 2, TextUtil::Tr(lang, "СПРИНТ", "SPRINT"), &cfg.keySprint, rx, ly + 88.0f },
                { 3, TextUtil::Tr(lang, "ДЕЙСТВИЕ", "INTERACT"), &cfg.keyInteract, rx, ly + 132.0f },
                { 4, TextUtil::Tr(lang, "ФОНАРЬ", "FLASHLIGHT"), &cfg.keyFlashlight, rx, ly + 176.0f },
                { 5, TextUtil::Tr(lang, "ЗУМ", "ZOOM"), &cfg.keyZoom, rx, ly + 220.0f }
            };
            for (auto& ks : slots) {
                bool waiting = (ui.waitingKeyFor == ks.idx);
                const char* txt = waiting ? TextUtil::Tr(lang, "НАЖМИТЕ КЛАВИШУ...", "PRESS ANY KEY...")
                                          : TextFormat("%s: [%s]", ks.name, ConfigSys::KeyToName(*ks.keyPtr));
                if (DrawAAAButton(font, { ks.x, ks.y, 360.0f, 36.0f }, txt, waiting, 110 + ks.idx, ui.hoveredId, audio, cfg.uiVolume)) {
                    ui.waitingKeyFor = waiting ? -1 : ks.idx;
                }
            }
            if (ui.waitingKeyFor >= 0) {
                int pressed = GetKeyPressed();
                if (pressed != 0 && pressed != KEY_ESCAPE) {
                    if (ui.waitingKeyFor == 0) cfg.keyForward = pressed;
                    else if (ui.waitingKeyFor == 1) cfg.keyBackward = pressed;
                    else if (ui.waitingKeyFor == 2) cfg.keySprint = pressed;
                    else if (ui.waitingKeyFor == 3) cfg.keyInteract = pressed;
                    else if (ui.waitingKeyFor == 4) cfg.keyFlashlight = pressed;
                    else if (ui.waitingKeyFor == 5) cfg.keyZoom = pressed;
                    ui.waitingKeyFor = -1;
                }
            }
        } else {
            DrawAAASlider(font, { lx, ly + 22.0f, 360.0f, 16.0f }, "FOV", cfg.fov, 55.0f, 95.0f, "%.0f deg", 120, ui.hoveredId);
            DrawAAASlider(font, { lx, ly + 86.0f, 360.0f, 16.0f }, TextUtil::Tr(lang, "ЯРКОСТЬ", "BRIGHTNESS"), cfg.brightness, 0.70f, 1.40f, "%.2f", 121, ui.hoveredId);
            DrawAAASlider(font, { lx, ly + 150.0f, 360.0f, 16.0f }, TextUtil::Tr(lang, "КОНТРАСТНОСТЬ", "CONTRAST"), cfg.contrast, 0.80f, 1.35f, "%.2f", 122, ui.hoveredId);

            if (DrawAAAButton(font, { rx, ly + 2.0f, 360.0f, 38.0f },
                              cfg.language == 0 ? "LANGUAGE: РУССКИЙ" : "LANGUAGE: ENGLISH",
                              false, 123, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.language = (cfg.language + 1) % 2;
            }
            if (DrawAAAButton(font, { rx, ly + 52.0f, 360.0f, 38.0f },
                              TextFormat("%s: %s", TextUtil::Tr(lang, "СУБТИТРЫ", "SUBTITLES"), cfg.subtitles ? "ON" : "OFF"),
                              cfg.subtitles, 124, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.subtitles = !cfg.subtitles;
            }
            const char* diffNames[3] = {
                TextUtil::Tr(lang, "СОЗЕРЦАНИЕ", "CONTEMPLATION"),
                TextUtil::Tr(lang, "СТАНДАРТ", "STANDARD"),
                TextUtil::Tr(lang, "АНОМАЛЬНЫЙ ШТОРМ", "ANOMALY STORM")
            };
            int dIdx = Clamp(cfg.difficulty, 0, 2);
            if (DrawAAAButton(font, { rx, ly + 102.0f, 360.0f, 38.0f },
                              TextFormat("%s: %s", TextUtil::Tr(lang, "СЛОЖНОСТЬ", "DIFFICULTY"), diffNames[dIdx]),
                              false, 125, ui.hoveredId, audio, cfg.uiVolume)) {
                cfg.difficulty = (cfg.difficulty + 1) % 3;
            }
        }

        float by = static_cast<float>(sh) - 62.0f;
        if (DrawAAAButton(font, { 48.0f, by, 230.0f, 42.0f },
                          TextUtil::Tr(lang, "ПО УМОЛЧАНИЮ", "RESET DEFAULTS"), false, 130, ui.hoveredId, audio, cfg.uiVolume)) {
            ConfigSys::SetDefaults(cfg);
            graphicsDirty = true;
            applyNow = true;
        }
        if (DrawAAAButton(font, { 295.0f, by, 230.0f, 42.0f },
                          TextUtil::Tr(lang, "СОХРАНИТЬ И НАЗАД", "SAVE & BACK"), true, 131, ui.hoveredId, audio, cfg.uiVolume)) {
            ConfigSys::SaveSettings(cfg);
            applyNow = true;
            ui.waitingKeyFor = -1;
            ui.screen = ui.prevScreen;
        }
    }
}

namespace GameApp {

    struct PlayerState {
        Vector3 pos = { 0.0f, 1.8f, 19.0f };
        Vector3 vel = { 0.0f, 0.0f, 0.0f };
        float yaw = 0.0f;
        float pitch = 0.0f;
        float stamina = 1.0f;
        bool flashlightOn = true;
        float battery = 1.0f;
        bool zoomed = false;
        float anomalyFactor = 0.0f;
        float stepTimer = 0.0f;
        float ambientTimer = 0.0f;
    };

    inline Vector3 CameraForward(const PlayerState& p) {
        float cp = cosf(p.pitch);
        return Vector3Normalize({
            sinf(p.yaw) * cp,
            sinf(p.pitch),
            -cosf(p.yaw) * cp
        });
    }

    inline void ApplyDisplayAndGraphics(const ConfigSys::GameSettings& cfg,
                                        ShaderSys::ShaderBundle& shaders,
                                        TextureSys::MaterialBank& mats,
                                        bool recreateTextures) {
        const int widths[3]  = { 1280, 1600, 1920 };
        const int heights[3] = { 720,  900,  1080 };
        int rIdx = Clamp(cfg.resIndex, 0, 2);
        int tw = widths[rIdx];
        int th = heights[rIdx];

        if (!IsWindowFullscreen() && (GetScreenWidth() != tw || GetScreenHeight() != th)) {
            SetWindowSize(tw, th);
        }
        if (IsWindowFullscreen() != cfg.fullscreen) {
            ToggleFullscreen();
        }
        if (cfg.vsync) SetWindowState(FLAG_VSYNC_HINT);
        else ClearWindowState(FLAG_VSYNC_HINT);

        SetTargetFPS(cfg.fpsLimit > 0 ? cfg.fpsLimit : 0);

        int wNow = std::max(GetScreenWidth(), 640);
        int hNow = std::max(GetScreenHeight(), 360);
        int sRes = ShaderSys::ShadowSizeFromQuality(cfg.shadowQuality);
        if (shaders.valid && (shaders.sceneW != wNow || shaders.sceneH != hNow || shaders.shadowRes != sRes)) {
            ShaderSys::CreateSceneTargets(shaders, wNow, hNow, sRes);
        }
        if (recreateTextures && mats.loaded) {
            TextureSys::UnloadMaterials(mats);
            mats = TextureSys::CreateMaterials(cfg.texQuality);
        }
    }

    inline void DrawOverlayScreen(Font font, int sw, int sh, UISys::UIContext& ui,
                                  WorldSys::WorldState& world, PlayerState& player,
                                  ConfigSys::SaveProgress& save, ConfigSys::GameSettings& cfg,
                                  AudioSys::EngineAudio& audio, bool& requestMainMenu, bool& resumeGame) {
        ui.hoveredId = -1;
        int lang = cfg.language;
        DrawRectangle(0, 0, sw, sh, { 7, 11, 17, 215 });

        if (ui.screen == UISys::SCREEN_PAUSED) {
            DrawTextEx(font, TextUtil::Tr(lang, "ПАУЗА", "PAUSED"), { 56.0f, 54.0f }, 36.0f, 1.5f, WHITE);
            float bx = 56.0f, by = 126.0f, bw = 310.0f, bh = 44.0f, gap = 12.0f;

            if (UISys::DrawAAAButton(font, { bx, by, bw, bh }, TextUtil::Tr(lang, "ПРОДОЛЖИТЬ", "RESUME"), true, 201, ui.hoveredId, audio, cfg.uiVolume)) {
                resumeGame = true;
            }
            if (UISys::DrawAAAButton(font, { bx, by + (bh + gap), bw, bh }, TextUtil::Tr(lang, "СОХРАНИТЬ ПРОГРЕСС", "SAVE PROGRESS"), false, 202, ui.hoveredId, audio, cfg.uiVolume)) {
                save.hasSave = true;
                save.sectorId = world.activeSector;
                save.posX = player.pos.x;
                save.posY = player.pos.y;
                save.posZ = player.pos.z;
                save.yaw = player.yaw;
                save.pitch = player.pitch;
                save.anomalyFactor = player.anomalyFactor;
                ConfigSys::SaveProgressFile(save);
                ui.statusBanner = TextUtil::Tr(lang, "ПРОГРЕСС СОХРАНЁН", "PROGRESS SAVED");
                ui.statusTimer = 2.2f;
            }
            if (UISys::DrawAAAButton(font, { bx, by + 2.0f * (bh + gap), bw, bh }, TextUtil::Tr(lang, "НАСТРОЙКИ", "SETTINGS"), false, 203, ui.hoveredId, audio, cfg.uiVolume)) {
                ui.prevScreen = UISys::SCREEN_PAUSED;
                ui.screen = UISys::SCREEN_SETTINGS;
            }
            if (UISys::DrawAAAButton(font, { bx, by + 3.0f * (bh + gap), bw, bh }, TextUtil::Tr(lang, "УПРАВЛЕНИЕ", "CONTROLS"), false, 204, ui.hoveredId, audio, cfg.uiVolume)) {
                ui.prevScreen = UISys::SCREEN_PAUSED;
                ui.screen = UISys::SCREEN_CONTROLS;
            }
            if (UISys::DrawAAAButton(font, { bx, by + 4.0f * (bh + gap), bw, bh }, TextUtil::Tr(lang, "В ГЛАВНОЕ МЕНЮ", "MAIN MENU"), false, 205, ui.hoveredId, audio, cfg.uiVolume)) {
                requestMainMenu = true;
            }
        } else if (ui.screen == UISys::SCREEN_LEVEL_SELECT) {
            DrawTextEx(font, TextUtil::Tr(lang, "ВЫБОР СЕКТОРА", "SECTOR SELECT"), { 56.0f, 48.0f }, 34.0f, 1.5f, WHITE);
            for (size_t i = 0; i < world.sectors.size(); i++) {
                const auto& sec = world.sectors[i];
                const char* sName = TextUtil::Tr(lang, sec.nameRu, sec.nameEn);
                float sy = 114.0f + static_cast<float>(i) * 56.0f;
                if (UISys::DrawAAAButton(font, { 56.0f, sy, 440.0f, 44.0f }, sName,
                                     world.activeSector == sec.id, 220 + static_cast<int>(i), ui.hoveredId, audio, cfg.uiVolume)) {
                    player.pos = sec.anchorPos;
                    player.vel = { 0.0f, 0.0f, 0.0f };
                    world.activeSector = sec.id;
                    ui.screen = UISys::SCREEN_PLAYING;
                    DisableCursor();
                }
            }
            if (UISys::DrawAAAButton(font, { 56.0f, static_cast<float>(sh) - 80.0f, 220.0f, 42.0f },
                                 TextUtil::Tr(lang, "НАЗАД", "BACK"), false, 229, ui.hoveredId, audio, cfg.uiVolume)) {
                ui.screen = ui.prevScreen;
            }
        } else if (ui.screen == UISys::SCREEN_CONTROLS) {
            DrawTextEx(font, TextUtil::Tr(lang, "УПРАВЛЕНИЕ И ИНТЕРАКТИВ", "CONTROLS & INTERACTION"), { 56.0f, 48.0f }, 34.0f, 1.5f, WHITE);
            const char* linesRu[8] = {
                "WASD / Стрелки — перемещение в пространстве",
                "Мышь — обзор (ПКМ / Z — оптический зум)",
                "Shift — ускорение (расходует выносливость)",
                "F — переключение налобного фонаря",
                "E — взаимодействие (лифт, терминалы и журналы)",
                "TAB — кодекс найденных записей",
                "R — мгновенный возврат в текущий сектор",
                "ESC — меню паузы / возврат назад"
            };
            const char* linesEn[8] = {
                "WASD / Arrows — move through the facility",
                "Mouse — look around (RMB / Z — optical zoom)",
                "Shift — sprint (consumes stamina)",
                "F — toggle headlamp",
                "E — interact (freight lift, terminals, logs)",
                "TAB — open collected archive codex",
                "R — reset to current sector anchor",
                "ESC — pause menu / navigate back"
            };
            for (int i = 0; i < 8; i++) {
                DrawTextEx(font, TextUtil::Tr(lang, linesRu[i], linesEn[i]),
                           { 56.0f, 112.0f + static_cast<float>(i) * 38.0f }, 20.0f, 0.8f, { 216, 226, 238, 255 });
            }
            if (UISys::DrawAAAButton(font, { 56.0f, static_cast<float>(sh) - 80.0f, 220.0f, 42.0f },
                                 TextUtil::Tr(lang, "НАЗАД", "BACK"), false, 240, ui.hoveredId, audio, cfg.uiVolume)) {
                ui.screen = ui.prevScreen;
            }
        } else if (ui.screen == UISys::SCREEN_CREDITS) {
            DrawTextEx(font, TextUtil::Tr(lang, "АВТОРЫ И ТЕХНОЛОГИИ", "CREDITS & TECH"), { 56.0f, 48.0f }, 34.0f, 1.5f, WHITE);
            const char* credRu[5] = {
                "Проект: MEGALOPHOBIA 3D — «Архитектура Тишины»",
                "Движок: C++17 + Raylib 5.x + RLGL",
                "Рендеринг: GLSL 330 Core PBR (GGX, Normal/ORM, PCF Shadows, ACES HDR, SSAO, God Rays)",
                "Аудио и текстуры: процедурный синтез волн и карт рельефа в реальном времени",
                "Дизайн мегаструктур: шлюз, мост, реактор «Гелиос-9», турбинный зал и бункер-архив"
            };
            const char* credEn[5] = {
                "Project: MEGALOPHOBIA 3D — Architecture of Silence",
                "Engine: C++17 + Raylib 5.x + RLGL",
                "Rendering: GLSL 330 Core PBR (GGX, Normal/ORM, PCF Shadows, ACES HDR, SSAO, God Rays)",
                "Audio & Textures: real-time procedural waveform and surface map synthesis",
                "Megastructure Design: airlock, chasm bridge, Helios-9 core, turbine hall & archive bunker"
            };
            for (int i = 0; i < 5; i++) {
                DrawTextEx(font, TextUtil::Tr(lang, credRu[i], credEn[i]),
                           { 56.0f, 118.0f + static_cast<float>(i) * 44.0f }, 20.0f, 0.8f, { 216, 226, 238, 255 });
            }
            if (UISys::DrawAAAButton(font, { 56.0f, static_cast<float>(sh) - 80.0f, 220.0f, 42.0f },
                                 TextUtil::Tr(lang, "НАЗАД", "BACK"), false, 250, ui.hoveredId, audio, cfg.uiVolume)) {
                ui.screen = ui.prevScreen;
            }
        }
    }

    inline void DrawHUD(Font font, int sw, int sh, const UISys::UIContext& ui,
                        const WorldSys::WorldState& world, const PlayerState& player,
                        const ConfigSys::GameSettings& cfg) {
        int lang = cfg.language;
        int secIdx = Clamp(world.activeSector, 0, static_cast<int>(world.sectors.size()) - 1);
        const auto& sec = world.sectors[static_cast<size_t>(secIdx)];

        DrawRectangle(20, 18, 380, 56, { 12, 17, 25, 165 });
        DrawRectangleLines(20, 18, 380, 56, { 75, 92, 115, 150 });
        DrawTextEx(font, TextUtil::Tr(lang, sec.nameRu, sec.nameEn), { 32.0f, 24.0f }, 18.0f, 0.8f, WHITE);
        DrawTextEx(font, TextFormat("POS: %.1f, %.1f, %.1f  |  ANOMALY: %.0f%%",
                                   player.pos.x, player.pos.y, player.pos.z, player.anomalyFactor * 100.0f),
                   { 32.0f, 48.0f }, 14.0f, 0.6f, { 145, 185, 225, 220 });

        float fsh = static_cast<float>(sh);
        DrawRectangle(20, sh - 64, 210, 10, { 20, 26, 35, 190 });
        DrawRectangle(20, sh - 64, static_cast<int>(210.0f * player.stamina), 10, { 110, 195, 255, 235 });
        DrawTextEx(font, TextUtil::Tr(lang, "ВЫНОСЛИВОСТЬ", "STAMINA"), { 20.0f, fsh - 82.0f }, 13.0f, 0.6f, { 195, 208, 224, 210 });

        DrawRectangle(20, sh - 36, 210, 10, { 20, 26, 35, 190 });
        DrawRectangle(20, sh - 36, static_cast<int>(210.0f * player.battery), 10,
                      player.flashlightOn ? Color{ 245, 205, 115, 235 } : Color{ 105, 115, 130, 180 });
        DrawTextEx(font, TextFormat("%s [%s]", TextUtil::Tr(lang, "ФОНАРЬ", "LIGHT"), player.flashlightOn ? "ON" : "OFF"),
                   { 20.0f, fsh - 53.0f }, 13.0f, 0.6f, { 195, 208, 224, 210 });

        int cx = sw / 2;
        int cy = sh / 2;
        float fcx = static_cast<float>(cx);
        float fcy = static_cast<float>(cy);
        DrawCircle(cx, cy, player.zoomed ? 2.0f : 3.0f, { 225, 235, 245, 185 });

        for (const auto& log : world.logs) {
            if (Vector3Distance(player.pos, log.pos) < 4.5f) {
                const char* hint = TextUtil::Tr(lang, "[E] ПРОЧИТАТЬ ЗАПИСЬ", "[E] READ LOG");
                Vector2 hs = MeasureTextEx(font, hint, 18.0f, 0.8f);
                DrawRectangle(cx - static_cast<int>(hs.x * 0.5f) - 14, cy + 28, static_cast<int>(hs.x) + 28, 34, { 14, 20, 28, 210 });
                DrawTextEx(font, hint, { fcx - hs.x * 0.5f, fcy + 35.0f }, 18.0f, 0.8f, { 140, 215, 255, 255 });
                break;
            }
        }
        if (Vector3Distance(player.pos, { 0.0f, 1.8f + world.elevatorY, -315.0f }) < 5.5f) {
            const char* lHint = TextUtil::Tr(lang, "[E] УПРАВЛЕНИЕ ЛИФТОМ", "[E] OPERATE LIFT");
            Vector2 ls = MeasureTextEx(font, lHint, 18.0f, 0.8f);
            DrawRectangle(cx - static_cast<int>(ls.x * 0.5f) - 14, cy + 68, static_cast<int>(ls.x) + 28, 34, { 14, 20, 28, 210 });
            DrawTextEx(font, lHint, { fcx - ls.x * 0.5f, fcy + 75.0f }, 18.0f, 0.8f, { 245, 190, 95, 255 });
        }

        if (ui.showCodex && ui.activeLogIndex >= 0 && ui.activeLogIndex < static_cast<int>(world.logs.size())) {
            const auto& entry = world.logs[static_cast<size_t>(ui.activeLogIndex)];
            float fsw = static_cast<float>(sw);
            Rectangle box = { fsw * 0.16f, fsh * 0.22f, fsw * 0.68f, fsh * 0.52f };
            DrawRectangleRec(box, { 11, 16, 24, 238 });
            DrawRectangleLinesEx(box, 1.5f, { 115, 180, 235, 230 });
            DrawTextEx(font, TextUtil::Tr(lang, entry.titleRu, entry.titleEn), { box.x + 28.0f, box.y + 24.0f }, 24.0f, 1.0f, { 135, 205, 255, 255 });
            DrawTextEx(font, TextUtil::Tr(lang, entry.textRu, entry.textEn), { box.x + 28.0f, box.y + 72.0f }, 20.0f, 0.8f, WHITE);
            DrawTextEx(font, TextUtil::Tr(lang, "Нажмите [E] или [TAB], чтобы закрыть", "Press [E] or [TAB] to close"),
                       { box.x + 28.0f, box.y + box.height - 42.0f }, 16.0f, 0.7f, { 165, 180, 198, 210 });
        }

        if (ui.statusTimer > 0.0f && !ui.statusBanner.empty()) {
            Vector2 bs = MeasureTextEx(font, ui.statusBanner.c_str(), 18.0f, 0.8f);
            DrawRectangle(static_cast<int>(static_cast<float>(sw) - bs.x - 44.0f), 20, static_cast<int>(bs.x) + 24, 36, { 24, 48, 72, 220 });
            DrawTextEx(font, ui.statusBanner.c_str(), { static_cast<float>(sw) - bs.x - 32.0f, 28.0f }, 18.0f, 0.8f, WHITE);
        }
    }

} // namespace GameApp

int main() {
    ConfigSys::GameSettings cfg{};
    ConfigSys::SetDefaults(cfg);
    ConfigSys::LoadSettings(cfg);

    ConfigSys::SaveProgress save{};
    ConfigSys::LoadProgressFile(save);

    const int widths[3]  = { 1280, 1600, 1920 };
    const int heights[3] = { 720,  900,  1080 };
    int initIdx = Clamp(cfg.resIndex, 0, 2);
    int initW = widths[initIdx];
    int initH = heights[initIdx];

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | (cfg.vsync ? FLAG_VSYNC_HINT : 0));
    InitWindow(initW, initH, "MEGALOPHOBIA 3D — Architecture of Silence");
    SetTargetFPS(cfg.fpsLimit > 0 ? cfg.fpsLimit : 0);
    if (cfg.fullscreen) ToggleFullscreen();

    Font uiFont = TextUtil::LoadRussianFont(24);
    AudioSys::EngineAudio audio = AudioSys::InitEngineAudio();
    TextureSys::MaterialBank mats = TextureSys::CreateMaterials(cfg.texQuality);
    ShaderSys::ShaderBundle shaders = ShaderSys::InitShaders(GetScreenWidth(), GetScreenHeight(), cfg.shadowQuality);
    WorldSys::WorldState world = WorldSys::CreateWorld();
    UISys::UIContext ui{};
    GameApp::PlayerState player{};

    float simTime = 0.0f;
    bool graphicsDirty = false;
    bool applyNow = false;

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.05f);
        simTime += dt;
        if (ui.statusTimer > 0.0f) ui.statusTimer = std::max(0.0f, ui.statusTimer - dt);

        if (applyNow) {
            GameApp::ApplyDisplayAndGraphics(cfg, shaders, mats, graphicsDirty);
            AudioSys::ApplyVolumes(audio, cfg, player.anomalyFactor, player.zoomed);
            graphicsDirty = false;
            applyNow = false;
        }

        world.rotorAngle = fmodf(world.rotorAngle + dt * (14.0f + player.anomalyFactor * 28.0f), 360.0f);
        world.turbineAngle = fmodf(world.turbineAngle + dt * 42.0f, 360.0f);
        if (world.elevatorMoving) {
            float diff = world.elevatorTargetY - world.elevatorY;
            if (fabsf(diff) < 0.05f) {
                world.elevatorY = world.elevatorTargetY;
                world.elevatorMoving = false;
            } else {
                world.elevatorY += Clamp(diff, -dt * 5.5f, dt * 5.5f);
            }
        }

        if (ui.screen == UISys::SCREEN_PLAYING) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                ui.screen = UISys::SCREEN_PAUSED;
                ui.showCodex = false;
                EnableCursor();
            } else if (IsKeyPressed(KEY_TAB)) {
                ui.showCodex = !ui.showCodex;
                if (ui.showCodex && ui.activeLogIndex < 0) ui.activeLogIndex = 0;
            } else if (!ui.showCodex) {
                Vector2 md = GetMouseDelta();
                float sens = 0.0022f * cfg.mouseSensitivity * (player.zoomed ? 0.45f : 1.0f);
                player.yaw += md.x * sens;
                float ySign = cfg.invertY ? -1.0f : 1.0f;
                player.pitch = Clamp(player.pitch - md.y * sens * ySign, -1.45f, 1.45f);

                player.zoomed = IsMouseButtonDown(MOUSE_RIGHT_BUTTON) || IsKeyDown(cfg.keyZoom);
                if (IsKeyPressed(cfg.keyFlashlight) && player.battery > 0.01f) {
                    player.flashlightOn = !player.flashlightOn;
                    if (audio.initialized) {
                        SetSoundVolume(audio.clickSwitch, Clamp(cfg.sfxVolume, 0.0f, 1.0f));
                        PlaySound(audio.clickSwitch);
                    }
                }

                Vector3 fwd = GameApp::CameraForward(player);
                Vector3 flatFwd = Vector3Normalize({ fwd.x, 0.0f, fwd.z });
                Vector3 right = Vector3Normalize({ -flatFwd.z, 0.0f, flatFwd.x });

                Vector3 moveDir = { 0.0f, 0.0f, 0.0f };
                if (IsKeyDown(cfg.keyForward) || IsKeyDown(KEY_UP))    moveDir = { moveDir.x + flatFwd.x, 0.0f, moveDir.z + flatFwd.z };
                if (IsKeyDown(cfg.keyBackward) || IsKeyDown(KEY_DOWN))  moveDir = { moveDir.x - flatFwd.x, 0.0f, moveDir.z - flatFwd.z };
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))           moveDir = { moveDir.x + right.x,   0.0f, moveDir.z + right.z };
                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))            moveDir = { moveDir.x - right.x,   0.0f, moveDir.z - right.z };

                float len = Vector3Length(moveDir);
                bool moving = len > 0.01f;
                bool sprinting = moving && IsKeyDown(cfg.keySprint) && player.stamina > 0.05f;
                if (moving) moveDir = Vector3Scale(moveDir, 1.0f / len);

                float speed = sprinting ? 11.5f : 6.2f;
                player.pos.x += moveDir.x * speed * dt;
                player.pos.z += moveDir.z * speed * dt;

                if (fabsf(player.pos.x) < 4.4f && fabsf(player.pos.z + 315.0f) < 4.4f && player.pos.y < 4.0f) {
                    player.pos.y = 1.8f + world.elevatorY;
                } else if (player.pos.x > 46.0f && player.pos.z < -303.0f && player.pos.z > -327.0f) {
                    player.pos.y = -19.5f;
                } else if (player.pos.z < -215.0f && player.pos.y < -12.0f) {
                    player.pos.y = -19.5f;
                } else if (!world.elevatorMoving || fabsf(player.pos.z + 315.0f) >= 4.4f) {
                    player.pos.y = 1.8f;
                }

                if (IsKeyPressed(KEY_R)) {
                    int sIdx = Clamp(world.activeSector, 0, static_cast<int>(world.sectors.size()) - 1);
                    player.pos = world.sectors[static_cast<size_t>(sIdx)].anchorPos;
                }

                if (sprinting) player.stamina = Clamp(player.stamina - dt * 0.28f, 0.0f, 1.0f);
                else player.stamina = Clamp(player.stamina + dt * 0.16f, 0.0f, 1.0f);

                if (player.flashlightOn) {
                    player.battery = Clamp(player.battery - dt * 0.008f, 0.0f, 1.0f);
                    if (player.battery <= 0.001f) player.flashlightOn = false;
                }

                if (moving) {
                    player.stepTimer += dt * (sprinting ? 1.55f : 1.0f);
                    float interval = sprinting ? 0.34f : 0.52f;
                    if (player.stepTimer >= interval) {
                        player.stepTimer = 0.0f;
                        if (audio.initialized) {
                            SetSoundVolume(audio.footstepMetal, Clamp(cfg.sfxVolume * (sprinting ? 0.65f : 0.42f), 0.0f, 1.0f));
                            PlaySound(audio.footstepMetal);
                        }
                    }
                }

                player.ambientTimer -= dt;
                if (player.ambientTimer <= 0.0f) {
                    player.ambientTimer = static_cast<float>(GetRandomValue(7, 15));
                    if (audio.initialized) {
                        SetSoundVolume(audio.ambientPulse, Clamp(cfg.ambientVolume * 0.55f, 0.0f, 1.0f));
                        PlaySound(audio.ambientPulse);
                    }
                }

                if (IsKeyPressed(cfg.keyInteract)) {
                    bool interacted = false;
                    for (size_t i = 0; i < world.logs.size(); i++) {
                        if (Vector3Distance(player.pos, world.logs[i].pos) < 4.5f) {
                            world.logs[i].read = true;
                            ui.activeLogIndex = static_cast<int>(i);
                            ui.showCodex = true;
                            interacted = true;
                            break;
                        }
                    }
                    if (!interacted && Vector3Distance(player.pos, { 0.0f, 1.8f + world.elevatorY, -315.0f }) < 5.5f) {
                        world.elevatorTargetY = (world.elevatorY > -11.0f) ? -22.0f : 0.0f;
                        world.elevatorMoving = true;
                        if (audio.initialized) {
                            SetSoundVolume(audio.liftHum, Clamp(cfg.sfxVolume * 0.8f, 0.0f, 1.0f));
                            PlaySound(audio.liftHum);
                        }
                    }
                }

                world.activeSector = WorldSys::DetectSector(world, player.pos);
                float targetAnomaly = (cfg.difficulty == 2) ? 0.85f : ((world.activeSector >= 2) ? 0.45f : 0.12f);
                player.anomalyFactor = Lerp(player.anomalyFactor, targetAnomaly, dt * 1.5f);
                AudioSys::ApplyVolumes(audio, cfg, player.anomalyFactor, player.zoomed);
            } else if (IsKeyPressed(cfg.keyInteract)) {
                ui.showCodex = false;
            }
        }

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        Vector3 camPos = player.pos;
        Vector3 camFwd = GameApp::CameraForward(player);
        Camera3D cam{};
        cam.position = camPos;
        cam.target = Vector3Add(camPos, camFwd);
        cam.up = { 0.0f, 1.0f, 0.0f };
        cam.fovy = player.zoomed ? (cfg.fov * 0.52f) : cfg.fov;
        cam.projection = CAMERA_PERSPECTIVE;

        if (shaders.valid) {
            BeginTextureMode(shaders.sceneColor);
            ClearBackground({ 10, 15, 24, 255 });
            BeginMode3D(cam);

            int secIdx = Clamp(world.activeSector, 0, static_cast<int>(world.sectors.size()) - 1);
            const auto& sec = world.sectors[static_cast<size_t>(secIdx)];
            BindPBRToActiveMaterial(mats.concrete.albedo, mats.concrete.normalMap, mats.concrete.ormMap, true, true, WHITE);
            DrawCube({ 0.0f, -1.0f, -45.0f }, 28.0f, 2.0f, 150.0f, { 120, 128, 140, 255 });
            DrawCube({ 0.0f, sec.waterLevel - 1.0f, -160.0f }, 900.0f, 2.0f, 900.0f, { 28, 52, 74, 255 });

            WorldSys::DrawMegaReactorAssembly(world.rotorAngle, WorldSys::ComputeLOD(110.0f), mats.concrete, mats.steel, mats.rust);
            WorldSys::DrawDetailedCryoTank({ 6.5f, 0.42f, -12.0f }, 24.0f, 0, mats.steel, mats.rust, mats.copper);
            WorldSys::DrawTurbineHallAndArchive(world, camPos, camFwd, cfg.drawDistance,
                                                mats.concrete, mats.steel, mats.rust, mats.grating, mats.copper);

            EndMode3D();
            EndTextureMode();
        }

        BeginDrawing();
        ClearBackground({ 8, 12, 18, 255 });

        if (shaders.valid) {
            BeginShaderMode(shaders.postProcess);
            SetShaderValue(shaders.postProcess, shaders.locPostTime, &simTime, SHADER_UNIFORM_FLOAT);
            int bOn = cfg.bloomEnabled ? 1 : 0;
            int sOn = cfg.ssaoEnabled ? 1 : 0;
            int hOn = cfg.hdrEnabled ? 1 : 0;
            int gOn = cfg.godRaysEnabled ? 1 : 0;
            int dOn = (cfg.dofEnabled && player.zoomed) ? 1 : 0;
            int cOn = cfg.chromaEnabled ? 1 : 0;
            int grOn = cfg.grainEnabled ? 1 : 0;
            Vector2 sunUV = { 0.55f, 0.72f };
            Vector2 res = { static_cast<float>(sw), static_cast<float>(sh) };

            SetShaderValue(shaders.postProcess, shaders.locPostBloom, &bOn, SHADER_UNIFORM_INT);
            SetShaderValue(shaders.postProcess, shaders.locPostSsao, &sOn, SHADER_UNIFORM_INT);
            SetShaderValue(shaders.postProcess, shaders.locPostHdr, &hOn, SHADER_UNIFORM_INT);
            SetShaderValue(shaders.postProcess, shaders.locPostGodRays, &gOn, SHADER_UNIFORM_INT);
            SetShaderValue(shaders.postProcess, shaders.locPostDof, &dOn, SHADER_UNIFORM_INT);
            SetShaderValue(shaders.postProcess, shaders.locPostChroma, &cOn, SHADER_UNIFORM_INT);
            SetShaderValue(shaders.postProcess, shaders.locPostGrain, &grOn, SHADER_UNIFORM_INT);
            SetShaderValue(shaders.postProcess, shaders.locPostBrightness, &cfg.brightness, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaders.postProcess, shaders.locPostContrast, &cfg.contrast, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaders.postProcess, shaders.locPostSunScreen, &sunUV, SHADER_UNIFORM_VEC2);
            SetShaderValue(shaders.postProcess, shaders.locPostResolution, &res, SHADER_UNIFORM_VEC2);

            Rectangle src = { 0.0f, 0.0f, static_cast<float>(shaders.sceneColor.texture.width), -static_cast<float>(shaders.sceneColor.texture.height) };
            DrawTextureRec(shaders.sceneColor.texture, src, { 0.0f, 0.0f }, WHITE);
            EndShaderMode();
        }

        if (ui.screen == UISys::SCREEN_PLAYING) {
            GameApp::DrawHUD(uiFont, sw, sh, ui, world, player, cfg);
        } else if (ui.screen == UISys::SCREEN_MAIN_MENU) {
            bool startNew = false, contGame = false, exitApp = false;
            UISys::DrawMainMenu(uiFont, sw, sh, ui, save, cfg, audio, startNew, contGame, exitApp);
            if (startNew) {
                player = GameApp::PlayerState{};
                world = WorldSys::CreateWorld();
                ui.screen = UISys::SCREEN_PLAYING;
                DisableCursor();
            } else if (contGame && save.hasSave) {
                player.pos = { save.posX, save.posY, save.posZ };
                player.yaw = save.yaw;
                player.pitch = save.pitch;
                player.anomalyFactor = save.anomalyFactor;
                world.activeSector = Clamp(save.sectorId, 0, static_cast<int>(world.sectors.size()) - 1);
                ui.screen = UISys::SCREEN_PLAYING;
                DisableCursor();
            } else if (exitApp) {
                break;
            }
        } else if (ui.screen == UISys::SCREEN_SETTINGS) {
            UISys::DrawSettingsScreen(uiFont, sw, sh, ui, cfg, audio, graphicsDirty, applyNow);
        } else {
            bool toMain = false, resume = false;
            GameApp::DrawOverlayScreen(uiFont, sw, sh, ui, world, player, save, cfg, audio, toMain, resume);
            if (resume) {
                ui.screen = UISys::SCREEN_PLAYING;
                DisableCursor();
            } else if (toMain) {
                ui.screen = UISys::SCREEN_MAIN_MENU;
                EnableCursor();
            }
        }

        EndDrawing();
    }

    ConfigSys::SaveSettings(cfg);
    ShaderSys::UnloadShaders(shaders);
    TextureSys::UnloadMaterials(mats);
    AudioSys::UnloadEngineAudio(audio);
    if (uiFont.texture.id != 0 && uiFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(uiFont);
    }
    CloseWindow();
    return 0;
}
