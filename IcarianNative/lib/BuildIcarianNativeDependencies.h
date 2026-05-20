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

    if (a_configuration == BuildConfiguration_Debug || a_configuration == BuildConfiguration_DebugFast)
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
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    case TargetPlatform_LinuxSteam:
    {
        CUBE_CProject_AppendSource(&project, "glslang/OSDependent/Unix/ossource.cpp");

        break;
    }
    }

    CUBE_CProject_AppendIncludePath(&project, ".");
    CUBE_CProject_AppendIncludePath(&project, "../gen/glslang/include");

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
    case BuildConfiguration_DebugFast:
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");
        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");
        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

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

    if (a_configuration == BuildConfiguration_Debug || a_configuration == BuildConfiguration_DebugFast)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendDefine(&project, "ENABLE_OPT=1");

    CUBE_CProject_AppendIncludePaths(&project, 
        ".",
        "./External/spirv-tools/include",
        "../gen/glslang/include"
    );

    CUBE_CProject_AppendSources(&project, 
        "./SPIRV/GlslangToSpv.cpp",
        "./SPIRV/InReadableOrder.cpp",
        "./SPIRV/Logger.cpp",
        "./SPIRV/SpvBuilder.cpp",
        "./SPIRV/SpvPostProcess.cpp",
        "./SPIRV/doc.cpp",
        "./SPIRV/SpvTools.cpp",
        "./SPIRV/disassemble.cpp",
        "./SPIRV/CInterface/spirv_c_interface.cpp"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_DebugFast:
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");
        // CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        // CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");
        // CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        // CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    }

    return project;
}

CUBE_CProject BuildSPIRVToolsProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("SPIRV-Tools");
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

    CUBE_CProject_AppendIncludePaths(&project, 
        ".",
        "./include",
        "./external/spirv-headers/include",
        "../../../gen/glslang/SPIRV-Tools"
    );

    CUBE_CProject_AppendSources(&project, 
        "./source/assembly_grammar.cpp",
        "./source/binary.cpp",
        "./source/diagnostic.cpp",
        "./source/disassemble.cpp",
        "./source/enum_string_mapping.cpp",
        "./source/ext_inst.cpp",
        "./source/extensions.cpp",
        "./source/libspirv.cpp",
        "./source/name_mapper.cpp",
        "./source/opcode.cpp",
        "./source/operand.cpp",
        "./source/parsed_operand.cpp",
        "./source/print.cpp",
        "./source/software_version.cpp",
        "./source/spirv_endian.cpp",
        "./source/spirv_fuzzer_options.cpp",
        "./source/spirv_optimizer_options.cpp",
        "./source/spirv_reducer_options.cpp",
        "./source/spirv_target_env.cpp",
        "./source/spirv_validator_options.cpp",
        "./source/table.cpp",
        "./source/text.cpp",
        "./source/text_handler.cpp",
        "./source/to_string.cpp",

        "./source/val/validate.cpp",
        "./source/val/validate_adjacency.cpp",
        "./source/val/validate_annotation.cpp",
        "./source/val/validate_arithmetics.cpp",
        "./source/val/validate_atomics.cpp",
        "./source/val/validate_barriers.cpp",
        "./source/val/validate_bitwise.cpp",
        "./source/val/validate_builtins.cpp",
        "./source/val/validate_capability.cpp",
        "./source/val/validate_cfg.cpp",
        "./source/val/validate_composites.cpp",
        "./source/val/validate_constants.cpp",
        "./source/val/validate_conversion.cpp",
        "./source/val/validate_debug.cpp",
        "./source/val/validate_decorations.cpp",
        "./source/val/validate_derivatives.cpp",
        "./source/val/validate_extensions.cpp",
        "./source/val/validate_execution_limitations.cpp",
        "./source/val/validate_function.cpp",
        "./source/val/validate_id.cpp",
        "./source/val/validate_image.cpp",
        "./source/val/validate_interfaces.cpp",
        "./source/val/validate_instruction.cpp",
        "./source/val/validate_layout.cpp",
        "./source/val/validate_literals.cpp",
        "./source/val/validate_logicals.cpp",
        "./source/val/validate_memory.cpp",
        "./source/val/validate_memory_semantics.cpp",
        "./source/val/validate_mesh_shading.cpp",
        "./source/val/validate_misc.cpp",
        "./source/val/validate_mode_setting.cpp",
        "./source/val/validate_non_uniform.cpp",
        "./source/val/validate_primitives.cpp",
        "./source/val/validate_ray_query.cpp",
        "./source/val/validate_ray_tracing.cpp",
        "./source/val/validate_ray_tracing_reorder.cpp",
        "./source/val/validate_scopes.cpp",
        "./source/val/validate_small_type_uses.cpp",
        "./source/val/validate_type.cpp",
        "./source/val/basic_block.cpp",
        "./source/val/construct.cpp",
        "./source/val/function.cpp",
        "./source/val/instruction.cpp",
        "./source/val/validation_state.cpp",

        "./source/opt/fix_func_call_arguments.cpp",
        "./source/opt/aggressive_dead_code_elim_pass.cpp",
        "./source/opt/amd_ext_to_khr.cpp",
        "./source/opt/analyze_live_input_pass.cpp",
        "./source/opt/basic_block.cpp",
        "./source/opt/block_merge_pass.cpp",
        "./source/opt/block_merge_util.cpp",
        "./source/opt/build_module.cpp",
        "./source/opt/ccp_pass.cpp",
        "./source/opt/cfg_cleanup_pass.cpp",
        "./source/opt/cfg.cpp",
        "./source/opt/code_sink.cpp",
        "./source/opt/combine_access_chains.cpp",
        "./source/opt/compact_ids_pass.cpp",
        "./source/opt/composite.cpp",
        "./source/opt/const_folding_rules.cpp",
        "./source/opt/constants.cpp",
        "./source/opt/control_dependence.cpp",
        "./source/opt/convert_to_sampled_image_pass.cpp",
        "./source/opt/convert_to_half_pass.cpp",
        "./source/opt/copy_prop_arrays.cpp",
        "./source/opt/dataflow.cpp",
        "./source/opt/dead_branch_elim_pass.cpp",
        "./source/opt/dead_insert_elim_pass.cpp",
        "./source/opt/dead_variable_elimination.cpp",
        "./source/opt/decoration_manager.cpp",
        "./source/opt/debug_info_manager.cpp",
        "./source/opt/def_use_manager.cpp",
        "./source/opt/desc_sroa.cpp",
        "./source/opt/desc_sroa_util.cpp",
        "./source/opt/dominator_analysis.cpp",
        "./source/opt/dominator_tree.cpp",
        "./source/opt/eliminate_dead_constant_pass.cpp",
        "./source/opt/eliminate_dead_functions_pass.cpp",
        "./source/opt/eliminate_dead_functions_util.cpp",
        "./source/opt/eliminate_dead_io_components_pass.cpp",
        "./source/opt/eliminate_dead_members_pass.cpp",
        "./source/opt/eliminate_dead_output_stores_pass.cpp",
        "./source/opt/feature_manager.cpp",
        "./source/opt/fix_storage_class.cpp",
        "./source/opt/flatten_decoration_pass.cpp",
        "./source/opt/fold.cpp",
        "./source/opt/folding_rules.cpp",
        "./source/opt/fold_spec_constant_op_and_composite_pass.cpp",
        "./source/opt/freeze_spec_constant_value_pass.cpp",
        "./source/opt/function.cpp",
        "./source/opt/graphics_robust_access_pass.cpp",
        "./source/opt/if_conversion.cpp",
        "./source/opt/inline_exhaustive_pass.cpp",
        "./source/opt/inline_opaque_pass.cpp",
        "./source/opt/inline_pass.cpp",
        "./source/opt/inst_debug_printf_pass.cpp",
        "./source/opt/instruction.cpp",
        "./source/opt/instruction_list.cpp",
        "./source/opt/instrument_pass.cpp",
        "./source/opt/interface_var_sroa.cpp",
        "./source/opt/invocation_interlock_placement_pass.cpp",
        "./source/opt/interp_fixup_pass.cpp",
        "./source/opt/opextinst_forward_ref_fixup_pass.cpp",
        "./source/opt/ir_context.cpp",
        "./source/opt/ir_loader.cpp",
        "./source/opt/licm_pass.cpp",
        "./source/opt/liveness.cpp",
        "./source/opt/local_access_chain_convert_pass.cpp",
        "./source/opt/local_redundancy_elimination.cpp",
        "./source/opt/local_single_block_elim_pass.cpp",
        "./source/opt/local_single_store_elim_pass.cpp",
        "./source/opt/loop_dependence.cpp",
        "./source/opt/loop_dependence_helpers.cpp",
        "./source/opt/loop_descriptor.cpp",
        "./source/opt/loop_fission.cpp",
        "./source/opt/loop_fusion.cpp",
        "./source/opt/loop_fusion_pass.cpp",
        "./source/opt/loop_peeling.cpp",
        "./source/opt/loop_utils.cpp",
        "./source/opt/loop_unroller.cpp",
        "./source/opt/loop_unswitch_pass.cpp",
        "./source/opt/mem_pass.cpp",
        "./source/opt/merge_return_pass.cpp",
        "./source/opt/modify_maximal_reconvergence.cpp",
        "./source/opt/module.cpp",
        "./source/opt/optimizer.cpp",
        "./source/opt/pass.cpp",
        "./source/opt/pass_manager.cpp",
        "./source/opt/private_to_local_pass.cpp",
        "./source/opt/propagator.cpp",
        "./source/opt/reduce_load_size.cpp",
        "./source/opt/redundancy_elimination.cpp",
        "./source/opt/register_pressure.cpp",
        "./source/opt/relax_float_ops_pass.cpp",
        "./source/opt/remove_dontinline_pass.cpp",
        "./source/opt/remove_duplicates_pass.cpp",
        "./source/opt/remove_unused_interface_variables_pass.cpp",
        "./source/opt/replace_desc_array_access_using_var_index.cpp",
        "./source/opt/replace_invalid_opc.cpp",
        "./source/opt/scalar_analysis.cpp",
        "./source/opt/scalar_analysis_simplification.cpp",
        "./source/opt/scalar_replacement_pass.cpp",
        "./source/opt/set_spec_constant_default_value_pass.cpp",
        "./source/opt/simplification_pass.cpp",
        "./source/opt/spread_volatile_semantics.cpp",
        "./source/opt/ssa_rewrite_pass.cpp",
        "./source/opt/strength_reduction_pass.cpp",
        "./source/opt/strip_debug_info_pass.cpp",
        "./source/opt/strip_nonsemantic_info_pass.cpp",
        "./source/opt/struct_cfg_analysis.cpp",
        "./source/opt/struct_packing_pass.cpp",
        "./source/opt/switch_descriptorset_pass.cpp",
        "./source/opt/trim_capabilities_pass.cpp",
        "./source/opt/type_manager.cpp",
        "./source/opt/types.cpp",
        "./source/opt/unify_const_pass.cpp",
        "./source/opt/upgrade_memory_model.cpp",
        "./source/opt/value_number_table.cpp",
        "./source/opt/vector_dce.cpp",
        "./source/opt/workaround1209.cpp",
        "./source/opt/wrap_opkill.cpp",

        "./source/util/bit_vector.cpp",
        "./source/util/parse_number.cpp",
        "./source/util/string_utils.cpp"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_DebugFast:
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");
        // CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        // CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");
        // CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        // CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

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

    if (a_configuration == BuildConfiguration_Debug || a_configuration == BuildConfiguration_DebugFast)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    // CUBE_CProject_AppendDefine(&project, "JPH_CROSS_PLATFORM_DETERMINISTIC");
    CUBE_CProject_AppendDefine(&project, "JPH_OBJECT_LAYER_BITS=16");

    CUBE_CProject_AppendSystemIncludePath(&project, ".");

    CUBE_CProject_AppendSources(&project, 
        "./Jolt/AABBTree/AABBTreeBuilder.cpp",

        "./Jolt/Core/Color.cpp",
        "./Jolt/Core/Factory.cpp",
        "./Jolt/Core/IssueReporting.cpp",
        "./Jolt/Core/JobSystemThreadPool.cpp",
        "./Jolt/Core/JobSystemWithBarrier.cpp",
        "./Jolt/Core/LinearCurve.cpp",
        "./Jolt/Core/Memory.cpp",
        "./Jolt/Core/Profiler.cpp",
        "./Jolt/Core/RTTI.cpp",
        "./Jolt/Core/Semaphore.cpp",
        "./Jolt/Core/StringTools.cpp",
        "./Jolt/Core/TickCounter.cpp",

        "./Jolt/Geometry/ConvexHullBuilder.cpp",
        "./Jolt/Geometry/ConvexHullBuilder2D.cpp",
        "./Jolt/Geometry/Indexify.cpp",
        "./Jolt/Geometry/OrientedBox.cpp",

        "./Jolt/Math/Vec3.cpp",

        "./Jolt/ObjectStream/ObjectStream.cpp",
        "./Jolt/ObjectStream/ObjectStreamBinaryIn.cpp",
        "./Jolt/ObjectStream/ObjectStreamBinaryOut.cpp",
        "./Jolt/ObjectStream/ObjectStreamIn.cpp",
        "./Jolt/ObjectStream/ObjectStreamOut.cpp",
        "./Jolt/ObjectStream/ObjectStreamTextIn.cpp",
        "./Jolt/ObjectStream/ObjectStreamTextOut.cpp",
        "./Jolt/ObjectStream/SerializableObject.cpp",
        "./Jolt/ObjectStream/TypeDeclarations.cpp",

        "./Jolt/Physics/Body/Body.cpp",
        "./Jolt/Physics/Body/BodyCreationSettings.cpp",
        "./Jolt/Physics/Body/BodyInterface.cpp",
        "./Jolt/Physics/Body/BodyManager.cpp",
        "./Jolt/Physics/Body/MassProperties.cpp",
        "./Jolt/Physics/Body/MotionProperties.cpp",

        "./Jolt/Physics/Character/Character.cpp",
        "./Jolt/Physics/Character/CharacterBase.cpp",
        "./Jolt/Physics/Character/CharacterVirtual.cpp",

        "./Jolt/Physics/Collision/BroadPhase/BroadPhase.cpp",
        "./Jolt/Physics/Collision/BroadPhase/BroadPhaseBruteForce.cpp",
        "./Jolt/Physics/Collision/BroadPhase/BroadPhaseQuadTree.cpp",
        "./Jolt/Physics/Collision/BroadPhase/QuadTree.cpp",

        "./Jolt/Physics/Collision/CastConvexVsTriangles.cpp",
        "./Jolt/Physics/Collision/CastSphereVsTriangles.cpp",
        "./Jolt/Physics/Collision/CollideConvexVsTriangles.cpp",
        "./Jolt/Physics/Collision/CollideSphereVsTriangles.cpp",
        "./Jolt/Physics/Collision/CollisionDispatch.cpp",
        "./Jolt/Physics/Collision/CollisionGroup.cpp",
        "./Jolt/Physics/Collision/EstimateCollisionResponse.cpp",
        "./Jolt/Physics/Collision/GroupFilter.cpp",
        "./Jolt/Physics/Collision/GroupFilterTable.cpp",
        "./Jolt/Physics/Collision/ManifoldBetweenTwoFaces.cpp",
        "./Jolt/Physics/Collision/NarrowPhaseQuery.cpp",
        "./Jolt/Physics/Collision/NarrowPhaseStats.cpp",
        "./Jolt/Physics/Collision/PhysicsMaterial.cpp",
        "./Jolt/Physics/Collision/PhysicsMaterialSimple.cpp",

        "./Jolt/Physics/Collision/Shape/BoxShape.cpp",
        "./Jolt/Physics/Collision/Shape/CapsuleShape.cpp",
        "./Jolt/Physics/Collision/Shape/CompoundShape.cpp",
        "./Jolt/Physics/Collision/Shape/ConvexHullShape.cpp",
        "./Jolt/Physics/Collision/Shape/ConvexShape.cpp",
        "./Jolt/Physics/Collision/Shape/CylinderShape.cpp",
        "./Jolt/Physics/Collision/Shape/DecoratedShape.cpp",
        "./Jolt/Physics/Collision/Shape/EmptyShape.cpp",
        "./Jolt/Physics/Collision/Shape/HeightFieldShape.cpp",
        "./Jolt/Physics/Collision/Shape/MeshShape.cpp",
        "./Jolt/Physics/Collision/Shape/MutableCompoundShape.cpp",
        "./Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.cpp",
        "./Jolt/Physics/Collision/Shape/PlaneShape.cpp",
        "./Jolt/Physics/Collision/Shape/RotatedTranslatedShape.cpp",
        "./Jolt/Physics/Collision/Shape/ScaledShape.cpp",
        "./Jolt/Physics/Collision/Shape/Shape.cpp",
        "./Jolt/Physics/Collision/Shape/SphereShape.cpp",
        "./Jolt/Physics/Collision/Shape/StaticCompoundShape.cpp",
        "./Jolt/Physics/Collision/Shape/TaperedCapsuleShape.cpp",
        "./Jolt/Physics/Collision/Shape/TaperedCylinderShape.cpp",
        "./Jolt/Physics/Collision/Shape/TriangleShape.cpp",

        "./Jolt/Physics/Collision/TransformedShape.cpp",

        "./Jolt/Physics/Constraints/ConeConstraint.cpp",
        "./Jolt/Physics/Constraints/Constraint.cpp",
        "./Jolt/Physics/Constraints/ConstraintManager.cpp",
        "./Jolt/Physics/Constraints/ContactConstraintManager.cpp",
        "./Jolt/Physics/Constraints/DistanceConstraint.cpp",
        "./Jolt/Physics/Constraints/FixedConstraint.cpp",
        "./Jolt/Physics/Constraints/GearConstraint.cpp",
        "./Jolt/Physics/Constraints/HingeConstraint.cpp",
        "./Jolt/Physics/Constraints/MotorSettings.cpp",
        "./Jolt/Physics/Constraints/PathConstraint.cpp",
        "./Jolt/Physics/Constraints/PathConstraintPath.cpp",
        "./Jolt/Physics/Constraints/PathConstraintPathHermite.cpp",
        "./Jolt/Physics/Constraints/PointConstraint.cpp",
        "./Jolt/Physics/Constraints/PulleyConstraint.cpp",
        "./Jolt/Physics/Constraints/RackAndPinionConstraint.cpp",
        "./Jolt/Physics/Constraints/SixDOFConstraint.cpp",
        "./Jolt/Physics/Constraints/SliderConstraint.cpp",
        "./Jolt/Physics/Constraints/SpringSettings.cpp",
        "./Jolt/Physics/Constraints/SwingTwistConstraint.cpp",
        "./Jolt/Physics/Constraints/TwoBodyConstraint.cpp",

        "./Jolt/Physics/Ragdoll/Ragdoll.cpp",

        "./Jolt/Physics/SoftBody/SoftBodyCreationSettings.cpp",
        "./Jolt/Physics/SoftBody/SoftBodyMotionProperties.cpp",
        "./Jolt/Physics/SoftBody/SoftBodyShape.cpp",
        "./Jolt/Physics/SoftBody/SoftBodySharedSettings.cpp",

        "./Jolt/Physics/DeterminismLog.cpp",
        "./Jolt/Physics/IslandBuilder.cpp",
        "./Jolt/Physics/LargeIslandSplitter.cpp",
        "./Jolt/Physics/PhysicsScene.cpp",
        "./Jolt/Physics/PhysicsSystem.cpp",
        "./Jolt/Physics/PhysicsUpdateContext.cpp",

        "./Jolt/Physics/StateRecorderImpl.cpp",

        "./Jolt/Physics/Vehicle/MotorcycleController.cpp",
        "./Jolt/Physics/Vehicle/TrackedVehicleController.cpp",
        "./Jolt/Physics/Vehicle/VehicleAntiRollBar.cpp",
        "./Jolt/Physics/Vehicle/VehicleCollisionTester.cpp",
        "./Jolt/Physics/Vehicle/VehicleConstraint.cpp",
        "./Jolt/Physics/Vehicle/VehicleController.cpp",
        "./Jolt/Physics/Vehicle/VehicleDifferential.cpp",
        "./Jolt/Physics/Vehicle/VehicleEngine.cpp",
        "./Jolt/Physics/Vehicle/VehicleTrack.cpp",
        "./Jolt/Physics/Vehicle/VehicleTransmission.cpp",
        "./Jolt/Physics/Vehicle/Wheel.cpp",
        "./Jolt/Physics/Vehicle/WheeledVehicleController.cpp",

        "./Jolt/RegisterTypes.cpp",

        "./Jolt/Renderer/DebugRenderer.cpp",
        "./Jolt/Renderer/DebugRendererPlayback.cpp",
        "./Jolt/Renderer/DebugRendererRecorder.cpp",

        "./Jolt/Skeleton/SkeletalAnimation.cpp",
        "./Jolt/Skeleton/Skeleton.cpp",
        "./Jolt/Skeleton/SkeletonMapper.cpp",
        "./Jolt/Skeleton/SkeletonPose.cpp",

        "./Jolt/TriangleSplitter/TriangleSplitter.cpp",
        "./Jolt/TriangleSplitter/TriangleSplitterBinning.cpp",
        "./Jolt/TriangleSplitter/TriangleSplitterMean.cpp"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_DebugFast:
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.1");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.1");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-O3");

        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    }

    CUBE_CProject_AppendCFlag(&project, "-ffp-contract=off");

    return project;
}

DependencyProject* BuildIcarianNativeIDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    *a_count = 4;

    DependencyProject* projects = (DependencyProject*)malloc(sizeof(DependencyProject) * (*a_count));

    projects[0].Project = BuildGLSLangProject(a_targetPlatform, a_configuration);
    projects[0].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[1].Project = BuildSPIRVProject(a_targetPlatform, a_configuration);
    projects[1].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[2].Project = BuildSPIRVToolsProject(a_targetPlatform, a_configuration);
    projects[2].WorkingDirectory = "IcarianNative/lib/glslang/External/spirv-tools";

    projects[3].Project = BuildJoltPhysicsProject(a_targetPlatform, a_configuration);
    projects[3].WorkingDirectory = "IcarianNative/lib/JoltPhysics";

    return projects;
}

#ifdef __cplusplus
}
#endif

#endif
