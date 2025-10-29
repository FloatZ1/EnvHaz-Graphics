#include "Animation/AnimatedModelManager.hpp"
#include "DataStructs.hpp"
#include "MeshManager.hpp"
#include "Utils/HashedStrings.hpp"
#include "Utils/Math_Utils.hpp"
#include "glm/fwd.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <set>
#include <vector>





using namespace eHazGraphics_Utils;
namespace eHazGraphics
{
template <typename T>
int LoadAccessor(const tinygltf::Accessor &accessor, tinygltf::Model &m_GltfModel, const T *&pointer,
                 uint *count = nullptr, int *type = nullptr)
{
    const tinygltf::BufferView &view = m_GltfModel.bufferViews[accessor.bufferView];
    pointer =
        reinterpret_cast<const T *>(&(m_GltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
    if (count)
    {
        *count = static_cast<uint>(accessor.count);
    }
    if (type)
    {
        *type = accessor.type;
    }
    return accessor.componentType;
}

template <typename T>
static void GetAttributeData(const tinygltf::Model &model, const tinygltf::Primitive &prim,
                             const std::string &attribName, std::vector<T> &outData, int expectedComponents)
{
    auto it = prim.attributes.find(attribName);
    if (it == prim.attributes.end())
        return;

    const tinygltf::Accessor &accessor = model.accessors[it->second];
    const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

    const unsigned char *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
    size_t stride = accessor.ByteStride(bufferView);
    if (stride == 0)
        stride = expectedComponents * sizeof(T);

    outData.resize(accessor.count * expectedComponents);

    for (size_t i = 0; i < accessor.count; i++)
    {
        const T *src = reinterpret_cast<const T *>(dataPtr + stride * i);
        for (int j = 0; j < expectedComponents; j++)
            outData[i * expectedComponents + j] = src[j];
    }
}







std::vector<uint32_t> GetPrimitiveIndices(const tinygltf::Model &model, const tinygltf::Primitive &primitive)
{
    std::vector<uint32_t> indices;

    // If the primitive doesn't have indices, return empty vector
    if (primitive.indices < 0)
        return indices;

    const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
    const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

    const unsigned char *dataPtr = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

    indices.resize(accessor.count);

    switch (accessor.componentType)
    {
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
        const uint32_t *buf = reinterpret_cast<const uint32_t *>(dataPtr);
        for (size_t i = 0; i < accessor.count; i++)
            indices[i] = buf[i];
        break;
    }
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
        const uint16_t *buf = reinterpret_cast<const uint16_t *>(dataPtr);
        for (size_t i = 0; i < accessor.count; i++)
            indices[i] = buf[i];
        break;
    }
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
        const uint8_t *buf = reinterpret_cast<const uint8_t *>(dataPtr);
        for (size_t i = 0; i < accessor.count; i++)
            indices[i] = buf[i];
        break;
    }
    default:
        std::cerr << "Unsupported index component type: " << accessor.componentType << "\n";
        break;
    }

    return indices;
}










void Skeleton::Update(float deltaTime)
{
    if (!m_IsAnimated)
    {
        for (auto &mat : finalMatrices)
            mat = glm::mat4(1.0f);
        return;
    }

    animator.Update(deltaTime);

    const auto &animMatrices = animator.GetSubmissionMatrices4x4();

    for (size_t i = 0; i < m_Joints.size(); ++i)
        finalMatrices[i] = animMatrices[i] * m_Joints[i].m_InverseBindMatrix;
}







void Skeleton::UpdateJoint(int jointIndex)
{
    auto &currentJoint = m_Joints[jointIndex]; // just a reference for easier code

    int16_t parentJoint = currentJoint.m_ParentJoint;
    if (parentJoint != -1)
    {
        finalMatrices[jointIndex] = finalMatrices[parentJoint] * finalMatrices[jointIndex];
    }

    // update children
    size_t numberOfChildren = currentJoint.m_Children.size();
    for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
    {
        int childJoint = currentJoint.m_Children[childIndex];
        UpdateJoint(childJoint);
    }
}



























void LoadJoint(int globalGltfNodeIndex, int parentJoint, tinygltf::Model &model, Skeleton &skeleton)
{

    int currentJoint = skeleton.m_GlobalGltfNodeToRootIndex[globalGltfNodeIndex];
    auto &joint = skeleton.m_Joints[currentJoint];


    joint.m_ParentJoint = parentJoint;


    size_t numberOfChildren = model.nodes[globalGltfNodeIndex].children.size();
    if (numberOfChildren > 0)
    {

        joint.m_Children.resize(numberOfChildren);
        for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
        {

            unsigned int globalGltfNodeIndexForChild = model.nodes[globalGltfNodeIndex].children[childIndex];
            joint.m_Children[childIndex] = skeleton.m_GlobalGltfNodeToRootIndex[globalGltfNodeIndexForChild];
            LoadJoint(globalGltfNodeIndexForChild, currentJoint, model, skeleton);
        }
    }
}

AnimatedModel AnimatedModelManager::LoadAnimatedModel(std::string path)
{

    tinygltf::Model model;


    HashedString hashedPath = computeHash(path);

    if (loadedModels.contains(hashedPath))
        return loadedModels[hashedPath];

    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    std::size_t l_filetype = path.find_last_of(".");
    std::string filetype = path.substr(l_filetype + 1);

    bool ret;
    if (filetype == "gltf")
    {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }
    else if (filetype == "glb")
    {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path); // for binary glTF(.glb)
    }

    if (!warn.empty())
    {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty())
    {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret)
    {
        printf("Failed to parse glTF\n");
    }

    Skeleton skeleton; // WARINING: ERROR  line below

    if (model.skins.empty())
    {
        std::cerr << "Error: glTF file has no skins! (" << path << ")\n";
        // You can either:
        // - return an empty AnimatedModel
        // - or handle it as a static model instead
        // return AnimatedModel();
    }



    tinygltf::Skin &skin = model.skins[0];
    int numJoints = skin.joints.size();
    skeleton.finalMatrices.resize(numJoints);


    auto &joints = skeleton.m_Joints;



    skeleton.m_Joints.resize(numJoints);
    skeleton.m_Name = model.skins[0].name;


    const glm::mat4 *inverseBindMatrices;
    {
        unsigned int count = 0;
        int type = 0;
        auto componentType = LoadAccessor<glm::mat4>(model.accessors[skin.inverseBindMatrices], model,
                                                     inverseBindMatrices, &count, &type);
    }






    for (int jointIndex = 0; jointIndex < numJoints; jointIndex++)
    {

        int globalGltfNodeIndex = skin.joints[jointIndex];
        auto &joint = joints[jointIndex];

        joint.m_InverseBindMatrix = inverseBindMatrices[jointIndex];
        joint.m_Name = model.nodes[globalGltfNodeIndex].name;

        skeleton.m_GlobalGltfNodeToRootIndex[globalGltfNodeIndex] = jointIndex;
    }




    std::set<int> allChildren;
    for (int j : skin.joints)
    {
        for (int c : model.nodes[j].children)
            allChildren.insert(c);
    }

    int rootJoint = -1;
    for (int j : skin.joints)
    {
        if (allChildren.find(j) == allChildren.end())
        {
            rootJoint = j;
            break;
        }
    }



    LoadJoint(rootJoint, -1, model, skeleton);



    skeletons.push_back(skeleton);


    AnimatedModel retModel;
    auto processedMeshes = ProcessMeshes(model);


    retModel.SetSkeletonID(skeletons.size() - 1);
    for (MeshID curMesh : processedMeshes)
    {

        retModel.AddMesh(curMesh);
    }




    return retModel;
}


std::vector<MeshID> AnimatedModelManager::ProcessMeshes(tinygltf::Model &model)
{

    std::vector<MeshID> meshLocations;

    for (unsigned int i = 0; i < model.meshes.size(); i++)
    {

        std::vector<Vertex> vertices;
        std::vector<GLuint> indecies;






        MeshData c_meshData;

        for (auto primitive : model.meshes[i].primitives)
        {




            std::vector<float> positions, normals, uvs, tangents, weights;
            std::vector<int> joints;




            for (auto &index : GetPrimitiveIndices(model, primitive))
                indecies.push_back(index);







            GetAttributeData(model, primitive, "POSITION", positions, 3);
            GetAttributeData(model, primitive, "NORMAL", normals, 3);
            GetAttributeData(model, primitive, "TEXCOORD_0", uvs, 2);

            GetAttributeData(model, primitive, "JOINTS_0", joints, 4);
            GetAttributeData(model, primitive, "WEIGHTS_0", weights, 4);
            GetAttributeData(model, primitive, "TANGENT", tangents, 4);




            int vertexCount = positions.size() / 3;





            for (size_t j = 0; j < vertexCount; ++j)
            {

                Vertex vertex;

                vertex.Postion = {positions[j * 3 + 0], positions[j * 3 + 1], positions[j * 3 + 2]};
                vertex.Normal = {normals[j * 3 + 0], normals[j * 3 + 1], normals[j * 3 + 2]};
                vertex.UV = {uvs[j * 2 + 0], uvs[j * 2 + 1]};
                if (tangents.size() > 0)
                {
                    vertex.Tangent = {tangents[j * 4 + 0], tangents[j * 4 + 1], tangents[j * 4 + 2],
                                      tangents[j * 4 + 3]};
                    glm::vec3 bitangent =
                        glm::cross(vertex.Normal, glm::vec3(vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z)) *
                        vertex.Tangent.w;
                    vertex.Bitangent = bitangent;
                }
                vertex.boneWeights = {weights[j * 4 + 0], weights[j * 4 + 1], weights[j * 4 + 2], weights[j * 4 + 3]};
                vertex.boneIDs = {joints[j * 4 + 0], joints[j * 4 + 1], joints[j * 4 + 2], joints[j * 4 + 3]};





                vertices.push_back(vertex);
            }
        }

        c_meshData.indecies = indecies;
        c_meshData.vertices = vertices;

        Mesh finalMesh = Mesh(c_meshData);
        meshes.try_emplace(maxID, finalMesh);
        VertexIndexInfoPair meshRange =
            bufferManager->InsertNewStaticData(vertices.data(), vertices.size() * sizeof(Vertex), indecies.data(),
                                               indecies.size() * sizeof(GLuint), TypeFlags::BUFFER_STATIC_MESH_DATA);


        AnimatedModelManager::meshLocations.emplace(maxID, meshRange);

        meshLocations.push_back(maxID);
        maxID++;
    }

    return meshLocations;
}




void AnimatedModelManager::LoadAnimation(int skeletonID, std::string &path, int &r_AnimationID)
{



    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    std::size_t extPos = path.find_last_of(".");
    std::string ext = path.substr(extPos + 1);
    bool ret = (ext == "glb") ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
                              : loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!ret)
    {
        printf("Failed to load animation from %s: %s\n", path.c_str(), err.c_str());
        return;
    }

    if (!warn.empty())
        printf("Warn: %s\n", warn.c_str());

    // Convert tinygltf::Animation -> eHazGraphics::Animation
    for (auto &gltfAnim : model.animations)
    {
        Animation anim;

        // Determine max keyframe count
        size_t maxFrames = 0;
        for (auto &sampler : gltfAnim.samplers)
        {
            const auto &timeAccessor = model.accessors[sampler.input];
            maxFrames = std::max(maxFrames, static_cast<size_t>(timeAccessor.count));
        }

        // Build keyframes
        for (size_t i = 0; i < maxFrames; ++i)
        {
            KeyFrame keyframe;
            keyframe.timeStamp = 0.0f; // we'll compute from input accessor

            keyframe.transforms.resize(model.skins[0].joints.size()); // one transform per joint

            anim.frames.push_back(keyframe);
        }

        // Populate joint transforms per keyframe
        for (auto &channel : gltfAnim.channels)
        {
            int nodeIndex = channel.target_node;
            if (!skeletons[skeletonID].m_GlobalGltfNodeToRootIndex.count(nodeIndex))
                continue;

            int jointIndex = skeletons[skeletonID].m_GlobalGltfNodeToRootIndex[nodeIndex];
            auto &sampler = gltfAnim.samplers[channel.sampler];

            const auto &timeAccessor = model.accessors[sampler.input];
            const auto &valueAccessor = model.accessors[sampler.output];

            const float *timeData = reinterpret_cast<const float *>(
                &model.buffers[model.bufferViews[timeAccessor.bufferView].buffer]
                     .data[timeAccessor.byteOffset + model.bufferViews[timeAccessor.bufferView].byteOffset]);

            const unsigned char *valueData =
                &model.buffers[model.bufferViews[valueAccessor.bufferView].buffer]
                     .data[valueAccessor.byteOffset + model.bufferViews[valueAccessor.bufferView].byteOffset];

            for (size_t k = 0; k < timeAccessor.count; ++k)
            {
                KeyFrame &kf = anim.frames[k];
                kf.timeStamp = timeData[k];

                JointTransform &jt = kf.transforms[jointIndex];

                if (channel.target_path == "translation")
                {
                    const glm::vec3 *vals = reinterpret_cast<const glm::vec3 *>(valueData);
                    jt.position = vals[k];
                }
                else if (channel.target_path == "rotation")
                {
                    const glm::quat *vals = reinterpret_cast<const glm::quat *>(valueData);
                    jt.rotation = vals[k];
                }
                else if (channel.target_path == "scale")
                {
                    const glm::vec3 *vals = reinterpret_cast<const glm::vec3 *>(valueData);
                    jt.scale = vals[k];
                }
            }
        }

        animations.push_back(anim);
        anim.owningSkeleton = skeletonID;
        // Automatically add to skeleton's animator (assuming skeleton 0 for now)
        r_AnimationID = skeletons[skeletonID].animator.AddAnimation(anim);

        // skeletons[skeletonID].animator.PlayAnimation(0); // first animation
    }
}

void AnimatedModelManager::UploadBonesToGPU(BufferRange &range, std::vector<glm::mat4> finalMatrices)
{

    if (range.OwningBuffer == -1)
    {
        range = bufferManager->InsertNewDynamicData(finalMatrices.data(), finalMatrices.size() * sizeof(glm::mat4),
                                                    TypeFlags::BUFFER_ANIMATION_DATA);
    }
    else
    {
        bufferManager->UpdateData(range, finalMatrices.data(), finalMatrices.size() * sizeof(glm::mat4));
    }
}

void AnimatedModelManager::Update(float deltaTime)
{
    for (Skeleton &skeleton : skeletons)
    {
        skeleton.Update(deltaTime);
    }

    // Submit to GPU or instance buffer
    for (AnimatedModel *model : submittedAnimatedModels)
    {
        Skeleton &skeleton = skeletons[model->GetSkeletonID()];
        UploadBonesToGPU(skeleton.GPUlocation, skeleton.finalMatrices); // your existing submission logic
    }
}

} // namespace eHazGraphics
