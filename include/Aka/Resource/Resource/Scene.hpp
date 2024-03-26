#pragma once 

#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Core/Container/String.h>
#include <Aka/Memory/Pool.h>
#include <Aka/Scene/NodeAllocator.hpp>

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
	void update(Time _deltaTime);
	void fixedUpdate(Time _deltaTime);
	void update(AssetLibrary* _library, Renderer* _renderer);

	aabbox<> getBounds() const { return m_bounds; }
	void visitChildrens(std::function<void(Node*)> _callback);
	Node* createChild(Node* parent, const char* name);
	void destroyChild(Node* node);

	// TODO: getmainCameraNode should be determined with cameracomponent :: main
	const Node* getMainCameraNode() const { return m_mainCamera; }
	void setMainCameraNode(Node* node);
private:
	aabbox<> m_bounds;
	NodeAllocator m_allocator;
	Node* m_mainCamera;
	Node* m_root;
};



};