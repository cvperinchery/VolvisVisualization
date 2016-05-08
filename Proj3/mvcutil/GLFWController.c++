// GLFWController.c++: A concrete Controller subclass using the GLFW window interface

#include <stdlib.h>

#include "GLFWController.h"
#include "ModelView.h"

bool GLFWController::debug = false;
bool GLFWController::glfwInitialized = false;

GLFWController::GLFWController(const std::string& windowTitle, int rcFlags) :
	returnFromRun(false), runWaitsForAnEvent(true),
	lastPixelPosX(0), lastPixelPosY(0)
{
	if (!glfwInitialized)
	{
		glfwInit();
		glfwInitialized = true;
	}

	// First create the window and its Rendering Context (RC)
	createWindowAndRC(windowTitle, rcFlags);
}

GLFWController::~GLFWController()
{
	// IF THIS IS THE LAST CONTROLLER
	{
		glfwTerminate();
		glfwInitialized = false;
	}
}

void GLFWController::charCB(GLFWwindow* window, unsigned int theChar)
{
	if (debug)
	{
		std::cout << "GLFWController::charCB: theChar = " << theChar << ", (";
		if (theChar < 128)
		{
			if (theChar >= 32)
				std::cout << static_cast<unsigned char>(theChar);
			else
				std::cout << "non-printable ASCII character";
		}
		else
			std::cout << "non-ASCII character";
		std::cout << ")\n";
	}
	if (theChar < 128)
	{
		GLFWController* c = dynamic_cast<GLFWController*>(curController);
		c->handleAsciiChar(
			static_cast<unsigned char>(theChar), c->lastPixelPosX, c->lastPixelPosY);
	}
}

void GLFWController::createWindowAndRC(const std::string& windowTitle, int rcFlags)
{
	// The following calls enforce use of only non-deprecated functionality.
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if ((rcFlags & MVC_USE_ALPHA_BIT) == 0)
		glfwWindowHint(GLFW_ALPHA_BITS, 0);
	else
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
	if ((rcFlags & MVC_USE_DEPTH_BIT) == 0)
		glfwWindowHint(GLFW_DEPTH_BITS, 0);
	else
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
	if ((rcFlags & MVC_USE_STENCIL_BIT) == 0)
		glfwWindowHint(GLFW_STENCIL_BITS, 0);
	else
		glfwWindowHint(GLFW_STENCIL_BITS, 1);
	setClearFlags(rcFlags);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	int minor = 8; // Start AT LEAST one greater than where you really want to start
	theWindow = NULL;
	while ((theWindow == NULL) && (minor > 0))
	{
		minor--;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		theWindow = glfwCreateWindow(newWindowWidth, newWindowHeight, titleString(windowTitle).c_str(), NULL, NULL);
	}
	if (theWindow == NULL)
	{
		std::cerr << "\n**** COULD NOT CREATE A 4.x RENDERING CONTEXT ****\n\n";
		glfwTerminate();
		glfwInitialized = false;
	}
	else if (debug)
		std::cout << "Created context 4." << minor << '\n';

	glfwMakeContextCurrent(theWindow);
	if ((rcFlags & MVC_USE_DEPTH_BIT) == 0)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);
	establishInitialCallbacksForRC();
}

void GLFWController::displayCB(GLFWwindow* window) // CLASS METHOD
{
	if (curController != NULL)
		dynamic_cast<GLFWController*>(curController)->handleDisplay();
}

void GLFWController::endProgram()
{
	GLFWController* theC = dynamic_cast<GLFWController*>(curController);
	theC->returnFromRun = true;
}

void GLFWController::establishInitialCallbacksForRC()
{
	glfwSetWindowSizeCallback(theWindow, reshapeCB);
	glfwSetCharCallback(theWindow, charCB);
	glfwSetKeyCallback(theWindow, keyboardCB);
	glfwSetMouseButtonCallback(theWindow, mouseFuncCB);
	glfwSetScrollCallback(theWindow, scrollCB);
	glfwSetCursorPosCallback(theWindow, mouseMotionCB);
}

void GLFWController::handleDisplay()
{
	glfwMakeContextCurrent(theWindow);
	int width, height;
	glfwGetFramebufferSize(theWindow, &width, &height);
	glViewport(0, 0, width, height);

	// clear the frame buffer
	glClear(glClearFlags);

	// draw the collection of models
	int which = 0;
	for (std::vector<ModelView*>::iterator it=models.begin() ; it<models.end() ; it++)
		if (visible[which++])
			(*it)->render();

	glfwSwapBuffers(theWindow);

	checkForErrors(std::cout, "GLFWController::handleDisplay");
}

void GLFWController::keyboardCB(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
	if (debug)
	{
		std::cout << "GLFWController::keyboardCB: key = " << key << ", (";
		if (key < 128)
			std::cout << static_cast<unsigned char>(key);
		else
			std::cout << "not an ASCII character";
		std::cout << "), scanCode = " << scanCode << ", action = " << action << ", mods = " << mods << " ...\n";
	}
	if (curController != NULL)
	{
		GLFWController* theC = dynamic_cast<GLFWController*>(curController);
		if (key == GLFW_KEY_ESCAPE)
			theC->handleAsciiChar(27, theC->lastPixelPosX, theC->lastPixelPosY);
		else if (key > 127)
		{
			// Translate to what Controller expects:
			bool haveSpecialKey = true;
			int fcnKey = -1;
			Controller::SpecialKey spKey;
			if (key == GLFW_KEY_RIGHT)
				spKey = Controller::RIGHT_ARROW;
			else if (key == GLFW_KEY_LEFT)
				spKey = Controller::LEFT_ARROW;
			else if (key == GLFW_KEY_DOWN)
				spKey = Controller::DOWN_ARROW;
			else if (key == GLFW_KEY_UP)
				spKey = Controller::UP_ARROW;
			else if (key == GLFW_KEY_HOME)
				spKey = Controller::HOME;
			else
				haveSpecialKey = false;
			if (action != GLFW_PRESS)
			{
				// only send one event per key strike, unless key repeating
				if (haveSpecialKey)
					theC->handleSpecialKey(
						spKey, theC->lastPixelPosX, theC->lastPixelPosY, mapMods(mods));
				else if ((key >= GLFW_KEY_F1) && (key <= GLFW_KEY_F25))
				{
					fcnKey = key - GLFW_KEY_F1 + 1;
					theC->handleFunctionKey(
						fcnKey, theC->lastPixelPosX, theC->lastPixelPosY, mapMods(mods));
				}
			}
			if (debug)
			{
				if (haveSpecialKey)
					std::cout << "\t... sent to Controller::handleSpecialKey\n";
				else if (fcnKey > 0)
					std::cout << "\t... sent to Controller::handleFunctionKey\n";
				else
					std::cout << "\t... key was ignored\n";
			}
		}
		else if (debug)
			std::cout << "\t... ignoring ASCII character; handled by charCB\n";
	}
}

int GLFWController::mapMods(int glfwMods)
{
	int controllerMods = 0;
	if ((glfwMods & GLFW_MOD_SHIFT) != 0)
		controllerMods = MVC_SHIFT_BIT;
	if ((glfwMods & GLFW_MOD_CONTROL) != 0)
		controllerMods |= MVC_CTRL_BIT;
	if ((glfwMods & GLFW_MOD_ALT) != 0)
		controllerMods |= MVC_ALT_BIT;
	return controllerMods;
}

void GLFWController::mouseFuncCB(GLFWwindow* window, int button, int action, int mods)
{
	if (debug)
		std::cout << "GLFWController::mouseFuncCB: button = " << button
		          << ", action = " << action << ", mods = " << mods << "\n";
	if (curController != NULL)
	{
		Controller::MouseButton mButton;
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			mButton = Controller::LEFT_BUTTON;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
			mButton = Controller::RIGHT_BUTTON;
		else
			mButton = Controller::MIDDLE_BUTTON;
		bool pressed = (action == GLFW_PRESS);
		GLFWController* c = dynamic_cast<GLFWController*>(curController);
		c->handleMouseButton(
			mButton, pressed, c->lastPixelPosX, c->lastPixelPosY, mapMods(mods));
	}
}

void GLFWController::mouseMotionCB(GLFWwindow* window, double x, double y)
{
	if (debug)
		std::cout << "GLFWController::mouseMotionCB, x = " << x << ", y = " << y << '\n';
	if (curController != NULL)
	{
		GLFWController* c = dynamic_cast<GLFWController*>(curController);
		c->lastPixelPosX = static_cast<int>(x + 0.5);
		c->lastPixelPosY = static_cast<int>(y + 0.5);
		c->handleMouseMotion(c->lastPixelPosX, c->lastPixelPosY);
	}
}

void GLFWController::reportWindowInterfaceVersion(std::ostream& os) const
{
	os << "        GLFW: " << glfwGetVersionString() << '\n';
}

void GLFWController::reshapeCB(GLFWwindow* window, int width, int height)
{
	if (debug)
		std::cout << "GLFWController::reshapeCB, width = " << width << ", height = " << height << '\n';
	dynamic_cast<GLFWController*>(curController)->handleReshape(width, height);
}

void GLFWController::run()
{
	while (!glfwWindowShouldClose(theWindow) && !returnFromRun)
	{
		if (runWaitsForAnEvent)
			glfwWaitEvents();
		else
			glfwPollEvents();
		handleDisplay();
	}
	glfwDestroyWindow(theWindow);
	theWindow = NULL;
}

void GLFWController::scrollCB(GLFWwindow* window, double xOffset, double yOffset)
{
	if (debug)
		std::cout << "GLFWController::scrollCB: xOffset = " << xOffset
		          << ", yOffset = " << yOffset << "\n";
	dynamic_cast<GLFWController*>(curController)->handleScroll(yOffset > 0.0);
}

void GLFWController::setWindowTitle(const std::string& title)
{
	glfwSetWindowTitle(theWindow, title.c_str());
}
