#ifndef PARTB_H
#define PARTB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void generate_file(char* filename, long size_in_bytes);
uint32_t generate_checksum(char* filename);
int delete_file(char* filename);

#endif