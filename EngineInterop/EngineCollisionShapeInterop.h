#pragma once

#include "InteropTypes.h"

/// @file EngineCollisionShapeInterop.h

/// @cond INTERNAL

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

#define ENGINE_COLLISIONSHAPE_EXPORT_TABLE(F) \
    F(void, IcarianEngine.Physics.Shapes, CollisionShapeInterop, DestroyShape, \
    { \
        class CollisionShapeDeletionObject : public DeletionObject \
        { \
        private: \
            uint32_t m_addr; \
        public: \
            CollisionShapeDeletionObject(uint32_t a_addr) : m_addr(a_addr) { } \
            virtual ~CollisionShapeDeletionObject() { } \
            virtual void Destroy() \
            { \
                Instance->DestroyCollisionShape(m_addr); \
            } \
        }; \
        DeletionQueue::Push(new CollisionShapeDeletionObject(a_addr), DeletionIndex_Update); \
    }, IOP_UINT32 a_addr) \

/// @endcond