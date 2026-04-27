#include "svg.h"

FILE* openSvg(char* path)
{
    FILE* svg = fopen(path, "w");
    if (!svg) return NULL;
    fprintf(svg,
        "<svg xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
        "     xmlns=\"http://www.w3.org/2000/svg\"\n"
        "     version=\"1.1\">\n");
    return svg;
}

void closeSvg(FILE* svg)
{
    if (!svg) return;
    fprintf(svg, "</svg>\n");
    fclose(svg);
}
