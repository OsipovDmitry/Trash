#ifndef SINGLETOON_H
#define SINGLETOON_H

#define SINGLETON(Class) \
    public: \
    static Class& instance() { \
        static Class p; \
        return p; \
    }


#endif
