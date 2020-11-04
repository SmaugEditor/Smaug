#include "svar.h"
#include "KeyValue.h"

char* CSVarTable::ToString()
{
	KeyValueRoot kv;
	AddToKV(&kv);
	return kv.ToString();
}

void CSVarTable::FromString(char* str)
{
	KeyValueRoot kvr(str);
	FromKV(&kvr);
}

void CSVarTable::AddToKV(KeyValue* kv)
{
	if (!kv)
		return;

	if (m_tableName)
	{
		kv = kv->AddNode(m_tableName);
	}

	for (int i = 0; i < m_varTable.size(); i++)
	{
		ISVar* var = m_varTable[i];
		char* value = var->ToString();
		kv->Add(var->GetName(), value);
		delete[] value;
	}
}

void CSVarTable::FromKV(KeyValue* kv)
{
	if (m_tableName)
	{
		KeyValue& kvref = kv->Get(m_tableName);
		if (kvref.IsValid())
			kv = &kvref;
	}

	for (int i = 0; i < m_varTable.size(); i++)
	{
		KeyValue& kvref = kv->Get(m_varTable[i]->GetName());
		if (kvref.IsValid())
			m_varTable[i]->FromString(kvref.value.string);
	}
}
