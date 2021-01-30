#include "ResourcesWidget.h"

#include "Modal.h"

namespace aka {

void ResourcesWidget::draw(World& world, Resources& resources)
{
	if (ImGui::Begin("Resources"))
	{
		if (ImGui::CollapsingHeader("Fonts", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto it : resources.font)
			{
				std::string name = it.first;
				Font* font = it.second;
				if (ImGui::TreeNode(name.c_str()))
				{
					ImGui::Text("Family : %s", font->family().c_str());
					ImGui::Text("Style : %s", font->style().c_str());
					ImGui::Text("Height : %upx", font->height());
					for (uint32_t i = 0; i < NUM_GLYPH; i++)
					{
						const Character &character = font->getCharacter(i);
						char buffer[256];
						int error = snprintf(buffer, 256, "Glyph %u", i);
						if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet))
						{
							ImGui::Text("Advance : %u", character.advance);
							ImGui::Text("Size : (%d, %d)", character.size.x, character.size.y);
							ImGui::Text("Bearing : (%d, %d)", character.bearing.x, character.bearing.y);
							ImGui::Image(
								(ImTextureID)character.texture.texture->handle().value(),
								ImVec2((float)character.size.x, (float)character.size.y),
								ImVec2(character.texture.get(0).u, character.texture.get(0).v),
								ImVec2(character.texture.get(1).u, character.texture.get(1).v)
							);
							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::Image((ImTextureID)character.texture.texture->handle().value(), ImVec2(300, 300));
								ImGui::EndTooltip();
							}
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
				ImGui::Separator();
			}
			static std::string error;
			static int height = 48;
			ImGui::SliderInt("Height", &height, 1, 200);
			Path path;
			if (Modal::LoadButton("Load font", &path))
			{
				if (file::exist(Path(path)))
				{
					try
					{
						if (resources.font.create(path.str(), new Font(path, height)) == nullptr)
						{
							error = "Failed to load the font";
						}
						else
						{
							error = "";
						}
					}
					catch (const std::exception& e)
					{
						error = e.what();
					}
				}
				else
				{
					error = "File does not exist";
				}
				path = "";
			}
			if (error.size() > 0)
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", error.c_str());
		}
		if (ImGui::CollapsingHeader("Sprites", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto it : resources.sprite)
			{
				std::string name = it.first;
				Sprite* sprite = it.second;
				if (ImGui::TreeNode(name.c_str()))
				{
					for (Sprite::Animation& animation : sprite->animations)
					{
						if (ImGui::TreeNodeEx(animation.name.c_str()))
						{
							uint32_t frameID = 0;
							for (Sprite::Frame& frame : animation.frames)
							{
								char buffer[256];
								int error = snprintf(buffer, 256, "Frame %u", frameID++);
								if (ImGui::TreeNodeEx(buffer, ImGuiTreeNodeFlags_Bullet))
								{
									int d = (int)frame.duration.milliseconds();
									if (ImGui::SliderInt("Duration", &d, 0, 1000))
									{
										frame.duration = Time::Unit::milliseconds(d);
									}
									int size[2]{ frame.width, frame.height };
									ImGui::InputInt2("Size", size);
									ImGui::Image((ImTextureID)frame.texture->handle().value(), ImVec2(200, 200));
									ImGui::TreePop();
								}
							}
							static std::string error;
							Path path;
							if (Modal::LoadButton("Load image", &path))
							{
								Image image = Image::load(path);
								if (image.bytes.size() == 0)
								{
									error = "Could not load image";
								}
								else
								{
									Sprite::Frame frame = Sprite::Frame::create(
										Texture::create(image.width, image.height, Texture::Format::Rgba, image.bytes.data(), Sampler::Filter::Nearest),
										Time::Unit::milliseconds(500)
									);
									animation.frames.push_back(frame);
								}
							}
							if (error.size() > 0)
								ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", error.c_str());
							ImGui::TreePop();
						}
					}
					static char buffer[256];
					ImGui::InputText("##Name", buffer, 256);
					ImGui::SameLine();
					if (ImGui::Button("Add animation"))
					{
						Sprite::Animation animation;
						animation.name = buffer;
						sprite->animations.push_back(animation);
					}

					ImGui::TreePop();
				}
				ImGui::Separator();
			}
		}
	}
	ImGui::End();
}

};