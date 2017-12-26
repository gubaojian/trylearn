#include <iostream>
#include <codecvt>

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::wstring u16 = L"hello world中国人";

    std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;

    std::wcout << u16 << std::endl;

    std::cout <<  convert.to_bytes(u16)<< std::endl;
    return 0;
}