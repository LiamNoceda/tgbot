#pragma once
#include "HttpClient.hpp"
#include <string>
#include <string_view>

namespace Bot::Core {

    class TelegramBot {
    public:
        explicit TelegramBot(std::string token);
        void run();

    private:
        void process_updates(std::string_view response);
        void handle_business_message(std::string_view response, size_t msg_pos);
        void handle_direct_message(std::string_view response, size_t msg_pos);
        
        void send_message(const std::string& chat_id, const std::string& text) const;
        void send_business_message(const std::string& chat_id, const std::string& text, const std::string& business_conn_id) const;

        [[nodiscard]] std::string find_json_value(std::string_view json, std::string_view key, size_t start_from = 0) const;

        std::string m_token;
        std::string m_base_url;
        std::string m_last_update_id{"0"};
        Network::HttpClient m_http_client;
    };

}
