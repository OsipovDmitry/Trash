#ifndef TEETH_H
#define TEETH_H

#include <memory>

#include <core/abstractgame.h>
#include <core/forwarddecl.h>

#include "teethglobal.h"

namespace trash
{
namespace teeth
{

class TEETHSHARED_EXPORT Teeth : public core::AbstractGame
{

public:
    Teeth();
    ~Teeth() override;

    void doInitialize() override;
    void doUnitialize() override;
    void doUpdate(uint64_t, uint64_t) override;
    void doMouseClick(uint32_t, int, int) override;
    void doMouseMove(uint32_t, int, int) override;

private:
    std::shared_ptr<core::Scene> m_grpahicsScene;
    int m_mouseX, m_mouseY;
    float m_modelAngleX = 0.f, m_modelAngleY = 0.f;
    //bool m_lmbIsPressed = false;

};

} // namespace
} // namespace

#endif // TEETH_H
