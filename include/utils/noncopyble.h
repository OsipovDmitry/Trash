#ifndef NONCOPYBLE_H
#define NONCOPYBLE_H

#define NONCOPYBLE(Class) \
    Class(const Class&) = delete; \
    Class& operator =(const Class&) = delete;

#endif // NONCOPYBLE_H
