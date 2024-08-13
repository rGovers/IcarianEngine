// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Physics/IcBodyActivationListener.h"

#include <Jolt/Physics/Body/BodyID.h>
#include <vector>

#include "DataTypes/TLockArray.h"

IcBodyActivationListener::IcBodyActivationListener()
{

}
IcBodyActivationListener::~IcBodyActivationListener()
{

}

Array<JPH::BodyID> IcBodyActivationListener::ToBodies()
{
    return m_bodies.ToArray();
}

void IcBodyActivationListener::OnBodyActivated(const JPH::BodyID& a_bodyID, JPH::uint64 a_userData)
{
    {
        TLockArray<JPH::BodyID> a = m_bodies.ToLockArray();
        for (JPH::BodyID& val : a)
        {
            if (val.IsInvalid())
            {
                val = a_bodyID;

                return;
            }
        }
    }

    m_bodies.Push(a_bodyID);
}
void IcBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& a_bodyID, JPH::uint64 a_userData)
{
    TLockArray<JPH::BodyID> a = m_bodies.ToLockArray();
    for (JPH::BodyID& val : a) 
    {
        if (val == a_bodyID) 
        {
            val = JPH::BodyID();

            return;
        }
    }
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.