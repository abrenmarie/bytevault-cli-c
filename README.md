# ByteVault-CLI

A high-performance, low-level steganographic tool written in pure C for macOS and Linux. It allows you to securely hide encrypted files or sensitive text payloads inside uncompressed BMP images using the **Least Significant Bit (LSB)** technique.

## Features
* **Zero Dependencies:** Built using only standard POSIX and C libraries.
* **Direct Binary Parsing:** Manually parses `BITMAPFILEHEADER` and `BITMAPINFOHEADER` byte-by-byte.
* **On-the-Fly Encryption:** Encrypts data using a custom cascading XOR cipher with a dynamic key prior to injection.
* **Anti-Forensic Steganography:** Modifies only the lowest bit of each pixel color channel, making the payload completely invisible to the human eye.
* **Memory Safe:** Strict manual memory management (`malloc`/`free`) with zero leaks.

## Installation & Compilation

Compile the project with a single command using the provided `Makefile`:

```bash
make
```

To clean up build artifacts and temporary files:

```bash
make clean
```

## Usage

1. Hide a Secret File inside an Image (Encode)

```bash
./bytevault -e -i input.bmp -f secret.txt -k YourSecureKey -o hidden.bmp
```

2. Extract the Hidden File (Decode)

```bash
./bytevault -d -i hidden.bmp -k YourSecureKey -o extracted.txt
```

## Technical Overview

The application reads the BMP structure, identifies the pixel data offset via bfOffBits, and embeds 1 bit of the encrypted payload into the youngest bit of each color byte. The first 4 bytes of the embedded stream are reserved for the payload length header, allowing the decoder to stop precisely when all data is recovered.