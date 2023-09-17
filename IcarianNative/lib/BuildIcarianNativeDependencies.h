#ifndef INCLUDED_HEADER_BUILDICARIANNATIVEDEPENDENCIES
#define INCLUDED_HEADER_BUILDICARIANNATIVEDEPENDENCIES

#include "../../BuildBase.h"

#ifdef __cplusplus
extern "C" {
#endif

CUBE_CProject BuildGLSLangProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("glslang");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendIncludePath(&project, "glslang/OSDependent");

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendDefine(&project, "WIN32");

        CUBE_CProject_AppendSource(&project, "glslang/OSDependent/Windows/ossource.cpp");

        break;
    }
    case TargetPlatform_Linux:
    {
        CUBE_CProject_AppendSource(&project, "glslang/OSDependent/Unix/ossource.cpp");

        break;
    }
    }

    CUBE_CProject_AppendIncludePath(&project, ".");

    CUBE_CProject_AppendSource(&project, "glslang/GenericCodeGen/CodeGen.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/GenericCodeGen/Link.cpp");

    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/glslang_tab.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/attribute.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/Constant.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/iomapper.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/InfoSink.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/Initialize.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/IntermTraverse.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/Intermediate.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/ParseContextBase.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/ParseHelper.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/PoolAlloc.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/RemoveTree.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/Scan.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/ShaderLang.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/SpirvIntrinsics.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/SymbolTable.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/Versions.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/intermOut.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/limits.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/linkValidate.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/parseConst.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/reflection.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/preprocessor/Pp.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/preprocessor/PpAtom.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/preprocessor/PpContext.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/preprocessor/PpScanner.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/preprocessor/PpTokens.cpp");
    CUBE_CProject_AppendSource(&project, "glslang/MachineIndependent/propagateNoContraction.cpp");

    CUBE_CProject_AppendSource(&project, "glslang/CInterface/glslang_c_interface.cpp");

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    return project;
}

CUBE_CProject BuildOGLCompilersProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("OGLCompiler");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendSource(&project, "OGLCompilersDLL/InitializeDll.cpp");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    return project;
}
CUBE_CProject BuildSPIRVProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("SPIRV");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendIncludePath(&project, ".");

    CUBE_CProject_AppendSource(&project, "SPIRV/GlslangToSpv.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/InReadableOrder.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/Logger.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/SpvBuilder.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/SpvPostProcess.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/doc.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/SpvTools.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/disassemble.cpp");
    CUBE_CProject_AppendSource(&project, "SPIRV/CInterface/spirv_c_interface.cpp");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    return project;
}

CUBE_CProject BuildJoltPhysicsProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("Jolt");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendDefine(&project, "JPH_CROSS_PLATFORM_DETERMINISTIC");
    CUBE_CProject_AppendDefine(&project, "JPH_OBJECT_LAYER_BITS=16");

    CUBE_CProject_AppendSystemIncludePath(&project, ".");

    CUBE_CProject_AppendSource(&project, "Jolt/AABBTree/AABBTreeBuilder.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/Color.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/Factory.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/IssueReporting.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/JobSystemThreadPool.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/JobSystemWithBarrier.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/LinearCurve.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/Memory.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/Profiler.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/RTTI.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/Semaphore.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/StringTools.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Core/TickCounter.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Geometry/ConvexHullBuilder.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Geometry/ConvexHullBuilder2D.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Geometry/Indexify.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Geometry/OrientedBox.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Math/Vec3.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/ObjectStream.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/ObjectStreamBinaryIn.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/ObjectStreamBinaryOut.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/ObjectStreamIn.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/ObjectStreamOut.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/ObjectStreamTextIn.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/ObjectStreamTextOut.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/SerializableObject.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/ObjectStream/TypeDeclarations.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Body/Body.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Body/BodyAccess.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Body/BodyCreationSettings.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Body/BodyInterface.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Body/BodyManager.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Body/MassProperties.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Body/MotionProperties.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Character/Character.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Character/CharacterBase.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Character/CharacterVirtual.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/BroadPhase/BroadPhase.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/BroadPhase/BroadPhaseBruteForce.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/BroadPhase/BroadPhaseQuadTree.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/BroadPhase/QuadTree.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/CastConvexVsTriangles.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/CastSphereVsTriangles.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/CollideConvexVsTriangles.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/CollideSphereVsTriangles.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/CollisionDispatch.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/CollisionGroup.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/EstimateCollisionResponse.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/GroupFilter.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/GroupFilterTable.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/ManifoldBetweenTwoFaces.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/NarrowPhaseQuery.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/NarrowPhaseStats.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/PhysicsMaterial.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/PhysicsMaterialSimple.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/BoxShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/CapsuleShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/CompoundShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/ConvexHullShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/ConvexShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/CylinderShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/DecoratedShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/HeightFieldShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/MeshShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/MutableCompoundShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/ScaledShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/Shape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/SphereShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/StaticCompoundShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/TaperedCapsuleShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/Shape/TriangleShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Collision/TransformedShape.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/ConeConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/Constraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/ConstraintManager.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/ContactConstraintManager.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/DistanceConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/FixedConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/GearConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/HingeConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/MotorSettings.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/PathConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/PathConstraintPath.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/PathConstraintPathHermite.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/PointConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/PulleyConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/RackAndPinionConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/SixDOFConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/SliderConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/SwingTwistConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Constraints/TwoBodyConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/DeterminismLog.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/IslandBuilder.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/LargeIslandSplitter.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/PhysicsLock.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/PhysicsScene.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/PhysicsSystem.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/PhysicsUpdateContext.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Ragdoll/Ragdoll.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/StateRecorderImpl.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/MotorcycleController.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/TrackedVehicleController.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleAntiRollBar.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleCollisionTester.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleConstraint.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleController.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleDifferential.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleEngine.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleTrack.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/VehicleTransmission.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/Wheel.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Physics/Vehicle/WheeledVehicleController.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/RegisterTypes.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Renderer/DebugRenderer.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Renderer/DebugRendererPlayback.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Renderer/DebugRendererRecorder.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Skeleton/SkeletalAnimation.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Skeleton/Skeleton.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Skeleton/SkeletonMapper.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/Skeleton/SkeletonPose.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleGrouper/TriangleGrouperClosestCentroid.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleGrouper/TriangleGrouperMorton.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleSplitter/TriangleSplitter.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleSplitter/TriangleSplitterBinning.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleSplitter/TriangleSplitterFixedLeafSize.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleSplitter/TriangleSplitterLongestAxis.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleSplitter/TriangleSplitterMean.cpp");
    CUBE_CProject_AppendSource(&project, "Jolt/TriangleSplitter/TriangleSplitterMorton.cpp");

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    return project;   
}

DependencyProject* BuildIcarianNativeIDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    *a_count = 4;

    DependencyProject* projects = (DependencyProject*)malloc(sizeof(DependencyProject) * (*a_count));
    
    projects[0].Project = BuildGLSLangProject(a_targetPlatform, a_configuration);
    projects[0].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[1].Project = BuildOGLCompilersProject(a_targetPlatform, a_configuration);
    projects[1].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[2].Project = BuildSPIRVProject(a_targetPlatform, a_configuration);
    projects[2].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[3].Project = BuildJoltPhysicsProject(a_targetPlatform, a_configuration);
    projects[3].WorkingDirectory = "IcarianNative/lib/JoltPhysics";

    return projects;
}

#ifdef __cplusplus
}
#endif

#endif