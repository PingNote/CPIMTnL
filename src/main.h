#ifndef CPIMTnL_MAIN_H
#define CPIMTnL_MAIN_H

#include <string>
#include <iostream>
#include <thread>
#include <cpprest/details/basic_types.h>
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

http_listener openListener(std::string address);
void handler(http_request request);

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
void coutHttpResponse(http_response response);

http_response sendAudio(http_client client, bool isNewTestament, uint year, uint day, bool disable_notification);
http_response sendPoll(http_client client, uint day, bool disable_notification);

void modeYearDay(http_client client, uint year, uint day, bool disable_notification);
void modeToday(http_client client, bool disable_notification);
#endif //CPIMTnL_MAIN_H
