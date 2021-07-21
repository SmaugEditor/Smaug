#include "texturebrowser.h"
#include "texturemanager.h"
#include <filesystem>
#include <string>
#include <cmath>
#include "imgui.h"

CTextureBrowser::CTextureBrowser()
{
	std::string path = "textures";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string p = entry.path().string();
		texture_t th = TextureManager().LoadTexture(p.c_str());
		m_textures.push_back({ p, th });
	}
	m_selectedTexture = INVALID_TEXTURE;
}


void CTextureBrowser::Show()
{
	if (ImGui::Begin("Texture Browser", 0, ImGuiWindowFlags_NoDecoration))
	{
		ImVec2 contentRegion = ImGui::GetContentRegionAvail();
		
		// We want to take up as much space as we can with the buttons
		ImVec2 buttonSize = ImVec2(100, 100);

		int rowCount = std::floor(contentRegion.x / 100.0f);
		// If we're wider than we are tall, we want to same line all of our buttons


		for (int i = 0; i < m_textures.size(); i++)
		{
			texture_t texture = std::get<1>(m_textures.at(i));
			// Add the button
			if(ImGui::ImageButton((ImTextureID)texture, buttonSize, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0)))
				m_selectedTexture = texture;

			if (ImGui::IsItemHovered())
			{
				// We want to say the tool's name when we hover
				ImGui::BeginTooltip();
				ImGui::SetTooltip(std::get<0>(m_textures.at(i)).c_str());
				ImGui::EndTooltip();
			}


			if (rowCount > 0 && (i+1) % rowCount)
				ImGui::SameLine();

		}
	}
	ImGui::End();
}
