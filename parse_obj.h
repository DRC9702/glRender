#ifndef PARSE_H
#define PARSE_H

#include <iostream>
#include <vector>

//
// Basic parser for the wavefront files, stolen from raytracer:
//
void read_wavefront_file (
            const char *file,
            std::vector< int > &tris,
            std::vector< float > &verts);

#endif
