#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Standard library functions (e.g., malloc, free)
#include <string.h>     // String manipulation functions
#include <fcntl.h>      // File control options for open()
#include <unistd.h>     // UNIX standard functions

// Define file permission constants for Windows compatibility
#ifdef _WIN32
    #include <io.h>
    #define S_IRUSR _S_IREAD
    #define S_IWUSR _S_IWRITE
    #define O_BINARY _O_BINARY
#else
    #define O_BINARY 0  // O_BINARY is not needed on non-Windows systems
#endif

// Define constants
#define MAX 100         // Maximum size for arrays (used for binary code representation)
#define isLeaf(node) ((node->l == NULL) && (node->r == NULL))  // Macro to check if a node is a leaf node
#define isroot(node) ((node->f == NULL) && (node->r == NULL) && (node->g != '\0'))  // Macro to check if a node is a root node

// Structure for Huffman Tree nodes used during compression
struct Node {
    char character;     // Character stored in this node
    int freq;           // Frequency of the character
    struct Node *l, *r; // Left and right child pointers
};

// Structure for Min Heap (Priority Queue)
struct Min_Heap {
    int size;           // Current size of the heap
    int capacity;       // Maximum capacity of the heap
    struct Node **array; // Array of node pointers
};

// Structure to store character codes in compressed file
typedef struct code {
    char k;             // Character
    int l;              // Length of the code
    int code_arr[16];   // Array to store binary code (max 16 bits)
    struct code* p;     // Pointer to next code (for linked list)
} code;

// Structure for rebuilding Huffman tree during decompression
typedef struct Tree {
    char g;             // Character stored in this node
    int len;            // Length of code for this character
    int dec;            // Decimal representation of the code
    struct Tree* f;     // Left child (represents 0 in code)
    struct Tree* r;     // Right child (represents 1 in code)
} Tree;

// Global variables
code* data;             // Pointer to current code being processed
code* front = NULL;     // Front of the linked list of codes
code* rear = NULL;      // Rear of the linked list of codes
int k = 0;              // Counter for code linked list initialization
Tree* tree = NULL;      // Root of the Huffman tree during decompression
Tree* tree_temp = NULL; // Temporary tree pointer for traversal
Tree* t = NULL;         // Temporary tree node pointer

// Function to create a new Huffman Tree node
struct Node* newNode(char character, int freq) {
    // Allocate memory for the new node
    struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    // Initialize left and right children as NULL
    temp->l = temp->r = NULL;
    // Set character and frequency values
    temp->character = character;
    temp->freq = freq;
    return temp;
}

// Function to create a min heap with given capacity
struct Min_Heap* createMinHeap(int capacity) {
    // Allocate memory for the min heap structure
    struct Min_Heap* minHeap = (struct Min_Heap*)malloc(sizeof(struct Min_Heap));
    // Initialize size to 0
    minHeap->size = 0;
    // Set the capacity
    minHeap->capacity = capacity;
    // Allocate memory for the array of node pointers
    minHeap->array = (struct Node**)malloc(minHeap->capacity * sizeof(struct Node*));
    return minHeap;
}

// Function to swap two min heap nodes (needed for heap operations)
void swapMinHeapNode(struct Node** a, struct Node** b) {
    struct Node* t = *a;
    *a = *b;
    *b = t;
}

// Standard min heapify function to maintain min heap property
void Heapify(struct Min_Heap* minHeap, int idx) {
    // Initialize smallest as the current index
    int smallest = idx;
    // Calculate indices of left and right children
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    // If left child exists and has smaller frequency than current smallest
    if (left < minHeap->size && 
        minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    // If right child exists and has smaller frequency than current smallest
    if (right < minHeap->size && 
        minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    // If smallest is not the current index, swap and recursively heapify
    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        Heapify(minHeap, smallest);
    }
}

// Function to check if size of heap is 1
int isSizeOne(struct Min_Heap* minHeap) {
    return (minHeap->size == 1);
}

// Function to extract the minimum value node from heap
struct Node* extractMinFromMin_Heap(struct Min_Heap* minHeap) {
    // Store the root node (minimum frequency node)
    struct Node* temp = minHeap->array[0];
    // Replace root with the last node
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    // Decrease the size of the heap
    --minHeap->size;
    // Heapify the root to maintain min heap property
    Heapify(minHeap, 0);
    // Return the extracted node
    return temp;
}

// Function to insert a new node into the Min Heap
void insertIntoMin_Heap(struct Min_Heap* minHeap, struct Node* minHeapNode) {
    // Increase heap size
    ++minHeap->size;
    // Start from the last position
    int i = minHeap->size - 1;
    
    // Move up the tree until finding the right position (upheap)
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    
    // Place the new node at the found position
    minHeap->array[i] = minHeapNode;
}

// Function to build a min heap from an array of nodes
void buildMinHeap(struct Min_Heap* minHeap) {
    int n = minHeap->size - 1;
    int i;
    // Start from last non-leaf node and heapify each node
    for (i = (n - 1) / 2; i >= 0; --i)
        Heapify(minHeap, i);
}

// Function to create and build min heap from character array and frequency array
struct Min_Heap* createAndBuildMin_Heap(char arr[], int freq[], int unique_size) {
    // Create a min heap with capacity equal to unique character count
    struct Min_Heap* minHeap = createMinHeap(unique_size);
    
    // Add all characters and their frequencies to the heap
    for (int i = 0; i < unique_size; ++i)
        minHeap->array[i] = newNode(arr[i], freq[i]);
    
    // Set the size of the heap
    minHeap->size = unique_size;
    
    // Build the min heap
    buildMinHeap(minHeap);
    
    return minHeap;
}

// Function to build Huffman Tree from character and frequency arrays
struct Node* buildHuffmanTree(char arr[], int freq[], int unique_size) {
    struct Node *left, *right, *top;
    
    // Create a min heap with unique characters and their frequencies
    struct Min_Heap* minHeap = createAndBuildMin_Heap(arr, freq, unique_size);

    // Iterate until there is only one node in the heap
    while (!isSizeOne(minHeap)) {
        // Extract two nodes with minimum frequency
        left = extractMinFromMin_Heap(minHeap);
        right = extractMinFromMin_Heap(minHeap);
        
        // Create a new internal node with '$' as character and sum of frequencies
        top = newNode('$', left->freq + right->freq);
        
        // Connect the two nodes as children of the new node
        top->l = left;
        top->r = right;
        
        // Add the new node back to the heap
        insertIntoMin_Heap(minHeap, top);
    }
    
    // The remaining node is the root of the Huffman tree
    return extractMinFromMin_Heap(minHeap);
}

// Function to convert binary array to decimal
int convertBinaryToDecimal(int arr[], int n) {
    int decimal = 0;
    // Convert binary representation to decimal
    for (int i = 0; i < n; i++)
        decimal = decimal * 2 + arr[i];
    return decimal;
}

// Function to convert decimal to binary representation
void convertDecimalToBinary(int bin[], int decimal, int size) {
    // Convert decimal to binary and store in array
    for (int i = size - 1; i >= 0; i--) {
        bin[i] = decimal % 2;
        decimal = decimal / 2;
    }
}

// Function to generate and store Huffman codes in file
void printCodesIntoFile(int fd2, struct Node* root, int arr[], int top) {
    // If there is a left child, add 0 to the code and recur
    if (root->l) {
        arr[top] = 0;
        printCodesIntoFile(fd2, root->l, arr, top + 1);
    }

    // If there is a right child, add 1 to the code and recur
    if (root->r) {
        arr[top] = 1;
        printCodesIntoFile(fd2, root->r, arr, top + 1);
    }

    // If this is a leaf node, store the character and its code
    if (isLeaf(root)) {
        // Allocate memory for the code structure
        data = (code*)malloc(sizeof(code));
        // Allocate memory for the tree node
        t = (Tree*)malloc(sizeof(Tree));
        
        // Initialize code structure
        data->p = NULL;
        data->k = root->character;
        
        // Initialize tree node
        t->g = root->character;
        // Write character to file
        write(fd2, &t->g, sizeof(char));
        
        // Copy binary code to code structure
        for (int i = 0; i < top; i++) {
            data->code_arr[i] = arr[i];
        }
        
        // Store code length
        t->len = top;
        // Write code length to file
        write(fd2, &t->len, sizeof(int));
        
        // Convert binary code to decimal
        t->dec = convertBinaryToDecimal(data->code_arr, top);
        // Write decimal representation to file
        write(fd2, &t->dec, sizeof(int));
        
        // Store code length in code structure
        data->l = top;
        data->p = NULL;
        
        // Add code to linked list
        if (k == 0) {
            // If first code, initialize front and rear
            front = rear = data;
            k++;
        }
        else {
            // Otherwise, add to end of linked list
            rear->p = data;
            rear = rear->p;
        }
    }
}

// Function to compress file using Huffman coding
void compressFile(int fd1, int fd2, unsigned char a) {
    char n;
    int h = 0, i;
    // Reset file pointer to beginning
    lseek(fd1, 0, SEEK_SET);

    // Process each character in the input file
    while (read(fd1, &n, sizeof(char)) != 0) {
        // Find the code for this character
        code* current = front;
        while (current != NULL && current->k != n) {
            current = current->p;
        }
        
        // If code exists, encode the character
        if (current != NULL) {
            // Process each bit in the code
            for (i = 0; i < current->l; i++) {
                if (h < 7) {
                    // If we haven't filled a byte yet
                    if (current->code_arr[i] == 1) {
                        // For a '1' bit, increment and shift
                        a++;
                        a = a << 1;
                        h++;
                    }
                    else if (current->code_arr[i] == 0) {
                        // For a '0' bit, just shift
                        a = a << 1;
                        h++;
                    }
                }
                else if (h == 7) {
                    // If we've reached 8 bits (a full byte)
                    if (current->code_arr[i] == 1) {
                        // For the last bit, increment if it's '1'
                        a++;
                        h = 0;
                    }
                    else {
                        // For the last bit, leave as '0'
                        h = 0;
                    }
                    // Write the byte to the output file
                    write(fd2, &a, sizeof(char));
                    // Reset byte accumulator
                    a = 0;
                }
            }
        }
    }
    
    // Pad the last byte with zeros if needed
    for (i = 0; i < 7 - h; i++) {
        a = a << 1;
    }
    // Write the final byte
    write(fd2, &a, sizeof(char));
}

// Function to read Huffman codes from a compressed file
void ExtractCodesFromFile(int fd1, Tree* t) {
    // Read character
    read(fd1, &t->g, sizeof(char));
    // Read code length
    read(fd1, &t->len, sizeof(int));
    // Read decimal representation of code
    read(fd1, &t->dec, sizeof(int));
}

// Function to rebuild the Huffman tree from the codes in the compressed file
void ReBuildHuffmanTree(int fd1, int size) {
    int i, j, k;
    
    // Allocate memory for the root of the tree
    tree = (Tree*)malloc(sizeof(Tree));
    tree_temp = tree;
    tree->f = NULL;
    tree->r = NULL;
    
    // Allocate memory for temporary node
    t = (Tree*)malloc(sizeof(Tree));
    t->f = NULL;
    t->r = NULL;
    
    // Process each character code
    for (k = 0; k < size; k++) {
        // Start from the root
        tree_temp = tree;
        // Extract code for current character
        ExtractCodesFromFile(fd1, t);
        
        // Arrays for binary representation
        int bin[MAX], bin_con[MAX];
        
        // Initialize arrays
        for (i = 0; i < MAX; i++) {
            bin[i] = bin_con[i] = 0;
        }
        
        // Convert decimal code to binary
        convertDecimalToBinary(bin, t->dec, t->len);
        
        // Copy binary code
        for (i = 0; i < t->len; i++) {
            bin_con[i] = bin[i];
        }

        // Traverse the tree according to the code
        for (j = 0; j < t->len; j++) {
            if (bin_con[j] == 0) {
                // If bit is 0, go left
                if (tree_temp->f == NULL) {
                    // Create left child if it doesn't exist
                    tree_temp->f = (Tree*)malloc(sizeof(Tree));
                    tree_temp->f->f = NULL;
                    tree_temp->f->r = NULL;
                }
                tree_temp = tree_temp->f;
            }
            else if (bin_con[j] == 1) {
                // If bit is 1, go right
                if (tree_temp->r == NULL) {
                    // Create right child if it doesn't exist
                    tree_temp->r = (Tree*)malloc(sizeof(Tree));
                    tree_temp->r->f = NULL;
                    tree_temp->r->r = NULL;
                }
                tree_temp = tree_temp->r;
            }
        }
        
        // At leaf node, store character and code information
        tree_temp->g = t->g;
        tree_temp->len = t->len;
        tree_temp->dec = t->dec;
    }
}

// Function to decompress the compressed file
void decompressFile(int fd1, int fd2, int totalChars) {
    int inp[8], i;
    int charsProcessed = 0;
    unsigned char p;
    // Start from the root of the Huffman tree
    tree_temp = tree;
    
    // Process each byte in the compressed file
    while (read(fd1, &p, sizeof(char)) != 0 && charsProcessed < totalChars) {
        // Convert byte to binary
        convertDecimalToBinary(inp, p, 8);
        
        // Process each bit
        for (i = 0; i < 8 && charsProcessed < totalChars; i++) {
            if (inp[i] == 0) {
                // If bit is 0, go left
                tree_temp = tree_temp->f;
            } else {
                // If bit is 1, go right
                tree_temp = tree_temp->r;
            }
            
            // If we've reached a leaf node
            if (tree_temp->f == NULL && tree_temp->r == NULL) {
                // Write the character to the output file
                write(fd2, &tree_temp->g, sizeof(char));
                // Increment character count
                charsProcessed++;
                // Reset to root for next character
                tree_temp = tree;
            }
        }
    }
}

// Function to compress a file using Huffman coding
void compress(const char* inputFile, const char* outputFile) {
    // Open input file in read-only mode (binary mode for Windows)
    #ifdef _WIN32
        int fd1 = open(inputFile, O_RDONLY | O_BINARY);
    #else
        int fd1 = open(inputFile, O_RDONLY);
    #endif
    
    if (fd1 == -1) {
        perror("Open Failed For Input File");
        exit(1);
    }
    
    // Create output file with write permissions
    #ifdef _WIN32
        int fd2 = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);
    #else
        int fd2 = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    #endif
    
    if (fd2 == -1) {
        perror("Open Failed For Output File");
        close(fd1);
        exit(1);
    }
    
    // Count character frequencies
    char ch;
    int freq[256] = {0};  // Initialize frequency array for all possible characters
    int totalChars = 0;   // Total number of characters in the file
    
    // Read each character and update frequencies
    while (read(fd1, &ch, sizeof(char)) != 0) {
        freq[(unsigned char)ch]++;
        totalChars++;
    }
    
    // Count unique characters
    int uniqueChars = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            uniqueChars++;
        }
    }
    
    // Create arrays for unique characters and their frequencies
    char arr[uniqueChars];
    int freqArr[uniqueChars];
    int index = 0;
    
    // Fill the arrays
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            arr[index] = (char)i;
            freqArr[index] = freq[i];
            index++;
        }
    }
    
    // Write header information to compressed file
    write(fd2, &uniqueChars, sizeof(int));  // Number of unique characters
    write(fd2, &totalChars, sizeof(int));   // Total number of characters
    
    // Build Huffman tree
    struct Node* root = buildHuffmanTree(arr, freqArr, uniqueChars);
    
    // Generate Huffman codes and write to file
    int codeArr[MAX];
    printCodesIntoFile(fd2, root, codeArr, 0);
    
    // Compress the file using generated codes
    compressFile(fd1, fd2, 0);
    
    // Close file descriptors
    close(fd1);
    close(fd2);
    
    printf("File compressed successfully.\n");
}

// Function to decompress a compressed file
void decompress(const char* inputFile, const char* outputFile) {
    // Open compressed file in read-only mode (binary mode for Windows)
    #ifdef _WIN32
        int fd1 = open(inputFile, O_RDONLY | O_BINARY);
    #else
        int fd1 = open(inputFile, O_RDONLY);
    #endif
    
    if (fd1 == -1) {
        perror("Open Failed For Input File");
        exit(1);
    }
    
    // Create output file with write permissions
    #ifdef _WIN32
        int fd2 = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);
    #else
        int fd2 = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    #endif
    
    if (fd2 == -1) {
        perror("Open Failed For Output File");
        close(fd1);
        exit(1);
    }
    
    // Read header information
    int uniqueChars, totalChars;
    read(fd1, &uniqueChars, sizeof(int));  // Number of unique characters
    read(fd1, &totalChars, sizeof(int));   // Total number of characters
    
    // Rebuild Huffman tree from codes in the compressed file
    ReBuildHuffmanTree(fd1, uniqueChars);
    
    // Decompress the file
    decompressFile(fd1, fd2, totalChars);
    
    // Close file descriptors
    close(fd1);
    close(fd2);
    
    printf("File decompressed successfully.\n");
}

// Main function
int main(int argc, char* argv[]) {
    // Check if enough command line arguments are provided
    if (argc < 4) {
        printf("Usage: %s [compress/decompress] [input_file] [output_file]\n", argv[0]);
        return 1;
    }
    
    // Determine operation: compress or decompress
    if (strcmp(argv[1], "compress") == 0) {
        compress(argv[2], argv[3]);
    } else if (strcmp(argv[1], "decompress") == 0) {
        decompress(argv[2], argv[3]);
    } else {
        printf("Invalid command. Use 'compress' or 'decompress'.\n");
        return 1;
    }
    
    return 0;
}
