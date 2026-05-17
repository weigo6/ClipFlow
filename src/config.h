#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QString>

struct ImageToMarkdownConfig {
    QString apiKey;
    QString model;
    QString baseUrl;
    QString prompt;
};

struct TranslationConfig {
    QString apiKey;
    QString model;
    QString baseUrl;
    QString promptAuto;
    QMap<QString, QString> promptMap;
    QString screenshotTargetLanguage;
};

struct BackgroundConfig {
    bool startInTray = false;
    QString screenshotHotkey;
    QString translateHotkey;
    QString hotkeyMode;
    QString sharedHotkeyAction;
    bool screenshotHotkeyEnabled = true;
    bool translateHotkeyEnabled = true;
    bool trayBubblesEnabled = false;
};

struct UiConfig {
    QString theme;
};

struct AppConfig {
    ImageToMarkdownConfig imageToMarkdown;
    TranslationConfig translation;
    BackgroundConfig background;
    UiConfig ui;
};

inline QMap<QString, QString> defaultPromptMap()
{
    return {
        {QStringLiteral("en2cn"), QStringLiteral("你是一个专业翻译助手，将以下英文内容准确、流畅地翻译成简体中文：")},
        {QStringLiteral("cn2en"), QStringLiteral("你是一个专业翻译助手，将以下中文内容准确、流畅地翻译成自然英文：")},
        {QStringLiteral("en2jp"), QStringLiteral("You are a professional translator, please accurately translate the following English content into natural Japanese:")},
        {QStringLiteral("jp2cn"), QStringLiteral("你是一个专业翻译助手，将以下日文内容准确、流畅地翻译成简体中文：")}
    };
}

inline AppConfig defaultConfig()
{
    AppConfig cfg;
    cfg.imageToMarkdown.apiKey = QStringLiteral("sk-xxxxxxxxxxxxxxxxxxxxxx");
    cfg.imageToMarkdown.model = QStringLiteral("gpt-4-vision-preview");
    cfg.imageToMarkdown.baseUrl = QStringLiteral("https://api.openai.com/v1");
    cfg.imageToMarkdown.prompt = QStringLiteral("将图片中的内容准确转换为Markdown格式，包含数学公式的请使用LaTeX格式输出");

    cfg.translation.apiKey = QStringLiteral("sk-xxxxxxxxxxxxxxxxxxxxxx");
    cfg.translation.model = QStringLiteral("gpt-4");
    cfg.translation.baseUrl = QStringLiteral("https://api.openai.com/v1");
    cfg.translation.promptAuto = QStringLiteral("你是一个专业翻译助手，能够识别原文语言并准确翻译成目标语言：");
    cfg.translation.promptMap = defaultPromptMap();
    cfg.translation.screenshotTargetLanguage = QStringLiteral("简体中文");

    cfg.background.startInTray = false;
    cfg.background.screenshotHotkey = QStringLiteral("Ctrl+Alt+S");
    cfg.background.translateHotkey = QStringLiteral("Ctrl+Alt+T");
    cfg.background.hotkeyMode = QStringLiteral("shared");
    cfg.background.sharedHotkeyAction = QStringLiteral("ocr");
    cfg.background.screenshotHotkeyEnabled = true;
    cfg.background.translateHotkeyEnabled = true;
    cfg.background.trayBubblesEnabled = false;

    cfg.ui.theme = QStringLiteral("dark");
    return cfg;
}

inline AppConfig configFromJsonObject(const QJsonObject& root)
{
    AppConfig cfg = defaultConfig();

    const auto imageObj = root.value(QStringLiteral("image_to_markdown")).toObject();
    if (!imageObj.isEmpty()) {
        cfg.imageToMarkdown.apiKey = imageObj.value(QStringLiteral("api_key")).toString(cfg.imageToMarkdown.apiKey);
        cfg.imageToMarkdown.model = imageObj.value(QStringLiteral("model")).toString(cfg.imageToMarkdown.model);
        cfg.imageToMarkdown.baseUrl = imageObj.value(QStringLiteral("base_url")).toString(cfg.imageToMarkdown.baseUrl);
        cfg.imageToMarkdown.prompt = imageObj.value(QStringLiteral("prompt")).toString(cfg.imageToMarkdown.prompt);
    }

    const auto transObj = root.value(QStringLiteral("translation")).toObject();
    if (!transObj.isEmpty()) {
        cfg.translation.apiKey = transObj.value(QStringLiteral("api_key")).toString(cfg.translation.apiKey);
        cfg.translation.model = transObj.value(QStringLiteral("model")).toString(cfg.translation.model);
        cfg.translation.baseUrl = transObj.value(QStringLiteral("base_url")).toString(cfg.translation.baseUrl);
        cfg.translation.promptAuto = transObj.value(QStringLiteral("prompt_auto")).toString(cfg.translation.promptAuto);
        cfg.translation.screenshotTargetLanguage =
            transObj.value(QStringLiteral("screenshot_target_language")).toString(cfg.translation.screenshotTargetLanguage);

        const auto promptMapObj = transObj.value(QStringLiteral("prompt_map")).toObject();
        if (!promptMapObj.isEmpty()) {
            QMap<QString, QString> map;
            for (auto it = promptMapObj.begin(); it != promptMapObj.end(); ++it) {
                map.insert(it.key(), it.value().toString());
            }
            if (!map.isEmpty()) {
                cfg.translation.promptMap = map;
            }
        }
    }

    const auto bgObj = root.value(QStringLiteral("background")).toObject();
    if (!bgObj.isEmpty()) {
        cfg.background.startInTray = bgObj.value(QStringLiteral("start_in_tray")).toBool(cfg.background.startInTray);
        cfg.background.screenshotHotkey =
            bgObj.value(QStringLiteral("screenshot_hotkey")).toString(cfg.background.screenshotHotkey);
        cfg.background.translateHotkey =
            bgObj.value(QStringLiteral("translate_hotkey")).toString(cfg.background.translateHotkey);
        cfg.background.hotkeyMode = bgObj.value(QStringLiteral("hotkey_mode")).toString(cfg.background.hotkeyMode);
        cfg.background.sharedHotkeyAction =
            bgObj.value(QStringLiteral("shared_hotkey_action")).toString(cfg.background.sharedHotkeyAction);
        cfg.background.screenshotHotkeyEnabled =
            bgObj.value(QStringLiteral("screenshot_hotkey_enabled")).toBool(cfg.background.screenshotHotkeyEnabled);
        cfg.background.translateHotkeyEnabled =
            bgObj.value(QStringLiteral("translate_hotkey_enabled")).toBool(cfg.background.translateHotkeyEnabled);
        cfg.background.trayBubblesEnabled =
            bgObj.value(QStringLiteral("tray_bubbles_enabled")).toBool(cfg.background.trayBubblesEnabled);
    }

    const auto uiObj = root.value(QStringLiteral("ui")).toObject();
    if (!uiObj.isEmpty()) {
        cfg.ui.theme = uiObj.value(QStringLiteral("theme")).toString(cfg.ui.theme);
    }

    return cfg;
}

inline QJsonObject configToJsonObject(const AppConfig& cfg)
{
    QJsonObject root;

    QJsonObject imageObj;
    imageObj.insert(QStringLiteral("api_key"), cfg.imageToMarkdown.apiKey);
    imageObj.insert(QStringLiteral("model"), cfg.imageToMarkdown.model);
    imageObj.insert(QStringLiteral("base_url"), cfg.imageToMarkdown.baseUrl);
    imageObj.insert(QStringLiteral("prompt"), cfg.imageToMarkdown.prompt);
    root.insert(QStringLiteral("image_to_markdown"), imageObj);

    QJsonObject transObj;
    transObj.insert(QStringLiteral("api_key"), cfg.translation.apiKey);
    transObj.insert(QStringLiteral("model"), cfg.translation.model);
    transObj.insert(QStringLiteral("base_url"), cfg.translation.baseUrl);
    transObj.insert(QStringLiteral("prompt_auto"), cfg.translation.promptAuto);
    transObj.insert(QStringLiteral("screenshot_target_language"), cfg.translation.screenshotTargetLanguage);

    QJsonObject promptMapObj;
    for (auto it = cfg.translation.promptMap.begin(); it != cfg.translation.promptMap.end(); ++it) {
        promptMapObj.insert(it.key(), it.value());
    }
    transObj.insert(QStringLiteral("prompt_map"), promptMapObj);
    root.insert(QStringLiteral("translation"), transObj);

    QJsonObject bgObj;
    bgObj.insert(QStringLiteral("start_in_tray"), cfg.background.startInTray);
    bgObj.insert(QStringLiteral("screenshot_hotkey"), cfg.background.screenshotHotkey);
    bgObj.insert(QStringLiteral("translate_hotkey"), cfg.background.translateHotkey);
    bgObj.insert(QStringLiteral("hotkey_mode"), cfg.background.hotkeyMode);
    bgObj.insert(QStringLiteral("shared_hotkey_action"), cfg.background.sharedHotkeyAction);
    bgObj.insert(QStringLiteral("screenshot_hotkey_enabled"), cfg.background.screenshotHotkeyEnabled);
    bgObj.insert(QStringLiteral("translate_hotkey_enabled"), cfg.background.translateHotkeyEnabled);
    bgObj.insert(QStringLiteral("tray_bubbles_enabled"), cfg.background.trayBubblesEnabled);
    root.insert(QStringLiteral("background"), bgObj);

    QJsonObject uiObj;
    uiObj.insert(QStringLiteral("theme"), cfg.ui.theme);
    root.insert(QStringLiteral("ui"), uiObj);

    return root;
}
