#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <functional>

using namespace std;

enum class DatabaseType
{
    MYSQL,
    POSTGRESQL,
    SQLITE
};

// Product interface
class IDatabase
{
public:
    virtual void connect() = 0;
    virtual void execute(const std::string& query) = 0;
    virtual void disconnect() = 0;
    virtual ~IDatabase() = default;
};

// Concrete Products
class DatabaseConfig
{
    string host, user, password, database;
public:
    DatabaseConfig() = delete;
    DatabaseConfig(string host, string user, string password, string database)
        : host(std::move(host)), user(std::move(user)), password(std::move(password)), database(std::move(database)) {}
};

class MySQLDatabase : public IDatabase
{
    public:
    void connect() override
    {
        cout << "Connecting to MySQL database..." << endl;
        // Implementation for connecting to MySQL database bussiness logic   
    }

    void execute(const std::string& query) override
    {
        cout << "Executing query on MySQL database: " << query << endl;
        // Implementation for executing query on MySQL database bussiness logic
        
    }

    void disconnect() override
    {
        cout << "Disconnecting from MySQL database..." << endl;
        // Implementation for disconnecting from MySQL database bussiness logic
    }
};

class PostgreSQLDatabase : public IDatabase
{
    const DatabaseConfig config;
public:
    PostgreSQLDatabase(const DatabaseConfig& config) : config(config) {}
    void connect() override
    {
        cout << "Connecting to PostgreSQL database..." << endl;
        // Implementation for connecting to PostgreSQL database bussiness logic 
    }

    void execute(const std::string& query) override
    {
        cout << "Executing query on PostgreSQL database: " << query << endl;
        // Implementation for executing query on PostgreSQL database bussiness logic
    }

    void disconnect() override
    {
        cout << "Disconnecting from PostgreSQL database..." << endl;
        // Implementation for disconnecting from PostgreSQL database bussiness logic
    }
};

class PostgreSQLDatabaseBuilder
{
    string host, user, password, database;
public:
    PostgreSQLDatabaseBuilder& setHost(const string& host)
    {
        if(host.empty())
        {
            throw std::invalid_argument("Host cannot be empty");
        }
        else
        {
            this->host = host;
        }
        return *this;
    }

    PostgreSQLDatabaseBuilder& setUser(const string& user)
    {
        if(user.empty())
        {
            throw std::invalid_argument("User cannot be empty");
        }
        else
        {
        this->user = user;
        }
        return *this;
    }

    PostgreSQLDatabaseBuilder& setPassword(const string& password)
    {
        if(password.empty())
        {
            throw std::invalid_argument("Password cannot be empty");
        }
        else
        {
        this->password = password;
        }
        return *this;
    }

    PostgreSQLDatabaseBuilder& setDatabase(const string& database)
    {
        if(database.empty())
        {
            throw std::invalid_argument("Database cannot be empty");
        }
        else
        {
        this->database = database;
        }
        return *this;
    }

    DatabaseConfig build()
    {
        return DatabaseConfig{host, user, password, database};
    }
};  

class SQLiteDatabase : public IDatabase
{
public:
    void connect() override
    {
        cout << "Connecting to SQLite database..." << endl;
        // Implementation for connecting to SQLite database bussiness logic
    }

    void execute(const std::string& query) override
    {
        cout << "Executing query on SQLite database: " << query << endl;
        // Implementation for executing query on SQLite database bussiness logic
    }

    void disconnect() override
    {
        cout << "Disconnecting from SQLite database..." << endl;
        // Implementation for disconnecting from SQLite database bussiness logic
    }
};

// Abstract Factory
class DatabaseFactory
{
public:
    static std::unique_ptr<IDatabase> createDatabase(DatabaseType dbtype)
    {
        switch (dbtype)
        {
            case DatabaseType::MYSQL:
                return std::make_unique<MySQLDatabase>();
            case DatabaseType::POSTGRESQL:
                {
                    auto config = PostgreSQLDatabaseBuilder()
                        .setHost("localhost")
                        .setUser("postgres_user")
                        .setPassword("postgres_password")
                        .setDatabase("my_database")
                        .build();
                    return std::make_unique<PostgreSQLDatabase>(std::move(config));
                }
            case DatabaseType::SQLITE:
                return std::make_unique<SQLiteDatabase>();
            default:
                throw std::invalid_argument("Invalid database type");
        }
    }
    
};

class DatabaseService 
{

    public:
    void executeQuery(const DatabaseType dbtype, const std::string& query)
    {
        auto database = DatabaseFactory::createDatabase(dbtype);
        database->connect();
        database->execute(query);
        database->disconnect();
    }
};


int main()
{
    DatabaseService dbService;

    DatabaseType dbType = DatabaseType::MYSQL;
    std::string query = "SELECT * FROM users";
    dbService.executeQuery(dbType, query);

    dbType = DatabaseType::POSTGRESQL;
    query = "INSERT INTO products (name, price) VALUES ('Product A', 10.99)";
    dbService.executeQuery(dbType, query);

    dbType = DatabaseType::SQLITE;
    query = "UPDATE orders SET status='shipped' WHERE id=1";
    dbService.executeQuery(dbType, query);

    return 0;
}