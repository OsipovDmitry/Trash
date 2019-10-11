#ifndef PIMPL_H
#define PIMPL_H

#define PIMPL(Class) \
    public: \
    template <typename T = Class##Private> inline T& m() { return dynamic_cast<T&>(*m_); } \
    template <typename T = Class##Private> inline const T& m() const { return dynamic_cast<T&>(*m_); } \

#endif // PIMPL_H
