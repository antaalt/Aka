#pragma once

#include <Aka/Core/Geometry.h>

#include <Aka/OS/Time.h>
#include <Aka/Scene/Component.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Renderer/View.hpp>

namespace aka {

struct ArchiveArcballComponent : ArchiveComponent
{
	ArchiveArcballComponent(ArchiveComponentVersionType _version);

	void parse(BinaryArchive& archive) override;
};

class ArcballComponent : public Component<ArcballComponent, ArchiveArcballComponent>
{
public:
	ArcballComponent(Node* node);
	~ArcballComponent();

	CameraArcball& getController() { return *m_controller; }
	const CameraArcball& getController() const { return *m_controller; }

public:
	// Set bounds to focus for camera
	void setBounds(const aabbox<>& bounds);
	// Set if updates are enabled on this camera.
	void setUpdateEnabled(bool value);
	// Check if updates are enabled on this camera.
	bool isUpdateEnabled() const;
public:
	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;
	void onUpdate(Time deltaTime) override;
public:
	void fromArchive(const ArchiveArcballComponent& archive) override;
	void toArchive(ArchiveArcballComponent& archive) override;

private:
	bool m_updateEnabled;
	bool m_dirtyBounds;
	aabbox<> m_bounds;
	CameraArcball* m_controller;
};

AKA_DECL_COMPONENT(ArcballComponent);


};