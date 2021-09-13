#include "actionmanager.h"
#include "shared.h"
#include "raytest.h"


CActionManager& GetActionManager()
{
	static CActionManager actionManager;
	
	return actionManager;
}


void CActionManager::CommitAction(IAction* action)
{
	bool success = action->Act();
	if (!success)
	{
		// Action failed! push it right into the can!
		delete action;
		return;
	}

	m_actionHistory.push_back(action);
	
	// Clear out the future we're overriding
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
