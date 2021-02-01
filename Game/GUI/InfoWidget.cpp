#include "InfoWidget.h"

#include "../Component/Collider2D.h"
#include "../Component/Player.h"
#include "../Component/Animator.h"
#include "../Component/Coin.h"

namespace aka {

void InfoWidget::draw(World& world, Resources& resources)
{
	if (ImGui::Begin("Infos"))
	{
		uint32_t width, height;
		PlatformBackend::getSize(&width, &height);
		static Device device = Device::getDefault();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Resolution : %ux%u", width, height);
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		//ImGui::Text("Draw call : %u", m_drawCall);
		const char* apiName[] = {
			"OpenGL",
			"DirectX11"
		};
		ImGui::Text("Api : %s", apiName[(int)GraphicBackend::api()]);
		ImGui::Text("Device : %s", device.vendor);

		static bool renderColliders = false;
		if (ImGui::Checkbox("Render colliders", &renderColliders))
		{
			world.each<Collider2D>([&](Entity* entity, Collider2D* collider) {
				if (renderColliders)
				{
					if (!entity->has<Animator>())
						entity->add<Animator>(Animator(resources.sprite.get("Collider"), 2));
				}
				else
				{
					if (entity->has<Animator>() && !entity->has<Player>() && !entity->has<Coin>())
						entity->remove<Animator>();
				}
			});
		}
		static bool vsync = true;
		if (ImGui::Checkbox("Vsync", &vsync))
		{
			GraphicBackend::vsync(vsync);
		}
	}
	ImGui::End();
}

};