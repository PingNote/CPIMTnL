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

    /// Standby send to Line
    http_client_config LineClientConfig;
    LineClientConfig.set_validate_certificates(false);
    LineClient = http_client(LineNotifyApiUrlBase, LineClientConfig);


    /// Standby Telegram Webhook
    http_listener listenTelegram = openListener(listen_to_telegram_address, handleTelegram);


    /// Standby send to Telegram
    std::ostringstream TelegramBotApiUrlBaseStream;
    TelegramBotApiUrlBaseStream << TelegramBotApiUrlBase << TelegramBotApiToken;

    http_client_config TelegramClientConfig;
    TelegramClientConfig.set_validate_certificates(false);
    TelegramClient = http_client(TelegramBotApiUrlBaseStream.str(), TelegramClientConfig);


    /// Standby Line Webhook
    http_listener listenLine     = openListener(listen_to_line_address    , handleLine);


    reportServiceRestarted();

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    std::cout << "Terminating JSON listener." << std::endl;
    listenTelegram.close().wait();
    listenLine.close().wait();

    return ExitCode::Normal;
}

void reportServiceRestarted()
{
    std::ostringstream outputString;
    outputString << "[ServiceRestarted]\n\n";

    json::value jsonBody;

    jsonBody = httpGetJson("http://v4.ipv6-test.com/api/myip.php?json", false, true, "reportServiceRestarted");
    if(jsonBody != NULL)
    {
        outputString << jsonBody.serialize() << "\n\n";
    }

    jsonBody = httpGetJson("http://v6.ipv6-test.com/api/myip.php?json", false, true, "reportServiceRestarted");
    if(jsonBody != NULL)
    {
        outputString << jsonBody.serialize() << "\n\n";
    }

    jsonBody = httpGetJson("https://api.myip.com", false, true, "reportServiceRestarted");
    if(jsonBody != NULL)
    {
        outputString << jsonBody.serialize();
    }

    http_response response = sendToTelegram(outputString.str());
    coutHttpResponse(response, "sendToTelegram",
                     false, true,
                     true, false);
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
    outputString << U("Body: ") << strBody << std::endl;
    std::cout << outputString.str() << std::endl;

    coutHeaders(request.headers());

    request.reply(status_codes::OK, "");

    http_response response = sendToLine(strBody);
    coutHttpResponse(response, "sendToLine",
                     false, true,
                     true, false);
}

void handleLine(http_request request)
{
    json::value jsonBody = request.extract_json().get();
    std::string strBody = jsonBody.serialize();

    std::ostringstream outputString;
    outputString << U("Receive (") << request.method() << U("): ");
    outputString << request.request_uri().to_string() << std::endl;
    outputString << U("Body: ") << strBody << std::endl;

    coutHeaders(request.headers());

    std::cout << outputString.str() << std::endl;

    request.reply(status_codes::OK, "");

    http_response response = sendToTelegram(strBody);
    coutHttpResponse(response, "sendToTelegram",
                     false, true,
                     true, false);
}

http_response sendToTelegram(std::string strText)
{
    std::string methodName = U("sendMessage");

    json::value requestBody = json::value::object();
    requestBody[U("chat_id")] = json::value::string(default_telegram_target);
    requestBody[U("text")] = json::value::string(strText);

    http_response response = TelegramClient.request(methods::POST, methodName, requestBody).get();

    return response;
}

http_response sendToLine(std::string strText)
{
    std::string methodName = U("notify");

    std::ostringstream outputString;
    outputString << "Bearer " << default_line_Target;

    http_request msg(methods::POST);
    msg.headers()["Authorization"] = outputString.str();

    msg.set_request_uri(methodName);

    uri_builder body_builder;
    body_builder.append_query(U("message"), U(strText));
    msg.set_body(body_builder.query(), "application/x-www-form-urlencoded; charset=utf-8");

    http_response response = LineClient.request(msg).get();

    return response;
}

json::value httpGetJson(std::string strUrl, bool validate_certs, bool ignore_content_type, std::string strPrefix)
{
    json::value jsonBody;

    http_client_config config;
    config.set_validate_certificates(validate_certs);

    http_client httpGetClient(strUrl, config);
    http_response response = httpGetClient.request(methods::GET).get();
    http::status_code status_code = response.status_code();
    if(status_code == http::status_codes::OK)
    {
        jsonBody = response.extract_json(ignore_content_type).get();
    }

    coutHttpResponse(status_code, jsonBody, response.headers(), strPrefix);

    return jsonBody;
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

void coutHeaders(http_headers headers)
{
    if(headers.size() <= 0)
    {
        return;
    }

    std::ostringstream outputString;
    for(auto header : headers)
    {
        outputString << "\theaders[\"" << header.first << "\"]=\"" << header.second << "\"" << std::endl;
    }
    std::cout << outputString.str() << std::endl;
}

void coutHttpResponse(http_response response, std::string strPrefix,
                      bool isShowBodyWhenOk, bool isShowHeadersWhenOk,
                      bool isShowBodyWhenNonOk, bool isShowHeadersWhenNonOk)
{
    coutHttpResponse(response.status_code(),
                     response.extract_json(true).get(),
                     response.headers(),
                     strPrefix,
                     isShowBodyWhenOk, isShowHeadersWhenOk,
                     isShowBodyWhenNonOk, isShowHeadersWhenNonOk);
}

void coutHttpResponse(http::status_code status_code, json::value jsonBody, http_headers headers,
                      std::string strPrefix,
                      bool isShowBodyWhenOk, bool isShowHeadersWhenOk,
                      bool isShowBodyWhenNonOk, bool isShowHeadersWhenNonOk)
{
    std::ostringstream outputString;
    outputString << strPrefix << U("(") << status_code << U(")");

    if(jsonBody != NULL &&
       ((isShowBodyWhenOk    && status_code == http::status_codes::OK) ||
        (isShowBodyWhenNonOk && status_code != http::status_codes::OK)   ))
    {
        outputString << ": " << jsonBody.serialize();
    }
    std::cout << outputString.str() << std::endl;

    if(((isShowHeadersWhenOk    && status_code == http::status_codes::OK) ||
        (isShowHeadersWhenNonOk && status_code != http::status_codes::OK)   ))
    {
        coutHeaders(headers);
    }
}