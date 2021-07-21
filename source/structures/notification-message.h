#pragma once

#include <vector>
#include <string>
#include <cstdarg>

#define PUSH_NOTIFICATION(...) NotificationMessage::PushNotification(__VA_ARGS__)
#define PUSH_NOTIFICATION_LIFETIME(InLifeTime, ...) NotificationMessage::PushNotification(__VA_ARGS__); NotificationMessage::SetLifeTime(InLifeTime);

struct NotificationMessage
{
    static void PushNotification(const char* InMessage, ...);
    static void SetLifeTime(const float InLifeTime);

    struct Message
    {
        std::string Message;
        float LifeTime = 2.0f;

        bool operator==(const NotificationMessage::Message& InOther)
	    {
		    return (this == &InOther);
	    }
    };

    static std::vector<Message> Messages;
};

