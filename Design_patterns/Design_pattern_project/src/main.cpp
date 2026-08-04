#include "../inc/Logger_factory.hpp"

int main() {
    LoggerAdapterFactory adapterFactory;
    auto logger = adapterFactory.createLogger();
    logger->log(LogLevel::INFO, "This is an info message.");
    logger->log(LogLevel::ERROR, "This is an error message.");
    logger->flush();
    logger->shutdown();

    
}



