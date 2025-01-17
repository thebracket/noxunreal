#include "GameAnalytics.h"
#if PLATFORM_IOS
#include "../GA-SDK-IOS/GameAnalyticsCpp.h"
#elif PLATFORM_ANDROID
#include "../GA-SDK-ANDROID/GameAnalyticsJNI.h"
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
#include "../GA-SDK-CPP/GameAnalytics.h"
#elif PLATFORM_HTML5
#include "Json.h"
#include "../GA-SDK-HTML5/GameAnalytics.h"
#endif

UGameAnalytics::UGameAnalytics(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UGameAnalytics::configureAvailableCustomDimensions01(const std::vector<std::string>& list)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureAvailableCustomDimensions01(list);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureAvailableCustomDimensions01(list);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureAvailableCustomDimensions01(list);
#elif PLATFORM_HTML5
    TArray<TSharedPtr<FJsonValue>> array;
    for (std::string s : list)
    {
        TSharedRef<FJsonValueString> JsonValueString = MakeShareable(new FJsonValueString(ANSI_TO_TCHAR(s.c_str())));
        array.Add(JsonValueString);
    }
    FString arrayString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&arrayString);
    FJsonSerializer::Serialize(array, Writer);
    js_configureAvailableCustomDimensions01(TCHAR_TO_ANSI(*arrayString));
#endif
}

void UGameAnalytics::configureAvailableCustomDimensions02(const std::vector<std::string>& list)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureAvailableCustomDimensions02(list);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureAvailableCustomDimensions02(list);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureAvailableCustomDimensions02(list);
#elif PLATFORM_HTML5
    TArray<TSharedPtr<FJsonValue>> array;
    for (std::string s : list)
    {
        TSharedRef<FJsonValueString> JsonValueString = MakeShareable(new FJsonValueString(ANSI_TO_TCHAR(s.c_str())));
        array.Add(JsonValueString);
    }
    FString arrayString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&arrayString);
    FJsonSerializer::Serialize(array, Writer);
    js_configureAvailableCustomDimensions02(TCHAR_TO_ANSI(*arrayString));
#endif
}

void UGameAnalytics::configureAvailableCustomDimensions03(const std::vector<std::string>& list)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureAvailableCustomDimensions03(list);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureAvailableCustomDimensions03(list);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureAvailableCustomDimensions03(list);
#elif PLATFORM_HTML5
    TArray<TSharedPtr<FJsonValue>> array;
    for (std::string s : list)
    {
        TSharedRef<FJsonValueString> JsonValueString = MakeShareable(new FJsonValueString(ANSI_TO_TCHAR(s.c_str())));
        array.Add(JsonValueString);
    }
    FString arrayString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&arrayString);
    FJsonSerializer::Serialize(array, Writer);
    js_configureAvailableCustomDimensions03(TCHAR_TO_ANSI(*arrayString));
#endif
}

void UGameAnalytics::configureAvailableResourceCurrencies(const std::vector<std::string>& list)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureAvailableResourceCurrencies(list);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureAvailableResourceCurrencies(list);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureAvailableResourceCurrencies(list);
#elif PLATFORM_HTML5
    TArray<TSharedPtr<FJsonValue>> array;
    for (std::string s : list)
    {
        TSharedRef<FJsonValueString> JsonValueString = MakeShareable(new FJsonValueString(ANSI_TO_TCHAR(s.c_str())));
        array.Add(JsonValueString);
    }
    FString arrayString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&arrayString);
    FJsonSerializer::Serialize(array, Writer);
    js_configureAvailableResourceCurrencies(TCHAR_TO_ANSI(*arrayString));
#endif
}

void UGameAnalytics::configureAvailableResourceItemTypes(const std::vector<std::string>& list)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureAvailableResourceItemTypes(list);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureAvailableResourceItemTypes(list);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureAvailableResourceItemTypes(list);
#elif PLATFORM_HTML5
    TArray<TSharedPtr<FJsonValue>> array;
    for (std::string s : list)
    {
        TSharedRef<FJsonValueString> JsonValueString = MakeShareable(new FJsonValueString(ANSI_TO_TCHAR(s.c_str())));
        array.Add(JsonValueString);
    }
    FString arrayString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&arrayString);
    FJsonSerializer::Serialize(array, Writer);
    js_configureAvailableResourceItemTypes(TCHAR_TO_ANSI(*arrayString));
#endif
}

void UGameAnalytics::configureBuild(const char *build)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureBuild(build);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureBuild(build);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureBuild(build);
#elif PLATFORM_HTML5
    js_configureBuild(build);
#endif
}

void UGameAnalytics::configureUserId(const char *userId)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureUserId(userId);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureUserId(userId);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureUserId(userId);
#elif PLATFORM_HTML5
    js_configureUserId(userId);
#endif
}

void UGameAnalytics::configureSdkGameEngineVersion(const char *gameEngineSdkVersion)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureSdkGameEngineVersion(gameEngineSdkVersion);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureSdkGameEngineVersion(gameEngineSdkVersion);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureSdkGameEngineVersion(gameEngineSdkVersion);
#elif PLATFORM_HTML5
    js_configureSdkGameEngineVersion(gameEngineSdkVersion);
#endif
}

void UGameAnalytics::configureGameEngineVersion(const char *gameEngineVersion)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::configureGameEngineVersion(gameEngineVersion);
#elif PLATFORM_ANDROID
    gameanalytics::jni_configureGameEngineVersion(gameEngineVersion);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::configureGameEngineVersion(gameEngineVersion);
#elif PLATFORM_HTML5
    js_configureGameEngineVersion(gameEngineVersion);
#endif
}

void UGameAnalytics::initialize(const char *gameKey, const char *gameSecret)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::initialize(gameKey, gameSecret);
#elif PLATFORM_ANDROID
    gameanalytics::jni_initialize(gameKey, gameSecret);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::initialize(gameKey, gameSecret);
#elif PLATFORM_HTML5
    js_initialize(gameKey, gameSecret);
#endif
}

#if PLATFORM_IOS
void UGameAnalytics::addBusinessEvent(const char *currency, int amount, const char *itemType, const char *itemId, const char *cartType, const char *receipt)
{
#if !WTIH_EDITOR
    GameAnalyticsCpp::addBusinessEvent(currency, amount, itemType, itemId, cartType, receipt);
#endif
}

void UGameAnalytics::addBusinessEventAndAutoFetchReceipt(const char *currency, int amount, const char *itemType, const char *itemId, const char *cartType)
{
#if !WITH_EDITOR
    GameAnalyticsCpp::addBusinessEventAndAutoFetchReceipt(currency, amount, itemType, itemId, cartType);
#endif
}
#elif PLATFORM_ANDROID
void UGameAnalytics::addBusinessEvent(const char *currency, int amount, const char *itemType, const char *itemId, const char *cartType, const char *receipt, const char *signature)
{
#if !WITH_EDITOR
    gameanalytics::jni_addBusinessEventWithReceipt(currency, amount, itemType, itemId, cartType, receipt, "google_play", signature);
#endif
}
#endif

void UGameAnalytics::addBusinessEvent(const char *currency, int amount, const char *itemType, const char *itemId, const char *cartType)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::addBusinessEvent(currency, amount, itemType, itemId, cartType, NULL);
#elif PLATFORM_ANDROID
    gameanalytics::jni_addBusinessEvent(currency, amount, itemType, itemId, cartType);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::addBusinessEvent(currency, amount, itemType, itemId, cartType);
#elif PLATFORM_HTML5
    js_addBusinessEvent(currency, amount, itemType, itemId, cartType);
#endif
}

void UGameAnalytics::addResourceEvent(EGAResourceFlowType flowType, const char *currency, float amount, const char *itemType, const char *itemId)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::addResourceEvent((int)flowType, currency, amount, itemType, itemId);
#elif PLATFORM_ANDROID
    gameanalytics::jni_addResourceEvent((int)flowType, currency, amount, itemType, itemId);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::addResourceEvent((gameanalytics::EGAResourceFlowType)((int)flowType), currency, amount, itemType, itemId);
#elif PLATFORM_HTML5
    js_addResourceEvent((int)flowType, currency, amount, itemType, itemId);
#endif
}

void UGameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const char *progression01)
{
#if PLATFORM_IOS
    addProgressionEvent(progressionStatus, progression01, (const char *)NULL, (const char *)NULL);
#else
    addProgressionEvent(progressionStatus, progression01, "", "");
#endif
}

void UGameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const char *progression01, int score)
{
#if PLATFORM_IOS
    addProgressionEvent(progressionStatus, progression01, (const char *)NULL, (const char *)NULL, score);
#else
    addProgressionEvent(progressionStatus, progression01, "", "", score);
#endif
}

void UGameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const char *progression01, const char *progression02)
{
#if PLATFORM_IOS
    addProgressionEvent(progressionStatus, progression01, progression02, (const char *)NULL);
#else
    addProgressionEvent(progressionStatus, progression01, progression02, "");
#endif
}

void UGameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const char *progression01, const char *progression02, int score)
{
#if PLATFORM_IOS
    addProgressionEvent(progressionStatus, progression01, progression02, (const char *)NULL, score);
#else
    addProgressionEvent(progressionStatus, progression01, progression02, "", score);
#endif
}

void UGameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const char *progression01, const char *progression02, const char *progression03)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::addProgressionEvent((int)progressionStatus, progression01, progression02, progression03);
#elif PLATFORM_ANDROID
    gameanalytics::jni_addProgressionEvent((int)progressionStatus, progression01, progression02, progression03);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::addProgressionEvent((gameanalytics::EGAProgressionStatus)((int)progressionStatus), progression01, progression02, progression03);
#elif PLATFORM_HTML5
    js_addProgressionEvent((int)progressionStatus, progression01, progression02, progression03);
#endif
}

void UGameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const char *progression01, const char *progression02, const char *progression03, int score)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::addProgressionEventWithScore((int)progressionStatus, progression01, progression02, progression03, score);
#elif PLATFORM_ANDROID
    gameanalytics::jni_addProgressionEventWithScore((int)progressionStatus, progression01, progression02, progression03, score);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::addProgressionEvent((gameanalytics::EGAProgressionStatus)((int)progressionStatus), progression01, progression02, progression03, score);
#elif PLATFORM_HTML5
    js_addProgressionEventWithScore((int)progressionStatus, progression01, progression02, progression03, score);
#endif
}

void UGameAnalytics::addDesignEvent(const char *eventId)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::addDesignEvent(eventId);
#elif PLATFORM_ANDROID
    gameanalytics::jni_addDesignEvent(eventId);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::addDesignEvent(eventId);
#elif PLATFORM_HTML5
    js_addDesignEvent(eventId);
#endif
}

void UGameAnalytics::addDesignEvent(const char *eventId, float value)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::addDesignEventWithValue(eventId, value);
#elif PLATFORM_ANDROID
    gameanalytics::jni_addDesignEventWithValue(eventId, value);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::addDesignEvent(eventId, value);
#elif PLATFORM_HTML5
    js_addDesignEventWithValue(eventId, value);
#endif
}

void UGameAnalytics::addErrorEvent(EGAErrorSeverity severity, const char *message)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::addErrorEvent((int)severity, message);
#elif PLATFORM_ANDROID
    gameanalytics::jni_addErrorEvent((int)severity, message);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::addErrorEvent((gameanalytics::EGAErrorSeverity)((int)severity), message);
#elif PLATFORM_HTML5
    js_addErrorEvent((int)severity, message);
#endif
}

void UGameAnalytics::setEnabledInfoLog(bool flag)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setEnabledInfoLog(flag);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setEnabledInfoLog(flag);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setEnabledInfoLog(flag);
#elif PLATFORM_HTML5
    js_setEnabledInfoLog(flag);
#endif
}

void UGameAnalytics::setEnabledVerboseLog(bool flag)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setEnabledVerboseLog(flag);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setEnabledVerboseLog(flag);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setEnabledVerboseLog(flag);
#elif PLATFORM_HTML5
    js_setEnabledVerboseLog(flag);
#endif
}

void UGameAnalytics::setEnabledManualSessionHandling(bool flag)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setEnabledManualSessionHandling(flag);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setEnabledManualSessionHandling(flag);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setEnabledManualSessionHandling(flag);
#elif PLATFORM_HTML5
    js_setManualSessionHandling(flag);
#endif
}

void UGameAnalytics::setCustomDimension01(const char *customDimension)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setCustomDimension01(customDimension);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setCustomDimension01(customDimension);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setCustomDimension01(customDimension);
#elif PLATFORM_HTML5
    js_setCustomDimension01(customDimension);
#endif
}

void UGameAnalytics::setCustomDimension02(const char *customDimension)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setCustomDimension02(customDimension);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setCustomDimension02(customDimension);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setCustomDimension02(customDimension);
#elif PLATFORM_HTML5
    js_setCustomDimension02(customDimension);
#endif
}

void UGameAnalytics::setCustomDimension03(const char *customDimension)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setCustomDimension03(customDimension);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setCustomDimension03(customDimension);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setCustomDimension03(customDimension);
#elif PLATFORM_HTML5
    js_setCustomDimension03(customDimension);
#endif
}

void UGameAnalytics::setFacebookId(const char *facebookId)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setFacebookId(facebookId);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setFacebookId(facebookId);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setFacebookId(facebookId);
#elif PLATFORM_HTML5
    js_setFacebookId(facebookId);
#endif
}

void UGameAnalytics::setGender(EGAGender gender)
{
    switch(gender)
    {
        case EGAGender::male:
        {
#if WITH_EDITOR
#elif PLATFORM_IOS
        	GameAnalyticsCpp::setGender("male");
#elif PLATFORM_ANDROID
        	gameanalytics::jni_setGender((int)gender);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
            gameanalytics::GameAnalytics::setGender((gameanalytics::EGAGender)((int)gender));
#elif PLATFORM_HTML5
            js_setGender((int)gender);
#endif
        }
        break;

        case EGAGender::female:
        {
#if WITH_EDITOR
#elif PLATFORM_IOS
        	GameAnalyticsCpp::setGender("female");
#elif PLATFORM_ANDROID
        	gameanalytics::jni_setGender((int)gender);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
            gameanalytics::GameAnalytics::setGender((gameanalytics::EGAGender)((int)gender));
#elif PLATFORM_HTML5
            js_setGender((int)gender);
#endif
        }
        break;
    }
}

void UGameAnalytics::setBirthYear(int birthYear)
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::setBirthYear(birthYear);
#elif PLATFORM_ANDROID
    gameanalytics::jni_setBirthYear(birthYear);
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::setBirthYear(birthYear);
#elif PLATFORM_HTML5
    js_setBirthYear(birthYear);
#endif
}

void UGameAnalytics::startSession()
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::startSession();
#elif PLATFORM_ANDROID
    gameanalytics::jni_startSession();
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::startSession();
#elif PLATFORM_HTML5
    js_startSession();
#endif
}

void UGameAnalytics::endSession()
{
#if WITH_EDITOR
#elif PLATFORM_IOS
    GameAnalyticsCpp::endSession();
#elif PLATFORM_ANDROID
    gameanalytics::jni_endSession();
#elif PLATFORM_MAC || PLATFORM_WINDOWS || PLATFORM_LINUX
    gameanalytics::GameAnalytics::endSession();
#elif PLATFORM_HTML5
    js_endSession();
#endif
}
