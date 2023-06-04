#include "WebrtcStreamer.h"
#include "broadcaster.hpp"
#include "mediasoupclient.hpp"
#include <cpr/cpr.h>
#include <csignal> // sigsuspend()
#include <cstdlib>
#include <iostream>
#include <string>
#include "engine.h"
#include "logger/u_logger.h"
#include "room_client.h"

using json = nlohmann::json;

void signalHandler(int signum)
{
    std::cout << "[INFO] interrupt signal (" << signum << ") received" << std::endl;

    std::cout << "[INFO] leaving!" << std::endl;

    std::exit(signum);
}

int run()
{
    // Register signal SIGINT and signal handler.
    signal(SIGINT, signalHandler);

    // Retrieve configuration from environment variables.
    //const char* envServerUrl    = std::getenv("SERVER_URL");
    //const char* envRoomId       = std::getenv("ROOM_ID");
    //const char* envEnableAudio  = std::getenv("ENABLE_AUDIO");
    //const char* envUseSimulcast = std::getenv("USE_SIMULCAST");
    //const char* envWebrtcDebug  = std::getenv("WEBRTC_DEBUG");
    //const char* envVerifySsl    = std::getenv("VERIFY_SSL");

    // SERVER_URL=https://v3demo.mediasoup.org:4443 ROOM_ID=lqsszgcs
    const char* envServerUrl    = "https://www.wevisit.cn:4443";
    const char* envRoomId       = "testxxx";
    const char* envEnableAudio  = std::getenv("ENABLE_AUDIO");
    const char* envUseSimulcast = std::getenv("USE_SIMULCAST");
    const char* envWebrtcDebug  = std::getenv("WEBRTC_DEBUG");
    const char* envVerifySsl    = std::getenv("VERIFY_SSL");

    if (envServerUrl == nullptr) {
        std::cerr << "[ERROR] missing 'SERVER_URL' environment variable" << std::endl;
        return 1;
    }

    if (envRoomId == nullptr) {
        std::cerr << "[ERROR] missing 'ROOM_ID' environment variable" << std::endl;
        return 1;
    }

    // vi::ULogger::init();
    // getEngine()->init();

    std::string baseUrl = envServerUrl;
    baseUrl.append("/rooms/").append(envRoomId);

    bool enableAudio = false;

    if (envEnableAudio && std::string(envEnableAudio) == "false") {
        enableAudio = false;
    }

    bool useSimulcast = false;

    if (envUseSimulcast && std::string(envUseSimulcast) == "false") {
        useSimulcast = false;
    }

    bool verifySsl = true;
    if (envVerifySsl && std::string(envVerifySsl) == "false") {
        verifySsl = false;
    }

    // // Set RTC logging severity.
    // if (envWebrtcDebug) {
    //     if (std::string(envWebrtcDebug) == "info") {
    //         rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_INFO);
    //     } else if (std::string(envWebrtcDebug) == "warn") {
    //         rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_WARNING);
    //     } else if (std::string(envWebrtcDebug) == "error") {
    //         rtc::LogMessage::LogToDebug(rtc::LoggingSeverity::LS_ERROR);
    //     }
    // }

    // auto logLevel = mediasoupclient::Logger::LogLevel::LOG_DEBUG;
    // mediasoupclient::Logger::SetLogLevel(logLevel);
    // mediasoupclient::Logger::SetDefaultHandler();

    // // Initilize mediasoupclient.
    // mediasoupclient::Initialize();

//    auto roomClient = getEngine()->createRoomClient();
//    roomClient->join("www.wevisit.cn", 4443, "testxxx", "jackie", nullptr);

    std::cout << "[INFO] welcome to mediasoup broadcaster app!\n" << std::endl;

    //std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "[INFO] verifying that room '" << envRoomId << "' exists..." << std::endl;
    auto r = cpr::GetAsync(cpr::Url{ baseUrl }, cpr::VerifySsl{ verifySsl }).get();

    if (r.status_code != 200) {
        std::cerr << "[ERROR] unable to retrieve room info" << " [status code:" << r.status_code << ", body:\"" << r.text << "\"]" << std::endl;
        return 1;
    } else {
        std::cout << "[INFO] found room" << envRoomId << std::endl;
    }

    auto response = nlohmann::json::parse(r.text);
    auto broadcaster = getEngine()->createBroadcaster();

    auto videoUrl = "rtsp://192.168.0.254:554/live/main_stream";
    broadcaster->Start(baseUrl, videoUrl, "T1", enableAudio, useSimulcast, response, verifySsl);

    //auto broadcaster2 = getEngine()->createBroadcaster();

    //broadcaster2->Start(baseUrl, videoUrl, "T2", enableAudio, useSimulcast, response, verifySsl);

    std::cout << "[INFO] press Ctrl+C or Cmd+C to leave..." << std::endl;

    // while (true) {
    //     std::cin.get();
    // }

    // mediasoupclient::Cleanup();
    // getEngine()->destroy();
    // vi::ULogger::destroy();

    return 0;
}
