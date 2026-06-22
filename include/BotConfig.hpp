#pragma once
#include <string>

namespace Bot::Config {
    // Токен считывается из переменных окружения для обеспечения безопасности
    inline std::string get_token_from_env() {
        const char* env_token = std::getenv("TELEGRAM_BOT_TOKEN");
        return env_token ? std::string(env_token) : "YOUR_BOT_TOKEN_HERE";
    }
    
    inline const std::string BASE_URL = "https://api.telegram.org/bot";
}
