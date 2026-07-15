# Factory Method + Builder Design Pattern — Notification Example

## Purpose

- **Factory Method**: Delegates object creation to subclasses — decides *which* notification to create.
- **Builder**: Constructs complex objects step-by-step — decides *how* to configure the notification.

---

## Class Diagram

```mermaid
classDiagram
    class INotification {
        <<interface>>
        +send()* void
        +~INotification()
    }

    class EmailNotification {
        -string senderEmailAddress
        -string subject
        -string body
        -string recipientEmailAddress
        +send() void
    }

    class SMSNotification {
        -string name
        -int phoneNumber
        +send() void
    }

    class WhatsAppNotification {
        -string name
        -int phoneNumber
        -string groupName
        +send() void
    }

    class EmailNotificationBuilder {
        -string senderEmailAddress
        -string subject
        -string body
        -string recipientEmailAddress
        +setSenderEmailAddress(string) EmailNotificationBuilder&
        +setSubject(string) EmailNotificationBuilder&
        +setBody(string) EmailNotificationBuilder&
        +setRecipientEmailAddress(string) EmailNotificationBuilder&
        +build() unique_ptr~EmailNotification~
    }

    class SMSNotificationBuilder {
        -string name
        -int phoneNumber
        +setName(string) SMSNotificationBuilder&
        +setPhoneNumber(int) SMSNotificationBuilder&
        +build() unique_ptr~SMSNotification~
    }

    class WhatsAppNotificationBuilder {
        -string name
        -int phoneNumber
        -string groupName
        +setName(string) WhatsAppNotificationBuilder&
        +setPhoneNumber(int) WhatsAppNotificationBuilder&
        +setGroupName(string) WhatsAppNotificationBuilder&
        +build() unique_ptr~WhatsAppNotification~
    }

    class NotificationFactory {
        <<interface>>
        +createNotification()* unique_ptr~INotification~
        +~NotificationFactory()
    }

    class EmailNotificationFactory {
        +createNotification() unique_ptr~INotification~
    }

    class SMSNotificationFactory {
        +createNotification() unique_ptr~INotification~
    }

    class WhatsAppNotificationFactory {
        +createNotification() unique_ptr~INotification~
    }

    class NotificationService {
        +sendNotification(NotificationFactory&) void
    }

    INotification <|.. EmailNotification
    INotification <|.. SMSNotification
    INotification <|.. WhatsAppNotification

    NotificationFactory <|.. EmailNotificationFactory
    NotificationFactory <|.. SMSNotificationFactory
    NotificationFactory <|.. WhatsAppNotificationFactory

    EmailNotification *-- EmailNotificationBuilder : nested class
    SMSNotification *-- SMSNotificationBuilder : nested class
    WhatsAppNotification *-- WhatsAppNotificationBuilder : nested class

    EmailNotificationFactory ..> EmailNotificationBuilder : uses
    SMSNotificationFactory ..> SMSNotificationBuilder : uses
    WhatsAppNotificationFactory ..> WhatsAppNotificationBuilder : uses

    NotificationService ..> NotificationFactory : depends on
```

---

## Sequence Diagram

```mermaid
sequenceDiagram
    participant Client as main()
    participant Service as NotificationService
    participant Factory as EmailNotificationFactory
    participant Builder as EmailNotificationBuilder
    participant Notif as EmailNotification

    Client->>Client: getFactory("email")
    Client->>Service: sendNotification(factory)
    Service->>Factory: createNotification()
    Factory->>Builder: EmailNotificationBuilder()
    Factory->>Builder: setSenderEmailAddress(...)
    Factory->>Builder: setSubject(...)
    Factory->>Builder: setBody(...)
    Factory->>Builder: setRecipientEmailAddress(...)
    Factory->>Builder: build()
    Builder-->>Factory: unique_ptr<EmailNotification>
    Factory-->>Service: unique_ptr<INotification>
    Service->>Notif: send()
    Notif-->>Service: "Sending Email Notification..."
```

---

## Key Design Decisions

| Aspect | Decision | Reason |
|--------|----------|--------|
| Object creation | Factory Method | Different notification types need different construction logic |
| Object configuration | Builder (nested class) | Notifications have multiple optional parameters |
| Constructor access | Private | Forces use of Builder, prevents invalid objects |
| Memory management | `unique_ptr` | Clear ownership, no memory leaks |
| Builder location | Nested inside product class | Can access private constructor |
