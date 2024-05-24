#include "Physics/IcObjectLayerPairFilter.h"

#include "Physics/PhysicsEngine.h"

IcObjectLayerPairFilter::IcObjectLayerPairFilter(PhysicsEngine* a_engine)
{
    m_engine = a_engine;
}
IcObjectLayerPairFilter::~IcObjectLayerPairFilter()
{

}

bool IcObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer a_lhs, JPH::ObjectLayer a_rhs) const
{
    return m_engine->CanObjectLayersCollide((uint32_t)a_lhs, (uint32_t)a_rhs);
}