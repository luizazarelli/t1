#include "libs.h"

#ifndef SVG_H
#define SVG_H

#define RECT_SVG \
    "<rect id=\"%d\" x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" " \
    "stroke=\"%s\" stroke-width=\"1\" fill=\"%s\" fill-opacity=\"75%%\"/>\n"

#define CIRC_SVG \
    "<circle id=\"%d\" cx=\"%lf\" cy=\"%lf\" r=\"%lf\" " \
    "stroke=\"%s\" stroke-width=\"1\" fill=\"%s\" fill-opacity=\"75%%\"/>\n"

#define LINE_SVG \
    "<line id=\"%d\" x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" " \
    "stroke=\"%s\" stroke-width=\"1\"/>\n"

#define TEXT_SVG \
    "<text id=\"%d\" x=\"%lf\" y=\"%lf\" stroke=\"%s\" stroke-width=\"1\" " \
    "fill=\"%s\" text-anchor=\"%s\"><![CDATA[%s]]></text>\n"

FILE* openSvg(char* path);

void closeSvg(FILE* svg);

#endif
