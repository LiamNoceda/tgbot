#include "TelegramBot.hpp"
#include "BotConfig.hpp"
#include <iostream>

int main() {
    // Глобальная инициализация curl один раз при старте приложения
    curl_global_init(CURL_GLOBAL_ALL);

    try {
        std::string token = Bot::Config::get_token_from_env();
        if (token == "YOUR_BOT_TOKEN_HERE") {
            std::cerr << "Warning: Default token placeholder detected. Ensure TELEGRAM_BOT_TOKEN environment variable is set.\n";
        }

        Bot::Core::TelegramBot bot(token);
        bot.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal runtime crash: " << e.what() << "\n";
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    curl_global_cleanup();
    return EXIT_SUCCESS;
}
