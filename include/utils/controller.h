#ifndef CONTROLLER_H
#define CONTROLLER_H

#define CONTROLLER(Type) \
    public: \
    static ControllerType controllerType() { \
        return Type; \
    }


#endif
