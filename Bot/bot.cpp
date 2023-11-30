#include "parser.h"
#include <memory>
#include <stdio.h>
#include <tgbot/tgbot.h>
#include <tgbot/types/InlineKeyboardButton.h>
#include <tgbot/types/InlineKeyboardMarkup.h>
#include <tgbot/types/Message.h>

size_t WriteCallback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int main() {
    TgBot::Bot bot("6735976367:AAG05pTJobzB7BFN5tNVEhjhZyboA1CZ3Tk");
    std::map<int64_t, bool> isFindingBook;
    std::map<int64_t, bool> isWaitingBook;
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) 
    {
        bot.getApi().sendMessage(message->chat->id, "Добро пожаловать! Введите команду /find для поиска книг ");
    });
    
    bot.getEvents().onCommand("find", [&bot, &isFindingBook](TgBot::Message::Ptr message) 
    {
        bot.getApi().sendMessage(message->chat->id, "Введите название книги: ");
        isFindingBook[message->chat->id] = true; 
    });
    bot.getEvents().onAnyMessage([&bot, &isFindingBook, &isWaitingBook](TgBot::Message::Ptr message) 
    {
        if(isFindingBook[message->chat->id])
        {
            bot.getApi().sendChatAction(message->chat->id, "typing");
            Parser parser;
            std::vector<book_node> book_list;
            book_list = parser.get_list(message->text);
            auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
            for(const auto& title : book_list)
            {
                std::vector<TgBot::InlineKeyboardButton::Ptr> row;
                //bot.getApi().sendMessage(message->chat->id, title.name);
                auto button = std::make_shared<TgBot::InlineKeyboardButton>();
                std::cout << title.name << std::endl;
                button->text = title.name;
                button->callbackData = title.url + "/fb2";
                row.push_back(button);
                keyboard->inlineKeyboard.push_back(row);
            }
            bot.getApi().sendMessage(message->chat->id, "Выберите книгу:", false, 0, keyboard);
            isFindingBook[message->chat->id] = false;
            isWaitingBook[message->chat->id] = true;
        }
    });
    bot.getEvents().onCallbackQuery([&bot, &isWaitingBook](TgBot::CallbackQuery::Ptr query) {
        if(isWaitingBook[query->from->id] == false)
        {
            return;
        }
        CURL *curl;
        curl = curl_easy_init();
        CURLcode res; 
        isWaitingBook[query->from->id] = false;
        std::string url = query->data;
        std::cout << url << std::endl;
        FILE *fp;
        std::string outfilename = "book_" + std::to_string(query->from->id) + ".zip";
        if (curl) 
        {
            fp = fopen(outfilename.c_str(),"wb");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);
            if (res == CURLE_OK) 
            {
                bot.getApi().sendDocument(query->message->chat->id, TgBot::InputFile::fromFile(outfilename, "application/octet-stream")); 
                remove(outfilename.c_str());
            } 
            else 
            {
                std::cout << "Ошибка отправки файла" << std::endl;
            }
        }
    });
    
    try {
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
