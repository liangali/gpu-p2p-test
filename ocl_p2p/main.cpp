#include <CL/cl.h>
#include <iostream>
#include <vector>

#include "context.h"

int main(int argc, char** argv) 
{
    oclContext ctx0, ctx1;

    ctx0.init(0);
    ctx1.init(1);

    return 0;
}
