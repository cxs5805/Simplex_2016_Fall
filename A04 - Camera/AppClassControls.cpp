#include "AppClass.h"
using namespace Simplex;
//Mouse
void Application::ProcessMouseMovement(sf::Event a_event)
{
	//get global mouse position
	sf::Vector2i mouse = sf::Mouse::getPosition();
	sf::Vector2i window = m_pWindow->getPosition();
	m_v3Mouse.x = static_cast<float>(mouse.x - window.x);
	m_v3Mouse.y = static_cast<float>(mouse.y - window.y);
	if(!m_pSystem->IsWindowFullscreen() && !m_pSystem->IsWindowBorderless())
		m_v3Mouse += vector3(-8.0f, -32.0f, 0.0f);
	gui.io.MousePos = ImVec2(m_v3Mouse.x, m_v3Mouse.y);
}
void Application::ProcessMousePressed(sf::Event a_event)
{
	switch (a_event.mouseButton.button)
	{
	default: break;
	case sf::Mouse::Button::Left:
		gui.m_bMousePressed[0] = true;
		break;
	case sf::Mouse::Button::Middle:
		gui.m_bMousePressed[1] = true;
		m_bArcBall = true;
		break;
	case sf::Mouse::Button::Right:
		gui.m_bMousePressed[2] = true;
		m_bFPC = true;
		break;
	}

	for (int i = 0; i < 3; i++)
		gui.io.MouseDown[i] = gui.m_bMousePressed[i];
}
void Application::ProcessMouseReleased(sf::Event a_event)
{
	switch (a_event.mouseButton.button)
	{
	default: break;
	case sf::Mouse::Button::Left:
		gui.m_bMousePressed[0] = false;
		break;
	case sf::Mouse::Button::Middle:
		gui.m_bMousePressed[1] = false;
		m_bArcBall = false;
		break;
	case sf::Mouse::Button::Right:
		gui.m_bMousePressed[2] = false;
		m_bFPC = false;
		break;
	}

	for (int i = 0; i < 3; i++)
		gui.io.MouseDown[i] = gui.m_bMousePressed[i];
}
void Application::ProcessMouseScroll(sf::Event a_event)
{
	gui.io.MouseWheel = a_event.mouseWheelScroll.delta;
	float fSpeed = a_event.mouseWheelScroll.delta;
	float fMultiplier = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
		sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

	if (fMultiplier)
		fSpeed *= 2.0f;
}
//Keyboard
void Application::ProcessKeyPressed(sf::Event a_event)
{
	switch (a_event.key.code)
	{
	default: break;
	case sf::Keyboard::Space:
		break;
	}
	//gui
	gui.io.KeysDown[a_event.key.code] = true;
	gui.io.KeyCtrl = a_event.key.control;
	gui.io.KeyShift = a_event.key.shift;
}
void Application::ProcessKeyReleased(sf::Event a_event)
{
	static bool bFPSControl = false;

	switch (a_event.key.code)
	{
	default: break;
	case sf::Keyboard::Escape:
		m_bRunning = false;
		break;
	case sf::Keyboard::F1:
		m_pCamera->SetPerspective();
		m_pCamera->CalculateProjectionMatrix();
		break;
	case sf::Keyboard::F2:
		m_pCamera->SetPerspective(false);
		m_pCamera->CalculateProjectionMatrix();
		break;
	case sf::Keyboard::Add:
		++m_uActCont;
		m_uActCont %= 8;
		if (m_uControllerCount > 0)
		{
			while (m_pController[m_uActCont]->uModel == SimplexController_NONE)
			{
				++m_uActCont;
				m_uActCont %= 8;
			}
		}
		break;
	case sf::Keyboard::Subtract:
		--m_uActCont;
		if (m_uActCont > 7)
			m_uActCont = 7;
		if (m_uControllerCount > 0)
		{
			while (m_pController[m_uActCont]->uModel == SimplexController_NONE)
			{
				--m_uActCont;
				if (m_uActCont > 7)
					m_uActCont = 7;
			}
		}
		break;
	}

	//gui
	gui.io.KeysDown[a_event.key.code] = false;
	gui.io.KeyCtrl = a_event.key.control;
	gui.io.KeyShift = a_event.key.shift;
}
//Joystick
void Application::ProcessJoystickConnected(uint nController)
{
	// if the index of the controller is larger than 7
	if (nController > 7)
		return;

	bool bConnected = sf::Joystick::isConnected(nController);
	if (bConnected)
	{
		SafeDelete(m_pController[nController]);
		sf::Joystick::Identification joyID = sf::Joystick::getIdentification(nController);
		String productName = joyID.name.toAnsiString();
		int nVendorID = joyID.vendorId;
		int nProductID = joyID.productId;
		m_pController[nController] = new ControllerInput(nVendorID, nProductID);
		//m_pWindow->setJoystickThreshold(7);
		++m_uControllerCount;
	}
}
void Application::ProcessJoystickPressed(sf::Event a_event)
{
	//Check the ID of the controller
	int nID = a_event.joystickButton.joystickId;

	//Identify the button pressed
	int nButton = a_event.joystickButton.button;
	//map the value to our control value
	m_pController[nID]->button[m_pController[nID]->mapButton[nButton]] = true;

	//Process...

	//If we press L3 + R3 on the active controller we quit the application
	if (m_pController[m_uActCont]->button[SimplexKey_L3] && m_pController[m_uActCont]->button[SimplexKey_R3])
		m_bRunning = false;
}
void Application::ProcessJoystickReleased(sf::Event a_event)
{
	//Check the ID of the controller
	int nID = a_event.joystickButton.joystickId;

	//Identify the button pressed
	int nButton = a_event.joystickButton.button;
	//map the value to our control value
	m_pController[nID]->button[m_pController[nID]->mapButton[nButton]] = false;

	//Process...

	//if we released the Pad key on the active controller quit the application
	if (m_pController[m_uActCont]->mapButton[nButton] == SimplexKey_Pad)
		m_bRunning = false;
}
void Application::ProcessJoystickMoved(sf::Event a_event)
{
	int nID = a_event.joystickMove.joystickId;
	float fPosition = a_event.joystickMove.position;
	int nAxis = a_event.joystickMove.axis;

	//invert vertical axis for sticks
	if (nAxis == sf::Joystick::Axis::Y || nAxis == sf::Joystick::Axis::R)
	{
		fPosition *= -1.0f;
	}

	//Adjust with threshold
	float fThreshold = 10.0f;
	if (nAxis != sf::Joystick::Axis::PovX || nAxis != sf::Joystick::Axis::PovY)
	{
		if (fPosition > -fThreshold && fPosition < fThreshold)
		{
			fPosition = 0.0f;
		}
	}

	/*
	SFML does not use XInput so XBOX one controller is a special snowflake,
	LTrigger and RTrigger share the same axis (sf::Joystick::Axis::Z)
	instead of separate ones as	in other controllers:
	LTrigger reports from 0 to 100 and RTrigger reports from 0 to -100
	I mapped the values from 0 to 100 in both cases to keep consistent
	with other controllers, but sharing the axis means that triggers cannot
	be independent from each other
	*/
	//PS4 Controller
	if (m_pController[nID]->uModel == SimplexController_DualShock4)
	{
		//For Axis U or V
		if (nAxis == sf::Joystick::Axis::U || nAxis == sf::Joystick::Axis::V)
		{
			fPosition = MapValue(fPosition, -100.0f, 100.0f, 0.0f, 100.0f);
		}
		m_pController[nID]->axis[m_pController[nID]->mapAxis[nAxis]] = fPosition;
	}
	//Nintendo Switch Pro
	else if (m_pController[nID]->uModel == SimplexController_SwitchPro)
	{
		if (nAxis != sf::Joystick::Axis::PovX && nAxis != sf::Joystick::Axis::PovY)
		{
			fPosition = MapValue(fPosition, -75.0f, 75.0f, -100.0f, 100.0f);
			if (fPosition > 100) fPosition = 100;
			if (fPosition < -100) fPosition = -100;
			if (fPosition > -10 && fPosition < 10) fPosition = 0;
		}
		m_pController[nID]->axis[m_pController[nID]->mapAxis[nAxis]] = fPosition;
	}
	//Microsoft controllers
	else if (m_pController[nID]->uModel == SimplexController_XBONE || m_pController[nID]->uModel == SimplexController_360)
	{
		//Axis different than Z
		if (nAxis != sf::Joystick::Axis::Z)
		{
			m_pController[nID]->axis[m_pController[nID]->mapAxis[nAxis]] = fPosition;
		}
		else //Z axis
		{
			//get the uncleaned value
			fPosition = a_event.joystickMove.position;
			//positive values go on the left trigger
			if (fPosition >= 0)
			{
				if (fPosition < 25.0f)
					fPosition = 0.0f;
				m_pController[nID]->axis[SimplexAxis_L] = fPosition;
			}
			//negative values go on the right trigger
			else
			{
				if (fPosition > -25.0f)
					fPosition = 0.0f;
				m_pController[nID]->axis[SimplexAxis_R] = -fPosition;
			}
		}
	}
	//8Bitdo NES30 PRO
	else if (m_pController[nID]->uModel == SimplexController_NES30PRO)
	{
		m_pController[nID]->axis[m_pController[nID]->mapAxis[nAxis]] = fPosition;
	}
	//Other controllers
	else
	{
		m_pController[nID]->axis[m_pController[nID]->mapAxis[nAxis]] = fPosition;
	}
}
/*
Continuous update (once per frame) for discreet input use
process events.
*/
//Mouse
void Application::ArcBall(float a_fSensitivity)
{
	//If the arcball is not enabled return
	if (!m_bArcBall)
		return;

	//static quaternion qArcBall;
	UINT	MouseX, MouseY;		// Coordinates for the mouse
	UINT	CenterX, CenterY;	// Coordinates for the center of the screen.

								//Initialize the position of the pointer to the middle of the screen
	CenterX = m_pSystem->GetWindowX() + m_pSystem->GetWindowWidth() / 2;
	CenterY = m_pSystem->GetWindowY() + m_pSystem->GetWindowHeight() / 2;

	//Calculate the position of the mouse and store it
	POINT pt;
	GetCursorPos(&pt);
	MouseX = pt.x;
	MouseY = pt.y;

	//Calculate the difference in position and update the quaternion orientation based on it
	float DeltaMouse;
	if (MouseX < CenterX)
	{
		DeltaMouse = static_cast<float>(CenterX - MouseX);
		m_qArcBall = quaternion(vector3(0.0f, glm::radians(a_fSensitivity * DeltaMouse), 0.0f)) * m_qArcBall;
	}
	else if (MouseX > CenterX)
	{
		DeltaMouse = static_cast<float>(MouseX - CenterX);
		m_qArcBall = quaternion(vector3(0.0f, glm::radians(-a_fSensitivity * DeltaMouse), 0.0f)) * m_qArcBall;
	}

	if (MouseY < CenterY)
	{
		DeltaMouse = static_cast<float>(CenterY - MouseY);
		m_qArcBall = quaternion(vector3(glm::radians(-a_fSensitivity * DeltaMouse), 0.0f, 0.0f)) * m_qArcBall;
	}
	else if (MouseY > CenterY)
	{
		DeltaMouse = static_cast<float>(MouseY - CenterY);
		m_qArcBall = quaternion(vector3(glm::radians(a_fSensitivity * DeltaMouse), 0.0f, 0.0f)) * m_qArcBall;
	}

	SetCursorPos(CenterX, CenterY);//Position the mouse in the center
								   //return qArcBall; // return the new quaternion orientation
}
void Application::CameraRotation(float a_fSpeed)
{
	if (m_bFPC == false)
		return;

	UINT	MouseX, MouseY;		// Coordinates for the mouse
	UINT	CenterX, CenterY;	// Coordinates for the center of the screen.

								//Initialize the position of the pointer to the middle of the screen
	CenterX = m_pSystem->GetWindowX() + m_pSystem->GetWindowWidth() / 2;
	CenterY = m_pSystem->GetWindowY() + m_pSystem->GetWindowHeight() / 2;

	//Calculate the position of the mouse and store it
	POINT pt;
	GetCursorPos(&pt);
	MouseX = pt.x;
	MouseY = pt.y;

	//Calculate the difference in view with the angle
	float fAngleX = 0.0f;
	float fAngleY = 0.0f;
	float fDeltaMouse = 0.0f;
	if (MouseX < CenterX)
	{
		fDeltaMouse = static_cast<float>(CenterX - MouseX);
		fAngleY += fDeltaMouse * a_fSpeed;
	}
	else if (MouseX > CenterX)
	{
		fDeltaMouse = static_cast<float>(MouseX - CenterX);
		fAngleY -= fDeltaMouse * a_fSpeed;
	}

	if (MouseY < CenterY)
	{
		fDeltaMouse = static_cast<float>(CenterY - MouseY);
		fAngleX -= fDeltaMouse * a_fSpeed;
	}
	else if (MouseY > CenterY)
	{
		fDeltaMouse = static_cast<float>(MouseY - CenterY);
		fAngleX += fDeltaMouse * a_fSpeed;
	}
	//Change the Yaw and the Pitch of the camera

	vector3 v3ForwardTemp = m_pCamera->GetMyForward();
	vector3 v3RightTemp = m_pCamera->GetMyRight();
	vector3 v3UpTemp = m_pCamera->GetMyUp();

	vector3 v3TempAngles = m_pCamera->GetOrientationAngles();
	
	// get angle in radians each frame
	v3TempAngles.x += fAngleX * (PI / 180.0f);
	v3TempAngles.y -= fAngleY * (PI / 180.0f);

	// wrap around for x and y
	if (v3TempAngles.x >= 2 * PI)
	{
		float fDifference = v3TempAngles.x - (2 * PI);
		v3TempAngles.x = 0.0f + fDifference;
	}
	else if (v3TempAngles.x <= 0)
	{
		float fDifference = 0 - v3TempAngles.x;
		v3TempAngles.x = (2 * PI) - fDifference;
	}

	if (v3TempAngles.y >= 2 * PI)
	{
		float fDifference = v3TempAngles.y - (2 * PI);
		v3TempAngles.y = 0.0f + fDifference;
	}
	else if (v3TempAngles.y <= 0)
	{
		float fDifference = 0 - v3TempAngles.y;
		v3TempAngles.y = (2 * PI) - fDifference;
	}

	m_pCamera->SetOrientationAngles(v3TempAngles);

	// then recalculate forward
	// rotate forward vector first about X, then about Y
	//v3ForwardTemp = glm::normalize(vector3(glm::sin(v3TempAngles.y), v3ForwardTemp.y, -glm::cos(v3TempAngles.y)));
	v3ForwardTemp = glm::normalize(vector3(v3ForwardTemp.x, -glm::sin(v3TempAngles.x), -glm::cos(v3TempAngles.x)));
	v3ForwardTemp = glm::normalize(vector3(glm::sin(v3TempAngles.y), v3ForwardTemp.y, -glm::cos(v3TempAngles.y)));
	
	// and recalculate right
	// since not rotating about Z, we only need to rotate the right vector about Y
	v3RightTemp = glm::normalize(vector3(glm::cos(v3TempAngles.y), v3RightTemp.y, glm::sin(v3TempAngles.y)));
	
	// DEBUG
	//std::cout << "(" << v3ForwardTemp.x << ", " << v3ForwardTemp.y << ", " << v3ForwardTemp.z << ")\n";
	//std::cout << "(" << v3RightTemp.x << ", " << v3RightTemp.y << ", " << v3RightTemp.z << ")\n";
	//std::cout << "(" << v3TempAngles.x << ", " << v3TempAngles.y << ", " << v3TempAngles.z << ")\n";

	// reassign forward and right and up
	m_pCamera->SetMyForward(v3ForwardTemp);
	m_pCamera->SetMyRight(v3RightTemp);
	//m_pCamera->SetMyUp(v3UpTemp);
	m_pCamera->SetMyUp();
	
	// set camera's orientation based on angle
	m_pCamera->SetOrientation(m_pCamera->GetOrientation() * glm::angleAxis(fAngleX, v3RightTemp));
	// for some reason I don't understand, the Y-axis angle gets doubled
	// i.e. if the camera visually rotates 360 degrees, forward vector becomes (0, 0, 1), which is literally backwards
	m_pCamera->SetOrientation(m_pCamera->GetOrientation() * 0.5f * glm::angleAxis(-fAngleY, v3UpTemp));

	SetCursorPos(CenterX, CenterY);//Position the mouse in the center
}
//Keyboard
void Application::ProcessKeyboard(void)
{
	/*
	This is used for things that are continuously happening,
	for discreet on/off use ProcessKeyboardPressed/Released
	*/

	// reset camera
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
	{
		m_pCamera->ResetCamera();
	}

#pragma region Camera Position
	float fSpeed = 0.1f;
	float fMultiplier = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
		sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

	if (fMultiplier)
		fSpeed *= 5.0f;

	// test everything in global space just to see what's really going on
	// first things first, what happens when view matrix gets updated every frame (already happens)

	// next, let's just move the camera in X and Z
	vector3 v3Temp;

	// move forwards
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		//v3Temp += fSpeed * vector3(0.0f, 0.0f, -1.0f);
		v3Temp += fSpeed * m_pCamera->GetMyForward();
	}

	// move backwards
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		//v3Temp += fSpeed * vector3(0.0f, 0.0f, 1.0f);
		v3Temp -= fSpeed * m_pCamera->GetMyForward();
	}

	// move left
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		//v3Temp += fSpeed * vector3(-1.0f, 0.0f, 0.0f);
		v3Temp -= fSpeed * m_pCamera->GetMyRight();
	}

	// move right
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		//v3Temp += fSpeed * vector3(1.0f, 0.0f, 0.0f);
		v3Temp += fSpeed * m_pCamera->GetMyRight();
	}

	// move in global up
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		//v3Temp += fSpeed * m_pCamera->GetMyUp();
		v3Temp += fSpeed * m_pCamera->GetUp();
	}

	// move in global down
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		//v3Temp -= fSpeed * m_pCamera->GetMyUp();
		v3Temp -= fSpeed * m_pCamera->GetUp();
	}

	// now, update position!
	vector3 v3OldPosition = m_pCamera->GetPosition();
	m_pCamera->SetPosition(v3OldPosition + v3Temp);

	// on every frame, set target to position + my forward
	//vector3 v3NewTarget = m_pCamera->GetPosition() + m_pCamera->GetMyForward();
	//m_pCamera->SetTarget(v3NewTarget);
#pragma endregion
	/*
#pragma region Camera Orientation
	// angle in DEGREES
	float fAngle = 1.0f;

	// hold shift to change direction
	// won't be needed after it's done via mouse controls
	if (fMultiplier)
		fAngle = -fAngle;
	
	// angle in RADIANS
	float fAngleRad = fAngle * (PI / 180.0f);
	//fAngleRad = 0.017453f;
	//if (fMultiplier)
	//	fAngleRad = -fAngleRad;

	// future me, what are we going to rotate about?
	// as of Friday it's about global X and global Y,
	// but that doesn't seem to be what we want

	vector3 v3ForwardTemp = m_pCamera->GetMyForward();
	vector3 v3RightTemp = m_pCamera->GetMyRight();
	vector3 v3UpTemp = m_pCamera->GetMyUp();

	// rotate X
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
	{
		std::cout << "rotate X\n";

		// get the rotation
		vector3 axis = AXIS_X;
		axis = m_pCamera->GetMyRight();
		quaternion qTemp = glm::angleAxis(fAngle, axis);

		// multiply it by existing orientation
		m_pCamera->SetOrientation(m_pCamera->GetOrientation() * qTemp);
		
		// add angle (in radians) to existing rotation vector
		vector3 v3TempAngles = m_pCamera->GetOrientationAngles();
		v3TempAngles.x += fAngleRad;
		// wrap around (if needed)
		if (v3TempAngles.x >= 2 * PI)
		{
			float fDifference = v3TempAngles.x - (2 * PI);
			v3TempAngles.x = 0.0f + fDifference;
		}
		else if (v3TempAngles.x <= 0)
		{
			float fDifference = 0 - v3TempAngles.x;
			v3TempAngles.x = (2 * PI) - fDifference;
		}

		m_pCamera->SetOrientationAngles(v3TempAngles);
		std::cout << "(" << v3TempAngles.x << ", " << v3TempAngles.y << ", " << v3TempAngles.z << ")\n";

		// then recalculate forward
		//vector3 v3ForwardTemp = m_pCamera->GetMyForward();
		//v3ForwardTemp = glm::normalize(vector3(glm::sin(v3TempAngles.y), 0, -glm::cos(v3TempAngles.y)));
		v3ForwardTemp = glm::normalize(vector3(v3ForwardTemp.x, -glm::sin(v3TempAngles.x), -glm::cos(v3TempAngles.x)));
		//std::cout << "(" << v3ForwardTemp.x << ", " << v3ForwardTemp.y << ", " << v3ForwardTemp.z << ")\n";

		// and recalculate up
		//vector3 v3RightTemp = m_pCamera->GetMyRight();
		//v3RightTemp = glm::normalize(vector3(glm::cos(v3TempAngles.y), 0, glm::sin(v3TempAngles.y)));
		v3UpTemp = glm::normalize(vector3(v3UpTemp.x, glm::cos(v3TempAngles.y), -glm::sin(v3TempAngles.y)));
		//std::cout << "(" << v3RightTemp.x << ", " << v3RightTemp.y << ", " << v3RightTemp.z << ")\n";
	}

	// rotate Y
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
	{
		m_pCamera->counter++;
		std::cout << "\n-\nrotate Y\n" <<
			"angle = " << fAngle <<
			"\nradians = " << fAngleRad <<
			"\nrad*360 = " << fAngleRad * 360 <<
			"\n2PI = " << 2 * PI <<
			"\ncounter = " << m_pCamera->counter << "\n";

		// get the rotation
		vector3 axis = AXIS_Y;
		axis = m_pCamera->GetMyUp();
		quaternion qTemp = glm::angleAxis(fAngle, axis);

		// multiply it by existing orientation
		m_pCamera->SetOrientation(m_pCamera->GetOrientation() * qTemp);

		// add angle (in radians) to existing rotation vector
		vector3 v3TempAngles = m_pCamera->GetOrientationAngles();
		v3TempAngles.y += fAngleRad;
		// wrap around (if needed)
		if (v3TempAngles.y >= 2 * PI)
		{
			float fDifference = v3TempAngles.y - (2 * PI);
			v3TempAngles.y = 0.0f + fDifference;
		}
		else if (v3TempAngles.y <= 0)
		{
			float fDifference = 0 - v3TempAngles.y;
			v3TempAngles.y = (2 * PI) - fDifference;
		}

		m_pCamera->SetOrientationAngles(v3TempAngles);
		std::cout << "(" << v3TempAngles.x << ", " << v3TempAngles.y << ", " << v3TempAngles.z << ")\n";

		// then recalculate forward
		//vector3 v3ForwardTemp = m_pCamera->GetMyForward();
		//v3ForwardTemp = glm::normalize(vector3(glm::sin(v3TempAngles.y), 0, -glm::cos(v3TempAngles.y)));
		v3ForwardTemp = glm::normalize(vector3(glm::sin(v3TempAngles.y), v3ForwardTemp.y, -glm::cos(v3TempAngles.y)));
		//std::cout << "(" << v3ForwardTemp.x << ", " << v3ForwardTemp.y << ", " << v3ForwardTemp.z << ")\n";

		// and recalculate right
		//vector3 v3RightTemp = m_pCamera->GetMyRight();
		//v3RightTemp = glm::normalize(vector3(glm::cos(v3TempAngles.y), 0, glm::sin(v3TempAngles.y)));
		v3RightTemp = glm::normalize(vector3(glm::cos(v3TempAngles.y), v3RightTemp.y, glm::sin(v3TempAngles.y)));
		//std::cout << "(" << v3RightTemp.x << ", " << v3RightTemp.y << ", " << v3RightTemp.z << ")\n";
		

		// reassign forward and right
		//m_pCamera->SetMyForward(v3ForwardTemp);
		//m_pCamera->SetMyRight(v3RightTemp);

		// just in case, reassign target to forward again
		//m_pCamera->SetTarget(m_pCamera->GetPosition() + m_pCamera->GetMyForward());
	}

	//vector3 v3Fwd = m_pCamera->GetTarget();
	//std::cout << "(" << v3Fwd.x << ", " << v3Fwd.y << ", " << v3Fwd.z << ")\n";

	// assignment doesn't require rotation in Z

#pragma endregion
	
	// reassign forward and right and up
	m_pCamera->SetMyForward(v3ForwardTemp);
	m_pCamera->SetMyRight(v3RightTemp);
	//m_pCamera->SetMyUp(v3UpTemp);
	m_pCamera->SetMyUp();
	//*/
	// on every frame, set target to position + my forward
	vector3 v3NewTarget = m_pCamera->GetPosition() + m_pCamera->GetMyForward();
	m_pCamera->SetTarget(v3NewTarget);
	// just in case, reassign target to forward again
	//m_pCamera->SetTarget(m_pCamera->GetMyForward());
}
//Joystick
void Application::ProcessJoystick(void)
{
	/*
	This is used for things that are continuously happening,
	for discreet on/off use ProcessJoystickPressed/Released
	*/
#pragma region Camera Position
	float fForwardSpeed = m_pController[m_uActCont]->axis[SimplexAxis_Y] / 150.0f;
	float fHorizontalSpeed = m_pController[m_uActCont]->axis[SimplexAxis_X] / 150.0f;
	float fVerticalSpeed = m_pController[m_uActCont]->axis[SimplexAxis_R] / 150.0f -
		m_pController[m_uActCont]->axis[SimplexAxis_L] / 150.0f;

	bool fMultiplier = m_pController[m_uActCont]->button[SimplexKey_L1] ||
		m_pController[m_uActCont]->button[SimplexKey_R1];

	if (fMultiplier)
	{
		fForwardSpeed *= 3.0f;
		fHorizontalSpeed *= 3.0f;
		fVerticalSpeed *= 3.0f;
	}
#pragma endregion
#pragma region Camera Orientation
	//Change the Yaw and the Pitch of the camera
#pragma endregion
#pragma region ModelOrientation Orientation
	m_qArcBall = quaternion(vector3(glm::radians(m_pController[m_uActCont]->axis[SimplexAxis_POVY] / 20.0f), 0.0f, 0.0f)) * m_qArcBall;
	if (fMultiplier)
	{
		m_qArcBall = quaternion(vector3(0.0f, 0.0f, glm::radians(m_pController[m_uActCont]->axis[SimplexAxis_POVX] / 20.0f))) * m_qArcBall;
	}
	else
	{
		m_qArcBall = quaternion(vector3(0.0f, glm::radians(m_pController[m_uActCont]->axis[SimplexAxis_POVX] / 20.0f), 0.0f)) * m_qArcBall;
	}
#pragma endregion
}