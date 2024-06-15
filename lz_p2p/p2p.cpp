#include <stdio.h>
#include <stdlib.h>

#include "context.h"

int main()
{
    lzContext ctx0, ctx1;
    ctx0.initZe(0);
    ctx1.initZe(1);

    queryP2P(ctx0.device(), ctx1.device());
    queryP2P(ctx1.device(), ctx0.device());

    void* buf0 = ctx0.initBuffer(1024);
    void* buf1 = ctx1.initBuffer(1024);

    printf("buf0 = %p, buf1 = %p\n", buf0, buf1);

    printf("done\n");
    return 0;
}