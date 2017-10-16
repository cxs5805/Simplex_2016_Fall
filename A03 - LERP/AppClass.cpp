#include "AppClass.h"
void Application::InitVariables(void)
{
	////Change this to your name and email
	m_sProgrammer = "Chris Schiff - cxs5805@g.rit.edu";

	////Alberto needed this at this position for software recording.
	//m_pWindow->setPosition(sf::Vector2i(710, 0));

	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}

	//if there are no segments create 7
	if (m_uOrbits < 1)
		m_uOrbits = 7;

	float fSize = 1.0f; //initial size of orbits

	// MY CODE STARTS HERE (SOME OF IT IS MIXED IN WITH THE STARTER CODE)
	// -------------------------


	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
					   //prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1 ? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
	/*
	This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
	*/
	uint uSides = 3; //start with the minimal 3 sides
	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager

		// generate angle of increment for current orbit based on # of sides
		float angle = 2 * glm::pi<float>() / i;
		float tempAngle = angle;
		vector2 tempPoint(fSize, 0.0f);

		// generate a circle of waypoints, then add to waypoints list
		std::vector<vector3> tempWaypoints;
		for (uint j = 0; j < i; j++)
		{
			// PLEASE REPLACE THIS SOON
			// just a temporary solution to see what starting point is
			//tempWaypoints.push_back(vector3(fSize, 0.0f, 0.0f));

			// actual implementation of waypoints
			tempWaypoints.push_back(vector3(tempPoint, 0.0f));

			// generate new waypoint based on angle, and increment angle temp
			tempPoint = fSize * vector2(glm::cos(tempAngle), glm::sin(tempAngle));
			tempAngle += angle;

		}
		waypoints.push_back(tempWaypoints);

		// set the starting point to zero,
		startPoints.push_back(0);

		// next point to one,
		nextPoints.push_back(1);

		// and last point to last index
		lastPoints.push_back(i - 1);

		// DEBUG
		//std::cout << "waypoints.size() = " << waypoints.size() << "\nversus i  = " << i << "\n";
		//std::cout << "fsize = " << fSize << "\n";

		fSize += 0.5f; //increment the size for the next orbit
		uColor -= static_cast<uint>(decrements); //decrease the wavelength
	}

	// set all current positions to respective starting points
	uint numWaypoints = waypoints.size();
	for (uint i = 0; i < numWaypoints; i++)
	{
		//currPositions[i] = waypoints[i][0];
		currPositions.push_back(waypoints[i][0]);
	}

	// DEBUG
	//std::cout << "size = " << waypoints.size() << "\n";
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

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
	The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	m4Offset = glm::rotate(IDENTITY_M4, 90.0f, AXIS_Z);

	// current time in interval
	static float timer = 0;

	// generate a clock to help keep track of current time
	static uint clock = m_pSystem->GenClock();

	// update time each frame
	timer += m_pSystem->GetDeltaTime(clock);

	// keep track of the duration of a given journey from point A to point B
	float duration = 0.5f;

	// map the time interval of the LERP from 0 to 1
	// so that we have an easy way of accessing current
	// percentage of interval travelled
	float percent = MapValue(timer,
		0.0f, duration,
		0.0f, 1.0f);

	// interval finished
	if (percent > 1.0f)
	{
		// reset timer
		clock = m_pSystem->GenClock();
		timer = 0;
		// reset percent so that the sphere doesn't
		// immediately jump to the next point for 1 frame
		percent = 0.0f;

		uint size = waypoints.size();
		for (uint i = 0; i < size; i++)
		{
			// move to the next point for each orbit
			startPoints[i] = nextPoints[i];
			// SPECIAL CASE: if next point is last point, wrap back to first point
			if (nextPoints[i] == lastPoints[i])
			{
				nextPoints[i] = 0;
			}
			// DEFAULT CASE: increment next point
			else
			{
				nextPoints[i]++;
			}

			// DEBUG
			//if (i == 0)
			//{
			//	std::cout << "(" << currPositions[i].x <<
			//		", " << currPositions[i].y << ", " <<
			//		currPositions[i].z << ")\n";
			//}
		}

	}

	// DEBUG
	//std::cout << "time = " << timer << "\n";
	//std::cout << "% = " << percent << "\n";
	//std::cout << "start= " << startPoints[0] << "\nnext = " << nextPoints[0] << "\n";

	// draw a shapes
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 90.0f, AXIS_X));

		//calculate the current position
		//vector3 v3CurrentPos = ZERO_V3;
		// test
		//v3CurrentPos = vector3(1, 0, 0);

		// test first waypoint
		//v3CurrentPos = waypoints[i][0];

		// test subsequent point
		//v3CurrentPos = waypoints[i][1];

		// test final point
		//v3CurrentPos = waypoints[i][lastPoints[i]];

		// test hard-coded LERP from index 0 to index 1
		//v3CurrentPos = glm::lerp(waypoints[i][0], waypoints[i][1], percent);

		// the important part: LERP from current to next
		currPositions[i] = glm::lerp(
			waypoints[i][startPoints[i]],
			waypoints[i][nextPoints[i]],
			percent);

		matrix4 m4Model = glm::translate(m4Offset, currPositions[i]);

		// DEBUG
		//if (i == 0)
		//{
		//	std::cout << "(" << v3CurrentPos.x <<
		//		", " << v3CurrentPos.y << ", " <<
		//		v3CurrentPos.z << ")\n";
		//}
		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

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
	//release GUI
	ShutdownGUI();
}