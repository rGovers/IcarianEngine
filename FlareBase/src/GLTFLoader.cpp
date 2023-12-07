#include "Flare/GLTFLoader.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Flare/IcarianDefer.h"

namespace FlareBase
{
    static bool IsBinary(const char* a_data, uint32_t a_size)
    {
        constexpr char Magic[] = "glTF";

        return *(uint32_t*)a_data == *(uint32_t*)Magic;
    }

    bool GLTFLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (a_size < 4)
        {
            return false;
        }

        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err;
        std::string warn;

        if (IsBinary(a_data, a_size))
        {
            if (!loader.LoadBinaryFromMemory(&model, &err, &warn, (unsigned char*)a_data, (unsigned int)a_size))
            {
                return false;
            }
        }
        else
        {
            if (!loader.LoadASCIIFromString(&model, &err, &warn, a_data, (unsigned int)a_size, ""))
            {
                return false;
            }
        }

        if (model.meshes.size() == 0)
        {
            return false;
        }

        for (const tinygltf::Mesh& mesh : model.meshes)
        {
            for (const tinygltf::Primitive& primitive : mesh.primitives)
            {
                if (primitive.indices < 0)
                {
                    return false;
                }

                const uint32_t firstIndex = (uint32_t)a_indices->size();
                const uint32_t firstVertex = (uint32_t)a_vertices->size();

                uint32_t vertexCount = 0;
                
                int32_t positionStride = 0;
                int32_t normalStride = 0;
                int32_t texCoordStride = 0;
                int32_t colorStride = 0;

                const float* positionBuffer = nullptr;
                const float* normalBuffer = nullptr;
                const float* texCoordBuffer = nullptr;
                const float* colorBuffer = nullptr;

                const auto positionAccessor = primitive.attributes.find("POSITION");
                if (positionAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[positionAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    positionBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    vertexCount = (uint32_t)accessor.count;
                    positionStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                const auto normalAccessor = primitive.attributes.find("NORMAL");
                if (normalAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[normalAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    normalBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    normalStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                const auto texCoordAccessor = primitive.attributes.find("TEXCOORD_0");
                if (texCoordAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[texCoordAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    texCoordBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    texCoordStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                const auto colorAccessor = primitive.attributes.find("COLOR_0");
                if (colorAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[colorAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    colorBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    colorStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                a_vertices->reserve(a_vertices->size() + vertexCount);
                for (uint32_t i = 0; i < vertexCount; ++i)
                {
                    Vertex vertex;

                    if (positionBuffer)
                    {
                        vertex.Position.x = positionBuffer[i * positionStride + 0];
                        vertex.Position.y = -positionBuffer[i * positionStride + 1];
                        vertex.Position.z = positionBuffer[i * positionStride + 2];
                        vertex.Position.w = 1.0f;

                        const float length = glm::length(vertex.Position.xyz());
                        if (length > *a_radius)
                        {
                            *a_radius = length;
                        }
                    }

                    if (normalBuffer)
                    {
                        vertex.Normal.x = normalBuffer[i * normalStride + 0];
                        vertex.Normal.y = -normalBuffer[i * normalStride + 1];
                        vertex.Normal.z = normalBuffer[i * normalStride + 2];
                    }

                    if (texCoordBuffer)
                    {
                        vertex.TexCoords.x = texCoordBuffer[i * texCoordStride + 0];
                        vertex.TexCoords.y = texCoordBuffer[i * texCoordStride + 1];
                    }

                    if (colorBuffer)
                    {
                        vertex.Color.x = colorBuffer[i * colorStride + 0];
                        vertex.Color.y = colorBuffer[i * colorStride + 1];
                        vertex.Color.z = colorBuffer[i * colorStride + 2];
                        vertex.Color.w = colorBuffer[i * colorStride + 3];
                    }

                    a_vertices->push_back(vertex);
                }

                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const uint32_t indexCount = (uint32_t)accessor.count;

                a_indices->reserve(a_indices->size() + indexCount);

                const uint32_t triangleCount = indexCount / 3;

                switch (accessor.componentType)
                {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                {
                    const uint32_t* indices = (uint32_t*)&buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    for (uint32_t i = 0; i < triangleCount; ++i)
                    {
                        a_indices->push_back(indices[i * 3 + 0] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 2] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 1] + firstVertex);
                    }

                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                {
                    const uint16_t* indices = (uint16_t*)&buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    for (uint32_t i = 0; i < triangleCount; ++i)
                    {
                        a_indices->push_back(indices[i * 3 + 0] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 2] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 1] + firstVertex);
                    }

                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                {
                    const uint8_t* indices = (uint8_t*)&buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    for (uint32_t i = 0; i < triangleCount; ++i)
                    {
                        a_indices->push_back(indices[i * 3 + 0] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 2] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 1] + firstVertex);
                    }

                    break;
                }      
                }          
            }
        }

        return true;
    }
    bool GLTFLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path, std::ios::binary);
            if (file.good() && file.is_open())
            {
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios_base::beg);

                if (size > std::numeric_limits<unsigned int>::max())
                {
                    return false;
                }

                char* data = new char[size];
                IDEFER(delete[] data);
                file.read(data, size);

                return GLTFLoader_LoadData(data, (uint32_t)size, a_vertices, a_indices, a_radius);
            }
        }

        return false;
    }

    bool GLTFLoader_LoadSkinnedData(const char* a_data, uint32_t a_size, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (a_size < 4)
        {
            return false;
        }

        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err;
        std::string warn;

        if (IsBinary(a_data, a_size))
        {
            if (!loader.LoadBinaryFromMemory(&model, &err, &warn, (unsigned char*)a_data, (unsigned int)a_size))
            {
                return false;
            }
        }
        else
        {
            if (!loader.LoadASCIIFromString(&model, &err, &warn, a_data, (unsigned int)a_size, ""))
            {
                return false;
            }
        }

        if (model.meshes.size() == 0)
        {
            return false;
        }

        for (const tinygltf::Mesh& mesh : model.meshes)
        {
            for (const tinygltf::Primitive& primitive : mesh.primitives)
            {
                if (primitive.indices < 0)
                {
                    return false;
                }

                const uint32_t firstIndex = (uint32_t)a_indices->size();
                const uint32_t firstVertex = (uint32_t)a_vertices->size();

                uint32_t vertexCount = 0;

                int32_t positionStride = 0;
                int32_t normalStride = 0;
                int32_t texCoordStride = 0;
                int32_t colorStride = 0;
                int32_t jointStride = 0;
                int32_t weightStride = 0;

                int32_t jointComponentType = 0;

                const float* positionBuffer = nullptr;
                const float* normalBuffer = nullptr;
                const float* texCoordBuffer = nullptr;
                const float* colorBuffer = nullptr;
                const void* jointBuffer = nullptr;
                const float* weightBuffer = nullptr;

                const auto positionAccessor = primitive.attributes.find("POSITION");
                if (positionAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[positionAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    positionBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    vertexCount = (uint32_t)accessor.count;
                    positionStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                const auto normalAccessor = primitive.attributes.find("NORMAL");
                if (normalAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[normalAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    normalBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    normalStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                const auto texCoordAccessor = primitive.attributes.find("TEXCOORD_0");
                if (texCoordAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[texCoordAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    texCoordBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    texCoordStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                const auto colorAccessor = primitive.attributes.find("COLOR_0");
                if (colorAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[colorAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    colorBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    colorStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                const auto jointAccessor = primitive.attributes.find("JOINTS_0");
                if (jointAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[jointAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    jointBuffer = (void*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    jointComponentType = accessor.componentType;
                    jointStride = accessor.ByteStride(bufferView) / tinygltf::GetComponentSizeInBytes(accessor.componentType);
                }

                const auto weightAccessor = primitive.attributes.find("WEIGHTS_0");
                if (weightAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[weightAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    weightBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                    weightStride = accessor.ByteStride(bufferView) / sizeof(float);
                }

                a_vertices->reserve(a_vertices->size() + vertexCount);

                for (uint32_t i = 0; i < vertexCount; ++i)
                {
                    SkinnedVertex vertex;

                    if (positionBuffer)
                    {
                        vertex.Position.x = positionBuffer[i * positionStride + 0];
                        vertex.Position.y = -positionBuffer[i * positionStride + 1];
                        vertex.Position.z = positionBuffer[i * positionStride + 2];
                        vertex.Position.w = 1.0f;

                        const float length = glm::length(vertex.Position.xyz());
                        if (length > *a_radius)
                        {
                            *a_radius = length;
                        }
                    }

                    if (normalBuffer)
                    {
                        vertex.Normal.x = normalBuffer[i * normalStride + 0];
                        vertex.Normal.y = -normalBuffer[i * normalStride + 1];
                        vertex.Normal.z = normalBuffer[i * normalStride + 2];
                    }

                    if (texCoordBuffer)
                    {
                        vertex.TexCoords.x = texCoordBuffer[i * texCoordStride + 0];
                        vertex.TexCoords.y = texCoordBuffer[i * texCoordStride + 1];
                    }

                    if (colorBuffer)
                    {
                        vertex.Color.x = colorBuffer[i * colorStride + 0];
                        vertex.Color.y = colorBuffer[i * colorStride + 1];
                        vertex.Color.z = colorBuffer[i * colorStride + 2];
                        vertex.Color.w = colorBuffer[i * colorStride + 3];
                    }

                    if (jointBuffer)
                    {
                        switch (jointComponentType)
                        {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            const uint32_t* joints = (uint32_t*)jointBuffer;
                            vertex.BoneIndices.x = (int)joints[i * jointStride + 0];
                            vertex.BoneIndices.y = (int)joints[i * jointStride + 1];
                            vertex.BoneIndices.z = (int)joints[i * jointStride + 2];
                            vertex.BoneIndices.w = (int)joints[i * jointStride + 3];

                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            const uint16_t* joints = (uint16_t*)jointBuffer;
                            vertex.BoneIndices.x = (int)joints[i * jointStride + 0];
                            vertex.BoneIndices.y = (int)joints[i * jointStride + 1];
                            vertex.BoneIndices.z = (int)joints[i * jointStride + 2];
                            vertex.BoneIndices.w = (int)joints[i * jointStride + 3];

                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            const uint8_t* joints = (uint8_t*)jointBuffer;
                            vertex.BoneIndices.x = (int)joints[i * jointStride + 0];
                            vertex.BoneIndices.y = (int)joints[i * jointStride + 1];
                            vertex.BoneIndices.z = (int)joints[i * jointStride + 2];
                            vertex.BoneIndices.w = (int)joints[i * jointStride + 3];

                            break;
                        }
                        }
                    }

                    if (weightBuffer)
                    {
                        vertex.BoneWeights.x = weightBuffer[i * weightStride + 0];
                        vertex.BoneWeights.y = weightBuffer[i * weightStride + 1];
                        vertex.BoneWeights.z = weightBuffer[i * weightStride + 2];
                        vertex.BoneWeights.w = weightBuffer[i * weightStride + 3];
                    }

                    a_vertices->push_back(vertex);
                }

                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const uint32_t indexCount = (uint32_t)accessor.count;

                a_indices->reserve(a_indices->size() + indexCount);

                const uint32_t triangleCount = indexCount / 3;

                switch (accessor.componentType)
                {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                {
                    const uint32_t* indices = (uint32_t*)&buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    for (uint32_t i = 0; i < triangleCount; ++i)
                    {
                        a_indices->push_back(indices[i * 3 + 0] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 2] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 1] + firstVertex);
                    }

                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                {
                    const uint16_t* indices = (uint16_t*)&buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    for (uint32_t i = 0; i < triangleCount; ++i)
                    {
                        a_indices->push_back(indices[i * 3 + 0] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 2] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 1] + firstVertex);
                    }

                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                {
                    const uint8_t* indices = (uint8_t*)&buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    for (uint32_t i = 0; i < triangleCount; ++i)
                    {
                        a_indices->push_back(indices[i * 3 + 0] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 2] + firstVertex);
                        a_indices->push_back(indices[i * 3 + 1] + firstVertex);
                    }

                    break;
                }
                }
            }
        }

        return true;
    }
    bool GLTFLoader_LoadSkinnedFile(const std::filesystem::path& a_path, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path, std::ios::binary);
            if (file.good() && file.is_open())
            {
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios_base::beg);

                if (size > std::numeric_limits<unsigned int>::max())
                {
                    return false;
                }

                char* data = new char[size];
                IDEFER(delete[] data);
                file.read(data, size);

                return GLTFLoader_LoadSkinnedData(data, (uint32_t)size, a_vertices, a_indices, a_radius);
            }
        }

        return false;
    }

    bool GLTFLoader_LoadBones(const char* a_data, uint32_t a_size, std::vector<BoneData>* a_bones)
    {
        if (a_size < 4)
        {
            return false;
        }

        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err;
        std::string warn;

        if (IsBinary(a_data, a_size))
        {
            if (!loader.LoadBinaryFromMemory(&model, &err, &warn, (unsigned char*)a_data, (unsigned int)a_size))
            {
                return false;
            }
        }
        else
        {
            if (!loader.LoadASCIIFromString(&model, &err, &warn, a_data, (unsigned int)a_size, ""))
            {
                return false;
            }
        }

        if (model.skins.size() == 0)
        {
            return false;
        }

        struct GLTFBone
        {
            std::string Name;
            std::string ParentName;
            glm::mat4 Transform;
        };
        std::vector<GLTFBone> bones;

        const uint32_t nodeCount = (uint32_t)model.nodes.size();

        for (const tinygltf::Skin& skin : model.skins)
        {
            const uint32_t jointCount = (uint32_t)skin.joints.size();
            for (uint32_t i = 0; i < jointCount; ++i)
            {
                const int jointIndex = skin.joints[i];

                const tinygltf::Node& joint = model.nodes[jointIndex];

                constexpr glm::mat4 Iden = glm::identity<glm::mat4>();

                GLTFBone bone;
                bone.Name = joint.name;
                bone.Transform = Iden;

                for (uint32_t i = 0; i < nodeCount; ++i)
                {
                    const tinygltf::Node& node = model.nodes[i];
                    for (int childIndex : node.children)
                    {
                        if (childIndex == jointIndex)
                        {
                            bone.ParentName = node.name;

                            goto End;
                        }
                    }
                }
End:;  

                if (skin.inverseBindMatrices > -1)
                {
                    const tinygltf::Accessor& accessor = model.accessors[skin.inverseBindMatrices];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                    const float* inverseBindMatrices = (float*)&buffer.data[accessor.byteOffset + bufferView.byteOffset];
                    const uint32_t bindMatrixStride = accessor.ByteStride(bufferView) / sizeof(float);

                    glm::mat4 inverseBindMatrix;
                    float* t = (float*)&inverseBindMatrix;

                    for (int j = 0; j < 16; ++j)
                    {
                        t[j] = inverseBindMatrices[i * bindMatrixStride + j];
                    }

                    const glm::mat4 bindMatrix = glm::inverse(inverseBindMatrix);

                    glm::vec3 translation;
                    glm::quat rotation;
                    glm::vec3 scale;
                    glm::vec3 skew;
                    glm::vec4 perspective;

                    glm::decompose(bindMatrix, scale, rotation, translation, skew, perspective);

                    const glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
                    const glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

                    const glm::vec3 tForward = glm::vec3(forward.x, -forward.y, forward.z);
                    const glm::vec3 tRight = glm::vec3(right.x, -right.y, right.z);
                    const glm::vec3 tUp = glm::cross(tForward, tRight);

                    const glm::mat4 translationMat = glm::translate(Iden, glm::vec3(translation.x, -translation.y, translation.z));
                    const glm::mat4 rotationMat = glm::mat4(glm::vec4(tRight, 0.0f), glm::vec4(tUp, 0.0f), glm::vec4(tForward, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                    const glm::mat4 scaleMat = glm::scale(Iden, scale);

                    bone.Transform = translationMat * rotationMat * scaleMat;
                }

                bones.push_back(bone);
            }
        }

        const uint32_t boneCount = (uint32_t)bones.size();
        a_bones->reserve(boneCount);
        for (const GLTFBone& bone : bones)
        {
            BoneData data;
            data.Name = bone.Name;
            data.Parent = -1;
            data.Transform = bone.Transform;

            for (uint32_t i = 0; i < boneCount; ++i)
            {
                if (bones[i].Name == bone.ParentName)
                {
                    data.Parent = i;

                    break;
                }
            }

            a_bones->push_back(data);
        }

        return true;
    }
    bool GLTFLoader_LoadBonesFile(const std::filesystem::path& a_path, std::vector<BoneData>* a_bones)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path, std::ios::binary);
            if (file.good() && file.is_open())
            {
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios_base::beg);

                if (size > std::numeric_limits<unsigned int>::max())
                {
                    return false;
                }

                char* data = new char[size];
                IDEFER(delete[] data);
                file.read(data, size);

                return GLTFLoader_LoadBones(data, (uint32_t)size, a_bones);
            }
        }

        return false;
    }

    bool GLTFLoader_LoadAnimationData(const char* a_data, uint32_t a_size, std::vector<GLTFAnimationData>* a_animation)
    {
        if (a_size < 4)
        {
            return false;
        }

        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string err;
        std::string warn;

        if (IsBinary(a_data, a_size))
        {
            if (!loader.LoadBinaryFromMemory(&model, &err, &warn, (unsigned char*)a_data, (unsigned int)a_size))
            {
                return false;
            }
        }
        else
        {
            if (!loader.LoadASCIIFromString(&model, &err, &warn, a_data, (unsigned int)a_size, ""))
            {
                return false;
            }
        }

        if (model.animations.size() == 0)
        {
            return false;
        }

        for (const tinygltf::Animation& animation : model.animations)
        {
            for (const tinygltf::AnimationChannel& channel : animation.channels)
            {
                const tinygltf::Node& node = model.nodes[channel.target_node];

                GLTFAnimationData data;
                data.Name = node.name;

                const tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];

                const tinygltf::Accessor& inputAccessor = model.accessors[sampler.input];
                const tinygltf::BufferView& inputBufferView = model.bufferViews[inputAccessor.bufferView];
                const tinygltf::Buffer& inputBuffer = model.buffers[inputBufferView.buffer];
                const float* input = (float*)&inputBuffer.data[inputAccessor.byteOffset + inputBufferView.byteOffset];
                const uint32_t inputStride = inputAccessor.ByteStride(inputBufferView) / sizeof(float);
                const uint32_t inputCount = (uint32_t)inputAccessor.count;

                const tinygltf::Accessor& outputAccessor = model.accessors[sampler.output];
                const tinygltf::BufferView& outputBufferView = model.bufferViews[outputAccessor.bufferView];
                const tinygltf::Buffer& outputBuffer = model.buffers[outputBufferView.buffer];
                const void* output = (float*)&outputBuffer.data[outputAccessor.byteOffset + outputBufferView.byteOffset];
                const uint32_t outputStride = outputAccessor.ByteStride(outputBufferView) / sizeof(float);

                glm::vec4* outputVec4 = nullptr;
                IDEFER(delete[] outputVec4);

                switch (outputAccessor.type)
                {
                case TINYGLTF_TYPE_VEC3:
                {
                    outputVec4 = new glm::vec4[inputCount];

                    const float* f = (float*)output;
                    for (uint32_t i = 0; i < inputCount; ++i)
                    {
                        outputVec4[i].x = f[i * outputStride + 0];
                        outputVec4[i].y = f[i * outputStride + 1];
                        outputVec4[i].z = f[i * outputStride + 2];
                        outputVec4[i].w = 0.0f;
                    }

                    break;
                }
                case TINYGLTF_TYPE_VEC4:
                {
                    outputVec4 = new glm::vec4[inputCount];

                    const float* f = (float*)output;
                    for (uint32_t i = 0; i < inputCount; ++i)
                    {
                        outputVec4[i].x = f[i * outputStride + 0];
                        outputVec4[i].y = f[i * outputStride + 1];
                        outputVec4[i].z = f[i * outputStride + 2];
                        outputVec4[i].w = f[i * outputStride + 3];
                    }

                    break;
                }
                }

                if (channel.target_path == "translation")
                {
                    data.Target = "Translation";
                    data.Frames.reserve(inputCount);

                    for (uint32_t i = 0; i < inputCount; ++i)
                    {
                        GLTFAnimationFrame frame;
                        frame.Time = input[i];
                        frame.Data = glm::vec4(outputVec4[i].x, -outputVec4[i].y, outputVec4[i].z, 1.0f);

                        data.Frames.push_back(frame);
                    }

                    a_animation->push_back(data);
                }
                else if (channel.target_path == "rotation")
                {
                    data.Target = "Rotation";
                    data.Frames.reserve(inputCount);

                    for (uint32_t i = 0; i < inputCount; ++i)
                    {
                        GLTFAnimationFrame frame;
                        frame.Time = input[i];
                        frame.Data = glm::vec4(outputVec4[i].x, -outputVec4[i].y, outputVec4[i].z, outputVec4[i].w);

                        data.Frames.push_back(frame);
                    }

                    a_animation->push_back(data);
                }
                else if (channel.target_path == "scale")
                {
                    data.Target = "Scale";
                    data.Frames.reserve(inputCount);

                    for (uint32_t i = 0; i < inputCount; ++i)
                    {
                        GLTFAnimationFrame frame;
                        frame.Time = input[i];
                        frame.Data = glm::vec4(outputVec4[i].x, outputVec4[i].y, outputVec4[i].z, 0.0f);

                        data.Frames.push_back(frame);
                    }

                    a_animation->push_back(data);
                }
            }
        }

        return true;
    }
    bool GLTFLoader_LoadAnimationFile(const std::filesystem::path& a_path, std::vector<GLTFAnimationData>* a_animation)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path, std::ios::binary);
            if (file.good() && file.is_open())
            {
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios_base::beg);

                if (size > std::numeric_limits<unsigned int>::max())
                {
                    return false;
                }

                char* data = new char[size];
                IDEFER(delete[] data);
                file.read(data, size);

                return GLTFLoader_LoadAnimationData(data, (uint32_t)size, a_animation);
            }
        }

        return false;
    }
}