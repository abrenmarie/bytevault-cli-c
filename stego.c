#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stego.h"

void crypt_data(uint8_t *data, uint32_t data_len, const char *key) {
    uint32_t key_len = strlen(key);
    for (uint32_t i = 0; i < data_len; i++) {
        data[i] ^= key[i % key_len] ^ (i & 0xFF);
    }
}

int encode_stego(const char *img_path, const char *secret_path, const char *key, const char *out_path) {
    FILE *f_img = fopen(img_path, "rb");
    FILE *f_sec = fopen(secret_path, "rb");
    
    if (!f_img || !f_sec) {
        fprintf(stderr, "[-] Error opening files for encoding.\n");
        if (f_img) fclose(f_img);
        if (f_sec) fclose(f_sec);
        return 0;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    fread(&file_header, sizeof(BMPFileHeader), 1, f_img);
    fread(&info_header, sizeof(BMPInfoHeader), 1, f_img);

    fseek(f_sec, 0, SEEK_END);
    uint32_t secret_len = ftell(f_sec);
    fseek(f_sec, 0, SEEK_SET);

    uint32_t available_bytes = info_header.biSizeImage;
    if (available_bytes == 0) {
        available_bytes = (info_header.biWidth * info_header.biHeight) * (info_header.biBitCount / 8);
    }

    if ((secret_len + 4) * 8 > available_bytes) {
        fprintf(stderr, "[-] Error: Image is too small to hide this file.\n");
        fclose(f_img); fclose(f_sec);
        return 0;
    }

    uint32_t payload_len = secret_len + 4;
    uint8_t *payload = (uint8_t *)malloc(payload_len);

    memcpy(payload, &secret_len, 4);
    fread(payload + 4, 1, secret_len, f_sec);
    fclose(f_sec);

    crypt_data(payload, payload_len, key);

    uint8_t *pixels = (uint8_t *)malloc(available_bytes);
    fseek(f_img, file_header.bfOffBits, SEEK_SET);
    fread(pixels, 1, available_bytes, f_img);

    uint32_t pixel_idx = 0;
    for (uint32_t i = 0; i < payload_len; i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint8_t current_bit = (payload[i] >> bit) & 1;
            pixels[pixel_idx] = (pixels[pixel_idx] & 0xFE) | current_bit;
            pixel_idx++;
        }
    }

    FILE *f_out = fopen(out_path, "wb");
    if (!f_out) {
        fprintf(stderr, "[-] Error creating output file.\n");
        free(payload); free(pixels); fclose(f_img);
        return 0;
    }

    fseek(f_img, 0, SEEK_SET);
    uint8_t *headers = (uint8_t *)malloc(file_header.bfOffBits);
    fread(headers, 1, file_header.bfOffBits, f_img);
    fwrite(headers, 1, file_header.bfOffBits, f_out);

    fwrite(pixels, 1, available_bytes, f_out);

    printf("[+] Success! Data hidden in '%s'\n", out_path);

    free(payload); free(pixels); free(headers);
    fclose(f_img); fclose(f_out);
    return 1;
}

int decode_stego(const char *img_path, const char *key, const char *out_path) {
    FILE *f_img = fopen(img_path, "rb");
    if (!f_img) {
        fprintf(stderr, "[-] Error opening image for decoding.\n");
        return 0;
    }

    BMPFileHeader file_header;
    fread(&file_header, sizeof(BMPFileHeader), 1, f_img);

    fseek(f_img, file_header.bfOffBits, SEEK_SET);

    uint8_t len_buffer[4] = {0};
    
    for (int i = 0; i < 4; i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint8_t pixel;
            fread(&pixel, 1, 1, f_img);
            uint8_t extracted_bit = pixel & 1;
            len_buffer[i] |= (extracted_bit << bit);
        }
    }

    crypt_data(len_buffer, 4, key);
    uint32_t secret_len;
    memcpy(&secret_len, len_buffer, 4);

    if (secret_len > 100 * 1024 * 1024) { 
        fprintf(stderr, "[-] Error: Failed to decode. Wrong encryption key or corrupted image.\n");
        fclose(f_img);
        return 0;
    }

    uint32_t payload_len = secret_len + 4;
    uint8_t *payload = (uint8_t *)malloc(payload_len);

    fseek(f_img, file_header.bfOffBits, SEEK_SET);

    memset(payload, 0, payload_len);
    for (uint32_t i = 0; i < payload_len; i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint8_t pixel;
            fread(&pixel, 1, 1, f_img);
            uint8_t extracted_bit = pixel & 1;
            payload[i] |= (extracted_bit << bit);
        }
    }
    fclose(f_img);

    crypt_data(payload, payload_len, key);

    FILE *f_out = fopen(out_path, "wb");
    if (!f_out) {
        fprintf(stderr, "[-] Error writing extracted file.\n");
        free(payload);
        return 0;
    }
    fwrite(payload + 4, 1, secret_len, f_out);
    fclose(f_out);

    printf("[+] Success! Secret extracted and saved to '%s'\n", out_path);
    free(payload);
    return 1;
}