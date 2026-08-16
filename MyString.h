#pragma once

#include <vector>
using namespace std;


class MyString
{
private:
    vector<char> string;
    int len;

    static bool isUpper(char c);
    static bool isLower(char c);
    static char toUpper(char c);
    static char toLower(char c);

public:

    MyString();
    MyString(const char* src);
    MyString(const MyString& other) = default;
    ~MyString() = default;

    MyString& operator=(const MyString& other) = default;
    MyString& operator=(const char* src);

    int length() const;
    const char* c_str() const;
    bool empty() const;
    void clear();

    char& operator[](int index);
    char operator[](int index) const;

    MyString& operator+=(const MyString& other);
    MyString& operator+=(char c);
    bool operator==(const MyString& other) const;

    void insertAt(int pos, char c);
    void insertAt(int pos, const MyString& s);
    void eraseAt(int pos, int count = 1);

    MyString substr(int pos, int count) const;
    int find(const MyString& pattern, int startPos = 0) const;
    void toggleCaseChar(int pos);

    MyString replaceAll(const MyString& oldSub, const MyString& newSub) const;
};
