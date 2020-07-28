#include "main.h"

int main(int argc, char *argv[])
{
    coutArgs(argc, argv);

    /*
    if (argc <= ArgIndex::Token)
    {
        std::cout << U("Token Not Found") << std::endl;
        return ExitCode::TokenNotFound;
    }
    token = U(argv[ArgIndex::Token]);

    if (argc <= ArgIndex::ChatId)
    {
        std::cout << U("Chat ID Not Found") << std::endl;
        return ExitCode::ChatIdNotFound;
    }
    chat_id = U(argv[ArgIndex::ChatId]);

    enum Mode mode = Mode::Default;
    if (argc > ArgIndex::Mode)
    {
        uint uintMode = 0;
        std::stringstream strMode(argv[ArgIndex::Mode]);
        strMode >> uintMode;
        mode = static_cast<enum Mode>(uintMode);
    }

    bool disable_notification = false;
    if (argc > ArgIndex::DisableNotification)
    {
        std::stringstream str_disable_notification(argv[ArgIndex::DisableNotification]);
        str_disable_notification >> disable_notification;
    }

    std::string urlBase = U("https://api.telegram.org/bot");

    std::ostringstream urlBaseStream;
    urlBaseStream << urlBase << token;

    http_client_config config;
    config.set_validate_certificates(false);
    http_client client(urlBaseStream.str(), config);

    switch (mode)
    {
        default:
        case Mode::Today:
            modeToday(client, disable_notification);
            break;
        case Mode::YearDay:
        {
            std::stringstream strYear(argv[ArgIndex::Year]);
            uint uintYear;
            strYear >> uintYear;

            std::stringstream strDay(argv[ArgIndex::Day]);
            uint uintDay;
            strDay >> uintDay;

            modeYearDay(client, uintYear, uintDay, disable_notification);
        }
            break;
    }
    */

    http_client_config LineClientConfig;
    LineClientConfig.set_validate_certificates(false);
    LineClient = http_client(LineNotifyApiUrlBase, LineClientConfig);

    http_listener listenTelegram = openListener(listen_to_telegram_address, handleTelegram);

    std::ostringstream TelegramBotApiUrlBaseStream;
    TelegramBotApiUrlBaseStream << TelegramBotApiUrlBase << TelegramBotApiToken;

    http_client_config TelegramClientConfig;
    TelegramClientConfig.set_validate_certificates(false);
    TelegramClient = http_client(TelegramBotApiUrlBaseStream.str(), TelegramClientConfig);

    http_listener listenLine     = openListener(listen_to_line_address    , handleLine);

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    std::cout << "Terminating JSON listener." << std::endl;
    listenTelegram.close().wait();
    listenLine.close().wait();

    return ExitCode::Normal;
}

http_listener openListener(std::string address, const std::function<void(http_request)>& handler)
{
    uri m_uri(U(address));
    http_listener listener(m_uri);

    listener.support(handler);
    listener.open().wait();

    std::cout << "Web server started on: " << listener.uri().to_string() << std::endl;

    return listener;
}

void handleTelegram(http_request request)
{
    json::value jsonBody = request.extract_json().get();
    std::string strBody = jsonBody.serialize();

    std::ostringstream outputString;
    outputString << U("Receive (") << request.method() << U("): ");
    outputString << request.request_uri().to_string() << std::endl;
    outputString << U("Body: ") << strBody;
    std::cout << std::endl << outputString.str() << std::endl;

    request.reply(status_codes::OK, "");

    sendToLine(strBody);
}

void handleLine(http_request request)
{
    json::value jsonBody = request.extract_json().get();
    std::string strBody = jsonBody.serialize();

    std::ostringstream outputString;
    outputString << U("Receive (") << request.method() << U("): ");
    outputString << request.request_uri().to_string() << std::endl;
    outputString << U("Body: ") << strBody;
    std::cout << std::endl << outputString.str() << std::endl;

    request.reply(status_codes::OK, "");

    http_response response = sendToTelegram(strBody);
    coutHttpResponse(response, "sendToTelegram");
}

http_response sendToTelegram(std::string strText)
{
    std::string methodName = U("sendMessage");

    json::value requestBody = json::value::object();
    requestBody[U("chat_id")] = json::value::string(default_telegram_target);
    requestBody[U("text")] = json::value::string(strText);

    uri_builder builder(methodName);
    http_response response = TelegramClient.request(methods::POST, builder.to_string(), requestBody).get();

    return response;
}

http_response sendToLine(std::string strText)
{

}

void modeYearDay(http_client client, uint year, uint day, bool disable_notification)
{
    std::cout << U("[modeYearDay] ")
        << U("year = ") << year
        << U(", day = ") << day
        << U(", disable_notification = ") << disable_notification
        << std::endl;

    http_response response;

    response = sendAudio(client, true, year, day, disable_notification);
    coutHttpResponse(response);
    std::cout << std::endl;

    response = sendAudio(client, false, year, day, disable_notification);
    coutHttpResponse(response);
    std::cout << std::endl;

    response = sendPoll(client, day, disable_notification);
    coutHttpResponse(response);
    std::cout << std::endl;
}

http_response sendAudio(http_client client, bool isNewTestament, uint year, uint day, bool disable_notification)
{
    std::string methodName = U("sendAudio");

    std::cout << U("[") << methodName << U("]") << std::endl;

    // audio
    std::ostringstream audioStream;
    audioStream
            << github_io_url << U("/audio/rmID3/")
            << (isNewTestament ? U("nt") : U("ot"))
            << U("-oneyear-")
            << std::setw(3) << std::setfill(U('0')) << day
            << U(".mp3");

    // web page
    std::ostringstream webPageStream;
    webPageStream
            << github_io_url << U("/")
            << (isNewTestament ? U("n") : U("o"))
            << std::setw(3) << std::setfill(U('0')) << day
            << U(".html");

    // caption
    std::ostringstream captionStream;
    captionStream
            << year << U(".")
            << std::setw(2) << std::setfill(U('0')) << getMonth(year, day) << U(".")
            << std::setw(2) << std::setfill(U('0')) << getDate(year, day) << U("\n")
            << U("@bible365_today\n")
            << U("\n")
            << (isNewTestament ? U("新約") : U("舊約"))
            << U("第 ") << day << U(" 天\n")
            << webPageStream.str();

    json::value requestBody = json::value::object();
    requestBody[U("chat_id")] = json::value::string(chat_id);
    requestBody[U("audio")] = json::value::string(audioStream.str());
    requestBody[U("caption")] = json::value::string(captionStream.str());
    requestBody[U("disable_notification")] = json::value::boolean(disable_notification);

    std::cout << U("Request: ") << requestBody.serialize() << std::endl;

    uri_builder builder(methodName);
    http_response response = client.request(methods::POST, builder.to_string(), requestBody).get();

    return response;
}

http_response sendPoll(http_client client, uint day, bool disable_notification)
{
    std::string methodName = U("sendPoll");

    std::cout << U("[") << methodName << U("]") << std::endl;

    // question
    std::ostringstream questionStream;
    questionStream
            << U("第 ") << day << U(" 天")
            << U("進度紀錄");

    // options
    json::value options = json::value::array();
    options[0] = json::value::string(U("已完成本日進度"));
    options[1] = json::value::string(U("已讀本日新約進度"));
    options[2] = json::value::string(U("已讀本日舊約進度"));
    options[3] = json::value::string(U("未讀本日進度"));

    json::value requestBody = json::value::object();
    requestBody[U("chat_id")] = json::value::string(chat_id);
    requestBody[U("question")] = json::value::string(questionStream.str());
    requestBody[U("options")] = options;
    requestBody[U("is_anonymous")] = json::value::boolean(true);
    requestBody[U("disable_notification")] = json::value::boolean(disable_notification);

    std::cout << U("Request: ") << requestBody.serialize() << std::endl;

    uri_builder builder(methodName);
    http_response response = client.request(methods::POST, builder.to_string(), requestBody).get();

    return response;
}

void coutArgs(int argc, char *argv[])
{
    std::cout << U("argc: ") << argc << std::endl;
    for (int i = 0; i < argc; ++i)
    {
        std::cout << U("argv[") << i << U("]: ") << argv[i] << std::endl;
    }
    std::cout << std::endl;
}

void coutHttpResponse(http_response response, std::string strPrefix)
{
    std::ostringstream outputString;
    outputString << strPrefix << U("(") << response.status_code() << U(")");

    if(response.status_code() != 200)
    {
        outputString << U(": ") << response.extract_json().get().serialize();
    }

    std::cout << outputString.str() << std::endl;
}

bool isLeapYear(uint year)
{
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    if (year % 4 == 0) return true;
    return false;
}

uint getMonth(uint year, uint day)
{
    if (day <= 31) return 1;
    day -= 31;

    uint febDay = isLeapYear(year) ? 29 : 28;
    if (day <= febDay) return 2;
    day -= febDay;

    if (day <= 31) return 3;
    day -= 31;

    if (day <= 30) return 4;
    day -= 30;

    if (day <= 31) return 5;
    day -= 31;

    if (day <= 30) return 6;
    day -= 30;

    if (day <= 31) return 7;
    day -= 31;

    if (day <= 31) return 8;
    day -= 31;

    if (day <= 30) return 9;
    day -= 30;

    if (day <= 31) return 10;
    day -= 31;

    if (day <= 30) return 11;

    return 12;
}

uint getDate(uint year, uint day)
{
    if (day <= 31) return day;
    day -= 31;

    uint febDay = isLeapYear(year) ? 29 : 28;
    if (day <= febDay) return day;
    day -= febDay;

    if (day <= 31) return day;
    day -= 31;

    if (day <= 30) return day;
    day -= 30;

    if (day <= 31) return day;
    day -= 31;

    if (day <= 30) return day;
    day -= 30;

    if (day <= 31) return day;
    day -= 31;

    if (day <= 31) return day;
    day -= 31;

    if (day <= 30) return day;
    day -= 30;

    if (day <= 31) return day;
    day -= 31;

    if (day <= 30) return day;
    day -= 30;

    return day;
};
