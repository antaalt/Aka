#pragma once 

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Core/Container/String.h>
#include <Aka/Memory/Pool.h>
#include <Aka/Scene/Node3D.hpp> // For Pool<Node3D>, might create cyclic dependencies

namespace aka {

struct ArchiveScene;
class AssetLibrary;
class Node3D;

class Scene : public Resource 
{
public:
	Scene();
	Scene(ResourceID _id, const String& _name);
	~Scene();
private:
	void create_internal(AssetLibrary* library, Renderer* _renderer, const Archive& _archive) override;
	void save_internal(AssetLibrary* library, Renderer* _renderer, Archive& _archive) override;
	void destroy_internal(AssetLibrary* library, Renderer* _renderer) override;

public:
	aabbox<> getBounds() const { return m_bounds; }
	Node3D& getRoot() { return *m_root; }
	const Node3D& getRoot() const { return *m_root; }
	Node3D* createChild(Node3D* parent, const char* name);
private:
	aabbox<> m_bounds;
	Pool<Node3D> m_nodePool;
	Node3D* m_root;
};



};