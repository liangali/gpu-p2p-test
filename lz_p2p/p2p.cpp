#include <stdio.h>
#include <stdlib.h>

#include "context.h"

void printBuf(std::vector<uint32_t> &buf, int size)
{
    for (int i = 0; i < size; i++) {
        printf("%d, ", buf[i]);
        if (i % 16 == 0)
            printf("\n");
    }  
    printf("\n");
}

int parseInput(const std::string& input) {
    int multiplier = 1; 
    int length = input.length();

    char lastChar = std::tolower(input[length - 1]);
    if (lastChar == 'k') {
        multiplier = 1024;
        length--;
    } else if (lastChar == 'm') {
        multiplier = 1024*1024;
        length--;
    }

    for (int i = 0; i < length; ++i) {
        if (!std::isdigit(input[i])) {
            std::cerr << "ERROR: Invalid input (requires number or number with k or m, e.g., 256, 2k, 4m)" << std::endl;
            exit(-1);
        }
    }

    int number = std::stoi(input.substr(0, length));

    return number * multiplier;
}

int main(int argc, char** argv)
{
    int elem_num = 1024;
    if (argc >= 2)
        elem_num = parseInput(argv[1]);
    printf("Number of test data = %d, size = %d\n", elem_num, elem_num * sizeof(uint32_t));

    lzContext ctx0, ctx1;
    ctx0.initZe(0);
    ctx1.initZe(1);

    queryP2P(ctx0.device(), ctx1.device());
    queryP2P(ctx1.device(), ctx0.device());

    void* buf0 = ctx0.initBuffer(elem_num);
    void* buf1 = ctx1.initBuffer(elem_num);
    printf("buf0 = %p, buf1 = %p\n", buf0, buf1);

    std::vector<uint32_t> hostBuf0(elem_num, 0);
    ctx0.copyBuffer(hostBuf0);
    printBuf(hostBuf0, 16);

    std::vector<uint32_t> hostBuf1(elem_num, 0);
    ctx1.copyBuffer(hostBuf1);
    printBuf(hostBuf0, 16);

    ctx0.runKernel("../add_kernel_dg2.spv", "vector_add", buf1);
    ctx0.copyBuffer(hostBuf0);
    printBuf(hostBuf0, 16);

    printf("done\n");
    return 0;
}