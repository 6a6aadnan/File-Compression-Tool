#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <memory>
#include <stdexcept>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>

struct Node {
    char ch;
    int freq;
    std::shared_ptr<Node> left, right;

    Node(char character, int frequency)
        : ch(character), freq(frequency), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(std::shared_ptr<Node> left, std::shared_ptr<Node> right) {
        return left->freq > right->freq;
    }
};

std::shared_ptr<Node> buildHuffmanTree(const std::unordered_map<char, int>& freqMap) {
    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, Compare> minHeap;
    for (const auto& pair : freqMap) {
        minHeap.push(std::make_shared<Node>(pair.first, pair.second));
    }
    
    while (minHeap.size() > 1) {
        auto left = minHeap.top(); minHeap.pop();
        auto right = minHeap.top(); minHeap.pop();
        auto merged = std::make_shared<Node>('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        minHeap.push(merged);
    }

    return minHeap.top();
}

void generateCodes(const std::shared_ptr<Node>& root, std::unordered_map<char, std::string>& huffmanCode, const std::string& str) {
    if (!root) return;

    if (!root->left && !root->right) {
        huffmanCode[root->ch] = str;
    }

    generateCodes(root->left, huffmanCode, str + "0");
    generateCodes(root->right, huffmanCode, str + "1");
}

void compressFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to open input file.");
    }

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open output file.");
    }

    std::unordered_map<char, int> freqMap;
    char ch;
    while (inFile.get(ch)) {
        freqMap[ch]++;
    }
    inFile.clear();
    inFile.seekg(0, std::ios::beg);

    auto root = buildHuffmanTree(freqMap);

    std::unordered_map<char, std::string> huffmanCode;
    generateCodes(root, huffmanCode, "");

    for (const auto& pair : freqMap) {
        outFile << pair.first << pair.second << '\n';
    }
    outFile << "END\n";

    std::string encodedString;
    while (inFile.get(ch)) {
        encodedString += huffmanCode[ch];
    }

    while (encodedString.size() % 8 != 0) {
        encodedString += '0';
    }

    for (size_t i = 0; i < encodedString.size(); i += 8) {
        std::bitset<8> bits(encodedString.substr(i, 8));
        outFile.put(static_cast<unsigned char>(bits.to_ulong()));
    }
}

void uploadToS3(const std::string& fileName, const std::string& bucketName) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::Client::ClientConfiguration config;
        config.region = "us-east-1";  // Change to your desired AWS region

        Aws::S3::S3Client s3_client(config);

        const Aws::String awsBucketName = bucketName.c_str();
        const Aws::String awsKeyName = fileName.c_str();

        Aws::S3::Model::PutObjectRequest object_request;
        object_request.WithBucket(awsBucketName).WithKey(awsKeyName);

        std::shared_ptr<Aws::IOStream> input_data = Aws::MakeShared<Aws::FStream>("SampleAllocationTag", fileName.c_str(), std::ios_base::in | std::ios_base::binary);
        object_request.SetBody(input_data);

        auto put_object_outcome = s3_client.PutObject(object_request);

        if (!put_object_outcome.IsSuccess()) {
            auto error = put_object_outcome.GetError();
            std::cerr << "Error: Upload failed: " << error.GetExceptionName() << ": " << error.GetMessage() << std::endl;
        } else {
            std::cout << "Upload succeeded." << std::endl;
        }
    }
    Aws::ShutdownAPI(options);
}

void downloadFromS3(const std::string& fileName, const std::string& bucketName) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::Client::ClientConfiguration config;
        config.region = "us-east-1";  // Change to your desired AWS region

        Aws::S3::S3Client s3_client(config);

        const Aws::String awsBucketName = bucketName.c_str();
        const Aws::String awsKeyName = fileName.c_str();

        Aws::S3::Model::GetObjectRequest object_request;
        object_request.WithBucket(awsBucketName).WithKey(awsKeyName);

        auto get_object_outcome = s3_client.GetObject(object_request);

        if (get_object_outcome.IsSuccess()) {
            auto& retrieved_file = get_object_outcome.GetResultWithOwnership().GetBody();

            std::ofstream local_file;
            local_file.open(fileName, std::ios::out | std::ios::binary);

            local_file << retrieved_file.rdbuf();

            local_file.close();
            std::cout << "Download succeeded." << std::endl;
        } else {
            auto error = get_object_outcome.GetError();
            std::cerr << "Error: Download failed: " << error.GetExceptionName() << ": " << error.GetMessage() << std::endl;
        }
    }
    Aws::ShutdownAPI(options);
}

void decompressFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to open input file.");
    }

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open output file.");
    }

    std::unordered_map<char, int> freqMap;
    char ch;
    int freq;
    while (inFile >> ch >> freq) {
        freqMap[ch] = freq;
    }
    inFile.ignore(4, '\n');

    auto root = buildHuffmanTree(freqMap);

    auto currentNode = root;
    char byte;
    while (inFile.get(byte)) {
        std::bitset<8> bits(byte);
        for (int i = 0; i < 8; ++i) {
            currentNode = bits.test(7 - i) ? currentNode->right : currentNode->left;
            if (!currentNode->left && !currentNode->right) {
                outFile.put(currentNode->ch);
                currentNode = root;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <compress/decompress> <input file/cloud file name> <output file> <bucket name>\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string fileName = argv[2];
    std::string outputFile = argv[3];
    std::string bucketName = argv[4];

    try {
        if (mode == "compress") {
            std::string compressedFile = "compressed_file.dat";
            compressFile(fileName, compressedFile);
            uploadToS3(compressedFile, bucketName);
            std::remove(compressedFile.c_str()); // Remove the local compressed file after upload
        } else if (mode == "decompress") {
            downloadFromS3(fileName, bucketName);
            decompressFile(fileName, outputFile);
            std::remove(fileName.c_str()); // Remove the downloaded compressed file after decompression
        } else {
            std::cerr << "Invalid mode. Use 'compress' or 'decompress'.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
