#include "kepch.h"
#include "MaterialGraph.h"
#include "Engine/Debugging/Log.h"
#include <fstream>

namespace Engine
{

// ----------------------------------------------------------------
// Helper — JSON <-> PinType
// ----------------------------------------------------------------
static const char* PinTypeToString(PinType t)
{
    switch (t) {
        case PinType::Float:     return "Float";
        case PinType::Vec2:      return "Vec2";
        case PinType::Vec3:      return "Vec3";
        case PinType::Vec4:      return "Vec4";
        case PinType::Texture2D: return "Texture2D";
        default:                 return "Float";
    }
}

static PinType PinTypeFromString(const std::string& s)
{
    if (s == "Vec2")      return PinType::Vec2;
    if (s == "Vec3")      return PinType::Vec3;
    if (s == "Vec4")      return PinType::Vec4;
    if (s == "Texture2D") return PinType::Texture2D;
    return PinType::Float;
}

// ----------------------------------------------------------------
// Pin serialisation helpers
// ----------------------------------------------------------------
static nlohmann::json SerializePin(const MaterialPin& p)
{
    return {
        {"id",       p.ID},
        {"name",     p.Name},
        {"type",     PinTypeToString(p.Type)},
        {"isOutput", p.IsOutput},
        {"default",  {p.DefaultValue.x, p.DefaultValue.y,
                      p.DefaultValue.z, p.DefaultValue.w}},
    };
}


static MaterialPin DeserializePin(const nlohmann::json& j)
{
    MaterialPin p;
    p.ID        = j.at("id").get<uint32_t>();
    p.Name      = j.at("name").get<std::string>();
    p.Type      = PinTypeFromString(j.at("type").get<std::string>());
    p.IsOutput  = j.at("isOutput").get<bool>();
    auto& d     = j.at("default");
    p.DefaultValue = { d[0].get<float>(), d[1].get<float>(),
                       d[2].get<float>(), d[3].get<float>() };
    return p;
}

static nlohmann::json SerializeNode(const MaterialNode& n)
{
    nlohmann::json j;
    j["id"]       = n.ID;
    j["type"]     = n.GetTypeName();
    j["name"]     = n.Name;
    j["posX"]     = n.Position.x;
    j["posY"]     = n.Position.y;

    auto& ins  = j["inputs"]  = nlohmann::json::array();
    auto& outs = j["outputs"] = nlohmann::json::array();
    for (auto& p : n.Inputs)  ins.push_back(SerializePin(p));
    for (auto& p : n.Outputs) outs.push_back(SerializePin(p));

    n.SerializeExtra(j);
    return j;
}

// ----------------------------------------------------------------
// TextureSampleNode extra
// ----------------------------------------------------------------
void TextureSampleNode::SerializeExtra(nlohmann::json& j) const
{
    j["textureHandle"] = static_cast<uint64_t>(TextureHandle);
}
void TextureSampleNode::DeserializeExtra(const nlohmann::json& j)
{
    if (j.contains("textureHandle"))
        TextureHandle = AssetHandle(j.at("textureHandle").get<uint64_t>());
}

// ----------------------------------------------------------------
// ConstantVec3Node extra
// ----------------------------------------------------------------
void ConstantVec3Node::SerializeExtra(nlohmann::json& j) const
{
    j["value"] = { Value.x, Value.y, Value.z };
}
void ConstantVec3Node::DeserializeExtra(const nlohmann::json& j)
{
    if (j.contains("value")) {
        auto& v = j.at("value");
        Value = { v[0].get<float>(), v[1].get<float>(), v[2].get<float>() };
    }
}


// ----------------------------------------------------------------
// Factory helpers — build wired-up nodes with correct pins
// ----------------------------------------------------------------
static uint32_t AllocPin(MaterialNode& n, const std::string& name, PinType type,
                         bool isOutput, uint32_t& pinCounter)
{
    MaterialPin p;
    p.ID       = ++pinCounter;
    p.Name     = name;
    p.Type     = type;
    p.IsOutput = isOutput;
    if (isOutput) n.Outputs.push_back(p);
    else          n.Inputs.push_back(p);
    return p.ID;
}

std::shared_ptr<PBROutputNode> PBROutputNode::Create(uint32_t id, glm::vec2 pos)
{
    auto n = std::make_shared<PBROutputNode>();
    n->ID = id; n->Name = "PBR Output"; n->Position = pos;
    uint32_t pid = id * 100;
    AllocPin(*n, "BaseColor",    PinType::Vec3,  false, pid);
    AllocPin(*n, "Metallic",     PinType::Float, false, pid);
    AllocPin(*n, "Roughness",    PinType::Float, false, pid);
    AllocPin(*n, "Normal",       PinType::Vec3,  false, pid);
    AllocPin(*n, "Emissive",     PinType::Vec3,  false, pid);
    AllocPin(*n, "AmbientOcc",   PinType::Float, false, pid);
    return n;
}

std::shared_ptr<TextureSampleNode> TextureSampleNode::Create(uint32_t id, glm::vec2 pos)
{
    auto n = std::make_shared<TextureSampleNode>();
    n->ID = id; n->Name = "Texture Sample"; n->Position = pos;
    uint32_t pid = id * 100;
    AllocPin(*n, "UV",    PinType::Vec2,      false, pid);
    AllocPin(*n, "RGBA",  PinType::Vec4,      true,  pid);
    AllocPin(*n, "RGB",   PinType::Vec3,      true,  pid);
    AllocPin(*n, "R",     PinType::Float,     true,  pid);
    return n;
}

std::shared_ptr<ConstantVec3Node> ConstantVec3Node::Create(uint32_t id, glm::vec2 pos)
{
    auto n = std::make_shared<ConstantVec3Node>();
    n->ID = id; n->Name = "Vec3"; n->Position = pos;
    uint32_t pid = id * 100;
    AllocPin(*n, "Value", PinType::Vec3, true, pid);
    return n;
}

std::shared_ptr<MultiplyNode> MultiplyNode::Create(uint32_t id, glm::vec2 pos)
{
    auto n = std::make_shared<MultiplyNode>();
    n->ID = id; n->Name = "Multiply"; n->Position = pos;
    uint32_t pid = id * 100;
    AllocPin(*n, "A",      PinType::Vec4,  false, pid);
    AllocPin(*n, "B",      PinType::Vec4,  false, pid);
    AllocPin(*n, "Result", PinType::Vec4,  true,  pid);
    return n;
}

std::shared_ptr<LerpNode> LerpNode::Create(uint32_t id, glm::vec2 pos)
{
    auto n = std::make_shared<LerpNode>();
    n->ID = id; n->Name = "Lerp"; n->Position = pos;
    uint32_t pid = id * 100;
    AllocPin(*n, "A",      PinType::Vec4,  false, pid);
    AllocPin(*n, "B",      PinType::Vec4,  false, pid);
    AllocPin(*n, "Alpha",  PinType::Float, false, pid);
    AllocPin(*n, "Result", PinType::Vec4,  true,  pid);
    return n;
}


// ----------------------------------------------------------------
// MaterialGraph — lookup
// ----------------------------------------------------------------
MaterialNode* MaterialGraph::FindNode(uint32_t id)
{
    for (auto& n : Nodes)
        if (n->ID == id) return n.get();
    return nullptr;
}

MaterialPin* MaterialGraph::FindPin(uint32_t id)
{
    for (auto& n : Nodes) {
        for (auto& p : n->Inputs)  if (p.ID == id) return &p;
        for (auto& p : n->Outputs) if (p.ID == id) return &p;
    }
    return nullptr;
}

void MaterialGraph::RemoveNode(uint32_t id)
{
    // Remove all links touching this node's pins first
    std::vector<uint32_t> pinIDs;
    if (auto* n = FindNode(id)) {
        for (auto& p : n->Inputs)  pinIDs.push_back(p.ID);
        for (auto& p : n->Outputs) pinIDs.push_back(p.ID);
    }
    Links.erase(std::remove_if(Links.begin(), Links.end(), [&](const MaterialLink& l){
        return std::find(pinIDs.begin(), pinIDs.end(), l.FromPin) != pinIDs.end() ||
               std::find(pinIDs.begin(), pinIDs.end(), l.ToPin)   != pinIDs.end();
    }), Links.end());

    Nodes.erase(std::remove_if(Nodes.begin(), Nodes.end(),
        [id](const std::shared_ptr<MaterialNode>& n){ return n->ID == id; }), Nodes.end());
}

void MaterialGraph::RemoveLink(uint32_t id)
{
    Links.erase(std::remove_if(Links.begin(), Links.end(),
        [id](const MaterialLink& l){ return l.ID == id; }), Links.end());
}

bool MaterialGraph::AddLink(uint32_t fromPin, uint32_t toPin)
{
    auto* src = FindPin(fromPin);
    auto* dst = FindPin(toPin);
    if (!src || !dst) return false;
    // Allow Vec3/Vec4 cross-connection; strict match otherwise
    bool compatible = (src->Type == dst->Type) ||
                      (src->Type == PinType::Vec4 && dst->Type == PinType::Vec3) ||
                      (src->Type == PinType::Vec3 && dst->Type == PinType::Vec4);
    if (!compatible) return false;

    MaterialLink lnk;
    lnk.ID      = NextLinkID();
    lnk.FromPin = fromPin;
    lnk.ToPin   = toPin;
    Links.push_back(lnk);
    return true;
}

void MaterialGraph::InitDefault()
{
    Nodes.clear(); Links.clear();
    m_NextNodeID = m_NextPinID = m_NextLinkID = 0;
    Nodes.push_back(PBROutputNode::Create(NextNodeID()));
}

void MaterialGraph::SyncIDCounters()
{
    for (auto& n : Nodes) {
        m_NextNodeID = std::max(m_NextNodeID, n->ID);
        for (auto& p : n->Inputs)  m_NextPinID = std::max(m_NextPinID, p.ID);
        for (auto& p : n->Outputs) m_NextPinID = std::max(m_NextPinID, p.ID);
    }
    for (auto& l : Links) m_NextLinkID = std::max(m_NextLinkID, l.ID);
}


// ----------------------------------------------------------------
// MaterialGraph::DeserializeNode — factory by type string
// ----------------------------------------------------------------
std::shared_ptr<MaterialNode> MaterialGraph::DeserializeNode(const nlohmann::json& j)
{
    std::string type = j.at("type").get<std::string>();
    uint32_t    id   = j.at("id").get<uint32_t>();
    glm::vec2   pos  = { j.at("posX").get<float>(), j.at("posY").get<float>() };

    std::shared_ptr<MaterialNode> node;

    if      (type == "PBROutput")    node = PBROutputNode::Create(id, pos);
    else if (type == "TextureSample")node = TextureSampleNode::Create(id, pos);
    else if (type == "ConstantVec3") node = ConstantVec3Node::Create(id, pos);
    else if (type == "Multiply")     node = MultiplyNode::Create(id, pos);
    else if (type == "Lerp")         node = LerpNode::Create(id, pos);
    else
    {
        LOG(LogLevel::Warning, "MaterialGraph: unknown node type '{0}', skipping.", type);
        return nullptr;
    }

    // Override pin IDs/defaults from the saved data
    node->Name = j.at("name").get<std::string>();

    auto overridePins = [](std::vector<MaterialPin>& pins, const nlohmann::json& arr) {
        for (size_t i = 0; i < pins.size() && i < arr.size(); ++i) {
            pins[i].ID           = arr[i].at("id").get<uint32_t>();
            pins[i].DefaultValue = DeserializePin(arr[i]).DefaultValue;
        }
    };
    if (j.contains("inputs"))  overridePins(node->Inputs,  j.at("inputs"));
    if (j.contains("outputs")) overridePins(node->Outputs, j.at("outputs"));

    node->DeserializeExtra(j);
    return node;
}


// ----------------------------------------------------------------
// SaveToFile / LoadFromFile
// ----------------------------------------------------------------
bool MaterialGraph::SaveToFile(const std::filesystem::path& path) const
{
    nlohmann::json root;
    root["name"]    = Name;
    root["version"] = 1;

    auto& jNodes = root["nodes"] = nlohmann::json::array();
    for (auto& n : Nodes)
        jNodes.push_back(SerializeNode(*n));

    auto& jLinks = root["links"] = nlohmann::json::array();
    for (auto& l : Links)
        jLinks.push_back({ {"id", l.ID}, {"fromPin", l.FromPin}, {"toPin", l.ToPin} });

    root["nextNodeID"] = m_NextNodeID;
    root["nextPinID"]  = m_NextPinID;
    root["nextLinkID"] = m_NextLinkID;

    std::ofstream file(path);
    if (!file.is_open()) {
        LOG(LogLevel::Error, "MaterialGraph: failed to open '{0}' for writing.", path.string());
        return false;
    }
    file << root.dump(4);
    return true;
}

bool MaterialGraph::LoadFromFile(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG(LogLevel::Error, "MaterialGraph: failed to open '{0}' for reading.", path.string());
        return false;
    }

    nlohmann::json root;
    try { file >> root; }
    catch (const nlohmann::json::exception& e) {
        LOG(LogLevel::Error, "MaterialGraph: JSON parse error in '{0}': {1}", path.string(), e.what());
        return false;
    }

    Nodes.clear(); Links.clear();
    Name = root.value("name", "Material");

    for (auto& jn : root.at("nodes")) {
        auto node = DeserializeNode(jn);
        if (node) Nodes.push_back(std::move(node));
    }
    for (auto& jl : root.at("links")) {
        MaterialLink l;
        l.ID      = jl.at("id").get<uint32_t>();
        l.FromPin = jl.at("fromPin").get<uint32_t>();
        l.ToPin   = jl.at("toPin").get<uint32_t>();
        Links.push_back(l);
    }

    m_NextNodeID = root.value("nextNodeID", 0u);
    m_NextPinID  = root.value("nextPinID",  0u);
    m_NextLinkID = root.value("nextLinkID", 0u);
    SyncIDCounters(); // guard against stale counters in older files
    return true;
}

} // namespace Engine
