#include <stdio.h>
#include <tgbot/tgbot.h>
#include <string>
#include <fstream>
#include <cstdio>
#include "INIReader.h"
#include <array>
#include <memory>

using namespace std;
using namespace TgBot;

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result = "";

    // Используем _popen для Windows и popen для POSIX систем (Linux/Mac)
#ifdef _WIN32
    auto pipe = _popen(cmd, "r");
#else
    auto pipe = popen(cmd, "r");
#endif

    if (!pipe) {
        throw runtime_error("popen() failed!");
    }

    // Читаем вывод команды порциями
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    if (result.empty()) {
        result = "Success";
    }

    // Закрываем поток и получаем код завершения
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return result;
}

bool isAuthorized(int64_t userId) {
    ifstream file("authorizedUsers.txt");
    if (!file.is_open()) {
        cerr << "Error: Can't open authorizedUsers.txt" << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        try {
            if (stoll(line) == userId) {
                return true;
            }
        }
        catch (...) {
            continue; // Пропускаем некорректные строки
        }
    }
    return false;
}

// Функция добавления пользователя в файл
void addUserToFile(int64_t userId) {
    ofstream file("authorizedUsers.txt", ios::app); // Открываем для дозаписи (append)
    if (file.is_open()) {
        file << userId << endl;
        file.close();
    }
}

int main() {

    INIReader reader("config.ini");

    Bot bot(reader.GetString("Telegram", "bot_api_key", "Unknown"));

    string correctPassword = reader.GetString("Secret", "bot_pswd", "Unknown"); // Пароль доступа
    string rconPassword = reader.GetString("Secret", "rcon_pswd", "Unknown");

    bot.getEvents().onCommand("start", [&](TgBot::Message::Ptr message) {
        if (isAuthorized(message->from->id)) {
            bot.getApi().sendMessage(message->chat->id, "Authorization successful");
        } else {
            bot.getApi().sendMessage(message->chat->id, "Please log in");
        }
        });

    bot.getEvents().onAnyMessage([&](Message::Ptr message) {
        // Игнорируем команды, чтобы они не обрабатывались дважды
        if (StringTools::startsWith(message->text, "/")) {
            return;
        }

        int64_t userId = message->chat->id;

        if (!isAuthorized(userId)) {
            // Проверка пароля
            if (message->text == correctPassword) {
                addUserToFile(userId);
                bot.getApi().sendMessage(userId, "Successfully authorized");
            }
            else {
                bot.getApi().sendMessage(userId, "Incorrect password");
            }
        }
        else {
            string command = "mcrcon -H 127.0.0.1 -p " + rconPassword + " \"" + message->text + "\"";
            bot.getApi().sendMessage(userId, exec(command.c_str()));
        }
        });

    // Пример защищенной команды
    bot.getEvents().onCommand("on", [&](Message::Ptr message) {
        string command = "sudo systemctl start minecraft && sudo systemctl enable minecraft";
        bot.getApi().sendMessage(message->chat->id, exec(command.c_str()));
        });
    bot.getEvents().onCommand("off", [&](Message::Ptr message) {
        string command = "sudo systemctl stop minecraft && sudo systemctl disable minecraft";
        bot.getApi().sendMessage(message->chat->id, exec(command.c_str()));
        });
    bot.getEvents().onCommand("status", [&](Message::Ptr message) {
        string command = "sudo systemctl status minecraft";
        bot.getApi().sendMessage(message->chat->id, exec(command.c_str()));
        });

    bot.getEvents().onCommand("info", [&](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "/info - available commands\n/on - run server\n/off - stop server\n/status - server status");
        });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}