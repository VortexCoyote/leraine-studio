#pragma once

#include <vector>
#include <string>
#include <cstdarg>

#define PUSH_NOTIFICATION(...) NotificationMessage::PushNotification(__VA_ARGS__)

struct NotificationMessage
{
    static void PushNotification(const char* InMessage, ...);

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

