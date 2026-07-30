#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <queue>
#include <condition_variable>

using namespace std;

class LeggacyLogger
{
    private:
        static int logCount;
        LeggacyLogger() {}
        LeggacyLogger(const LeggacyLogger&) = delete;
        LeggacyLogger& operator=(const LeggacyLogger&) = delete;
    public:
        static LeggacyLogger& getInstance()
        {
            logCount++;
            static LeggacyLogger instance;
            return instance;
        }
        void writeLog(const string& message)
        {
            cout << "Legacy Logger: " << message << endl;
            cout << "Log count: " << logCount << endl;
        }
};
int LeggacyLogger::logCount = 0;

class Logger{
    public:
        void writeLog(const string& message)
        {
            cout << "Logger: " << message << endl;
        }
};

class Queue_logger
{
    private:
        std::queue<string> logQueue;
        std::mutex mtx;
        std::condition_variable cv;
        public:
        void writeLog(const string& message)
        {
            {
                std::lock_guard<std::mutex> lock(mtx);
                logQueue.push(message);
            }
            cv.notify_one();
        }
        void processLogs()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return !logQueue.empty(); });

                string message = logQueue.front();
                logQueue.pop();
                lock.unlock();

                cout << "Queue Logger: " << message << endl;
            }
        }
        
};

int main()
{
    Queue_logger queueLogger;

    thread producer1([&queueLogger] {
        queueLogger.writeLog("Thread 1 logging");
    });
    thread producer2([&queueLogger] {
        queueLogger.writeLog("Thread 2 logging");
    });
    thread producer3([&queueLogger] {
        queueLogger.writeLog("Thread 3 logging");
    });

    thread consumer([&queueLogger] {
        queueLogger.processLogs();
    });

    producer1.join();
    producer2.join();
    producer3.join();
    consumer.join();

    return 0;
}