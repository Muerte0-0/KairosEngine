#pragma once

#include <entt.hpp>
#include <unordered_map>
#include <vector>

namespace Engine
{
    // EntityID is entt::entity — SceneGraph is an entt-agnostic data layer
    // UUID-based identity is only used at the serialization boundary.

    using EntityID = entt::entity;
    static constexpr EntityID INVALID_ENTITY = entt::null;

    struct SceneNode
    {
        EntityID            Entity   = INVALID_ENTITY;
        EntityID            Parent   = INVALID_ENTITY;
        std::vector<EntityID> Children;
    };

    class SceneGraph
    {
    public:
        // Register / unregister entities
        void AddEntity(EntityID id);
        void RemoveEntity(EntityID id);

        // Returns nullptr if entity not in graph
        SceneNode* GetNode(EntityID id);
        const SceneNode* GetNode(EntityID id) const;

        // Parenting — safe, cycle-checked
        // Preserving world transform is caller's responsibility (editor drag-drop).
        void SetParent(EntityID child, EntityID newParent);
        void RemoveParent(EntityID child);

        // Traversal roots
        const std::vector<EntityID>& GetRootNodes() const { return m_RootNodes; }

        // Cycle detection helper
        bool IsAncestor(EntityID potentialAncestor, EntityID of) const;

    private:
        void RemoveFromParent(EntityID child);   // internal: detach from current parent
        void AddToRoots(EntityID id);
        void RemoveFromRoots(EntityID id);

        std::unordered_map<EntityID, SceneNode> m_Nodes;
        std::vector<EntityID>                   m_RootNodes;
    };
}
