#include <Arduino.h>
#include <SD.h>

#define DEBUG 0

#define SERIAL_BAUD 115200

#define COMMAND_START_SYMBOL '#'

#define SD_CS 10
bool SD_initialised = false;
char current_path[100] = "/";

#define BUZZER_PIN 5

void init_SD();
void serial_menu();
void list_files(const char *ls_path = nullptr);
void change_dir(const char *path);
void play(const char *file);
void normalize_path(char *path);
void make_absolute_path(const char *relative, char *abs_path, size_t size);
void play_custom(const char *file);

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println();

    init_SD();

    pinMode(BUZZER_PIN, OUTPUT);

    Serial.println(F("Input command! (#help for help)"));
    Serial.println(F("#<command> [arguments]\n"));
}

void loop() {
    serial_menu();
}

void serial_menu() {
    if (Serial.available()) {
        char command[128];
        int len = Serial.readBytesUntil('\n', command, sizeof(command) - 1);
        command[len] = '\0';

        // Эхо введенной команды
        Serial.println(command);

        if (command[0] == '\0') return;
        else if (command[0] == COMMAND_START_SYMBOL) {
            char *cmd = command + 1;
            while (*cmd == ' ') cmd++;
            char *arg = strchr(cmd, ' ');
            if (arg) {
                *arg = '\0';
                arg++;
                while (*arg == ' ') arg++;
                if (*arg == '\0') arg = nullptr;
            }

#if DEBUG == 1
            Serial.print(F("Command: "));
            Serial.println(cmd);
            Serial.print(F("Arg: "));
            Serial.println(arg ? arg : "null");
            Serial.print(F("\n"));
#endif

            if (strcmp(cmd, "help") == 0) {
                Serial.println(F("Help\n================="));
                Serial.println(F("help - this message"));
                Serial.println(F("convert - site and instructions to convert midi"));
                Serial.println(F("ls [path] - list files"));
                Serial.println(F("cd [path] - change dir"));
                Serial.println(F("pwd - show current dir"));
                Serial.println(F("play <file> - play melody file"));
                Serial.println();
            }
            else if (strcmp(cmd, "convert") == 0) {
                Serial.println(F("Site: https://www.extramaster.net/tools/midiToArduino/"));
                Serial.println(F("Select midi, then select channel, then select Raw"));
                Serial.println(F("Then paste got text to text file"));
            }
            else if (strcmp(cmd, "ls") == 0) {
                list_files(arg);
            }
            else if (strcmp(cmd, "cd") == 0) {
                change_dir(arg);
            }
            else if (strcmp(cmd, "pwd") == 0) {
                Serial.print(F("Current dir: "));
                Serial.println(current_path);
            }
            else if (strcmp(cmd, "play") == 0) {
                if (!arg) {
                    Serial.println(F("Error: Missing filename"));
                    return;
                }

                char abs_path[128];
                if (arg[0] == '/') {
                    strlcpy(abs_path, arg, sizeof(abs_path));
                } else {
                    make_absolute_path(arg, abs_path, sizeof(abs_path));
                }
                play(abs_path);
            }
            else {
                Serial.println(F("Error: Unknown command"));
            }
        } else {
            Serial.println(F("Error: Use # prefix for commands"));
        }
    }
}

void init_SD() {
    pinMode(SD_CS, OUTPUT);
    if (SD.begin(SD_CS)) {
        Serial.println(F("SD card initialized"));
        SD_initialised = true;
    } else {
        Serial.println(F("Error: SD card initialization failed"));
        SD_initialised = false;
    }
}

void list_files(const char *ls_path) {
    if (!SD_initialised) {
        Serial.println(F("Error: SD card not initialized"));
        return;
    }

    char path[100];
    if (!ls_path || ls_path[0] == '\0') {
        strlcpy(path, current_path, sizeof(path));
    } else {
        make_absolute_path(ls_path, path, sizeof(path));
    }

    File dir = SD.open(path);
    if (!dir) {
        Serial.print(F("Error: Cannot open "));
        Serial.println(path);
        return;
    }
    if (!dir.isDirectory()) {
        Serial.print(F("Error: Not a directory - "));
        Serial.println(path);
        dir.close();
        return;
    }

    Serial.print(F("Listing: "));
    Serial.println(path);
    Serial.println(F("================="));

    File entry = dir.openNextFile();
    while (entry) {
        if (entry.isDirectory()) {
            Serial.print(F("[DIR]\t"));
        } else {
            Serial.print(F("[FILE]\t"));
            Serial.print(entry.size());
            Serial.print(F(" bytes\t"));
        }
        Serial.println(entry.name());
        entry.close();
        entry = dir.openNextFile();
    }
    dir.close();
    Serial.println(F("=================\n"));
}

void make_absolute_path(const char *relative, char *abs_path, size_t size) {
    if (relative[0] == '/') {
        strlcpy(abs_path, relative, size);
    } else {
        if (strcmp(current_path, "/") == 0) {
            snprintf(abs_path, size, "/%s", relative);
        } else {
            snprintf(abs_path, size, "%s/%s", current_path, relative);
        }
    }
    normalize_path(abs_path);
}

void normalize_path(char *path) {
    char temp_path[128];
    strlcpy(temp_path, path, sizeof(temp_path));

    char *parts[20];
    int count = 0;

    char *ptr = strtok(temp_path, "/");
    while (ptr != nullptr && count < 19) {
        if (strcmp(ptr, ".") == 0) {
            // Пропустить
        } else if (strcmp(ptr, "..") == 0) {
            if (count > 0) count--;
        } else {
            parts[count++] = ptr;
        }
        ptr = strtok(nullptr, "/");
    }

    path[0] = '\0';
    if (count == 0) {
        strcpy(path, "/");
        return;
    }

    for (int i = 0; i < count; i++) {
        strcat(path, "/");
        strcat(path, parts[i]);
    }
}

void change_dir(const char *path) {
    if (!SD_initialised) {
        Serial.println(F("Error: SD card not initialized"));
        return;
    }

    char new_path[100];
    if (!path || path[0] == '\0') {
        strcpy(new_path, "/");
    } else {
        make_absolute_path(path, new_path, sizeof(new_path));
    }

    File dir = SD.open(new_path);
    if (!dir) {
        Serial.print(F("Error: Directory not found - "));
        Serial.println(new_path);
        return;
    }
    if (!dir.isDirectory()) {
        Serial.print(F("Error: Not a directory - "));
        Serial.println(new_path);
        dir.close();
        return;
    }
    dir.close();

    strlcpy(current_path, new_path, sizeof(current_path));
    Serial.print(F("Current dir: "));
    Serial.println(current_path);
}

void play(const char *file) {
    if (!SD_initialised) {
        Serial.println(F("Error: SD card not initialized"));
        return;
    }
    play_custom(file);
}

void play_custom(const char *file) {
    File f = SD.open(file);
    if (!f) {
        Serial.print(F("Error: Cannot open file "));
        Serial.println(file);
        return;
    }

    Serial.print(F("Playing: "));
    Serial.println(file);
    Serial.println(F("Press any key to stop..."));

    char buffer[128];
    int lineNum = 0;
    bool playbackStopped = false;

    while (f.available() && !playbackStopped) {
        // Проверка на остановку
        if (Serial.available()) {
            Serial.read();
            Serial.println(F("Playback stopped"));
            playbackStopped = true;
            break;
        }

        // Чтение строки
        int len = f.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        if (len <= 0) continue;

        buffer[len] = '\0';
        // Удаление возврата каретки, если есть
        if (buffer[len - 1] == '\r') {
            buffer[len - 1] = '\0';
            len--;
        }

        lineNum++;

        // Пропуск пустых строк и комментариев
        if (buffer[0] == '#' || buffer[0] == '\0') {
            continue;
        }

        // Разделение строки на токены
        char *tokens[4];
        int tokenCount = 0;

        char *ptr = strtok(buffer, ",");
        while (ptr != nullptr && tokenCount < 4) {
            // Удаление пробелов вокруг токена
            while (*ptr == ' ') ptr++;
            char *end = ptr + strlen(ptr) - 1;
            while (end > ptr && *end == ' ') *end-- = '\0';

            tokens[tokenCount++] = ptr;
            ptr = strtok(nullptr, ",");
        }

        // Обработка строки
        if (tokenCount >= 2) {
            char eventType = tokens[0][0];

            if (eventType == 'T' && tokenCount >= 4) {
                // Извлечение параметров тона
                float freq = atof(tokens[1]);
                float actualDuration = atof(tokens[3]);

                unsigned long duration_ms = static_cast<unsigned long>(actualDuration + 0.5f);

                if (freq > 0) {
                    tone(BUZZER_PIN, static_cast<int>(freq));
                }

                // Ожидание с проверкой прерывания
                unsigned long start = millis();
                while (millis() - start < duration_ms) {
                    if (Serial.available()) {
                        Serial.read();
                        Serial.println(F("Playback stopped"));
                        playbackStopped = true;
                        break;
                    }
                    delay(10);
                }

                noTone(BUZZER_PIN);

            } else if (eventType == 'D') {
                // Извлечение длительности задержки
                float delayDuration = atof(tokens[1]);
                unsigned long delay_ms = static_cast<unsigned long>(delayDuration + 0.5f);

                // Ожидание с проверкой прерывания
                unsigned long start = millis();
                while (millis() - start < delay_ms) {
                    if (Serial.available()) {
                        Serial.read();
                        Serial.println(F("Playback stopped"));
                        playbackStopped = true;
                        break;
                    }
                    delay(10);
                }
            }
        }

        if (playbackStopped) break;
    }

    noTone(BUZZER_PIN);
    f.close();

    if (!playbackStopped) {
        Serial.println(F("Playback finished"));
    }
}
