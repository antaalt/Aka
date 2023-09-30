#pragma once 

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Core/Container/String.h>
#include <Aka/Memory/Pool.h>
#include <Aka/Scene/Node.hpp> // For Pool<Node>, might create cyclic dependencies

namespace aka {

struct ArchiveScene;
class AssetLibrary;
class Node3D;

class Scene : public Resource 
{
public:
	Scene();
	Scene(AssetID _id, const String& _name);
	~Scene();
private:
	void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) override;
	void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) override;
	void destroy_internal(AssetLibrary* library, Renderer* _renderer) override;

public:
	aabbox<> getBounds() const { return m_bounds; }
	Node& getRoot() { return *m_root; }
	const Node& getRoot() const { return *m_root; }
	Node* createChild(Node* parent, const char* name);
	void destroyChild(Node* node);
private:
	aabbox<> m_bounds;
	Pool<Node> m_nodePool;
	Node* m_root;
};



};