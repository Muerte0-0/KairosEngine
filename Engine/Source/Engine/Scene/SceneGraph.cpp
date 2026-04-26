#include "kepch.h"
#include "SceneGraph.h"

namespace Engine
{
    void SceneGraph::AddEntity(EntityID id)
    {
        ASSERT(id != INVALID_ENTITY, "SceneGraph::AddEntity — invalid entity");
        if (m_Nodes.contains(id)) return;

        SceneNode node;
        node.Entity = id;
        m_Nodes.emplace(id, std::move(node));
        AddToRoots(id);
    }

    void SceneGraph::RemoveEntity(EntityID id)
    {
        auto it = m_Nodes.find(id);
        if (it == m_Nodes.end()) return;

        SceneNode& node = it->second;

        // Promote children to roots
        for (EntityID child : node.Children)
        {
            if (auto* childNode = GetNode(child))
            {
                childNode->Parent = INVALID_ENTITY;
                AddToRoots(child);
            }
        }

        // Detach from parent
        RemoveFromParent(id);

        RemoveFromRoots(id);
        m_Nodes.erase(it);
    }

    SceneNode* SceneGraph::GetNode(EntityID id)
    {
        auto it = m_Nodes.find(id);
        return it != m_Nodes.end() ? &it->second : nullptr;
    }

    const SceneNode* SceneGraph::GetNode(EntityID id) const
    {
        auto it = m_Nodes.find(id);
        return it != m_Nodes.end() ? &it->second : nullptr;
    }

    bool SceneGraph::IsAncestor(EntityID potentialAncestor, EntityID of) const
    {
        EntityID cur = of;
        while (cur != INVALID_ENTITY)
        {
            if (cur == potentialAncestor) return true;
            const SceneNode* n = GetNode(cur);
            if (!n) break;
            cur = n->Parent;
        }
        return false;
    }

    void SceneGraph::SetParent(EntityID child, EntityID newParent)
    {
        ASSERT(child != INVALID_ENTITY, "SceneGraph::SetParent — invalid child");
        ASSERT(m_Nodes.contains(child), "SceneGraph::SetParent — child not in graph");
        ASSERT(m_Nodes.contains(newParent), "SceneGraph::SetParent — newParent not in graph");
        ASSERT(child != newParent, "SceneGraph::SetParent — self-parenting");
        ASSERT(!IsAncestor(child, newParent), "SceneGraph::SetParent — cycle detected");

        SceneNode& childNode = m_Nodes.at(child);
        if (childNode.Parent == newParent) return;

        RemoveFromParent(child);        // detach from old parent / roots
        RemoveFromRoots(child);         // ensure not in roots

        childNode.Parent = newParent;
        m_Nodes.at(newParent).Children.push_back(child);
    }

    void SceneGraph::RemoveParent(EntityID child)
    {
        ASSERT(m_Nodes.contains(child), "SceneGraph::RemoveParent — child not in graph");
        SceneNode& node = m_Nodes.at(child);
        if (node.Parent == INVALID_ENTITY) return;

        RemoveFromParent(child);
        AddToRoots(child);
    }

    // ---------- private helpers ----------

    void SceneGraph::RemoveFromParent(EntityID child)
    {
        SceneNode& node = m_Nodes.at(child);
        if (node.Parent == INVALID_ENTITY) return;

        auto* parentNode = GetNode(node.Parent);
        if (parentNode)
        {
            auto& ch = parentNode->Children;
            ch.erase(std::remove(ch.begin(), ch.end(), child), ch.end());
        }
        node.Parent = INVALID_ENTITY;
    }

    void SceneGraph::AddToRoots(EntityID id)
    {
        if (std::find(m_RootNodes.begin(), m_RootNodes.end(), id) == m_RootNodes.end())
            m_RootNodes.push_back(id);
    }

    void SceneGraph::RemoveFromRoots(EntityID id)
    {
        m_RootNodes.erase(std::remove(m_RootNodes.begin(), m_RootNodes.end(), id), m_RootNodes.end());
    }
}
