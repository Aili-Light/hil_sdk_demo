#pragma once
#include <string>

class Source
{
public:
    Source() = default;
    virtual ~Source() = default;

    virtual bool Read(char** buf, int& len) = 0;
    virtual bool Save(const char* buf, const int len) = 0;
    virtual std::string Key() = 0;
};

class FileSource : public Source
{
public:
    FileSource(std::string filename);
    ~FileSource() = default;

    bool Read(char** buf, int& len) override;
    bool Save(const char* buf, const int len) override;
    std::string Key() override;
private:
    std::string filename;
};

