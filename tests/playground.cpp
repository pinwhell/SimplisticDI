#include <simplistic/di.h>
#include <iostream>

namespace sdi = simplistic::di;

class IConsoleLogger {
public:
    virtual ~IConsoleLogger() = default;
    virtual void Log(const char* str) = 0;
};

class ConsoleLogger : public IConsoleLogger {
public:
    ConsoleLogger(int id = 0)
        : mID(id)
    {
        std::cout << "ConsoleLogger(" << mID << ")\n";
    }

    ~ConsoleLogger()
    {
        std::cout << "~ConsoleLogger(" << mID << ")\n";
    }

    void Log(const char* str)
    {
        std::cout << str << std::endl;
    }

    int mID;
};

struct Foo {
    int x;
    float y;
    char c;
};

int main()
{
    auto conLogger = std::make_unique<ConsoleLogger>(1);
    auto conLoggerShared = std::make_shared<ConsoleLogger>(2);
    ConsoleLogger conLogger_(3);
    float x = 1.2f;
    int y = 10;
    Foo f{ 1, 1.1f, 'Y' };

    sdi::Container container;
    container
        .Install<IConsoleLogger>(std::make_unique<ConsoleLogger>(4))    // Container takes ownership of this unique_ptr.
        .Install<IConsoleLogger>(std::move(conLogger))                  // Container takes ownership of this unique_ptr.
        .Install<IConsoleLogger>(conLoggerShared)                       // Container shares ownership with this shared_ptr.
        .Bind<IConsoleLogger>(&conLogger_)                              // Container binds to this raw reference; ownership is managed outside the container.
        .Install(x)                                                     // Container binds the value 'x' to the int type.
        .Install(y)                                                     // Container binds the value 'y' to the float type.
        .Install(f)                                                     // Container binds the value 'f' to the Foo type.
        .Get<IConsoleLogger>()->Log("Hello Container!");                // Container fetches the bound IConsoleLogger instance and uses it.

    std::cout <<
        container.GetO<int>()        // Container fetches the bound value of type 'int'.
        << " " <<
        container.GetO<float>()      // Container fetches the bound value of type 'float'.
        << " " <<
        container.GetO<Foo>().c      // Container fetches the bound value of type 'Foo' and accesses its 'c' member.
        << "\n";

    return 0;
}