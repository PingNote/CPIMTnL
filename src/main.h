#ifndef CPIMTnL_MAIN_H
#define CPIMTnL_MAIN_H

#include <string>
#include <iostream>
#include <thread>
#include <cpprest/details/basic_types.h>
#include <cpprest/http_headers.h>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <boost/date_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

#include "config.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace web::http::experimental::listener;
using namespace concurrency::streams;       // Asynchronous streams
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;

http_listener openListener(std::string address, const std::function<void(http_request)>& handler);
void handleTelegram(http_request request);
void handleLine(http_request request);

std::string TelegramBotApiUrlBase = U("https://api.telegram.org/bot");
std::string LineNotifyApiUrlBase = U("https://notify-api.line.me/api");

http_client TelegramClient = http_client(TelegramBotApiUrlBase);
http_client LineClient = http_client(LineNotifyApiUrlBase);

http_response sendToTelegram(std::string strText);
http_response sendToLine(std::string strText);

void reportServiceRestarted();

json::value httpGetJson(std::string strUrl, bool validate_certs = false, bool ignore_content_type = true, std::string strPrefix  = "Response");

void coutHeaders(http_headers headers);

void coutHttpResponse(http_response response,
                      std::string strPrefix = "Response",
                      bool isShowBodyWhenOk = true, bool isShowHeadersWhenOk  = true,
                      bool isShowBodyWhenNonOk = true, bool isShowHeadersWhenNonOk  = true);

void coutHttpResponse(http::status_code status_code, json::value jsonBody, http_headers headers,
                      std::string strPrefix = "Response",
                      bool isShowBodyWhenOk = true, bool isShowHeadersWhenOk  = true,
                      bool isShowBodyWhenNonOk = true, bool isShowHeadersWhenNonOk  = true);

enum ArgIndex : uint
{
    Command = 0,
    Token = 1,
    ChatId = 2,
    Mode = 3,
    DisableNotification = 4,

    /// For Mode::YearDay
    Year = 5,
    Day = 6
};

enum Mode : uint
{
    Default = 0,
    Today = Default,
    YearDay = 1
};

enum ExitCode : int
{
    Normal = 0,
    TokenNotFound = -1,
    ChatIdNotFound = -2
};

bool isLeapYear(uint year);
uint getMonth(uint year, uint day);
uint getDate(uint year, uint day);

void coutArgs(int argc, char *argv[]);

http_response sendAudio(http_client client, bool isNewTestament, uint year, uint day, bool disable_notification);
http_response sendPoll(http_client client, uint day, bool disable_notification);

void modeYearDay(http_client client, uint year, uint day, bool disable_notification);
void modeToday(http_client client, bool disable_notification);
#endif //CPIMTnL_MAIN_H
