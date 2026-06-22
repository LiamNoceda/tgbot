#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <chrono>
#include <curl/curl.h>

// Конфигурация (Обязательно вставьте ваш актуальный токен от BotFather)
inline const std::string BOT_TOKEN = "YOUR_BOT_TOKEN_HERE"; // <-- Замените на ваш токен
inline const std::string BASE_URL  = "https://api.telegram.org/bot" + BOT_TOKEN;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    const size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string send_request(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

// 1. Функция отправки обычного сообщения (в личный чат бота)
void send_message(const std::string& chat_id, const std::string& text) {
    CURL* curl = curl_easy_init();
    if (!curl) return;

    char* escaped = curl_easy_escape(curl, text.c_str(), static_cast<int>(text.length()));
    std::string encoded_text(escaped);
    curl_free(escaped);
    curl_easy_cleanup(curl);

    std::string url = BASE_URL + "/sendMessage?chat_id=" + chat_id + "&text=" + encoded_text;
    send_request(url);
}

// 2. Функция отправки бизнес-сообщения (от вашего лица в личный чат)
void send_business_message(const std::string& chat_id, const std::string& text, const std::string& business_conn_id) {
    CURL* curl = curl_easy_init();
    if (!curl) return;

    char* escaped = curl_easy_escape(curl, text.c_str(), static_cast<int>(text.length()));
    std::string encoded_text(escaped);
    curl_free(escaped);
    curl_easy_cleanup(curl);

    std::string url = BASE_URL + "/sendMessage?chat_id=" + chat_id + 
                      "&text=" + encoded_text + 
                      "&business_connection_id=" + business_conn_id;
    send_request(url);
}

std::string find_json_value(std::string_view json, std::string_view key, size_t start_from = 0) {
    const size_t keyPos = json.find("\"" + std::string(key) + "\"", start_from);
    if (keyPos == std::string_view::npos) return "";

    const size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string_view::npos) return "";

    const size_t startPos = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (startPos == std::string_view::npos) return "";

    if (json[startPos] == '"') {
        const size_t endPos = json.find('"', startPos + 1);
        if (endPos == std::string_view::npos) return "";
        return std::string(json.substr(startPos + 1, endPos - startPos - 1));
    } else {
        const size_t endPos = json.find_first_of(",]}", startPos);
        if (endPos == std::string_view::npos) return "";
        return std::string(json.substr(startPos, endPos - startPos));
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    std::cout << "Гибридный Бот-Ассистент (Обычный + Бизнес) на C++23 запущен...\n";
    std::string lastUpdateId = "0";

    while (true) {
        std::string updateUrl = BASE_URL + "/getUpdates?offset=" + lastUpdateId + "&timeout=10";
        std::string response = send_request(updateUrl);

        if (response.empty() || !response.contains("\"ok\":true")) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // Всегда обновляем сдвиг очереди обновлений, чтобы бот не зависал
        std::string updateId = find_json_value(response, "update_id");
        if (!updateId.empty()) {
            lastUpdateId = std::to_string(std::stoll(updateId) + 1);
        }

        // ВЕТКА №1: ОБРАБОТКА ТЕЛЕГРАМ ДЛЯ БИЗНЕСА ---
        if (response.contains("\"business_message\"")) {
            const size_t msgPos = response.find("\"business_message\"");
            size_t chatPos = response.find("\"chat\"", msgPos);
            
            if (chatPos != std::string::npos) {
                std::string businessId  = find_json_value(response, "business_connection_id", msgPos);
                std::string chatBlock   = response.substr(chatPos);
                std::string chatId      = find_json_value(chatBlock, "id");
                std::string userText    = find_json_value(response, "text", msgPos);
                std::string firstName   = find_json_value(response, "first_name", msgPos);

                if (!chatId.empty() && !userText.empty() && !businessId.empty()) {
                    std::cout << "[Бизнес-чат] Сообщение от " << firstName << ": " << userText << "\n";
                    std::string reply = "Здравствуйте! Я автоматический ассистент. Мой владелец сейчас занят. Он ответит вам лично, как только освободится.";
                    send_business_message(chatId, reply, businessId);
                }
            }
        }
        // ВЕТКА №2: ОБРАБОТКА ОБЫЧНЫХ ПРЯМЫХ СООБЩЕНИЙ БОТУ ---
        else if (response.contains("\"message\"")) {
            const size_t msgPos = response.find("\"message\"");
            size_t chatPos = response.find("\"chat\"", msgPos);
            
            if (chatPos != std::string::npos) {
                std::string chatBlock   = response.substr(chatPos);
                std::string chatId      = find_json_value(chatBlock, "id");
                std::string userText    = find_json_value(response, "text", msgPos);
                std::string firstName   = find_json_value(response, "first_name", msgPos);

                if (!chatId.empty() && !userText.empty()) {
                    std::cout << "[Прямой чат] Сообщение от " << firstName << ": " << userText << "\n";
                    std::string reply;
                    if (userText == "/start") {
                        reply = "Здравствуйте, " + firstName + "! Вы написали моему ассистенту. Оставьте ваш вопрос прямо здесь, и владелец свяжется с вами.";
                    } else {
                        reply = "Спасибо! Ваше сообщение принято и передано владельцу аккаунта.";
                    }
                    send_message(chatId, reply);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    curl_global_cleanup();
    return 0;
}
