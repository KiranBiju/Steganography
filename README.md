# Image Steganography in C

A command-line based **Image Steganography** application developed in C that hides secret files inside BMP images using the **Least Significant Bit (LSB) technique**.

The project supports both encoding and decoding operations, allowing users to securely hide information inside an image and retrieve it whenever required.

## 📌 Project Overview

Steganography is the technique of hiding secret information inside an ordinary file so that the existence of the information itself remains concealed.

In this project, a secret file is embedded into a 24-bit BMP image by modifying the Least Significant Bit (LSB) of its pixel data.

Since only the least significant bit of each image byte is modified, the visual difference between the original image and the encoded image is generally imperceptible.

The application provides two operations:

- **Encoding:** Hides a secret file inside a BMP image and generates a stego image.
- **Decoding:** Extracts the hidden secret file from the stego image and reconstructs it.

## ⚙️ Technologies & Concepts Used

- **Programming Language:** C
- **File Handling:** `fopen()`, `fclose()`, `fread()`, `fwrite()`, `fgetc()`, `fputc()`
- **Command-Line Arguments:** `argc`, `argv`
- **String Handling:** `strcmp()`, `strcpy()`, `strrchr()`, `strlen()`
- **Bitwise Operations:** Bitwise AND, OR, NOT, left shift and right shift
- **Pointers and Structures:** Managing file pointers and project data
- **Dynamic File Processing:** Reading and writing binary files
- **LSB (Least Significant Bit) Steganography:** Embedding and extracting secret information

## 🧠 Working Principle

The project uses the Least Significant Bit technique to embed secret information into the pixel data of a BMP image.

Each byte of secret information contains 8 bits. These bits are embedded individually into the LSBs of 8 image bytes.

### Encoding Example

Suppose an image byte is:

```text
Original image byte:  10110100
Secret bit:                   1
Modified image byte:  10110101
```

Only the least significant bit is modified.

By repeating this process, the entire secret file can be embedded into the image.

### Decoding Example

The decoder reads 8 image bytes, extracts their LSBs and combines them to reconstruct one original secret byte.

```text
Extracted bits: 00100011

Recovered byte: 00100011
Decimal value:  35
ASCII character: #
```

## 🔐 Encoding Process

The encoding operation follows these steps:

1. Validate command-line arguments and file extensions.
2. Open the source BMP image, secret file and output image.
3. Calculate the image capacity and required encoding space.
4. Copy the original BMP header into the output image.
5. Encode the magic string (`#*`) to identify the presence of hidden data.
6. Encode the secret file extension length.
7. Encode the secret file extension.
8. Encode the secret file size.
9. Encode the actual secret file contents.
10. Copy the remaining image data into the output image.
11. Close all opened files and report the encoding status.

### Embedded Data Layout

The following information is stored sequentially in the image:

| Order | Information |
|---|---|
| 1 | Magic string (`#*`) |
| 2 | Secret file extension length |
| 3 | Secret file extension |
| 4 | Secret file size |
| 5 | Secret file contents |

## 🔓 Decoding Process

The decoding operation reverses the encoding process:

1. Validate the encoded BMP filename and optional output filename.
2. Open the stego image.
3. Skip the BMP header and move to the pixel data.
4. Extract and verify the magic string.
5. Extract the secret file extension length.
6. Extract the secret file extension.
7. Generate the output filename if one was not supplied.
8. Extract the secret file size.
9. Decode the secret contents using LSB extraction.
10. Write the recovered bytes into the output secret file.
11. Close all opened files and report the decoding status.

## 📂 Project Structure

```text
Steganography/
│
├── main.c
│
├── common.h
│
├── encode.h
├── encode.c
│
├── decode.h
├── decode.c
│
├── beautiful.bmp
├── secret.txt
│
├── stego.bmp
│
└── README.md
```

### File Description

| File | Description |
|---|---|
| `main.c` | Entry point; handles command-line arguments and selects the operation. |
| `common.h` | Contains common macros, enumerations and status definitions. |
| `encode.h` | Contains encoding structure definitions and function declarations. |
| `encode.c` | Implements the encoding process. |
| `decode.h` | Contains decoding structure definitions and function declarations. |
| `decode.c` | Implements the decoding process. |

## 💻 Compilation

Compile the project using GCC:

```bash
gcc -Wall -Wextra main.c encode.c decode.c -o a.out
```

## 🚀 Execution

### 1. Encoding

**Syntax:**

```bash
./a.out -e <source_image.bmp> <secret_file> [output_image.bmp]
```

**Example:**

```bash
./a.out -e beautiful.bmp secret.txt stego.bmp
```

This embeds `secret.txt` inside `beautiful.bmp` and generates `stego.bmp`.

If an output image filename is not specified, the default filename is `stego.bmp`.

### 2. Decoding

**Syntax:**

```bash
./a.out -d <stego_image.bmp> [output_secret_file]
```

**Example 1 — Default output filename:**

```bash
./a.out -d stego.bmp
```

The decoder extracts the embedded extension and generates an output filename using the `decoded` prefix.

For example:

```text
decoded.txt
```

**Example 2 — Custom output filename:**

```bash
./a.out -d stego.bmp recovered.txt
```

The recovered secret contents are written into `recovered.txt`.

## 📊 Capacity Calculation

Each image byte stores one secret bit.

Therefore:

```text
Required image bytes =
(
    Magic string length
    + Extension length field size
    + Secret extension length
    + Secret file size field size
    + Secret file size
) × 8
```

For example, consider a secret file containing 100 bytes with a `.txt` extension.

```text
Magic string       = 2 bytes
Extension length   = 4 bytes
Extension          = 4 bytes
Secret file size   = 4 bytes
Secret contents    = 100 bytes

Total              = 114 bytes
Required capacity  = 114 × 8

                   = 912 image bytes
```

The image must have sufficient capacity to store the complete payload.

## 🎯 Key Learnings

Through this project, I gained practical experience in:

- Understanding and implementing the Least Significant Bit (LSB) steganography technique.
- Performing bitwise operations to manipulate individual bits.
- Understanding how binary files are handled in C.
- Working with file pointers and binary file I/O operations.
- Using structures to organize project-related information.
- Passing data between functions using pointers.
- Understanding command-line argument handling.
- Implementing modular programming through header and source files.
- Designing encoding and decoding algorithms that work in reverse order.
- Handling file-opening failures, invalid inputs and insufficient image capacity.
- Understanding how data can be represented and reconstructed at the bit level.

## ⚠️ Limitations

- The implementation is designed around conventional 24-bit BMP images.
- The current implementation assumes a 54-byte BMP header.
- Image capacity calculation is based on width × height × 3 and does not account for BMP row padding or other BMP variations.
- The secret file size is stored in a four-byte integer, limiting the supported file size.
- LSB steganography is not encryption. Anyone who knows the embedding format can potentially extract the hidden information.
- Modifying or compressing the encoded image using lossy formats can destroy the embedded data.

## 🔮 Future Improvements

- Support additional image formats where suitable lossless pixel access is available.
- Improve BMP header and pixel-data offset handling.
- Add encryption before embedding secret information.
- Implement stronger validation and error handling.
- Support hiding multiple files inside a single image.
- Improve capacity estimation and support larger secret files.

---

## 👨‍💻 Author

**Kiran Biju** - Aspiring Embedded Systems Engineer

B.Tech – Electrical and Electronics Engineering

Developed as part of the Embedded Systems learning journey, focusing on C programming, pointers, structures, bitwise operations and file handling.
