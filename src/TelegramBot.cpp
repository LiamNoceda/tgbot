#include "TelegramBot.hpp"
#include "BotConfig.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace Bot::Core {

    TelegramBot::TelegramBot(std::string token) 
        : m_token(std::move(token)), m_base_url(Config::BASE_URL + m_token) {}

    std::string TelegramBot::find_json_value(std::string_view json, std::string_view key, size_t start_from) const {
        const size_t key_pos = json.find("\"" + std::string(key) + "\"", start_from);
        if (key_pos == std::string_view::npos) return "";

        const size_t colon_pos = json.find(':', key_pos);
        if (colon_pos == std::string_view::npos) return "";

        const size_t start_pos = json.find_first_not_of(" \t\n\r", colon_pos + 1);
        if (start_pos == std::string_view::npos) return "";

        if (json[start_pos] == '"') {
            const size_t end_pos = json.find('"', start_pos + 1);
            if (end_pos == std::string_view::npos) return "";
            return std::string(json.substr(start_pos + 1, end_pos - start_pos - 1));
        } else {
            const size_t end_pos = json.find_first_of(",]}", start_pos);
            if (end_pos == std::string_view::npos) return "";
            return std::string(json.substr(start_pos, end_pos - start_pos));
        }
    }

    void TelegramBot::send_message(const std::string& chat_id, const std::string& text) const {
        std::string encoded_text = m_http_client.url_encode(text);
        std::string url = m_base_url + "/sendMessage?chat_id=" + chat_id + "&text=" + encoded_text;
        m_http_client.send_request(url);
    }

    void TelegramBot::send_business_message(const std::string& chat_id, const std::string& text, const std::string& business_conn_id) const {
        std::string encoded_text = m_http_client.url_encode(text);
        std::string url = m_base_url + "/sendMessage?chat_id=" + chat_id + "&text=" + encoded_text + "&business_connection_id=" + business_conn_id;
        m_http_client.send_request(url);
    }

    void TelegramBot::handle_business_message(std::string_view response, size_t msg_pos) {
        size_t chat_pos = response.find("\"chat\"", msg_pos);
        if (chat_pos == std::string_view::npos) return;

        std::string business_id = find_json_value(response, "business_connection_id", msg_pos);
        std::string chat_block  = std::string(response.substr(chat_pos));
        std::string chat_id     = find_json_value(chat_block, "id");
        std::string user_text   = find_json_value(response, "text", msg_pos);
        std::string first_name  = find_json_value(response, "first_name", msg_pos);

        if (!chat_id.empty() && !user_text.empty() && !business_id.empty()) {
            std::cout << "[Business Chat] Message from " << first_name << ": " << user_text << "\n";
            std::string reply = "Здравствуйте! Я автоматический ассистент. Мой владелец сейчас занят. Он ответит вам лично, как только освободится.";
            send_business_message(chat_id, reply, business_id);
        }
    }

    void TelegramBot::handle_direct_message(std::string_view response, size_t msg_pos) {
        size_t chat_pos = response.find("\"chat\"", msg_pos);
        if (chat_pos == std::string_view::npos) return;

        std::string chat_block = std::string(response.substr(chat_pos));
        std::string chat_id    = find_json_value(chat_block, "id");
        std::string user_text  = find_json_value(response, "text", msg_pos);
        std::string first_name = find_json_value(response, "first_name", msg_pos);

        if (!chat_id.empty() && !user_text.empty()) {
            std::cout << "[Direct Chat] Message from " << first_name << ": " << user_text << "\n";
            std::string reply = (user_text == "/start") 
                ? "Здравствуйте, " + first_name + "! Вы написали моему ассистенту. Оставьте ваш вопрос прямо здесь."
                : "Спасибо! Ваше сообщение принято и передано владельцу аккаунта.";
            send_message(chat_id, reply);
        }
    }

    void TelegramBot::process_updates(std::string_view response) {
        std::string update_id = find_json_value(response, "update_id");
        if (!update_id.empty()) {
            m_last_update_id = std::to_string(std::stoll(update_id) + 1);
        }

        if (response.contains("\"business_message\"")) {
            handle_business_message(response, response.find("\"business_message\""));
        } else if (response.contains("\"message\"")) {
            handle_direct_message(response, response.find("\"message\""));
        }
    }

    void TelegramBot::run() {
        std::cout << "Hybrid Telegram Assistant Service Started (C++23)...\n";
        while (true) {
            std::string url = m_base_url + "/getUpdates?offset=" + m_last_update_id + "&timeout=10";
            std::string response = m_http_client.send_request(url);

            if (response.empty() || !response.contains("\"ok\":true")) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            process_updates(response);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

}
