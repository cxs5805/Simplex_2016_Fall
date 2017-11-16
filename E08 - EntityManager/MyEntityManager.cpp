#include "MyEntityManager.h"
using namespace Simplex;
//  MyEntityManager
MyEntityManager* MyEntityManager::m_pInstance = nullptr;
void MyEntityManager::Init(void)
{
	m_uEntityCount = 0;
	m_entityList.clear();
}
void MyEntityManager::Release(void)
{
	for (uint i = 0; i < m_uEntityCount; i++)
	{
		SafeDelete(m_entityList[i]);
	}
	m_uEntityCount = 0;
	m_entityList.clear();
}
MyEntityManager* MyEntityManager::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new MyEntityManager();
	}
	return m_pInstance;
}
void MyEntityManager::ReleaseInstance()
{
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
int Simplex::MyEntityManager::GetEntityIndex(String a_sUniqueID)
{
	for (uint i = 0; i < m_uEntityCount; i++)
	{
		String id = m_entityList[i]->GetUniqueID();

		if (a_sUniqueID == id)
		{
			// if valid ID, this should be called only once
			return i;
		}
	}
	// if not found, return -1
	return -1;
}
//Accessors
Model* Simplex::MyEntityManager::GetModel(uint a_uIndex)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return nullptr;

	// check bounds
	if (a_uIndex >= m_uEntityCount)
		a_uIndex = m_uEntityCount - 1;

	return m_entityList[a_uIndex]->GetModel();
	/*
	for (uint i = 0; i < m_uEntityCount; i++)
	{
		if (i == a_uIndex)
		{
			return m_entityList[i]->GetModel();
		}
	}
	//*/

	return nullptr;
}
Model* Simplex::MyEntityManager::GetModel(String a_sUniqueID)
{
	MyEntity* entity;
	entity = MyEntity::GetEntity(a_sUniqueID);
	if (entity)
		return entity->GetModel();
	else
		return nullptr;
}
RigidBody* Simplex::MyEntityManager::GetRigidBody(uint a_uIndex)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return nullptr;

	// check bounds
	if (a_uIndex >= m_uEntityCount)
		a_uIndex = m_uEntityCount - 1;

	return m_entityList[a_uIndex]->GetRigidBody();
	return nullptr;
}
RigidBody* Simplex::MyEntityManager::GetRigidBody(String a_sUniqueID)
{
	MyEntity* entity;
	entity = MyEntity::GetEntity(a_sUniqueID);
	if (entity)
		return entity->GetRigidBody();
	else
		return nullptr;
}
matrix4 Simplex::MyEntityManager::GetModelMatrix(uint a_uIndex)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return IDENTITY_M4;

	// check bounds
	if (a_uIndex >= m_uEntityCount)
		a_uIndex = m_uEntityCount - 1;

	return m_entityList[a_uIndex]->GetModelMatrix();
	return IDENTITY_M4;
}
matrix4 Simplex::MyEntityManager::GetModelMatrix(String a_sUniqueID)
{
	MyEntity* entity;
	entity = MyEntity::GetEntity(a_sUniqueID);
	if (entity)
		return entity->GetModelMatrix();
	else
		return IDENTITY_M4;
}
void Simplex::MyEntityManager::SetModelMatrix(matrix4 a_m4ToWorld, String a_sUniqueID)
{
	MyEntity* entity = MyEntity::GetEntity(a_sUniqueID);
	if (entity)
		entity->SetModelMatrix(a_m4ToWorld);
}
void Simplex::MyEntityManager::SetModelMatrix(matrix4 a_m4ToWorld, uint a_uIndex)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return;

	// check bounds
	if (a_uIndex >= m_uEntityCount)
		a_uIndex = m_uEntityCount - 1;
	
	int index = a_uIndex;
	m_entityList[index]->SetModelMatrix(a_m4ToWorld);
}
//The big 3
MyEntityManager::MyEntityManager(){Init();}
MyEntityManager::MyEntityManager(MyEntityManager const& other){ }
MyEntityManager& MyEntityManager::operator=(MyEntityManager const& other) { return *this; }
MyEntityManager::~MyEntityManager(){Release();};
// other methods
void Simplex::MyEntityManager::Update(void)
{
	for (uint i = 0; i < m_uEntityCount; i++)
	{
		for (uint j = i + 1; j < m_uEntityCount; j++)
		{
			m_entityList[i]->IsColliding(m_entityList[j]);
		}
	}
}
void Simplex::MyEntityManager::AddEntity(String a_sFileName, String a_sUniqueID)
{
	// make sure id isn't already present
	for (uint i = 0; i < m_uEntityCount; i++)
	{
		if (a_sUniqueID == m_entityList[i]->GetUniqueID())
		{
			return;
		}
	}

	MyEntity* entityPtr = new MyEntity(a_sFileName, a_sUniqueID);
	if (entityPtr->IsInitialized())
	{
		m_entityList.push_back(entityPtr);
		
		// increment # entities
		m_uEntityCount++;
	}
}
void Simplex::MyEntityManager::RemoveEntity(uint a_uIndex)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return;

	// check bounds
	if (a_uIndex >= m_uEntityCount)
		a_uIndex = m_uEntityCount - 1;

	// move to back so we can pop it rather than use erase()
	if (a_uIndex != m_uEntityCount - 1)
		std::swap(m_entityList[a_uIndex], m_entityList[m_uEntityCount - 1]);

	MyEntity* entity = m_entityList[a_uIndex];
	SafeDelete(entity);
	m_entityList.pop_back();

	// decrement # entities
	m_uEntityCount--;
}
void Simplex::MyEntityManager::RemoveEntity(String a_sUniqueID)
{
	uint index = GetEntityIndex(a_sUniqueID);
	RemoveEntity(index);
}
String Simplex::MyEntityManager::GetUniqueID(uint a_uIndex)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return "";

	// check bounds
	if (a_uIndex >= m_uEntityCount)
		a_uIndex = m_uEntityCount - 1;

	return m_entityList[a_uIndex]->GetUniqueID();
}
MyEntity* Simplex::MyEntityManager::GetEntity(uint a_uIndex)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return nullptr;

	// check bounds
	if (a_uIndex >= m_uEntityCount)
		a_uIndex = m_uEntityCount - 1;

	return m_entityList[a_uIndex];
}
void Simplex::MyEntityManager::AddEntityToRenderList(uint a_uIndex, bool a_bRigidBody)
{
	// do nothing if nothing
	if (m_uEntityCount == 0) return;

	// check bounds
	if (a_uIndex >= m_uEntityCount)
	{
		for (a_uIndex = 0; a_uIndex < m_uEntityCount; a_uIndex++)
		{
			m_entityList[a_uIndex]->AddToRenderList(a_bRigidBody);
		}
	}
	else
	{
		m_entityList[a_uIndex]->AddToRenderList(a_bRigidBody);
	}
		
}
void Simplex::MyEntityManager::AddEntityToRenderList(String a_sUniqueID, bool a_bRigidBody)
{
	MyEntity* entity = MyEntity::GetEntity(a_sUniqueID);
	if (entity)
	{
		entity->AddToRenderList(a_bRigidBody);
	}
}
