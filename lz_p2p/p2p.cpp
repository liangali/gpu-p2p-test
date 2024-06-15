#include <stdio.h>
#include <stdlib.h>

#include "context.h"

int main()
{
    lzContext ctx;

    ctx.initZe();
    ctx.queryP2P();

    printf("done\n");
    return 0;
}