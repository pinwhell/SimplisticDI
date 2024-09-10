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

int main()
{
    auto conLogger = std::make_unique<ConsoleLogger>(1);
    auto conLoggerShared = std::make_shared<ConsoleLogger>(2);
    ConsoleLogger conLogger_(3);
    float x = 1.2f;
    int y = 10;

    sdi::Container container; container
        .Install<IConsoleLogger>(std::make_unique<ConsoleLogger>(4))    // Container Takes ownership of the unique_ptr!
        .Install<IConsoleLogger>(std::move(conLogger))                  // Container Takes ownership of the unique_ptr!
        .SharedInstall(conLoggerShared)                                 // Container Takes shared ownership with the shared_ptr!
        .BindIface<IConsoleLogger>(&conLogger_)                         // Container just take a ref for the object, you manage ownership!
        .Bind(x)                                                        // Container binds 'x' to int type!
        .Bind(y)                                                        // Container binds 'y' to float type!
        .GetIface<IConsoleLogger>()->Log("Hello Container!");           // Container Fetch Bound Interface & Usage!

    std::cout << 
        container.Get<int>()        // Container Fetch Bound value to 'int' type
        << " " << 
        container.Get<float>()      // Container Fetch Bound value to 'float' type
        << "\n";

    return 0;
}