#include "actionmanager.h"
#include "smaugapp.h"
#include "utils.h"
#include "cursor.h"
#include "raytest.h"


CActionManager& GetActionManager()
{
	static CActionManager actionManager;
	
	return actionManager;
}


void CActionManager::CommitAction(IAction* action)
{
	action->Act();
	m_actionHistory.push_back(action);
	
	for (auto a : m_redoStack)
		delete a;
	m_redoStack.clear();
}

void CActionManager::Undo()
{
	if (m_actionHistory.size() == 0)
		return;

	IAction* a = m_actionHistory.back();
	m_actionHistory.pop_back();
	a->Undo();
	m_redoStack.push_back(a);
}

void CActionManager::Redo()
{
	if (m_redoStack.size() == 0)
		return;

	IAction* a = m_redoStack.back();
	m_redoStack.pop_back();
	a->Redo();
	m_actionHistory.push_back(a);
}

void CActionManager::Clear()
{
	for (auto a : m_actionHistory)
		delete a;
	for (auto a : m_redoStack)
		delete a;
	m_actionHistory.clear();
	m_redoStack.clear();
}

void CActionManager::Update()
{
	int redoTo = INT_MAX;
	int undoTo = INT_MAX;

	if (ImGui::Begin("Edit History"))
	{
		if (m_redoStack.size())
		{
			for (int i = 0; i < m_redoStack.size(); i++)
			{
				ImGui::Text(m_redoStack[i]->GetName());
				if (ImGui::IsItemClicked())
				{
					redoTo = i;
				}
			}
			ImGui::Separator();
		}
		for (int i = m_actionHistory.size() - 1; i >= 0; i--)
		{
			ImGui::Text(m_actionHistory[i]->GetName());
			if (ImGui::IsItemClicked())
			{
				undoTo = i;
			}
		}
	}
	ImGui::End();

	if(m_actionHistory.size())
		for (int i = m_actionHistory.size() - 1; i >= undoTo; i--)
		{
			Undo();
		}
	
	if (m_redoStack.size())
		//for (int i = 0; i < redoTo+1; i++)
		for (int i = m_redoStack.size() - 1; i >= redoTo; i--)
		{
			Redo();
		}


	static bool keyUp = true;
	if (Input().IsDown({ GLFW_KEY_LEFT_CONTROL, false }) && Input().IsDown({ GLFW_KEY_Z, false }))
	{
		if(keyUp)
			Undo();
		keyUp = false;
	}
	else if (Input().IsDown({ GLFW_KEY_LEFT_CONTROL, false }) && Input().IsDown({ GLFW_KEY_Y, false }))
	{
		if (keyUp)
			Redo();
		keyUp = false;
	}
	else
	{
		keyUp = true;
	}
}
