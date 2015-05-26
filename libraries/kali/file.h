
#ifndef KALI_FILE_INCLUDED
#define KALI_FILE_INCLUDED

#include <stdio.h>
#include <sys/stat.h>

// FIXME:
#define fstat _fstat
#define stat  _stat

// ............................................................................

struct File
{
    int open(const char* filename, const char* mode)
    {
        close();
        handle = fopen(filename, mode);
        return handle ? 0 : -1 /*ERROR_OPEN_FAILED*/;
    }

    void close()
    {
        if (handle)
        {
            fclose(handle);
            handle = 0;
        }
    }

    int size() const
    {
        struct stat s = {0};
        return (fstat(fileno(handle), &s)) ? 0 : s.st_size;
    }

    int read(void* data, size_t size) const
    {
        return (fread(data, 1, size, handle) == size)
            ? 0 : -1 /*ERROR_DISK_OPERATION_FAILED*/;
    }

    int write(const void* data, size_t size)
    {
        return (fwrite(data, 1, size, handle) == size)
            ? 0 : -1 /*ERROR_DISK_OPERATION_FAILED*/;
    }

    File() : handle(0) {}
    ~File() {close();}

private:
    FILE* handle;
    File(const File&);
    File& operator = (const File&);
};

// ............................................................................

#endif // ~ KALI_FILE_INCLUDED
