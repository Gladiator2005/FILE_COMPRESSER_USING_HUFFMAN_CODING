# FILE_COMPRESSER_USING_HUFFMAN_CODING


This project implements **Huffman Coding**, a lossless data compression algorithm, in C. It can compress and decompress text files efficiently using character frequency analysis and binary tree encoding.

---

### 🔧 Features

* Compresses a text file using Huffman Coding.
* Decompresses the compressed file back to its original content.
* Builds a binary Huffman tree based on character frequency.
* Stores the Huffman codes and compressed data in a single file.
* Reconstructs the Huffman tree from encoded metadata during decompression.

---

### 📁 File Structure

| File Name          | Description                                                     |
| ------------------ | --------------------------------------------------------------- |
| `main.c`           | Main program logic with compression and decompression functions |
| `input.txt`        | Sample input text file to compress                              |
| `output.huff`      | Compressed binary file with metadata                            |
| `decompressed.txt` | Output after decompression                                      |
| `README.md`        | This documentation file                                         |

---

### 🛠 How It Works

#### 🗜 Compression Steps:

1. Count frequency of each character in the input.
2. Build a Huffman tree using a min-heap.
3. Generate binary codes for each character.
4. Store the code table and encoded data in a `.huff` file.

#### 📂 Decompression Steps:

1. Read metadata from the compressed file.
2. Rebuild the Huffman tree.
3. Decode the binary data using the tree.
4. Write the original content to a `.txt` file.

---

### 🚀 How to Run

#### 1. **Compile the program**

```bash
gcc main.c -o huffman
```

#### 2. **Compress a file**

```bash
./huffman compress input.txt output.huff
```

#### 3. **Decompress the file**

```bash
./huffman decompress output.huff decompressed.txt
```

---

### 🧪 Sample Output

**Input File (`input.txt`)**

```
hello huffman
```

**Compressed File (`output.huff`)**

* Contains binary data + Huffman code metadata.

**Decompressed File (`decompressed.txt`)**

```
hello huffman
```

---

### 📚 Concepts Used

* **Huffman Coding**
* **Min Heap / Priority Queue**
* **Binary Tree Traversal**
* **Bitwise File I/O**
* **Dynamic Memory Allocation**

---

### 🧑‍💻 Authors

* Developed by \[Your Name]
* Guided by Huffman coding algorithm and file systems concepts

---

### 📜 License

This project is licensed under the MIT License - feel free to use, modify, and distribute.

