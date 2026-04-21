#pragma once
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include "Engine/Assets/Asset.h"

namespace Engine
{
    // ----------------------------------------------------------------
    // Pin types
    // ----------------------------------------------------------------
    enum class PinType : uint8_t
    {
        Float = 0,
        Vec2,
        Vec3,
        Vec4,
        Texture2D,
    };

    // ----------------------------------------------------------------
    // Serialisable pin descriptor (not a live ImNodeFlow pin)
    // ----------------------------------------------------------------
    struct MaterialPin
    {
        uint32_t    ID       = 0;
        std::string Name;
        PinType     Type     = PinType::Float;
        bool        IsOutput = false;

        // Default value used when pin is unconnected
        glm::vec4 DefaultValue{ 0.0f };
    };


    // ----------------------------------------------------------------
    // Link between two pins
    // ----------------------------------------------------------------
    struct MaterialLink
    {
        uint32_t ID      = 0;
        uint32_t FromPin = 0; // output pin
        uint32_t ToPin   = 0; // input pin
    };

    // ----------------------------------------------------------------
    // Base node
    // ----------------------------------------------------------------
    struct MaterialNode
    {
        uint32_t    ID = 0;
        std::string Name;
        glm::vec2   Position{ 0.0f };

        std::vector<MaterialPin> Inputs;
        std::vector<MaterialPin> Outputs;

        virtual ~MaterialNode() = default;
        virtual std::string GetTypeName()                              const = 0;
        virtual void SerializeExtra(nlohmann::json&)                   const {}
        virtual void DeserializeExtra(const nlohmann::json&)                 {}
    };

    // ----------------------------------------------------------------
    // Concrete node types
    // ----------------------------------------------------------------

    struct PBROutputNode : MaterialNode
    {
        std::string GetTypeName() const override { return "PBROutput"; }
        static std::shared_ptr<PBROutputNode> Create(uint32_t id, glm::vec2 pos = { 600.0f, 200.0f });
    };


    struct TextureSampleNode : MaterialNode
    {
        AssetHandle TextureHandle = AssetHandle(NullAssetHandle);

        std::string GetTypeName()                          const override { return "TextureSample"; }
        void SerializeExtra(nlohmann::json& j)             const override;
        void DeserializeExtra(const nlohmann::json& j)           override;

        static std::shared_ptr<TextureSampleNode> Create(uint32_t id, glm::vec2 pos = { 100.0f, 100.0f });
    };

    struct ConstantVec3Node : MaterialNode
    {
        glm::vec3 Value{ 1.0f, 1.0f, 1.0f };

        std::string GetTypeName()                          const override { return "ConstantVec3"; }
        void SerializeExtra(nlohmann::json& j)             const override;
        void DeserializeExtra(const nlohmann::json& j)           override;

        static std::shared_ptr<ConstantVec3Node> Create(uint32_t id, glm::vec2 pos = { 100.0f, 100.0f });
    };

    struct MultiplyNode : MaterialNode
    {
        std::string GetTypeName() const override { return "Multiply"; }
        static std::shared_ptr<MultiplyNode> Create(uint32_t id, glm::vec2 pos = { 300.0f, 200.0f });
    };

    struct LerpNode : MaterialNode
    {
        std::string GetTypeName() const override { return "Lerp"; }
        static std::shared_ptr<LerpNode> Create(uint32_t id, glm::vec2 pos = { 300.0f, 350.0f });
    };


    // ----------------------------------------------------------------
    // The graph
    // ----------------------------------------------------------------
    class MaterialGraph
    {
    public:
        std::string                                Name = "NewMaterial";
        std::vector<std::shared_ptr<MaterialNode>> Nodes;
        std::vector<MaterialLink>                  Links;

        // ID allocators
        uint32_t NextNodeID() { return ++m_NextNodeID; }
        uint32_t NextPinID()  { return ++m_NextPinID;  }
        uint32_t NextLinkID() { return ++m_NextLinkID; }

        // Lookup
        MaterialNode* FindNode(uint32_t id);
        MaterialPin*  FindPin (uint32_t id);

        // Mutation
        void RemoveNode(uint32_t nodeID);
        void RemoveLink(uint32_t linkID);
        bool AddLink(uint32_t fromPin, uint32_t toPin); // false = type mismatch

        // Serialisation
        bool SaveToFile  (const std::filesystem::path& path) const;
        bool LoadFromFile(const std::filesystem::path& path);

        // Reset to a single PBROutputNode
        void InitDefault();

    private:
        uint32_t m_NextNodeID = 0;
        uint32_t m_NextPinID  = 0;
        uint32_t m_NextLinkID = 0;

        std::shared_ptr<MaterialNode> DeserializeNode(const nlohmann::json& j);
        void SyncIDCounters();
    };

    // ----------------------------------------------------------------
    // Asset wrapper
    // ----------------------------------------------------------------
    class MaterialAsset : public Asset
    {
    public:
        MaterialGraph Graph;

        AssetType        GetType()        const override { return AssetType::Material; }
        static AssetType GetStaticType()               { return AssetType::Material; }
    };

} // namespace Engine
