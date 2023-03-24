#include <bit>
#include <iostream>
#include <arpa/inet.h>
#include <iomanip>
#include <vector>

// JS
// let str = "fact"
// "0x"+str.split("").map(el => el.charCodeAt(0).toString(16)).join("") 

int main() {
    std::string str = "data";
    uint32_t num{};
    uint8_t shift = 24;

    for(std::size_t i = 0; i < 4 && shift >=0; ++i, shift -= 8) {
        num |= str.at(i) << shift;
    }

    std::cout << std::hex << "0x" <<htonl(num) << std::endl;
}