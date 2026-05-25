#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "stego.h"

void print_usage(const char *prog_name) {
    printf("=========================================================\n");
    printf("  ByteVault-CLI v1.0 | Steganographic Crypto Container\n");
    printf("=========================================================\n");
    printf("Usage:\n");
    printf("  Encode (Hide): %s -e -i <image.bmp> -f <secret.txt> -k <key> -o <output.bmp>\n", prog_name);
    printf("  Decode (Extract): %s -d -i <image.bmp> -k <key> -o <output.txt>\n\n", prog_name);
    printf("Options:\n");
    printf("  -e          Encode mode (Hide data)\n");
    printf("  -d          Decode mode (Extract data)\n");
    printf("  -i <file>   Input BMP image path\n");
    printf("  -f <file>   Secret file path (only for encoding)\n");
    printf("  -k <string> Encryption key\n");
    printf("  -o <file>   Output file path\n");
    printf("=========================================================\n");
}

int main(int argc, char *argv[]) {
    int opt;
    int mode = 0; // 1 = Encode, 2 = Decode
    char *input_img = NULL;
    char *secret_file = NULL;
    char *key = NULL;
    char *output_file = NULL;

    while ((opt = getopt(argc, argv, "edi:f:k:o:")) != -1) {
        switch (opt) {
            case 'e':
                mode = 1;
                break;
            case 'd':
                mode = 2;
                break;
            case 'i':
                input_img = optarg;
                break;
            case 'f':
                secret_file = optarg;
                break;
            case 'k':
                key = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (mode == 0 || !input_img || !key || !output_file) {
        fprintf(stderr, "[-] Error: Missing required arguments.\n\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (mode == 1 && !secret_file) {
        fprintf(stderr, "[-] Error: Encode mode requires a secret file (-f).\n\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    printf("[*] Initialization successful.\n");
    if (mode == 1) {
        printf("[+] Mode: ENCODE (Hiding data)\n");
        printf("[+] Target Image: %s\n", input_img);
        printf("[+] Secret Payload: %s\n", secret_file);
        printf("[+] Output Image: %s\n", output_file);
    } else {
        printf("[+] Mode: DECODE (Extracting data)\n");
        printf("[+] Source Image: %s\n", input_img);
        printf("[+] Output Payload: %s\n", output_file);
    }
    printf("[*] Cryptographic key set: [PROTECTED]\n");

    if (mode == 1) {
        if (!encode_stego(input_img, secret_file, key, output_file)) {
            return EXIT_FAILURE;
        }
    } else if (mode == 2) {
        if (!decode_stego(input_img, key, output_file)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}