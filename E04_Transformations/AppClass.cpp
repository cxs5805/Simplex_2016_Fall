#include "AppClass.h"
void Application::InitVariables(void)
{
	////Change this to your name and email
	//m_sProgrammer = "Alberto Bobadilla - labigm@rit.edu";

	////Alberto needed this at this position for software recording.
	//m_pWindow->setPosition(sf::Vector2i(710, 0));

	//Make MyMesh object
	m_pMesh = new MyMesh();
	//m_pMesh->AddTri(vector3(0.03f, 0.47f, 1.0f), vector3(2, 2, 2), vector3(6, 6, 6));
	m_pMesh->GenerateCube(2.0f, C_BROWN);
	// Adding information about color
	m_pMesh->CompleteMesh(C_RED);
	m_pMesh->CompileOpenGL3X();

	//Make MyMesh object
	m_pMesh1 = new MyMesh();
	m_pMesh1->GenerateCube(1.0f, C_WHITE);

	// E04 new stuff
	// generate each cube manually
	// really processor intensive, but it just works
	// pointer array of 46 cubes
	cubes = new MyMesh[NUM_CUBES];

	std::vector<vector3> positions =
	{
		vector3 (2.0f, 7.0f, 0.0f),
		vector3 (8.0f, 7.0f, 0.0f),
		
		vector3 (3.0f, 6.0f, 0.0f),
		vector3 (7.0f, 6.0f, 0.0f),
		
		vector3 (2.0f, 5.0f, 0.0f),
		vector3 (3.0f, 5.0f, 0.0f),
		vector3 (4.0f, 5.0f, 0.0f),
		vector3 (5.0f, 5.0f, 0.0f),
		vector3 (6.0f, 5.0f, 0.0f),
		vector3 (7.0f, 5.0f, 0.0f),
		vector3 (8.0f, 5.0f, 0.0f),

		vector3 (1.0f, 4.0f, 0.0f),
		vector3 (2.0f, 4.0f, 0.0f),
		vector3 (4.0f, 4.0f, 0.0f),
		vector3 (5.0f, 4.0f, 0.0f),
		vector3 (6.0f, 4.0f, 0.0f),
		vector3 (8.0f, 4.0f, 0.0f),
		vector3 (9.0f, 4.0f, 0.0f),
		
		vector3 (0.0f, 3.0f, 0.0f),
		vector3 (1.0f, 3.0f, 0.0f),
		vector3 (2.0f, 3.0f, 0.0f),
		vector3 (3.0f, 3.0f, 0.0f),
		vector3 (4.0f, 3.0f, 0.0f),
		vector3 (5.0f, 3.0f, 0.0f),
		vector3 (6.0f, 3.0f, 0.0f),
		vector3 (7.0f, 3.0f, 0.0f),
		vector3 (8.0f, 3.0f, 0.0f),
		vector3 (9.0f, 3.0f, 0.0f),
		vector3 (10.0f, 3.0f, 0.0f),

		vector3 (0.0f, 2.0f, 0.0f),
		vector3 (2.0f, 2.0f, 0.0f),
		vector3 (3.0f, 2.0f, 0.0f),
		vector3 (4.0f, 2.0f, 0.0f),
		vector3 (5.0f, 2.0f, 0.0f),
		vector3 (6.0f, 2.0f, 0.0f),
		vector3 (7.0f, 2.0f, 0.0f),
		vector3 (8.0f, 2.0f, 0.0f),
		vector3 (10.0f, 2.0f, 0.0f),

		vector3 (0.0f, 1.0f, 0.0f),
		vector3 (2.0f, 1.0f, 0.0f),
		vector3 (8.0f, 1.0f, 0.0f),
		vector3 (10.0f, 1.0f, 0.0f),

		vector3 (3.0f, 0.0f, 0.0f),
		vector3 (4.0f, 0.0f, 0.0f),
		vector3 (6.0f, 0.0f, 0.0f),
		vector3 (7.0f, 0.0f, 0.0f)/*,
		vector3 (.0f, 0.0f, 0.0f),
		vector3 (.0f, 0.0f, 0.0f)*/
	};


	for (int i = 0; i < NUM_CUBES; i++)
	{
		// generate each cube
		cubes[i].GenerateCube(1.0f, C_BLACK);

		// position each cube accordingly
		matrices.push_back(matrix4());
		matrices[i] = glm::translate(matrices[i], positions[i]);

		std::cout << "position #" << i << ": (" << positions[i].x << ", " << positions[i].y << ", " << positions[i].z << ")\n";
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	//static matrix4 m4Model;//ToMatrix4(m_qArcBall);

	// aahhhhhhhhhhh processor-dependent operations
	//m4Model = glm::rotate(m4Model, 1.0f, vector3(0.0f, 0.0f, 1.0f));//ToMatrix4(m_qArcBall);
	//m4Model = glm::scale(m4Model, vector3(1.1f, 1.0f, 1.0f));//ToMatrix4(m_qArcBall);
	//m4Model = glm::translate(m4Model, vector3(0.1f, 0.0f, 0.0f));//ToMatrix4(m_qArcBall);

	// remember, order matters in matrix multiplication
	//m_pMesh->Render(m4Projection, m4View, m4Model);
	//m_pMesh1->Render(m_pCameraMngr->GetProjectionMatrix(), m_pCameraMngr->GetViewMatrix(), glm::translate(vector3( 3.0f, 0.0f, 0.0f)));
	
	// render each of the invader cubes
	for (int i = 0; i < NUM_CUBES; i++)
	{
		matrices[i] = glm::translate(matrices[i], vector3(0.1f, 0.0f, 0.0f));
		cubes[i].Render(m4Projection, m4View, matrices[i]);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	if (cubes != nullptr)
	{
		delete[] cubes;
		cubes = nullptr;
	}

	if (m_pMesh != nullptr)
	{
		delete m_pMesh;
		m_pMesh = nullptr;
	}
	SafeDelete(m_pMesh1);
	//release GUI
	ShutdownGUI();
}