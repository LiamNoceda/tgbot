#pragma once
#include <string>
#include <curl/curl.h>

namespace Bot::Network {

    class HttpClient {
    public:
        HttpClient();
        ~HttpClient();

        // Запрет копирования класса (управление ресурсом CURL по RAII)
        HttpClient(const HttpClient&) = delete;
        HttpClient& operator=(const HttpClient&) = delete;

        [[nodiscard]] std::string send_request(const std::string& url) const;
        [[nodiscard]] std::string url_encode(const std::string& text) const;

    private:
        static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp);
        CURL* m_curl;
    };

}
