# Introduction to SimplisticDI

**SimplisticDI** is a minimalist Dependency Injection (DI) container implemented in C++20. It is designed to facilitate object management and dependency resolution in a straightforward manner. By leveraging modern C++ features, it provides a flexible and efficient way to handle object lifetimes and dependencies in a clean and organized manner.


## Getting Started with SimplisticDI

**SimplisticDI** is a single-header library. It is easy to integrate into your project using CMake or by directly including the header.

### **Installation Options:**

1.  **Using CMake:**
    
Install:
```sh
cmake -DCMAKE_INSTALL_PREFIX=/your/install/path ..
make
make install
```
In Your CMakeLists.txt:
```cmake
find_package(simplistic-di REQUIRED)
target_link_libraries(your_target PRIVATE simplistic::di)
```
2. **Add as Subdirectory:**

In Your CMakeLists.txt:
```cmake
add_subdirectory(path/to/simplistic-di)
target_link_libraries(your_target PRIVATE simplistic::di)
```
3. **Direct Header Inclusion:**

-   Simply download `simplistic/di.h` and include it in your project:
        
```cpp
#include "path/to/simplistic-di.h"
```

**Quick Example:**

```cpp
#include <simplistic/di.h>
#include <iostream>

namespace sdi = simplistic::di;

class IPrinter {
public:
    virtual ~IPrinter() = default;
    virtual void Print(const std::string& msg) = 0;
};

class Printer : public IPrinter {
public:
    void Print(const std::string& msg) override {
        std::cout << msg << std::endl;
    }
};

int main() {
    sdi::Container container;

    // Install and bind
    container.Install<IPrinter>(std::make_unique<Printer>());

    // Retrieve and use
    auto printer = container.Get<IPrinter>();
    printer->Print("Hello, SimplisticDI!");

    return 0;
}
```
This example demonstrates how to install a `Printer` object into the `Container`, bind it to an interface, and retrieve it for use.

## Handling Ownership and References in SimplisticDI

SimplisticDI handles three types of ownership and references:

1.  **Unique Ownership:** The container takes full ownership of a `std::unique_ptr` to an object.
```cpp
auto uniqueObj = std::make_unique<ConsoleLogger>();
container.Install<IConsoleLogger>(std::move(uniqueObj)); // Takes ownership
//container.Install<IConsoleLogger>(std::make_unique<ConsoleLogger>()); // Takes ownership
```
2. **Shared Ownership:** The container shares ownership using `std::shared_ptr`.
```cpp
container.Install(std::make_shared<ConsoleLogger>(2)); // Shares ownership
```
3. **Non-Ownership Reference:** The container stores a raw pointer without taking ownership.
```cpp
ConsoleLogger localObj(3);
container.Bind<IConsoleLogger>(&localObj); // No ownership, user manages lifetime
```

**Compact Example:**

```cpp
#include <iostream>
#include <simplistic/di.h>

namespace sdi = simplistic::di;

class IPrinter {
public:
    virtual ~IPrinter() = default;
    virtual void Print(const std::string& msg) = 0;
};

class Printer : public IPrinter {
public:
    void Print(const std::string& msg) override {
        std::cout << msg << std::endl;
    }
};

int main() {
    auto uniquePrinter = std::make_unique<Printer>();
    auto sharedPrinter = std::make_shared<Printer>();
    Printer localPrinter;

    sdi::Container container;
    container
        .Install<IPrinter>(std::move(uniquePrinter))     // Unique ownership
        .Install(sharedPrinter)                          // Shared ownership
        .Bind<IPrinter>(&localPrinter)                   // No ownership, reference
        .Install(42)                                     // Bind value
        .Install(3.14f);                                 // Bind another value

    container.Get<IPrinter>()->Print("Hello, DI!");

    std::cout << container.GetO<int>() << " " << container.GetO<float>() << std::endl;

    return 0;
}
```


### **Lifecycle Management in SimplisticDI**

In SimplisticDI:

1.  **Ownership Management:**
    
    -   **Unique Ownership:** When a `std::unique_ptr` is installed, the container takes full ownership. The object is freed when the container is destroyed or if the unique pointer is replaced by another.
    -   **Shared Ownership:** When a `std::shared_ptr` is installed, the container shares ownership with other parts of the program. The object remains valid as long as any `shared_ptr` references it, and is not replaced by another one.
2.  **Non-Ownership References:**
    
    -   **Raw Pointers:** When a raw pointer is bound, the container does not manage the object's lifecycle. The object must be managed and freed by the user. The container only stores the pointer.
3.  **Container Lifecycle:**
    
    -   The lifecycle of objects bound with unique or shared ownership is tied to the lifecycle of the container, or whenever a new object is bound, which will trigger the release of the previous ones. When the container is destroyed, all owned objects are freed. For raw pointers, only the container’s reference is removed; the object remains valid as managed by the user.

**Example:**

```cpp
{
    simplistic::di::Container container;
    container.Install<IPrinter>(std::make_unique<Printer>()); // Unique ownership

    // Container's destruction will automatically clean up the Printer object
}
```
In this example, the `Printer` object will be destroyed when the container goes out of scope. For `std::shared_ptr`, the object will be freed only when the last `shared_ptr` reference is released. For raw pointers, the object’s lifecycle is managed externally.

## **About SimplisticDI**

SimplisticDI is crafted with care to address the need for a straightforward, efficient DI container in C++. It’s built with love and aims to simplify dependency management in your projects. Contributions are warmly welcomed—feel free to improve or extend the library!

**License:** MIT License.


