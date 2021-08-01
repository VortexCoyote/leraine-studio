#include "notification-message.h"

std::vector<NotificationMessage::Message> NotificationMessage::Messages;

void NotificationMessage::PushNotification(const char* InMessage, ...) 
{
     Message message;

        int size = 512;
        char* buffer = 0;
        
        buffer = new char[size];
        va_list vl;
        va_start(vl, InMessage);
        
        int nsize = vsnprintf(buffer, size, InMessage, vl);
        
        if(size<=nsize)
        {
            delete[] buffer;
            buffer = 0;
            buffer = new char[nsize+1]; //+1 for /0
            nsize = vsnprintf(buffer, size, InMessage, vl);
        }

        message.NotiMessage = buffer;

        va_end(vl);
        delete[] buffer;

        Messages.insert(Messages.begin(), message);
}

void NotificationMessage::SetLifeTime(const float InLifeTime) 
{
    if(Messages.size())
        Messages[0].LifeTime = InLifeTime;
}