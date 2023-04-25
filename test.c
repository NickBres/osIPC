#include "partB.h"
int main(){
    generate_file("test.txt", 100 * 1024 * 1024);
    generate_checksum("test.txt");
    delete_file("test.txt");
    return 0;
}