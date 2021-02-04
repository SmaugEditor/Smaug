#include "texturebrowser.h"
#include "texturemanager.h"
#include <filesystem>
#include <string>
#include "imgui.h"

CTextureBrowser::CTextureBrowser()
{
	std::string path = "D:\\game\\baseq2\\textures\\e1u1";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string p = entry.path().string();
		bgfx::TextureHandle th = TextureManager().LoadTexture(p.c_str());
		m_textures.push_back({ p, th });
	}
}


void CTextureBrowser::Show()
{
	static bool open = true;
	if (open && ImGui::Begin("Texture Browser", &open))
	{
		ImVec2 contentRegion = ImGui::GetContentRegionAvail();
		
		// We want to take up as much space as we can with the buttons
		ImVec2 buttonSize = ImVec2(100, 100);

		int rowCount = floor(contentRegion.x / 100.0f);
		// If we're wider than we are tall, we want to same line all of our buttons


		for (int i = 0; i < m_textures.size(); i++)
		{
			// Add the button
			ImGui::ImageButton((ImTextureID)std::get<1>(m_textures.at(i)).idx, buttonSize, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));

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
		ImGui::End();
	}
}

CTextureBrowser& TextureBrowser()
{
	static CTextureBrowser s_tb;
	return s_tb;
}
