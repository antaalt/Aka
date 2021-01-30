#include "Modal.h"

namespace aka {

bool Modal::LoadButton(const char* label, Path* resultPath) {
	char buffer[256];
	snprintf(buffer, 256, "%s##popupmodal", label);
	static char currentPathBuffer[256];
	static Path currentPath;
	static Path* selectedPath;
	static std::vector<Path> paths;
	if (ImGui::Button(label))
	{
		ImGui::OpenPopup(buffer);
		currentPath = Asset::path("");
		strcpy_s(currentPathBuffer, 256, currentPath.c_str());
		selectedPath = nullptr;
		paths = Path::enumerate(currentPath);
	}
	bool loading = false;
	bool openFlag = true;
	if (ImGui::BeginPopupModal(buffer, &openFlag, ImGuiWindowFlags_AlwaysAutoResize))
	{
		bool updatedList = false;
		// Go to parent folder
		if (ImGui::ArrowButton("##Up", ImGuiDir_Up))
		{
			std::string path = currentPath.str();
			const size_t separatorCount = std::count(path.begin(), path.end(), '/');
			const size_t lastCharacterOffset = path.size() - 1;
			size_t offset = path.find_last_of('/');
			if (1 == separatorCount)
			{
				if (offset != lastCharacterOffset)
				{
					currentPath = Path(path.substr(0, offset + 1));
					updatedList = true;
				}
			}
			else
			{
				if (offset == lastCharacterOffset)
				{
					offset = path.substr(0, offset).find_last_of('/');
					currentPath = path.substr(0, offset + 1);
					updatedList = true;
				}
				else
				{
					currentPath = path.substr(0, offset + 1);
					updatedList = true;
				}
			}
		}
		// Refresh directory
		ImGui::SameLine();
		if (ImGui::Button("Refresh"))
		{
			updatedList = true;
		}
		ImGui::SameLine();
		// Path 
		if (ImGui::InputText("##Path", currentPathBuffer, 256))
		{
			currentPath = currentPathBuffer;
			updatedList = true;
		}

		/*ImVec4 dirColor = ImVec4(0.93f, 0.04f, 0.26f, 1.f);
		ImVec4 fileColor = ImVec4(0.01f, 0.47f, 0.96f, 1.f);
		ImGui::TextColored(dirColor, "Directory"); ImGui::SameLine();
		ImGui::TextColored(fileColor, "File");*/
		if (ImGui::BeginChild("##list", ImVec2(0, 200), true))
		{
			for (Path& path : paths)
			{
				bool selected = (&path == selectedPath);
				if (ImGui::Selectable(path.c_str(), &selected))
				{
					selectedPath = &path;
				}
				if (path.str().back() == '/' && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					currentPath += path;
					updatedList = true;
				}
			}
		}
		ImGui::EndChild();
		if (updatedList)
		{
			strcpy_s(currentPathBuffer, 256, currentPath.c_str());
			paths = Path::enumerate(currentPath);
			selectedPath = nullptr;
		}
		if (ImGui::Button("Load"))
		{
			if (selectedPath != nullptr)
			{
				loading = true;
				*resultPath = currentPath + *selectedPath;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	return (loading && resultPath->str().size() > 0);
};

};