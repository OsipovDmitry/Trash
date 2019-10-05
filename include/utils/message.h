#ifndef MESSAGE_H
#define MESSAGE_H

#define MESSAGE(Type) \
    public: \
    static ControllerMessageType messageType() { \
        return Type; \
    }


#endif
