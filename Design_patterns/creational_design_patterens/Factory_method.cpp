#include <iostream>
#include <string>
#include <memory>

using namespace std;

// when we will use the factory method pattern, we will not create the object directly, 
// instead we will use a factory to create the object for us. The factory will decide 
// which concrete class to instantiate based on some input or configuration. This allows
// us to decouple the client code from the concrete classes and makes it easier to add 
// new types of loggers in the future without modifying the client code.

// ──────────────────────────────────────────────────────────────
// PRODUCT INTERFACE
// Defines the interface that all concrete loggers must implement.
// The client (LoggerClient) only depends on this abstract type.
// ──────────────────────────────────────────────────────────────
class Logger
{
public:
    virtual void log(const string& message) = 0; // Pure virtual function
    virtual void log(const string& message, const string& level) = 0; // Overloaded pure virtual function
    virtual ~Logger() = default;
};

// ──────────────────────────────────────────────────────────────
// CONCRETE PRODUCTS
// Each subclass implements the Logger interface differently.
// The client never creates these directly — factories do.
// ──────────────────────────────────────────────────────────────
class ConsoleLogger : public Logger
{
public:
    void log(const string& message) override
    {
        cout << "Console Logger: " << message << endl;
    }

    void log(const string& message, const string& level) override
    {
        cout << "Console Logger [" << level << "]: " << message << endl;
    }
};

class FileLogger : public Logger
{
public:
    void log(const string& message) override
    {
        // For demonstration, we'll just print to console instead of writing to a file
        cout << "File Logger: " << message << endl;
    }

    void log(const string& message, const string& level) override
    {
        // For demonstration, we'll just print to console instead of writing to a file
        cout << "File Logger [" << level << "]: " << message << endl;
    }
};

class DLLLogger : public Logger
{
public:
    void log(const string& message) override
    {
        // For demonstration, we'll just print to console instead of using a DLL
        cout << "DLL Logger: " << message << endl;
    }

    void log(const string& message, const string& level) override
    {
        // For demonstration, we'll just print to console instead of using a DLL
        cout << "DLL Logger [" << level << "]: " << message << endl;
    }
};

// ──────────────────────────────────────────────────────────────
// FACTORY INTERFACE (Creator)
// Declares the factory method that subclasses override.
// "Define an interface for creating an object, but let subclasses
//  decide which class to instantiate."
// ──────────────────────────────────────────────────────────────
class LoggerFactory
{
public:
    virtual std::unique_ptr<Logger> createLogger() = 0; // Pure virtual function
    virtual ~LoggerFactory() = default;
};

// ──────────────────────────────────────────────────────────────
// CONCRETE FACTORIES (Concrete Creators)
// Each factory overrides createLogger() to return its specific
// Logger type. This is where "subclasses decide which class to
// instantiate."
// ──────────────────────────────────────────────────────────────
class FileLoggerFactory : public LoggerFactory
{
public:
    std::unique_ptr<Logger> createLogger() override
    {
        cout<<"Creating File Logger using File Logger Factory"<<endl;
        return std::make_unique<FileLogger>();
    }
};

class DLLLoggerFactory : public LoggerFactory
{
public:
    std::unique_ptr<Logger> createLogger() override
    {
        cout<<"Creating DLL Logger using DLL Logger Factory"<<endl;
        return std::make_unique<DLLLogger>();
    }
};

class ConsoleLoggerFactory : public LoggerFactory
{
public: 
    std::unique_ptr<Logger> createLogger() override
    {
        cout<<"Creating Console Logger using Console Logger Factory"<<endl;
        return std::make_unique<ConsoleLogger>();
    }
};

// ──────────────────────────────────────────────────────────────
// CLIENT
// Depends ONLY on abstract types (Logger & LoggerFactory).
// It does not know which concrete logger it is using.
// This is the key benefit — adding a new logger type requires
// NO changes to this class (Open/Closed Principle).
// ──────────────────────────────────────────────────────────────
class LoggerClient
{
private:
    std::unique_ptr<Logger> logger;
    
public:
    LoggerClient(LoggerFactory& factory)
    {
        logger = factory.createLogger();
    }
    
    void logMessage(const std::string& message)
    {
        logger->log(message);
    }

    void logMessage(const std::string& message, const std::string& level)
    {
        logger->log(message, level);
    }
};

// ──────────────────────────────────────────────────────────────
// FACTORY SELECTOR
// The only place that maps a string to a concrete factory.
// In a real application, this string could come from a config
// file, command-line argument, or user input.
// ──────────────────────────────────────────────────────────────
std::unique_ptr<LoggerFactory> getFactory(const string& type)
{
    if (type == "file")    return std::make_unique<FileLoggerFactory>();
    if (type == "dll")     return std::make_unique<DLLLoggerFactory>();
    if (type == "console") return std::make_unique<ConsoleLoggerFactory>();
    return nullptr;
}

int main()
{
    // main() no longer names any concrete factory
    // main()  ──string──►  getFactory("file")  ──returns──►  LoggerFactory (abstract)
    //                                                           │
    //                                                           ▼
    //                  LoggerClient only sees ──────►  LoggerFactory&
    auto factory1 = getFactory("file");
    LoggerClient fileClient(*factory1);
    fileClient.logMessage("This is a message for the file logger.");
    fileClient.logMessage("This is a message for the file logger with level.", "INFO");

    //FileLoggerFactory fileFactory;
    //LoggerClient logClient(fileFactory);
    //logClient.logMessage("This is a message for the file logger.");

    auto factory2 = getFactory("dll");
    LoggerClient dllClient(*factory2);
    dllClient.logMessage("This is a message for the DLL logger.");

    auto factory3 = getFactory("console");
    LoggerClient consoleClient(*factory3);
    consoleClient.logMessage("This is a message for the console logger.");

    return 0;
}
        
