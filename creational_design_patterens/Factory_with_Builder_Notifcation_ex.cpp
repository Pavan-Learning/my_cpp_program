#include <iostream>
#include <string>
#include <memory>

using namespace std;

/*
 * ============================================================================
 *  DESIGN PATTERN: Factory Method + Builder (Combined)
 * ============================================================================
 *
 *  Purpose:
 *    - Factory Method: Delegates object creation to subclasses (which notification to create)
 *    - Builder: Constructs complex objects step-by-step (how to configure the notification)
 *
 * ============================================================================
 *  CLASS DIAGRAM (UML-style ASCII)
 * ============================================================================
 *
 *        +---------------------+              +------------------------+
 *        | NotificationService |              |    <<interface>>       |
 *        +---------------------+              |    INotification       |
 *        | +sendNotification() |              +------------------------+
 *        +----------+----------+              | +send() = 0            |
 *                   |                         | +~INotification()      |
 *                   | uses                    +----------+-------------+
 *                   v                                    ^
 *        +---------------------+                        | implements
 *        |   <<interface>>     |          +-------------+-------------+
 *        | NotificationFactory |          |             |             |
 *        +---------------------+          |             |             |
 *        | +createNotification()=0 |      v             v             v
 *        +----------+----------+   +-----------+ +-----------+ +-------------+
 *                   ^              | Email     | | SMS       | | WhatsApp    |
 *                   | extends      | Notif.   | | Notif.    | | Notif.      |
 *          +--------+--------+     +-----------+ +-----------+ +-------------+
 *          |        |        |     | -sender   | | -name     | | -name       |
 *          v        v        v     | -subject  | | -phone    | | -phone      |
 *   +--------+ +-------+ +------+ | -body     | +-----------+ | -groupName  |
 *   | Email  | | SMS   | | WApp | | -recipient| | +send()   | +-------------+
 *   | Notif. | | Notif.| | Notif| +-----------+ +-----+-----+ | +send()     |
 *   | Factory| | Fact. | | Fact.| | +send()   |       ^       +------+------+
 *   +--------+ +-------+ +------+ +-----+-----+      |              ^
 *   |create()|  |create()| |create()|    ^            |              |
 *   +---+----+  +---+----+ +---+---+    |            |              |
 *       |            |          |        | builds     | builds       | builds
 *       |            |          |        |            |              |
 *       v            v          v        |            |              |
 *  +-----------+ +---------+ +---------+ |            |              |
 *  | Email     | | SMS     | | WApp    | |            |              |
 *  | Notif.    | | Notif.  | | Notif.  |-+            |              |
 *  | Builder   | | Builder | | Builder |              |              |
 *  +-----------+ +---------+ +---------+--------------+--------------+
 *  | +setSender()| +setName() | +setName()  |
 *  | +setSubject()| +setPhone()| +setPhone() |
 *  | +setBody()  | +build()  | +setGroup() |
 *  | +setRecip() |           | +build()    |
 *  | +build()    |           |             |
 *  +-------------+-----------+-------------+
 *
 * ============================================================================
 *  FLOW / SEQUENCE
 * ============================================================================
 *
 *   Client (main)
 *       |
 *       |  1. getFactory("email") --> returns EmailNotificationFactory
 *       |
 *       v
 *   NotificationService::sendNotification(factory)
 *       |
 *       |  2. factory.createNotification()
 *       |         |
 *       |         v  (inside EmailNotificationFactory)
 *       |     EmailNotificationBuilder()
 *       |         .setSenderEmailAddress(...)
 *       |         .setSubject(...)
 *       |         .setBody(...)
 *       |         .setRecipientEmailAddress(...)
 *       |         .build()  --> returns unique_ptr<EmailNotification>
 *       |
 *       |  3. notification->send()  --> "Sending Email Notification..."
 *       v
 *
 * ============================================================================
 */

// here we are using the both factory and builder design patterns to create differnt types of
// notfications like(email,sms, whatsapp). 
// Here the factory pattern used to instantiate the diffenren type of notfication(mean create the object of the notification) and 
// the builder pattern is used to build the object of the notification with different parameters.


class INotification
{
public:
    virtual void send() = 0;
    virtual ~INotification() = default;
};

class EmailNotification : public INotification
{
    string senderEmailAddress, subject, body, recipientEmailAddress;
    EmailNotification(string senderEmailAddress, string subject, string body, string recipientEmailAddress)
        : senderEmailAddress(senderEmailAddress), subject(subject), body(body), recipientEmailAddress(recipientEmailAddress) {}
public:
        
    void send() override
    {
        cout << "Sending Email Notification from: " << senderEmailAddress << " to: " << recipientEmailAddress << " with subject: " << subject << " and body: " << body << endl;
    }
    
    class EmailNotificationBuilder
    {
        string senderEmailAddress, subject, body, recipientEmailAddress;
        public:
            EmailNotificationBuilder& setSenderEmailAddress(const string& senderEmailAddress)
            {
                this->senderEmailAddress = senderEmailAddress;
                return *this;
            }
            EmailNotificationBuilder& setSubject(const string& subject)
            {
                this->subject = subject;
                return *this;
            }
            EmailNotificationBuilder& setBody(const string& body)
            {
                this->body = body;
                return *this;
            }
            EmailNotificationBuilder& setRecipientEmailAddress(const string& recipientEmailAddress)
            {
                this->recipientEmailAddress = recipientEmailAddress;
                return *this;
            }
            std::unique_ptr<EmailNotification> build()
            {
                return std::make_unique<EmailNotification>(senderEmailAddress, subject, body, recipientEmailAddress);
            }
        };
};

class SMSNotification : public INotification
{
    string name;
    int phoneNumber;
    SMSNotification(string name, int phoneNumber)
        : name(name), phoneNumber(phoneNumber) {}
public:
    void send() override
    {
        cout << "Sending SMS Notification with name: " << name << " and phone number: " << phoneNumber << endl;

    }
    class SMSNotificationBuilder
    {
        string name;
        int phoneNumber;
    public:
        SMSNotificationBuilder& setName(const string& name)
        {
            this->name = name;
            return *this;
        }
        SMSNotificationBuilder& setPhoneNumber(int phoneNumber) 
        {
            this->phoneNumber = phoneNumber;
            return *this;
        }
        std::unique_ptr<SMSNotification> build()
        {
            return std::make_unique<SMSNotification>(name, phoneNumber);
        }
    };
};

class WhatsAppNotification : public INotification
{
    string name, groupName;
    int phoneNumber;
    WhatsAppNotification(string name, int phoneNumber, string groupName)
        : name(name), phoneNumber(phoneNumber), groupName(groupName) {}
public:
    void send() override
    {
        cout << "Sending WhatsApp Notification with name: " << name << ", phone number: " << phoneNumber << ", and group name: " << groupName << endl;
    }
    class WhatsAppNotificationBuilder
    {
        string name, groupName;
        int phoneNumber;
    public:
        WhatsAppNotificationBuilder& setName(const string& name)    
        {
            this->name = name;
            return *this;
        }
        WhatsAppNotificationBuilder& setPhoneNumber(int phoneNumber)        
        {
            this->phoneNumber = phoneNumber;
            return *this;
        }
        WhatsAppNotificationBuilder& setGroupName(const string& groupName)
        {
            this->groupName = groupName;
            return *this;
        }
        std::unique_ptr<WhatsAppNotification> build()
        {
            return std::make_unique<WhatsAppNotification>(name, phoneNumber, groupName);
        }
    };
};


class NotificationFactory
{
public:
    virtual std::unique_ptr<INotification> createNotification() = 0;
    virtual ~NotificationFactory() = default;
};

class EmailNotificationFactory : public NotificationFactory
{
public:
    std::unique_ptr<INotification> createNotification() override
    {
        cout << "Creating Email Notification using Email Notification Factory" << endl;
        std::unique_ptr<EmailNotification> email = EmailNotification::EmailNotificationBuilder()
            .setSenderEmailAddress("pavankumar@gmail.com")
            .setSubject("Hello")
            .setBody("This is a test email notification.")
            .setRecipientEmailAddress("recipient@example.com")
            .build();
        return email;
    }
};

class SMSNotificationFactory : public NotificationFactory
{
public:
    std::unique_ptr<INotification> createNotification() override
    {
        cout << "Creating SMS Notification using SMS Notification Factory" << endl;
        std::unique_ptr<SMSNotification> sms = SMSNotification::SMSNotificationBuilder()
            .setName("Pavan Kumar")
            .setPhoneNumber(1234567890)
            .build();
        return sms;
    }
};

class WhatsAppNotificationFactory : public NotificationFactory
{
public:
    std::unique_ptr<INotification> createNotification() override
    {
        cout << "Creating WhatsApp Notification using WhatsApp Notification Factory" << endl;
        std::unique_ptr<WhatsAppNotification> whatsapp = WhatsAppNotification::WhatsAppNotificationBuilder()
            .setName("Pavan Kumar")
            .setPhoneNumber(1234567890)
            .setGroupName("Friends")
            .build();
        return whatsapp;
    }
};

class NotificationService
{
public:
    void sendNotification(NotificationFactory& factory)
    {
        std::unique_ptr<INotification> notification = factory.createNotification();
        notification->send();
    }
};

std::unique_ptr<NotificationFactory> getFactory(const string& type)
{
    if (type == "email")
    {
        return std::make_unique<EmailNotificationFactory>();
    }
    else if (type == "sms")
    {
        return std::make_unique<SMSNotificationFactory>();
    }
    else if (type == "whatsapp")
    {
        return std::make_unique<WhatsAppNotificationFactory>();
    }
    else
    {
        throw invalid_argument("Unknown notification type");
    }
}

int main()
{
    NotificationService service;
    string type;

    cout << "Enter notification type (email, sms, whatsapp): ";
    cin >> type;

    try
    {
        std::unique_ptr<NotificationFactory> factory = getFactory(type);
        service.sendNotification(*factory);
    }
    catch (const invalid_argument& e)
    {
        cout << e.what() << endl;
    }

    return 0;
}