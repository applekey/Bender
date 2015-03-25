// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


// Initial position : on +Z
float radius = 3.0;
glm::vec3 position = glm::vec3( 0, 0, radius ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 0.0f;
// Initial vertical angle : none
float verticalAngle = 1.5707f;
// Initial Field of View
float initialFoV = 45.0f;

glm::vec3 direction;
glm::vec3 up;

float mouseSpeed = 0.005f;

int pxPos = -1;
int pyPos = -1;

float FoV= initialFoV;
int pfovy = -1;
float fovScalingFactor = 0.04;

int pvxPos = -1;
int pvyPos = -1;
float vSpeed = 0.001f;

float xOffset = 0.0f;
float yOffset = 0.0f;

void initialViewSetup()
{
		pxPos = -1;
		pyPos = -1;
		pvxPos = -1;
		pvyPos = -1;
	    FoV = initialFoV;
		horizontalAngle = 0.0f;
		verticalAngle = 1.5707f;
		position = glm::vec3( 0, 0, radius ); 
	    direction = vec3(0,0,-1);
		up = vec3(0,1,0);
		ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.5f, 0.1f, 100.0f);
		// Camera matrix
		ViewMatrix       = glm::lookAt(
									position,           // Camera is here
									position+direction, // and looks here : at the same position, plus "direction"
									up                  // Head is up (set to 0,-1,0 to look upside-down)
								);
}


void computeMatricesFromInputs(bool pause){

	// calculating view zoom
	if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		int xpos, ypos;
		glfwGetMousePos(&xpos, &ypos);

		if(pfovy == -1)
		{
			pfovy = ypos;
		}
		else
		{
			FoV += fovScalingFactor*(float)(ypos - pfovy);
			ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.5f, 0.1f, 100.0f);
			pfovy = ypos;
		}
	}
	else
	{
		pfovy = -1;
	}

	// calculating view direction
	if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		int xpos, ypos;
		glfwGetMousePos(&xpos, &ypos);

		if(pxPos == -1 ) // this is the first drag click
		{
			pxPos = xpos;
			pyPos = ypos;
		}
		else
		{
			// Compute new orientation
			horizontalAngle -= mouseSpeed * float(xpos - pxPos );
			

			if(verticalAngle >= 3.0 )
			{
				verticalAngle = 2.99;
			}
			else if(verticalAngle <=0.2)
			{
				verticalAngle = 0.22;
			}
			else
			{

			  verticalAngle   += mouseSpeed * float(ypos - pyPos );
			}

			pxPos = xpos;
			pyPos = ypos;

			// Direction : Spherical coordinates to Cartesian coordinates conversion

		
			float vxOff = cos(horizontalAngle)*xOffset*sin(verticalAngle);
			float vyOff = sin(verticalAngle)*yOffset;
			float vzOff = sin(horizontalAngle)*xOffset*sin(verticalAngle);

			position = vec3(radius*sin(horizontalAngle)*sin(verticalAngle) - vxOff,radius*cos(verticalAngle)+ vyOff,radius*cos(horizontalAngle)*sin(verticalAngle)+vzOff);
	        
	        direction = vec3(-1.0*sin(horizontalAngle)*sin(verticalAngle),-1.0*cos(verticalAngle),-1.0*cos(horizontalAngle)*sin(verticalAngle));
			


			ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.5f, 0.001f, 100.0f);
			// Camera matrix
			ViewMatrix       = glm::lookAt(
										position,           // Camera is here
										position+direction, // and looks here : at the same position, plus "direction"
										up                  // Head is up (set to 0,-1,0 to look upside-down)
								   );

				}
	}
	else 
	{
		pxPos = -1;
		pyPos = -1;
	}

	// calculate view direction
	if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
	{
		int xpos, ypos;
		glfwGetMousePos(&xpos, &ypos);
		if(pvxPos == -1 ) // this is the first drag click
		{
			pvxPos = xpos;
			pvyPos = ypos;
		}
		else
		{

			xOffset += mouseSpeed * float(xpos - pvxPos );

		

			yOffset += mouseSpeed * float(ypos - pvyPos );

			pvxPos = xpos;
			pvyPos = ypos;

			float vxOff = cos(horizontalAngle)*xOffset*sin(verticalAngle);
			float vyOff = sin(verticalAngle)*yOffset;
			float vzOff = sin(horizontalAngle)*xOffset*sin(verticalAngle);

			position = vec3(radius*sin(horizontalAngle)*sin(verticalAngle) - vxOff,radius*cos(verticalAngle)+ vyOff,radius*cos(horizontalAngle)*sin(verticalAngle)+vzOff);
	        direction = vec3(-1.0*sin(horizontalAngle)*sin(verticalAngle),-1.0*cos(verticalAngle),-1.0*cos(horizontalAngle)*sin(verticalAngle));
			
		
			//FoV = initialFoV - 5 * glfwGetMouseWheel();

			ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.5f, 0.1f, 100.0f);
			// Camera matrix
			ViewMatrix       = glm::lookAt(
										position,           // Camera is here
										position+direction, // and looks here : at the same position, plus "direction"
										up                  // Head is up (set to 0,-1,0 to look upside-down)
								   );

				
			
		}
	}
	else
	{
		pvxPos = -1;
		pvyPos = -1;
	}
}