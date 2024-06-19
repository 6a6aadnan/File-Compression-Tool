# File-Compression-Tool

This project demonstrates file compression and decompression using Huffman coding, integrated with AWS S3 for cloud storage.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Dependencies](#dependencies)
- [Usage](#usage)
  - [Prerequisites](#prerequisites)
  - [Running the Program](#running-the-program)
    - [Compression](#compression)
    - [Decompression](#decompression)
- [AWS S3 Integration](#aws-s3-integration)
- [Contributing](#contributing)

## Introduction

Huffman coding is a popular algorithm used for lossless data compression. This project implements Huffman coding to compress and decompress files. It also integrates with AWS S3 for storing and retrieving compressed files from the cloud.

## Features

- **Huffman Coding**:
  - Constructs Huffman tree based on character frequencies.
  - Generates Huffman codes for efficient variable-length encoding.
  - Compresses files using generated Huffman codes.

- **AWS S3 Integration**:
  - Uploads compressed files to an AWS S3 bucket.
  - Downloads compressed files from AWS S3 for decompression.

## Dependencies

- **AWS SDK for C++**: Ensure you have the AWS SDK installed and configured. See [AWS SDK for C++ Installation Guide](https://github.com/aws/aws-sdk-cpp#installation) for instructions.
- **C++ Standard Libraries**: Standard headers like `<iostream>`, `<fstream>`, `<unordered_map>`, etc.

## Usage

### Prerequisites

Before running the program:
- Configure AWS credentials either via environment variables or AWS CLI configuration.
- Install AWS SDK for C++ as per the [installation guide](https://github.com/aws/aws-sdk-cpp#installation).

### Running the Program

#### Compression

To compress a file and upload it to AWS S3:
```bash
./huffman_tool compress <input_file> <output_file> <bucket_name>
```
##### Example:
```bash
./huffman_tool compress input.txt compressed_output.dat my-s3-bucket
```
#### Decompression

To download a compressed file from AWS S3 and decompress it:
```bash
./huffman_tool decompress <cloud_file_name> <output_file> <bucket_name>
```
##### Example:
```bash
./huffman_tool decompress compressed_output.dat output.txt my-s3-bucket
```
## AWS S3 Integration
The program uses the AWS SDK for C++ to interact with AWS S3. Ensure your AWS credentials and region are correctly configured.
## Contributing
Contributions are welcome! If you find any issues or have suggestions for improvement, please open an issue or submit a pull request.

