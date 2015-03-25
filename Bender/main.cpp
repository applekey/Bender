#include <Windows.h>
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GL/glfw.h>
// Include GLM
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// BOOST
#define BOOST_ALL_NO_LIB 
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/lockfree/queue.hpp>

//ADDITION 
#include "common.h"
//SHADERS
#include "..\common\shader.hpp"
#include "..\common\shader.cpp"
//TEXTURES
#include "..\common\texture.hpp"
#include "..\common\texture.cpp"

//CONTROLS
#include "..\common\controls.hpp"
#include "..\common\controls.cpp"

//OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "pgcalib.h"

#include "FlyCapture2.h"
using namespace FlyCapture2;

// globals
int mode = 0; 
int displayMode = 2; // 0 is capture mode, 1 is reference mode, 2 is depth mode
bool skip = false;
boost::lockfree::queue<cv::Mat **> PackedImages(500);

boost::lockfree::queue<Image*> RawImages(500);
float depthdivide = 8.0;

float qualityThreshold = 0.2;
float depthCull = 2.0;
bool pause = false;

int numberOfMesh = 0;

void CalculateFrameRate() {
	static float framesPerSecond = 0.0f;       // This will store our fps
	static float lastTime = 0.0f;       // This will hold the time from the last frame
	float currentTime = GetTickCount() * 0.001f;    
	++framesPerSecond;
	if( currentTime - lastTime > 1.0f ){
		lastTime = currentTime;
		if(SHOW_FPS == 1) fprintf(stderr, "\nCurrent Frames Per Second: %d\n\n", (int)framesPerSecond);
		framesPerSecond = 0;
	}
}

void GLFWCALL My_Key_Callback(int key, int action)
{
	if(key == 'S' && action == GLFW_PRESS)
	{
		Sleep(3000);
	}

	if(key == 'N' && action == GLFW_PRESS)
	{
		if(numberOfMesh<4)
			numberOfMesh++;
	}

	if(key == 32 && action == GLFW_PRESS)
	{
		pause = ! pause;
	}

	if(key == 'V'&& action == GLFW_PRESS)
	{
		initialViewSetup();
	}

	if(key == 'R'&& action == GLFW_PRESS)
	{
		if(mode == 0)
		{
			mode =1;
			printf("mode1\n");
		}
		else
		{
			mode = 0;
			printf("mode0\n");
		}
	}

	if(key == 'D'&& action == GLFW_PRESS)
	{
		// have to drop a frame here
		skip = true;
	}

	if(key == 'G'&& action == GLFW_PRESS)
	{
		if(depthdivide>1)
			depthdivide-=0.3;
	}

	if(key == 'H'&& action == GLFW_PRESS)
	{
		depthdivide +=0.3;
	}

	if(key == 'Q'&& action == GLFW_PRESS)
	{
		displayMode = 0;
		
	}

	if(key == 'W'&& action == GLFW_PRESS)
	{
		displayMode = 1;
		
	}

	if(key == 'E'&& action == GLFW_PRESS)
	{
		displayMode =2;
		
		printf("display mode 2\n");
	}
	if(key == 'O'&& action == GLFW_PRESS)
	{
		qualityThreshold -= 0.02;
	}
	if(key == 'P'&& action == GLFW_PRESS)
	{
		qualityThreshold += 0.02;
	}

	if(key == 'I'&& action == GLFW_PRESS)
	{
		depthCull -= 0.1;
	}
	if(key == 'U'&& action == GLFW_PRESS)
	{
		depthCull += 0.1;
	}


}

void CaptureImageCallBack(Image* capImage, cv::Mat* frame){
    FlyCapture2::Error error;
 
    *frame = cv::Mat(IMAGE_ROW,IMAGE_COLUMN,CV_8UC1);
	
    // Create a converted image
    FlyCapture2::Image convertedImage;
    // Convert the raw image
    error = capImage->Convert( FlyCapture2::PIXEL_FORMAT_MONO8, &convertedImage );
   
	//int size = frame->datastart - frame->datastart;
    //Copy the image into the IplImage of OpenCV
	memcpy(frame->data, convertedImage.GetData(), CONVERTED_DATA_SIZE);
}

int lastCount = -1;

FlyCapture2::Image* newImages = new FlyCapture2::Image [PREALLOCATED_STORAGE];
int flyCapIndex = 0;
void OnImageGrabbed(Image* pImage, const void* pCallbackData)
{
	ImageMetadata metadata = pImage->GetMetadata();
	int newCount = metadata.embeddedFrameCounter;
	if(lastCount == -1)
	{
		lastCount = newCount;
	}
	else
	{
		if((newCount - lastCount)!=1)
		{
			printf("skipped %d %d\n",newCount,lastCount);
			lastCount = newCount;
		}
		else
		{
			lastCount = newCount;
		}
	}
	int index = flyCapIndex %PREALLOCATED_STORAGE;
	flyCapIndex ++;
	newImages[index].SetData(pImage->GetData(),307200);
	RawImages.push(&newImages[index]);

}

int ConnectCam(FlyCapture2::Camera* cam){
for(int i =0;i<PREALLOCATED_STORAGE;i++)
{
	newImages[i] =  FlyCapture2::Image(480,640,640,NULL,307200,PIXEL_FORMAT_RAW8);
}


#ifndef MOCK_INPUT
	FlyCapture2::Error error;
    FlyCapture2::PGRGuid guid;
    FlyCapture2::BusManager busMgr;
    
    //Getting the GUID of the cam
    error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != FlyCapture2::PGRERROR_OK){
        error.PrintErrorTrace();
        return -1;
    }
    // Connect to a camera
    error = cam->Connect(&guid);
    if (error != FlyCapture2::PGRERROR_OK){
        error.PrintErrorTrace();
        return -1;
    }

	// set the image format
	 // Query for available Format 7 modes
    Format7Info fmt7Info;
	
	Format7ImageSettings fmt7ImageSettings;
    fmt7ImageSettings.mode = MODE_0;
    fmt7ImageSettings.offsetX = 320;
    fmt7ImageSettings.offsetY = 272;
    fmt7ImageSettings.width = 640;
    fmt7ImageSettings.height = 480;
    fmt7ImageSettings.pixelFormat = PIXEL_FORMAT_RAW8;

	float percentageSpeed = 100.0;
	error = cam->SetFormat7Configuration(&fmt7ImageSettings,percentageSpeed);

	// set the trigger mode
	 TriggerMode triggerMode;
    error = cam ->GetTriggerMode( &triggerMode );


    // Set camera to trigger mode 0
    triggerMode.onOff = true;
    triggerMode.mode = 0;
    triggerMode.parameter = 0;
	triggerMode.source = 0;

    
    error = cam->SetTriggerMode( &triggerMode );
    if (error != PGRERROR_OK)
    {
        printf("couldn'tn set trigger mode\n");
        return -1;
    }

    
	// enable the framecount
	EmbeddedImageInfo EmbeddedInfo;
	cam->GetEmbeddedImageInfo(&EmbeddedInfo);

	if (EmbeddedInfo.frameCounter.available == true) {
		EmbeddedInfo.frameCounter.onOff = true; 
	}
	else {
		cout << "Framecounter is not available!" << endl;
	}

	cam->SetEmbeddedImageInfo(&EmbeddedInfo);

	
    //Starting the capture
    error = cam->StartCapture(OnImageGrabbed);
    if (error != FlyCapture2::PGRERROR_OK) {
        error.PrintErrorTrace();
        return -1;
    }

#endif
    return 0;
}

void CaptureImage(FlyCapture2::Camera* cam, cv::Mat* frame){
    FlyCapture2::Error error;

    FlyCapture2::Image rawImage;
    cam->RetrieveBuffer(&rawImage);


    // Get the raw image dimensions
    FlyCapture2::PixelFormat pixFormat;
    unsigned int rows, cols, stride;
    rawImage.GetDimensions( &rows, &cols, &stride, &pixFormat );
    
	int columns = rawImage.GetCols();
	int rowws = rawImage.GetRows();
    *frame = cv::Mat(rawImage.GetRows(),rawImage.GetCols(),CV_8UC1);
	
    // Create a converted image
    FlyCapture2::Image convertedImage;
    // Convert the raw image
    error = rawImage.Convert( FlyCapture2::PIXEL_FORMAT_MONO8, &convertedImage );
    if (error != FlyCapture2::PGRERROR_OK)
    {
        error.PrintErrorTrace();
        throw "Fly Cap cannot convert.\n";
    }

	//int size = frame->datastart - frame->datastart;
    //Copy the image into the IplImage of OpenCV
	memcpy(frame->data, convertedImage.GetData(), convertedImage.GetDataSize());
}


void CaptureThread(FlyCapture2::Camera* cam) {

	// intilize all pre intilized opencv mats

	if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
	{
		printf("un oh\n");
	}

	DWORD  dwThreadPri = GetThreadPriority(GetCurrentThread());
	printf("Current thread priority is 0x%x\n", dwThreadPri );

	ConnectCam(cam);

	cv::Mat **returnImages = new cv::Mat * [2];
	cv::Mat packImages[6];
	int index= 0;
	while(true)
	{
        if(skip == true)
        {
                skip = false;
                printf("dropping\n");
                index = 0;
                continue; // if skip, skip a frame
        }
                        

        int packIndex = index %6;
		cv::Mat image;
        FlyCapture2::Image* rawImage;
#ifndef MOCK_INPUT
		if(!RawImages.pop(rawImage))
			continue;

		CaptureImageCallBack(rawImage,&image);
		//delete rawImage;

#endif
#ifdef MOCK_INPUT

        int mockIndex = (index) %6;
        char * mockImages[6];
        if(mode == 1)
        {
                mockImages[0] = "images\\mockImages\\Capture54_1.bmp";
                mockImages[1] = "images\\mockImages\\Capture54_2.bmp";
                mockImages[2] = "images\\mockImages\\Capture54_3.bmp";
                mockImages[3] = "images\\mockImages\\\Capture60_1.bmp";
                mockImages[4] = "images\\mockImages\\Capture60_2.bmp";
                mockImages[5] = "images\\mockImages\\Capture60_3.bmp";
        }
        else
        {
                mockImages[0] = "images\\mockReference\\Ref54_1.bmp";
                mockImages[1] = "images\\mockReference\\Ref54_2.bmp";
                mockImages[2] = "images\\mockReference\\Ref54_3.bmp";
                mockImages[3] = "images\\mockReference\\\Ref60_1.bmp";
                mockImages[4] = "images\\mockReference\\Ref60_2.bmp";
                mockImages[5] = "images\\mockReference\\Ref60_3.bmp";
        }
                
        image = cv::imread(mockImages[mockIndex],CV_LOAD_IMAGE_GRAYSCALE);
        //Sleep(50);
                
#endif
        packImages[packIndex] = image;

        if(packIndex == 2)
        {
                cv::Mat out;
                cv::Mat in[3] = {packImages[0],packImages[1],packImages[2]};
                cv::merge(in,3,out);
                cv::Mat* store = new cv::Mat(out.rows,out.cols,CV_8UC3);
                memcpy(store->data,out.data,(out.dataend- out.datastart));
                returnImages[0] = store;
        }

        if(packIndex == 5)
        {
                // pack the images
                cv::Mat out;
                cv::Mat in[3] = {packImages[3],packImages[4],packImages[5]};
                cv::merge(in,3,out);
                cv::Mat* store = new cv::Mat(out.rows,out.cols,CV_8UC3);
                memcpy(store->data,out.data,(out.dataend- out.datastart));
                returnImages[1] = store;
                PackedImages.push(returnImages);
        }

        index++;	
		//Sleep(2.1);
	}
}

int checkFBOBinding()
{
	GLenum e = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (e != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("There is a problem with the FBO\n");
		switch(e)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			printf("here");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		printf("here");
		break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		printf("here");
		break;
			
		case GL_FRAMEBUFFER_UNSUPPORTED:
			printf("here");
			break;
		default:
			printf("here");
			break;
		}
		return -1;
	}
	else
	{
		return 0;
	}
}

int main(){
	// Calibration prompt
	cout << "Do you want to calibrate? Y/N\n";
	char choice;
	cin >> choice;
	if (choice=='y' || choice =='Y')
		pgcalib();
	
	// read the calibration file
	const string calibFile = "C:\\Users\\vincent\\Documents\\Ece496\\Code\\Bender_Opengl\\build\\Debug\\out_camera_data.xml";
	cv::FileStorage fs(calibFile, cv::FileStorage::READ); // Read the settings
    if (!fs.isOpened()) {
        cout << "Could not open the calibration file: \"" << calibFile << "\"" << endl;
        return -1;
    }
    cv::Mat cameraMatrix, distCoeffs;
	fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distCoeffs;
	cv::Size imageSize(640,480);

	Camera cam;
	int    desiredFPS = 100;
	double msPerFrame = 1000 / desiredFPS;
	double frameStartTime, frameEndTime, frameDrawTime;

    //Define the width and the height of the window
    int     width=640, height=480;
    bool    running = true;
    
    int error = glfwInit();
	glewExperimental = GL_TRUE;
	
	//glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
     boost::thread captureThread(CaptureThread,&cam);

	 Sleep(1000);
    //Create a window with the width and the height
    //if we want it fullscreen we change GLFW_WINDOW with GLFW_FULLSCREEN
    if(!glfwOpenWindow( width, height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW )){
        glfwTerminate();
        return 0;
    }
	
	int major, minor, rev;

	glfwGetGLVersion(&major, &minor, &rev);

	fprintf(stderr, "OpenGL version recieved: %d.%d.%d", major, minor, rev);

	if (glewInit() != GLEW_OK)
        throw("Failed to initialize GLEW\n");

	// set up shader programs
	
    GLuint wrapProgram = LoadShaders( "wrap.vert", "wrap.frag" );
	GLuint maskProgram = LoadShaders( "passThrough.vert", "mask.frag" );
	GLuint gaussPass1Program = LoadShaders( "gaussPass1.vert", "gauss.frag" );
	GLuint gaussPass2Program = LoadShaders("gaussPass2.vert", "gauss.frag");
	GLuint gaussPass3Program = LoadShaders( "gaussPass1.vert", "gauss2.frag" );
	GLuint gaussPass4Program = LoadShaders("gaussPass2.vert", "gauss2.frag");

	GLuint medianProgram = LoadShaders("median.vert", "median.frag");
	
	
	GLuint depthProgram = LoadShaders( "passThrough.vert", "depth.frag" );

	GLuint normalProgram = LoadShaders( "normal.vert", "normal.frag" );
	GLuint meshProgram = LoadShaders( "mesh.vert", "mesh.frag" );

	GLuint displayProgram = LoadShaders("display.vert", "display.frag");



    glfwSetWindowTitle("BENDER");
	glfwSetKeyCallback(My_Key_Callback);
	glfwSwapInterval(1);

	//uniforms for wrap program
	GLuint packedCaptures_firstWavelength = glGetUniformLocation(wrapProgram, "packedCaptures_firstWavelength");
	GLuint packedCaptures_secondWavelength = glGetUniformLocation(wrapProgram, "packedCaptures_secondWavelength");
	GLuint referenceCaptures_firstWavelength = glGetUniformLocation(wrapProgram, "referenceCaptures_firstWavelength");
	GLuint referenceCaptures_secondWavelength = glGetUniformLocation(wrapProgram, "referenceCaptures_secondWavelength");

	// uniforms for median program
	GLuint medianSource = glGetUniformLocation(medianProgram, "tex");


	// uniformsfor maks program
	GLuint packedImages = glGetUniformLocation(maskProgram, "packedImages");
	GLuint qualityThresParm = glGetUniformLocation(maskProgram, "quality_threshold");
	
	// uniforms for gauss1 program
	GLuint wrapImage1 = glGetUniformLocation(gaussPass1Program, "wrapImage");
	GLuint modeFilter1 = glGetUniformLocation(gaussPass1Program, "mode");
	
	// uniforms for gauss2 program
	GLuint wrapImage2 = glGetUniformLocation(gaussPass2Program, "wrapImage");
	GLuint modeFilter2 = glGetUniformLocation(gaussPass2Program, "mode");

	// uniforms for gauss3 program
	GLuint wrapImage3 = glGetUniformLocation(gaussPass3Program, "wrapImage");
	GLuint modeFilter3 = glGetUniformLocation(gaussPass3Program, "mode");
	
	// uniforms for gauss4 program
	GLuint wrapImage4 = glGetUniformLocation(gaussPass4Program, "wrapImage");
	GLuint modeFilter4 = glGetUniformLocation(gaussPass4Program, "mode");

	// uniforms for depth program
	GLuint filteredPhasesCos = glGetUniformLocation(depthProgram, "filteredPhasesCos");
	GLuint filteredPhasesSin = glGetUniformLocation(depthProgram, "filteredPhasesSin");

	GLuint div = glGetUniformLocation(depthProgram, "div");
	GLuint displayModeId = glGetUniformLocation(depthProgram, "displayMode");


	// set up uniform variables for normal program
	GLuint frameBufferTexture = glGetUniformLocation(normalProgram, "fboImage");
	GLuint normalShowMode = glGetUniformLocation(normalProgram, "displayMode");

	// get the uniforms for the mesh program
	GLuint meshDepthImageImage = glGetUniformLocation(meshProgram, "meshDepthImage");
	GLuint maskImage = glGetUniformLocation(meshProgram, "maskImage");
	GLuint meshMode = glGetUniformLocation(meshProgram, "mode");
	GLuint depthCullParm = glGetUniformLocation(meshProgram, "depthCull");

	// set up the uniforms for the third program
	
	GLuint normalImage = glGetUniformLocation(displayProgram, "normalImage");
	GLuint depthImage = glGetUniformLocation(displayProgram, "meshDepthImage");
	GLuint showMode = glGetUniformLocation(displayProgram, "mode");



	// set up textures
	GLuint  captureTexture_1 = loadBMP_custom(STOCK_IMAGE1);
	GLuint  captureTexture_2 = loadBMP_custom(STOCK_IMAGE1);
	GLuint  referenceTexture_1 = loadBMP_custom(STOCK_IMAGE1);
	GLuint  referenceTexture_2 = loadBMP_custom(STOCK_IMAGE1);
    
	// set up previous image
	cv::Mat * previousImage_w1 = new cv::Mat(240, 320, CV_8UC3);
	cv::Mat * previousImage_w2 = new cv::Mat(240, 320, CV_8UC3);
	
	// get the first container so that we can have a previous
	cv::Mat ** imageContainerPrevious = nullptr;
	cv::Mat ** imageContainer;

	// get the first image to know the size of the texture
	while(PackedImages.pop(imageContainer) != true)
	{
	}
	imageContainerPrevious = imageContainer;

	cv::Mat *firstImage = imageContainer[0];
	cv::Mat tmp1, tmp2,map1,map2;
	initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(),
		getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
		imageSize, CV_16SC2, map1, map2);

	// if calibrated, then remap the image
	tmp1 = *firstImage;
	remap(tmp1, tmp2, map1, map2, cv::INTER_LINEAR);
	*firstImage = tmp2;

	//*****---------------------------SET UP A WRAP BUFFER----------------------------------------****///
	GLuint wrapBuffer;  
	glGenFramebuffers(1, &wrapBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, wrapBuffer);

	GLuint wrapTex;
	glGenTextures(1, &wrapTex);
	glBindTexture(GL_TEXTURE_2D, wrapTex);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint wrapMaskTex;
	glGenTextures(1, &wrapMaskTex);
	glBindTexture(GL_TEXTURE_2D, wrapMaskTex);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, wrapTex, 0);

	//*****---------------------------SET UP A MASK BUFFER----------------------------------------****///
	
	GLuint maskBuffer;  
	glGenFramebuffers(1, &maskBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, maskBuffer);

	GLuint maskTex;
	glGenTextures(1, &maskTex);
	glBindTexture(GL_TEXTURE_2D, maskTex);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, maskTex, 0);


	//*****---------------------------SET UP A MEDIAN BUFFER----------------------------------------****///
	
	GLuint medianBuffer;  
	glGenFramebuffers(1, &medianBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, medianBuffer);

	GLuint medianTex;
	glGenTextures(1, &medianTex);
	glBindTexture(GL_TEXTURE_2D, medianTex);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, medianTex, 0);


	////*****---------------------------SET UP A gauss BUFFER 1----------------------------------------****///

	GLuint gaussBuffer1;  
	glGenFramebuffers(1, &gaussBuffer1);
	glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer1);
	GLuint gaussTex1;
	glGenTextures(1, &gaussTex1);
	glBindTexture(GL_TEXTURE_2D, gaussTex1);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, gaussTex1, 0);

	////*****---------------------------SET UP A gauss BUFFER 2----------------------------------------****///

	GLuint gaussBuffer2;  
	glGenFramebuffers(1, &gaussBuffer2);
	glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer2);
	GLuint gaussTex2;
	glGenTextures(1, &gaussTex2);
	glBindTexture(GL_TEXTURE_2D, gaussTex2);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, gaussTex2, 0);

	////*****---------------------------SET UP A gauss BUFFER 2----------------------------------------****///

	GLuint gaussBuffer3;  
	glGenFramebuffers(1, &gaussBuffer3);
	glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer3);
	GLuint gaussTex3;
	glGenTextures(1, &gaussTex3);
	glBindTexture(GL_TEXTURE_2D, gaussTex3);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, gaussTex3, 0);

	////*****---------------------------SET UP A gauss BUFFER 2----------------------------------------****///

	GLuint gaussBuffer4;  
	glGenFramebuffers(1, &gaussBuffer4);
	glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer4);
	GLuint gaussTex4;
	glGenTextures(1, &gaussTex4);
	glBindTexture(GL_TEXTURE_2D, gaussTex4);
	glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, gaussTex4, 0);


	//*****---------------------------SET UP A DEPTH BUFFER----------------------------------------****///
	// set up frame buffer stuff
	GLuint depthFrameBuffer;  
	glGenFramebuffers(1, &depthFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);

	// generate four textures, since we are allowing four views
	GLuint depthTex;
	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexImage2D(
	GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	

	// attach the texture to the fbo
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, depthTex, 0);
	

	// ** ----------------------SET UP A NORMAL CALCULATOR BUFFER -----------------------------****///
	GLuint normalBuffer;  
	glGenFramebuffers(1, &normalBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, normalBuffer);

	GLuint normalTex[NUMBER_OF_VIEWS];
	for(int i =0;i<NUMBER_OF_VIEWS;i++)
	{
		glGenTextures(1, &normalTex[i]);
		glBindTexture(GL_TEXTURE_2D, normalTex[i]);
		glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
	}


	//*-------------------SET UP A MESH DO SOTUFF BUFFER -------------------------------------***////
	GLuint meshBuffer;  
	glGenFramebuffers(1, &meshBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, meshBuffer);

	GLuint meshTex[NUMBER_OF_VIEWS];
	for(int i =0;i<NUMBER_OF_VIEWS;i++)
	{
		glGenTextures(1, &meshTex[i]);
		glBindTexture(GL_TEXTURE_2D, meshTex[i]);
		glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA32F, firstImage->cols, firstImage->rows, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
	}


	// generate number of verticies as well as their uv coordinates
	int mrows = firstImage->rows;
	int mcols = firstImage->cols;
	GLfloat* g_vertex_buffer_data = new GLfloat[mrows*mcols*3];
	
	int indexBuffer = 0;
	for(int r = 0;r<mrows; r ++)
	{
		for(int c =0; c<mcols;c++)
		{
			GLfloat vxCoord =(GLfloat)c/(GLfloat)(mcols-1);
			GLfloat vyCoord = (GLfloat)r/(GLfloat)(mrows-1);
			GLfloat vzCoord = 1.0;
			g_vertex_buffer_data[indexBuffer] = vxCoord;
			g_vertex_buffer_data[indexBuffer +1] = vyCoord;
			g_vertex_buffer_data[indexBuffer +2] = vzCoord;
			indexBuffer +=3;
		}
	}
	// generate the indicies
	int stopIndex = (mcols*2+1)*(mrows-1)-1;
	int* iIndices = new int[stopIndex];//have to had each row for the restart
  	int iIndex = 0;
	for(int r=0;r<(mrows -1);r++)
	{
		int c;
		for(c =0; c<(mcols +1);c ++)
		{
			if(iIndex == stopIndex)
				break;
			if(c == mcols )
			{
				iIndices[iIndex] = mcols*mrows;
				iIndex++;
			}
			else
			{
				iIndices[iIndex] = mcols*r+c;
				iIndices[iIndex+1] =mcols*(r+1)+c;
				iIndex +=2;
			}
			
		}
	}
	// draw the first mesh
	GLuint  mesh[NUMBER_OF_VIEWS],meshData[NUMBER_OF_VIEWS],meshIndicies[NUMBER_OF_VIEWS];
	glm::mat4 rtmatrixes[NUMBER_OF_VIEWS];
	// hard code these for now
	rtmatrixes[0] = glm::mat4(1.0);

	for(int i =0;i<NUMBER_OF_VIEWS;i++)
	{
		rtmatrixes[0] = glm::mat4(1.0);
		glGenVertexArrays(1, &mesh[i]); // Create one VAO
		glGenBuffers(1, &meshData[i]); // One VBO for data
		glGenBuffers(1, &meshIndicies[i]); // And finally one VBO for indices

		glBindVertexArray(mesh[i]);
		glBindBuffer(GL_ARRAY_BUFFER, meshData[i]);
		int buffSize = sizeof(GLfloat) *mrows*mcols*3;
		glBufferData(GL_ARRAY_BUFFER, buffSize,g_vertex_buffer_data, GL_STATIC_DRAW);


		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &meshIndicies[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIndicies[i]);
		int indexSize = sizeof(int) * stopIndex;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, iIndices, GL_STATIC_DRAW);
		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(mcols*mrows);
	}
	rtmatrixes[1] = glm::rotate(glm::mat4(1.0),90.0f,vec3(0.0,1.0,0.0));
	rtmatrixes[2] = glm::rotate(glm::mat4(1.0),180.0f,vec3(0.0,1.0,0.0));
	rtmatrixes[3] = glm::rotate(glm::mat4(1.0),270.0f,vec3(0.0,1.0,0.0));


	//draw the second mesh




	glEnable(GL_TEXTURE_2D);
	glViewport( 0, 0, width, height );
	glfwEnable(GLFW_MOUSE_CURSOR);

	//// set up antweek bar
	//float g_Zoom = 1.0f;

	//TwBar *bar;
	////TwInit(TW_OPENGL, NULL);

	//TwBar *myBar;
	//myBar = TwNewBar("Options");

	//TwWindowSize(300, 300);
	//TwAddVarRW(myBar, "Depth Cull", TW_TYPE_FLOAT, &depthCull, 
 //              " min=0.01 max=2.5 step=0.01 keyIncr=z keyDecr=Z help='Scale the object (1=original size).' ");

	//glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW);
	//glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
	//glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
	//glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
	//glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);



	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	initialViewSetup();
	
	int count = 0;
	cv::Mat* image_1;
	cv::Mat* image_2;
	cv::Mat* pimage_1;
	cv::Mat* pimage_2;

	while(running) {

		        //clear to background color	
		glViewport( 0, 0, width, height );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glEnable(GL_DEPTH_TEST);
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		glOrtho(0,width,height,0,0,1);
		glMatrixMode( GL_MODELVIEW );
		glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		
		glUseProgram(wrapProgram);
		frameStartTime = glfwGetTime();


		GLint textureId;
		// check how many images are in the buffer
		if(PackedImages.pop(imageContainer) == false)
		{

			//continue, the branch prediction is killer here
		}
		else
		{

			if(count ==0)
			{
				
				count ++;
			}
			else if (count ==1)
			{
				pimage_1 = image_1;
				pimage_2 = image_2;
				
				count++;
			}
			else if(count ==2)
			{
				pimage_1->release();
				pimage_2->release();
				pimage_1 = image_1;
				pimage_2 = image_2;
			

			}
			image_1 = imageContainer[0];
			image_2 = imageContainer[1];



			if(pause)
			{
				image_1->release();
				image_2->release();
				image_1 = pimage_1;
				image_2 = pimage_2;
			}
		
			
		
			//imageContainerPrevious = imageContainer; // set the current image container
		}

	
	
		if(mode == 1) // capture mode
		{
				//glUniform1f(div, depthdivide);

				textureId = packedCaptures_firstWavelength;
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, captureTexture_1);
				
				glUniform1i(textureId, 0);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_1->cols, image_1->rows, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)image_1->data);
				
				textureId = packedCaptures_secondWavelength;
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_2D, captureTexture_2);

				glUniform1i(textureId, 1);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_2->cols, image_2->rows, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)image_2->data);
			
		}
		else  // reference mode
		{
			
				//glUniform1f(div, depthdivide);

				textureId = referenceCaptures_firstWavelength;
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_2D, referenceTexture_1);
				
				glUniform1i(textureId, 2);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_1->cols, image_1->rows, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)image_1->data);

		
				textureId = referenceCaptures_secondWavelength;
				glActiveTexture(GL_TEXTURE0 + 3);
				glBindTexture(GL_TEXTURE_2D, referenceTexture_2);

				glUniform1i(textureId, 3);

				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_2->cols, image_2->rows, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)image_2->data);
				
		}


		// bind a the first frame buffer
		///****  CALCULATE THE PHASE ***/////////////////////////////
		glBindFramebuffer(GL_FRAMEBUFFER,  wrapBuffer);
		if(checkFBOBinding() != 0)
		{
		  printf("depth framebuffer not binding\n");
		}

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		///// find a mask
		glUseProgram(maskProgram);

		glBindFramebuffer(GL_FRAMEBUFFER,  maskBuffer);

		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, captureTexture_1);
		glUniform1i(packedImages, 0);
		glUniform1f(qualityThresParm,qualityThreshold);
		

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		// bind the median buffer
		glUseProgram(medianProgram);

		glBindFramebuffer(GL_FRAMEBUFFER,  medianBuffer);

		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, wrapTex);
		glUniform1i(medianSource, 0);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind


		// bind the gauss frame buffer
		glUseProgram(gaussPass1Program);
		glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer1);
		if(checkFBOBinding() != 0)
		{
		  printf("depth framebuffer not binding\n");
		}

		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, medianTex);
		glUniform1i(wrapImage1, 0);
		glUniform1i(modeFilter1,0);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		// bind the gauss frame buffer
		glUseProgram(gaussPass2Program);
		glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer2);
		if(checkFBOBinding() != 0)
		{
		  printf("depth framebuffer not binding\n");
		}

		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, gaussTex1);
		glUniform1i(wrapImage2, 0);
		glUniform1i(modeFilter2,1);
		
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		// bind the gauss frame buffer
		glUseProgram(gaussPass3Program);
		glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer3);
		if(checkFBOBinding() != 0)
		{
		  printf("depth framebuffer not binding\n");
		}

		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, medianTex);
		glUniform1i(wrapImage3, 0);
		glUniform1i(modeFilter3,0);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		// bind the gauss frame buffer
		glUseProgram(gaussPass4Program);
		glBindFramebuffer(GL_FRAMEBUFFER, gaussBuffer4);
		if(checkFBOBinding() != 0)
		{
		  printf("depth framebuffer not binding\n");
		}

		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, gaussTex3);
		glUniform1i(wrapImage4, 0);
		glUniform1i(modeFilter4,1);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		//////////// cauculate the depth
		glUseProgram(depthProgram);

		glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, depthTex, 0);

		glActiveTexture(GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_2D, gaussTex2);
		glUniform1i(filteredPhasesSin, 0);
		glUniform1i(displayModeId,displayMode);

		glActiveTexture(GL_TEXTURE0 +1);
		glBindTexture(GL_TEXTURE_2D, gaussTex4);
		glUniform1i(filteredPhasesCos, 1);

		glUniform1f(div, depthdivide);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

	
		///****  CALCULATE THE NORMALS    ***/////////////////////////////
		glBindFramebuffer(GL_FRAMEBUFFER, normalBuffer);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, normalTex[numberOfMesh], 0);

		glUseProgram(normalProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		glUniform1i(frameBufferTexture, 0);

		glUniform1i(normalShowMode,displayMode);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		///****** MESH_PROGRAM ****///////////////////
		glUseProgram(meshProgram);
		glBindFramebuffer(GL_FRAMEBUFFER, meshBuffer);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, meshTex[numberOfMesh], 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, maskTex);
		glUniform1i(maskImage, 0);
		
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		glUniform1i(meshDepthImageImage, 1);

		glUniform1i(meshMode,displayMode);
		glUniform1f(depthCullParm,depthCull);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(width, 0);
		glEnd();

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind



		///****** DISPLAY, PHONG SHADING, CAMERA STUFF ****///////////////////
		// now draw the object onscreen
		glPopMatrix();
		glUseProgram(displayProgram);

		glViewport( 0, 0, width*2, height*2 );
		
		computeMatricesFromInputs(pause);
		
		//glm::mat4 rotationTranslationMatrix = glm::mat4(1.0);

		

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		GLuint MatrixID = glGetUniformLocation(displayProgram, "MVP");
		glUniform1i(showMode,displayMode);

		for(int i =0;i<=numberOfMesh;i++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, normalTex[i]);
			glUniform1i(normalImage, 0);
		
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, meshTex[i]);
			glUniform1i(depthImage, 1);

		
			glm::mat4 rotationTranslationMatrix = rtmatrixes[i];
			ProjectionMatrix = getProjectionMatrix();
			ViewMatrix = getViewMatrix();
			glm::mat4 ModelMatrix = glm::mat4(1.0);

			// centering matrix
			glm::mat4 ViewTranslate = glm::translate(
			glm::mat4(1.0f),
			glm::vec3(-0.5f, 0.0f, -0.5));


			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * rotationTranslationMatrix*ViewTranslate*ModelMatrix;

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			
			glBindVertexArray(mesh[i]);
			glEnableVertexAttribArray(0);
			glDrawElements(GL_TRIANGLE_STRIP, stopIndex, GL_UNSIGNED_INT, 0);
			glDisableVertexAttribArray(0);
			glBindVertexArray(0);
		}
		
		
		glUseProgram(0);
		// draw anttweakbar
		//TwDraw();

			// if new frame save old into a texture, save it and then bind a new texture
		//glGetTexImage 

		glfwSwapBuffers();

        // exit if ESC was pressed or window was closed
        // delete the images

		CalculateFrameRate();
		running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam( GLFW_OPENED);
		
	
		// Lock to ~60fps
		frameEndTime = glfwGetTime();
		frameDrawTime = frameEndTime - frameStartTime;
		if (frameDrawTime < msPerFrame){
			glfwSleep((msPerFrame - frameDrawTime)/1000);
		}
	}

    glfwTerminate();
    return 0;
}
