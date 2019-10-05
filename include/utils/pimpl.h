#ifndef PIMPL_H
#define PIMPL_H

#define PIMPL(Class) \
    public: \
    inline Class##Private& m() { return *m_; } \
    inline const Class##Private& m() const { return *m_; }

#endif // PIMPL_H
