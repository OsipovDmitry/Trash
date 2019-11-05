#ifndef ENUMCLASS_H
#define ENUMCLASS_H

#define ENUMCLASS(Name, type, ...) \
    enum class Name : type { \
        __VA_ARGS__ , Count \
    }; \
    constexpr Name castTo##Name(type value) { return static_cast< Name >(value); } \
    constexpr type castFrom##Name(Name value) { return static_cast< type >(value); } \
    constexpr type numElements##Name() { return castFrom##Name(Name::Count); }

#endif // ENUMCLASS_H
