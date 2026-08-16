#include "MyString.h"

bool MyString::isUpper(char c)
{
    return c >= 'A' and c <= 'Z';
}

bool MyString::isLower(char c)
{
    return c >= 'a' and c <= 'z';
}

char MyString::toUpper(char c)
{
    if (isLower(c))
    {
        return c - 32;
    }
    return c;
}

char MyString::toLower(char c)
{
    if (isUpper(c))
    {
        return c + 32;
    }
    return c;
}

MyString::MyString()
{
    string.assign(1, '\0');
    len = 0;
}

MyString::MyString(const char* src)
{
    if (!src)
    {
        src = "";
    }

    int n = 0;
    while (src[n] != '\0')
    {
        n++;
    }

    string.resize(n + 1);
    for (int i = 0; i < n; i++)
    {
        string[i] = src[i];
    }
    string[n] = '\0';
    len = n;
}

MyString& MyString::operator=(const char* src)
{
    if (!src)
    {
        src = "";
    }

    int n = 0;
    while (src[n] != '\0')
    {
        n++;
    }

    string.assign(n + 1, '\0');
    for (int i = 0; i < n; i++)
    {
        string[i] = src[i];
    }
    len = n;
    return *this;
}

int MyString::length() const
{
    return len;
}

const char* MyString::c_str() const
{
    return string.data();
}

bool MyString::empty() const
{
    return len == 0;
}

void MyString::clear()
{
    string.assign(1, '\0');
    len = 0;
}

char& MyString::operator[](int index)
{
    return string[index];
}

char MyString::operator[](int index) const
{
    return string[index];
}

MyString& MyString::operator+=(const MyString& other)
{
    int n = other.len;

    vector<char> added(other.string.begin(), other.string.begin() + n);

    string.pop_back();
    for (int i = 0; i < n; i++)
    {
        string.push_back(added[i]);
    }
    string.push_back('\0');

    len += n;
    return *this;
}

MyString& MyString::operator+=(char c)
{
    string.back() = c;
    string.push_back('\0');
    len += 1;
    return *this;
}

bool MyString::operator==(const MyString& other) const
{
    if (len != other.len)
    {
        return false;
    }

    for (int i = 0; i < len; i++)
    {
        if (string[i] != other.string[i])
        {
            return false;
        }
    }
    return true;
}

void MyString::insertAt(int pos, char c)
{
    if (pos < 0)
    {
        pos = 0;
    }
    if (pos > len)
    {
        pos = len;
    }

    string.insert(string.begin() + pos, c);
    len += 1;
}

void MyString::insertAt(int pos, const MyString& s)
{
    if (pos < 0)
    {
        pos = 0;
    }
    if (pos > len)
    {
        pos = len;
    }

    string.insert(string.begin() + pos, s.string.begin(), s.string.begin() + s.len);
    len += s.len;
}

void MyString::eraseAt(int pos, int count)
{
    if (pos < 0 or pos >= len or count <= 0)
    {
        return;
    }
    if (pos + count > len)
    {
        count = len - pos;
    }

    string.erase(string.begin() + pos, string.begin() + pos + count);
    len -= count;
}

MyString MyString::substr(int pos, int count) const
{
    if (pos < 0)
    {
        pos = 0;
    }
    if (pos >= len)
    {
        return MyString("");
    }
    if (count < 0 or pos + count > len)
    {
        count = len - pos;
    }

    MyString result;
    result.string.assign(count + 1, '\0');
    for (int i = 0; i < count; i++)
    {
        result.string[i] = string[pos + i];
    }
    result.string[count] = '\0';
    result.len = count;
    return result;
}

int MyString::find(const MyString& pattern, int startPos) const
{
    if (pattern.len == 0)
    {
        return -1;
    }
    if (startPos < 0)
    {
        startPos = 0;
    }

    for (int i = startPos; i <= len - pattern.len; i++)
    {
        bool match = true;

        for (int j = 0; j < pattern.len; j++)
        {
            if (string[i + j] != pattern.string[j])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            return i;
        }
    }
    return -1;
}

void MyString::toggleCaseChar(int pos)
{
    if (pos < 0 or pos >= len)
    {
        return;
    }

    if (isUpper(string[pos]))
    {
        string[pos] = toLower(string[pos]);
    }
    else if (isLower(string[pos]))
    {
        string[pos] = toUpper(string[pos]);
    }
}

MyString MyString::replaceAll(const MyString& oldSub, const MyString& newSub) const
{
    if (oldSub.len == 0)
    {
        return *this;
    }

    MyString result("");
    int i = 0;

    while (i < len)
    {
        int pos = this->find(oldSub, i);

        if (pos == -1)
        {
            result += this->substr(i, len - i);
            break;
        }

        result += this->substr(i, pos - i);
        result += newSub;
        i = pos + oldSub.len;
    }
    return result;
}
