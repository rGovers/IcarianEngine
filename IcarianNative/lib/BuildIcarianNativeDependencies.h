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
    CUBE_CProject_AppendIncludePath(&project, "../gen/glslang/include");

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

    CUBE_CProject_AppendIncludePath(&project, ".");
    CUBE_CProject_AppendIncludePath(&project, "./include");
    CUBE_CProject_AppendIncludePath(&project, "external/spirv-headers/include");
    CUBE_CProject_AppendIncludePath(&project, "../gen/SPIRV-Tools/include");

    CUBE_CProject_AppendSource(&project, "source/assembly_grammar.cpp");
    CUBE_CProject_AppendSource(&project, "source/binary.cpp");
    CUBE_CProject_AppendSource(&project, "source/diagnostic.cpp");
    CUBE_CProject_AppendSource(&project, "source/disassemble.cpp");
    CUBE_CProject_AppendSource(&project, "source/enum_string_mapping.cpp");
    CUBE_CProject_AppendSource(&project, "source/ext_inst.cpp");
    CUBE_CProject_AppendSource(&project, "source/extensions.cpp");
    CUBE_CProject_AppendSource(&project, "source/libspirv.cpp");
    CUBE_CProject_AppendSource(&project, "source/name_mapper.cpp");
    CUBE_CProject_AppendSource(&project, "source/opcode.cpp");
    CUBE_CProject_AppendSource(&project, "source/operand.cpp");
    CUBE_CProject_AppendSource(&project, "source/parsed_operand.cpp");
    CUBE_CProject_AppendSource(&project, "source/print.cpp");
    CUBE_CProject_AppendSource(&project, "source/software_version.cpp");
    CUBE_CProject_AppendSource(&project, "source/spirv_endian.cpp");
    CUBE_CProject_AppendSource(&project, "source/spirv_fuzzer_options.cpp");
    CUBE_CProject_AppendSource(&project, "source/spirv_optimizer_options.cpp");
    CUBE_CProject_AppendSource(&project, "source/spirv_reducer_options.cpp");
    CUBE_CProject_AppendSource(&project, "source/spirv_target_env.cpp");
    CUBE_CProject_AppendSource(&project, "source/spirv_validator_options.cpp");
    CUBE_CProject_AppendSource(&project, "source/table.cpp");
    CUBE_CProject_AppendSource(&project, "source/text.cpp");
    CUBE_CProject_AppendSource(&project, "source/text_handler.cpp");

    CUBE_CProject_AppendSource(&project, "source/val/validate.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_adjacency.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_annotation.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_arithmetics.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_atomics.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_barriers.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_bitwise.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_builtins.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_capability.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_cfg.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_composites.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_constants.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_conversion.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_debug.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_decorations.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_derivatives.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_extensions.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_execution_limitations.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_function.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_id.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_image.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_interfaces.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_instruction.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_layout.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_literals.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_logicals.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_memory.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_memory_semantics.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_mesh_shading.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_misc.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_mode_setting.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_non_uniform.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_primitives.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_ray_query.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_ray_tracing.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_ray_tracing_reorder.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_scopes.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_small_type_uses.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validate_type.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/basic_block.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/construct.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/function.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/instruction.cpp");
    CUBE_CProject_AppendSource(&project, "source/val/validation_state.cpp");

    CUBE_CProject_AppendSource(&project, "source/opt/fix_func_call_arguments.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/aggressive_dead_code_elim_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/amd_ext_to_khr.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/analyze_live_input_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/basic_block.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/block_merge_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/block_merge_util.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/build_module.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/ccp_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/cfg_cleanup_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/cfg.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/code_sink.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/combine_access_chains.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/compact_ids_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/composite.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/const_folding_rules.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/constants.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/control_dependence.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/convert_to_sampled_image_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/convert_to_half_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/copy_prop_arrays.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/dataflow.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/dead_branch_elim_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/dead_insert_elim_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/dead_variable_elimination.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/decoration_manager.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/debug_info_manager.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/def_use_manager.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/desc_sroa.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/desc_sroa_util.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/dominator_analysis.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/dominator_tree.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/eliminate_dead_constant_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/eliminate_dead_functions_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/eliminate_dead_functions_util.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/eliminate_dead_io_components_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/eliminate_dead_members_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/eliminate_dead_output_stores_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/feature_manager.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/fix_storage_class.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/flatten_decoration_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/fold.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/folding_rules.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/fold_spec_constant_op_and_composite_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/freeze_spec_constant_value_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/function.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/graphics_robust_access_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/if_conversion.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/inline_exhaustive_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/inline_opaque_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/inline_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/inst_bindless_check_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/inst_buff_addr_check_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/inst_debug_printf_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/instruction.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/instruction_list.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/instrument_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/interface_var_sroa.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/invocation_interlock_placement_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/interp_fixup_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/ir_context.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/ir_loader.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/licm_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/liveness.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/local_access_chain_convert_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/local_redundancy_elimination.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/local_single_block_elim_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/local_single_store_elim_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_dependence.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_dependence_helpers.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_descriptor.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_fission.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_fusion.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_fusion_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_peeling.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_utils.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_unroller.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/loop_unswitch_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/mem_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/merge_return_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/module.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/optimizer.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/pass_manager.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/private_to_local_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/propagator.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/reduce_load_size.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/redundancy_elimination.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/register_pressure.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/relax_float_ops_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/remove_dontinline_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/remove_duplicates_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/remove_unused_interface_variables_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/replace_desc_array_access_using_var_index.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/replace_invalid_opc.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/scalar_analysis.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/scalar_analysis_simplification.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/scalar_replacement_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/set_spec_constant_default_value_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/simplification_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/spread_volatile_semantics.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/ssa_rewrite_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/strength_reduction_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/strip_debug_info_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/strip_nonsemantic_info_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/struct_cfg_analysis.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/switch_descriptorset_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/trim_capabilities_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/type_manager.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/types.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/unify_const_pass.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/upgrade_memory_model.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/value_number_table.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/vector_dce.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/workaround1209.cpp");
    CUBE_CProject_AppendSource(&project, "source/opt/wrap_opkill.cpp");

    CUBE_CProject_AppendSource(&project, "source/util/bit_vector.cpp");
    CUBE_CProject_AppendSource(&project, "source/util/parse_number.cpp");
    CUBE_CProject_AppendSource(&project, "source/util/string_utils.cpp");

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
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    CUBE_CProject_AppendCFlag(&project, "-ffp-contract=off");

    return project;   
}

CUBE_CProject BuildOpenALSoft(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("OpenALSoft");
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
    CUBE_CProject_AppendIncludePath(&project, "common");
    CUBE_CProject_AppendIncludePath(&project, "include");

    CUBE_CProject_AppendDefine(&project, "RESTRICT=");
    CUBE_CProject_AppendDefine(&project, "AL_LIBTYPE_STATIC");
    CUBE_CProject_AppendDefine(&project, "AL_ALEXT_PROTOTYPES");

    {
        // Common
        CUBE_CProject_AppendSource(&project, "common/alcomplex.cpp");
        CUBE_CProject_AppendSource(&project, "common/alfstream.cpp");
        CUBE_CProject_AppendSource(&project, "common/almalloc.cpp");
        // CUBE_CProject_AppendSource(&project, "common/alsem.cpp");
        CUBE_CProject_AppendSource(&project, "common/alstring.cpp");
        // CUBE_CProject_AppendSource(&project, "common/althrd_setname.cpp");
        CUBE_CProject_AppendSource(&project, "common/dynload.cpp");
        CUBE_CProject_AppendSource(&project, "common/polyphase_resampler.cpp");
        CUBE_CProject_AppendSource(&project, "common/ringbuffer.cpp");
        CUBE_CProject_AppendSource(&project, "common/strutils.cpp");
        CUBE_CProject_AppendSource(&project, "common/threads.cpp");
    }

    {
        // Core
        CUBE_CProject_AppendSource(&project, "core/ambdec.cpp");
        CUBE_CProject_AppendSource(&project, "core/ambidefs.cpp");
        CUBE_CProject_AppendSource(&project, "core/bformatdec.cpp");
        CUBE_CProject_AppendSource(&project, "core/bs2b.cpp");
        CUBE_CProject_AppendSource(&project, "core/bsinc_tables.cpp");
        CUBE_CProject_AppendSource(&project, "core/buffer_storage.cpp");
        CUBE_CProject_AppendSource(&project, "core/context.cpp");
        CUBE_CProject_AppendSource(&project, "core/converter.cpp");
        CUBE_CProject_AppendSource(&project, "core/cpu_caps.cpp");
        CUBE_CProject_AppendSource(&project, "core/cubic_tables.cpp");
        // CUBE_CProject_AppendSource(&project, "core/dbus_wrap.cpp");
        CUBE_CProject_AppendSource(&project, "core/devformat.cpp");
        CUBE_CProject_AppendSource(&project, "core/device.cpp");
        CUBE_CProject_AppendSource(&project, "core/effectslot.cpp");
        CUBE_CProject_AppendSource(&project, "core/except.cpp");     
        CUBE_CProject_AppendSource(&project, "core/fmt_traits.cpp");
        CUBE_CProject_AppendSource(&project, "core/fpu_ctrl.cpp");
        CUBE_CProject_AppendSource(&project, "core/helpers.cpp");
        CUBE_CProject_AppendSource(&project, "core/hrtf.cpp");
        CUBE_CProject_AppendSource(&project, "core/logging.cpp");
        CUBE_CProject_AppendSource(&project, "core/mastering.cpp");
        CUBE_CProject_AppendSource(&project, "core/mixer.cpp");
        // CUBE_CProject_AppendSource(&project, "core/rkit.cpp");
        CUBE_CProject_AppendSource(&project, "core/uhjfilter.cpp");
        CUBE_CProject_AppendSource(&project, "core/uiddefs.cpp");
        CUBE_CProject_AppendSource(&project, "core/voice.cpp");

        CUBE_CProject_AppendSource(&project, "core/filters/biquad.cpp");
        CUBE_CProject_AppendSource(&project, "core/filters/nfc.cpp");
        CUBE_CProject_AppendSource(&project, "core/filters/splitter.cpp");

        CUBE_CProject_AppendSource(&project, "core/mixer/mixer_c.cpp");
    }

    {
        // OpenAL
        CUBE_CProject_AppendSource(&project, "al/auxeffectslot.cpp");
        CUBE_CProject_AppendSource(&project, "al/buffer.cpp");
        // CUBE_CProject_AppendSource(&project, "al/debug.cpp");
        CUBE_CProject_AppendSource(&project, "al/effect.cpp");
        CUBE_CProject_AppendSource(&project, "al/error.cpp");
        CUBE_CProject_AppendSource(&project, "al/event.cpp");
        CUBE_CProject_AppendSource(&project, "al/extension.cpp");
        CUBE_CProject_AppendSource(&project, "al/filter.cpp");
        CUBE_CProject_AppendSource(&project, "al/listener.cpp");
        CUBE_CProject_AppendSource(&project, "al/source.cpp");
        CUBE_CProject_AppendSource(&project, "al/state.cpp");

        CUBE_CProject_AppendSource(&project, "al/effects/autowah.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/chorus.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/compressor.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/convolution.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/dedicated.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/distortion.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/echo.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/effects.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/equalizer.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/fshifter.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/modulator.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/null.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/pshifter.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/reverb.cpp");
        CUBE_CProject_AppendSource(&project, "al/effects/vmorpher.cpp");
    }

    {
        // ALC
        CUBE_CProject_AppendSource(&project, "alc/alc.cpp");
        CUBE_CProject_AppendSource(&project, "alc/alconfig.cpp");
        CUBE_CProject_AppendSource(&project, "alc/alu.cpp");
        CUBE_CProject_AppendSource(&project, "alc/context.cpp");
        CUBE_CProject_AppendSource(&project, "alc/device.cpp");
        // CUBE_CProject_AppendSource(&project, "alc/events.cpp");
        CUBE_CProject_AppendSource(&project, "alc/panning.cpp");

        CUBE_CProject_AppendSource(&project, "alc/effects/autowah.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/chorus.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/compressor.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/convolution.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/dedicated.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/distortion.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/echo.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/equalizer.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/fshifter.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/modulator.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/null.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/pshifter.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/reverb.cpp");
        CUBE_CProject_AppendSource(&project, "alc/effects/vmorpher.cpp");

        CUBE_CProject_AppendSource(&project, "alc/backends/base.cpp");
        CUBE_CProject_AppendSource(&project, "alc/backends/null.cpp");
        CUBE_CProject_AppendSource(&project, "alc/backends/loopback.cpp");
    }

    {
        // EAX
        // CUBE_CProject_AppendSource(&project, "al/eax/api.cpp");
        // CUBE_CProject_AppendSource(&project, "al/eax/call.cpp");
        // CUBE_CProject_AppendSource(&project, "al/eax/exception.cpp");
        // CUBE_CProject_AppendSource(&project, "al/eax/fx_slot_index.cpp");
        // CUBE_CProject_AppendSource(&project, "al/eax/fx_slots.cpp");
        // CUBE_CProject_AppendSource(&project, "al/eax/utils.cpp");
    }

    {
        // SIMD
        CUBE_CProject_AppendSource(&project, "core/mixer/mixer_sse.cpp");
        CUBE_CProject_AppendSource(&project, "core/mixer/mixer_sse2.cpp");
    }

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendDefine(&project, "WIN32");
        CUBE_CProject_AppendDefine(&project, "_WIN32");

        CUBE_CProject_AppendIncludePath(&project, "../gen/openal/platform/windows/include");

        // WASAPI cannot seem to get to work
        // CUBE_CProject_AppendSource(&project, "alc/backends/wasapi.cpp");
        CUBE_CProject_AppendSource(&project, "alc/backends/winmm.cpp");
        // CUBE_CProject_AppendSource(&project, "alc/backends/dsound.cpp");
        // DirectSound needs a bit more work to get to work with this build system

        CUBE_CProject_AppendCFlag(&project, "-municode");

        break;
    }
    case TargetPlatform_Linux:
    {
        CUBE_CProject_AppendIncludePath(&project, "../gen/openal/platform/linux/include");

        CUBE_CProject_AppendSource(&project, "alc/backends/alsa.cpp");
        CUBE_CProject_AppendSource(&project, "alc/backends/jack.cpp");
        CUBE_CProject_AppendSource(&project, "alc/backends/oss.cpp");
        // CUBE_CProject_AppendSource(&project, "alc/backends/pipewire.cpp");
        // This one is being a prick ^
        CUBE_CProject_AppendSource(&project, "alc/backends/portaudio.cpp");
        CUBE_CProject_AppendSource(&project, "alc/backends/pulseaudio.cpp");
        CUBE_CProject_AppendSource(&project, "alc/backends/wave.cpp");

        break;
    }
    }

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

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    return project;
}

DependencyProject* BuildIcarianNativeIDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    *a_count = 6;

    // *a_count = 1;

    DependencyProject* projects = (DependencyProject*)malloc(sizeof(DependencyProject) * (*a_count));

    // projects[0].Project = BuildSPIRVToolsProject(a_targetPlatform, a_configuration);
    // projects[0].WorkingDirectory = "IcarianNative/lib/SPIRV-Tools";

    projects[0].Project = BuildGLSLangProject(a_targetPlatform, a_configuration);
    projects[0].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[1].Project = BuildOGLCompilersProject(a_targetPlatform, a_configuration);
    projects[1].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[2].Project = BuildSPIRVProject(a_targetPlatform, a_configuration);
    projects[2].WorkingDirectory = "IcarianNative/lib/glslang";

    projects[3].Project = BuildSPIRVToolsProject(a_targetPlatform, a_configuration);
    projects[3].WorkingDirectory = "IcarianNative/lib/SPIRV-Tools";

    projects[4].Project = BuildJoltPhysicsProject(a_targetPlatform, a_configuration);
    projects[4].WorkingDirectory = "IcarianNative/lib/JoltPhysics";

    projects[5].Project = BuildOpenALSoft(a_targetPlatform, a_configuration);
    projects[5].WorkingDirectory = "IcarianNative/lib/openal-soft";

    return projects;
}

#ifdef __cplusplus
}
#endif

#endif