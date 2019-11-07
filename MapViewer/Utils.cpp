#include "Utils.h"
#include <Windows.h>
#include <string>
#include <fstream>

std::string ReadFile(const std::string& _Filename)
{
    if (_Filename.empty())
    {
        return "";
    }
    std::ifstream stream{ _Filename, std::ios_base::in | std::ios_base::binary };
    if (!stream)
    {
        throw std::runtime_error{ std::string{"Can not open file '"} +_Filename + "'" };
    }
    stream.exceptions(std::ifstream::failbit);
    std::string buffer{ std::istreambuf_iterator<char>(stream.rdbuf()), std::istreambuf_iterator<char>() };
    stream.close();
    return buffer;
}


std::string GetResourcePath()
{
    char outPath[MAX_PATH] = { 0 };
    {
        wchar_t* pPath = new wchar_t[MAX_PATH];
        GetModuleFileNameW(NULL, pPath, MAX_PATH);
        WideCharToMultiByte(CP_ACP, 0, pPath, -1, outPath, MAX_PATH, "", false);
    }
    int pos = (int)strlen(outPath);
    while (--pos)
    {
        if (outPath[pos] == '\\')
        {
            outPath[pos] = '\0';
            break;
        }
        else
            outPath[pos + 1] = ' ';
    }
    return std::string(outPath);
}
