#include "HttpClient.hpp"
#include <stdexcept>

namespace Bot::Network {

    HttpClient::HttpClient() {
        m_curl = curl_easy_init();
        if (!m_curl) {
            throw std::runtime_error("Failed to initialize libcurl handle");
        }
    }

    HttpClient::~HttpClient() {
        if (m_curl) {
            curl_easy_cleanup(m_curl);
        }
    }

    size_t HttpClient::write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        const size_t total_size = size * nmemb;
        userp->append(static_cast<char*>(contents), total_size);
        return total_size;
    }

    std::string HttpClient::send_request(const std::string& url) const {
        std::string response;
        curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 15L);

        CURLcode res = curl_easy_perform(m_curl);
        if (res != CURLE_OK) {
            return ""; // В продакшене здесь должен быть проброс логов или исключения
        }
        return response;
    }

    std::string HttpClient::url_encode(const std::string& text) const {
        char* escaped = curl_easy_escape(m_curl, text.c_str(), static_cast<int>(text.length()));
        if (!escaped) return "";
        std::string result(escaped);
        curl_free(escaped);
        return result;
    }

}
