/*
 The MIT License (MIT)

Copyright (c) 2022 Aili-Light. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

unsigned char crc_array(unsigned char *p, unsigned char counter);
uint64_t milliseconds (void);
uint64_t macroseconds (void);
int fatal(const char *msg);
void safe_free(void *p);
void load_image_path(string img_dir_path, vector<string> &img_path);
int load_image(const char *filename, uint8_t *buffer, uint32_t *data_len);
int find_highest_frame(std::vector<int>& ch_ids, std::vector<unsigned short>& frame_ids, std::vector<int>& max_ids);
template <typename T>
bool check_vector_same(std::vector<T>& vals)
{
    if (vals.size() < 1)
        return false;

    T v0 = vals[0];
    for(T v : vals)
    {
        if(v!=v0)
        {
            return false;
        }
    }

    return true;
}

#endif