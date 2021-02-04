#include "EntityWidget.h"

#include <Aka.h>

#include "../Component/Transform2D.h"
#include "../Component/Collider2D.h"
#include "../Component/Camera2D.h"
#include "../Component/Text.h"
#include "../Component/TileMap.h"
#include "../Component/TileLayer.h"
#include "../Component/Player.h"
#include "../Component/Animator.h"
#include "../Component/Coin.h"

#include "../System/SoundSystem.h"

#include "Modal.h"

namespace aka {

void EntityWidget::draw(World& world, Resources& resources)
{
	if (ImGui::Begin("Entities##window"))
	{
		ImVec4 color = ImVec4(236.f / 255.f, 11.f / 255.f, 67.f / 255.f, 1.f);
		static const int componentCount = 12;
		static const char* components[componentCount] = {
			"None",
			"Transform2D",
			"Animator",
			"Collider2D",
			"RigidBody2D",
			"Text",
			"TileMap",
			"TileLayer",
			"Coin",
			"Player",
			"Camera2D",
			"SoundInstance"
		};
		static int currentFilter = 0;
		ImGui::Combo("Filter", &currentFilter, components, componentCount);
		uint32_t index = 0;
		if (ImGui::BeginChild("##list", ImVec2(0, -30), true))
		{
			world.each([&](Entity* entity) {
				index++;
				switch (currentFilter)
				{
				case 1: if (!entity->has<Transform2D>()) return; break;
				case 2: if (!entity->has<Animator>()) return; break;
				case 3: if (!entity->has<Collider2D>()) return; break;
				case 4: if (!entity->has<RigidBody2D>()) return; break;
				case 5: if (!entity->has<Text>()) return; break;
				case 6: if (!entity->has<TileMap>()) return; break;
				case 7: if (!entity->has<TileLayer>()) return; break;
				case 8: if (!entity->has<Coin>()) return; break;
				case 9: if (!entity->has<Player>()) return; break;
				case 10: if (!entity->has<Camera2D>()) return; break;
				case 11: if (!entity->has<SoundInstance>()) return; break;
				}
				char buffer[256];
				snprintf(buffer, 256, "Entity %u", index);
				if (ImGui::CollapsingHeader(buffer))
				{
					// --- Transform2D
					if (entity->has<Transform2D>())
					{
						Transform2D* transform = entity->get<Transform2D>();
						snprintf(buffer, 256, "Transform2D##%p", transform);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat2("Position", transform->model[2].data, 3);
							vec2f size = vec2f(transform->model[0].x, transform->model[1].y);
							if (ImGui::InputFloat2("Size", size.data, 3))
							{
								transform->model[0].x = size.x;
								transform->model[1].y = size.y;
							}
							//ImGui::InputFloat("Rotation", &transform->rotation(), 0.1f, 1.f, 3);

							if (ImGui::Button("Remove")) { entity->remove<Transform2D>(); }

							ImGui::TreePop();
						}
					}
					// --- Animator
					if (entity->has<Animator>())
					{
						Animator* animator = entity->get<Animator>();
						snprintf(buffer, 256, "Animator##%p", animator);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							char currentSpriteName[256] = "None";
							Sprite* currentSprite = animator->sprite;
							for (auto& sprite : resources.sprite)
							{
								if (currentSprite == sprite.second)
								{
									strcpy_s(currentSpriteName, 256, sprite.first.c_str());
								}
							}
							snprintf(buffer, 256, "%s", currentSpriteName);
							if (ImGui::BeginCombo("Sprite", buffer))
							{
								for (auto& sprite : resources.sprite)
								{
									bool sameSprite = (currentSprite == sprite.second);
									snprintf(buffer, 256, "%s", sprite.first.c_str());
									if (ImGui::Selectable(buffer, sameSprite))
									{
										if (!sameSprite)
										{
											animator->sprite = sprite.second;
											animator->currentAnimation = 0;
											animator->currentFrame = 0;
											animator->update();
											strcpy_s(currentSpriteName, 256, sprite.first.c_str());
										}
									}
									if (sameSprite)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}
							uint32_t i = 0;
							for (Sprite::Animation& anim : animator->sprite->animations)
							{
								snprintf(buffer, 256, "%s (%llu ms)", anim.name.c_str(), anim.duration().milliseconds());
								bool currentAnimation = animator->currentAnimation == i;
								if (ImGui::RadioButton(buffer, currentAnimation))
								{
									animator->currentAnimation = i;
									animator->currentFrame = 0;
									animator->update();
								}
								i++;
							}
							ImGui::SliderInt("Frame", reinterpret_cast<int*>(&animator->currentFrame), 0, animator->sprite ? (int)animator->sprite->animations[animator->currentAnimation].frames.size() - 1 : 0);
							ImGui::SliderInt("Layer", &animator->layer, -20, 20);

							if (ImGui::Button("Remove")) { entity->remove<Animator>(); }

							ImGui::TreePop();
						}
					}
					// --- Collider2D
					if (entity->has<Collider2D>())
					{
						Collider2D* collider = entity->get<Collider2D>();
						snprintf(buffer, 256, "Collider2D##%p", collider);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat2("Position", collider->position.data, 3);
							ImGui::InputFloat2("Size", collider->size.data, 3);
							ImGui::InputFloat("Bouncing", &collider->bouncing, 0.1f, 1.f, 2);
							ImGui::InputFloat("Friction", &collider->friction, 0.1f, 1.f, 2);

							if (ImGui::Button("Remove")) { entity->remove<Collider2D>(); }

							ImGui::TreePop();
						}
					}
					// --- RigidBody2D
					if (entity->has<RigidBody2D>())
					{
						RigidBody2D* rigid = entity->get<RigidBody2D>();
						snprintf(buffer, 256, "RigidBody2D##%p", rigid);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat("Mass", &rigid->mass, 0.1f, 1.f, 2);
							ImGui::InputFloat2("Acceleration", rigid->acceleration.data, 3);
							ImGui::InputFloat2("Velocity", rigid->velocity.data, 3);

							if (ImGui::Button("Remove")) { entity->remove<RigidBody2D>(); }

							ImGui::TreePop();
						}
					}
					// --- TileMap
					if (entity->has<TileMap>())
					{
						TileMap* map = entity->get<TileMap>();
						snprintf(buffer, 256, "TileMap##%p", map);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							vec2i gridCount = vec2i(map->gridCount);
							vec2i gridSize = vec2i(map->gridSize);
							if (ImGui::InputInt2("Grid count", gridCount.data))
								map->gridCount = vec2u(gridCount);
							if (ImGui::InputInt2("Grid size", gridSize.data))
								map->gridSize = vec2u(gridSize);
							if (map->texture == nullptr)
							{
								Path path;
								if (Modal::LoadButton("Load image", &path))
								{
									try
									{
										Image image = Image::load(path);
										Sampler sampler;
										sampler.filterMag = aka::Sampler::Filter::Nearest;
										sampler.filterMin = aka::Sampler::Filter::Nearest;
										sampler.wrapS = aka::Sampler::Wrap::Clamp;
										sampler.wrapT = aka::Sampler::Wrap::Clamp;
										map->texture = Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), sampler);
									}
									catch (const std::exception&) {}
								}
							}
							else
							{
								float ratio = static_cast<float>(map->texture->width()) / static_cast<float>(map->texture->height());
								ImGui::Image((void*)map->texture->handle().value(), ImVec2(384, 384 * 1 / ratio), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), color);
							}
							if (ImGui::Button("Remove")) { entity->remove<TileMap>(); }

							ImGui::TreePop();
						}
					}
					// --- TileLayer
					if (entity->has<TileLayer>())
					{
						TileLayer* layer = entity->get<TileLayer>();
						snprintf(buffer, 256, "TileLayer##%p", layer);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							vec2i gridCount = vec2i(layer->gridCount);
							vec2i gridSize = vec2i(layer->gridSize);
							if (ImGui::InputInt2("Grid count", gridCount.data))
								layer->gridCount = vec2u(gridCount);
							if (ImGui::InputInt2("Grid size", gridSize.data))
								layer->gridSize = vec2u(gridSize);

							static int id = 0;
							if (layer->tileID.size() >= 4)
							{
								ImGui::SliderInt("Index", &id, 0, (int)layer->tileID.size() - 4);
								ImGui::InputInt4("TileID", layer->tileID.data() + id);
							}
							ImGui::SliderInt("Layer", &layer->layer, -20, 20);
							ImGui::InputFloat4("Color", layer->color.data, 3);

							if (ImGui::Button("Remove")) { entity->remove<TileLayer>(); }

							ImGui::TreePop();
						}
					}
					// --- Text
					if (entity->has<Text>())
					{
						Text* text = entity->get<Text>();
						snprintf(buffer, 256, "Text##%p", text);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat4("Color", text->color.data, 3);
							ImGui::SliderInt("Layer", &text->layer, -20, 20);

							uint32_t currentHeight = text->font->height();
							const char* currentFont = text->font->family().c_str();
							snprintf(buffer, 256, "%s (%u)", text->font->family().c_str(), text->font->height());
							if (ImGui::BeginCombo("Font", buffer))
							{
								for (auto& font : resources.font)
								{
									bool sameHeight = (currentHeight == font.second->height());
									bool sameFamily = (currentFont == font.second->family().c_str());
									bool sameFont = sameHeight && sameFamily;
									snprintf(buffer, 256, "%s (%u)", font.second->family().c_str(), font.second->height());
									if (ImGui::Selectable(buffer, sameFont))
									{
										if (!sameFont)
											text->font = font.second;
									}
									if (sameFont)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}

							char t[256];
							strcpy_s(t, text->text.c_str());
							if (ImGui::InputText("Text", t, 256))
								text->text = t;

							if (ImGui::Button("Remove")) { entity->remove<Text>(); }

							ImGui::TreePop();
						}
					}
					// --- Coin
					if (entity->has<Coin>())
					{
						Coin* coin = entity->get<Coin>();
						snprintf(buffer, 256, "Coin##%p", coin);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Checkbox("Picked", &coin->picked);

							if (ImGui::Button("Remove")) { entity->remove<Coin>(); }

							ImGui::TreePop();
						}
					}
					// --- Player
					if (entity->has<Player>())
					{
						Player* player = entity->get<Player>();
						snprintf(buffer, 256, "Player##%p", player);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::SliderInt("Coin", &player->coin, 0, 50);
							float metric = player->speed.metric();
							if (ImGui::SliderFloat("Speed", &metric, 0.f, 50.f))
							{
								player->speed = Speed(metric);
							}
							player->state;
							player->jump;
							player->left;
							player->right;

							if (ImGui::Button("Remove")) { entity->remove<Player>(); }

							ImGui::TreePop();
						}
					}
					// --- Camera2D
					if (entity->has<Camera2D>())
					{
						Camera2D* camera = entity->get<Camera2D>();
						snprintf(buffer, 256, "Camera2D##%p", camera);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::InputFloat2("Position", camera->position.data, 3);
							ImGui::InputFloat2("Viewport", camera->viewport.data, 3);

							if (ImGui::Button("Remove")) { entity->remove<Camera2D>(); }

							ImGui::TreePop();
						}
					}
					// --- SoundInstance
					if (entity->has<SoundInstance>())
					{
						SoundInstance* sound = entity->get<SoundInstance>();
						snprintf(buffer, 256, "SoundInstance##%p", sound);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Text("Sound : %s", sound->path.c_str());
							ImGui::SliderFloat("Volume", &sound->volume, 0.f, 2.f);
							ImGui::Checkbox("Loop", &sound->loop);

							if (ImGui::Button("Remove")) { entity->remove<SoundInstance>(); }

							ImGui::TreePop();
						}
					}
					// --- Add component
					static int currentComponent = 0;
					if (ImGui::Button("Add")) {
						switch (currentComponent)
						{
						case 1: entity->add<Transform2D>(Transform2D()); break;
						case 2: entity->add<Animator>(Animator(resources.sprite.getDefault(), 0)); break;
						case 3: entity->add<Collider2D>(Collider2D()); break;
						case 4: entity->add<RigidBody2D>(RigidBody2D()); break;
						case 5: entity->add<Text>(Text()); break;
						case 6: entity->add<TileMap>(TileMap()); break;
						case 7: entity->add<TileLayer>(TileLayer()); break;
						case 8: entity->add<Coin>(Coin()); break;
						case 9: entity->add<Player>(Player()); break;
						case 10: entity->add<Camera2D>(Camera2D()); break;
						case 11: entity->add<SoundInstance>(SoundInstance()); break;
						}
					}
					ImGui::SameLine();
					ImGui::Combo("Component", &currentComponent, components, componentCount);
				}
			});
		}
		ImGui::EndChild();
		if (ImGui::Button("Add entity")) {
			world.createEntity();
		}
	}
	ImGui::End();
}

};