#include "Flare/GLTFLoader.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <cstring>

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
                }

                const auto normalAccessor = primitive.attributes.find("NORMAL");
                if (normalAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[normalAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    normalBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                }

                const auto texCoordAccessor = primitive.attributes.find("TEXCOORD_0");
                if (texCoordAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[texCoordAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    texCoordBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                }

                const auto colorAccessor = primitive.attributes.find("COLOR_0");
                if (colorAccessor != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[colorAccessor->second];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    colorBuffer = (float*)&model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                }

                a_vertices->reserve(a_vertices->size() + vertexCount);
                for (uint32_t i = 0; i < vertexCount; ++i)
                {
                    Vertex vertex;

                    if (positionBuffer)
                    {
                        vertex.Position.x = positionBuffer[i * 3 + 0];
                        vertex.Position.y = -positionBuffer[i * 3 + 1];
                        vertex.Position.z = positionBuffer[i * 3 + 2];
                        vertex.Position.w = 1.0f;
                    }

                    if (normalBuffer)
                    {
                        vertex.Normal.x = normalBuffer[i * 3 + 0];
                        vertex.Normal.y = normalBuffer[i * 3 + 1];
                        vertex.Normal.z = normalBuffer[i * 3 + 2];
                    }

                    if (texCoordBuffer)
                    {
                        vertex.TexCoords.x = texCoordBuffer[i * 2 + 0];
                        vertex.TexCoords.y = texCoordBuffer[i * 2 + 1];
                    }

                    if (colorBuffer)
                    {
                        vertex.Color.x = colorBuffer[i * 4 + 0];
                        vertex.Color.y = colorBuffer[i * 4 + 1];
                        vertex.Color.z = colorBuffer[i * 4 + 2];
                        vertex.Color.w = colorBuffer[i * 4 + 3];
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
}