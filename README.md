# simple-rcon-bot
simple-rcon-bot is a simple rcon telegrm bot for minecraft servers installed as service
![alt text](https://github.com/noauth/simple-rcon-bot/blob/main/logic.jpg)

# Installing:
This guide provide installation for Ubuntu Server

1. Download mcrcon https://github.com/Tiiffi/mcrcon
2. Download inih: ```sudo apt install libinih-dev```
3. Add this lines to /etc/sudoers:
   ```bash
   username ALL=(ALL) NOPASSWD: /usr/bin/systemctl start minecraft
   username ALL=(ALL) NOPASSWD: /usr/bin/systemctl stop minecraft
   username ALL=(ALL) NOPASSWD: /usr/bin/systemctl enable minecraft
   username ALL=(ALL) NOPASSWD: /usr/bin/systemctl disable minecraft
   username ALL=(ALL) NOPASSWD: /usr/bin/systemctl status minecraft
   ```
   Where ```username``` is your user
5. Edit server.properties:
   ```bash
   enable-rcon=true
   rcon.password=<your rcon password>
   ```
6. Add this files to folder where main.cpp is:
   - Blank text file ```authorizedUsers.txt```
   - Config file ```config.ini```

   Add this to ```config.ini```:
   ```bash
   [Telegram]
   bot_api_key = <your api key from @BotFather>
   
   [Secret]
   bot_pswd = <your bot password>
   rcon_pswd = <your rcon password>
   ```
7. Now you can build project:
   ```bash
   g++ main.cpp -o rcon-bot --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lINIReader
   ```

# Run as service:
To use this bot as service, follow these steps:
```bash
cd /etc/systemd/system/
sudo nano rcon.service
```

Add this to ```rcon.service```:
```bash
[Unit]
Description=Rcon Telegram Bot
Wants=network-online.target
After=network-online.target

[Service]
User=username
WorkingDirectory=<path to rcon-bot folder>

ExecStart=<path to rcon-bot executable>

Restart=always
RestartSec=30

StandardInput=null

[Install]
WantedBy=multi-user.target
```
Where ```username``` is your user

Enable and start service:
```bash
systemctl start rcon && systemctl enable rcon
```

