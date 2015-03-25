#include "pgcalib.h"
using namespace cv;
using namespace std;

using namespace FlyCapture2;
Camera camera;

#define MAX_IMAGES 8

class Settings
{
public:
    Settings() : goodInput(false) {}
    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

	void init(){
		boardSize.width=4;
		boardSize.height=11;
		patternToUse="ASYMMETRIC_CIRCLES_GRID";
		squareSize=50;
		nrFrames=25;
		aspectRatio=1;
		calibZeroTangentDist=1;
		calibFixPrincipalPoint=1;
		outputFileName="C:\\Users\\vincent\\Documents\\Ece496\\Code\\Bender_Opengl\\build\\Debug\\out_camera_data.xml";
		input="1";
		delay=100;
		showUndistorsed=1;
		bwritePoints=1;
		bwriteExtrinsics=1;
		flipVertical=0;
		interprate();
	}
    void interprate()
    {
        goodInput = true;
        if (boardSize.width <= 0 || boardSize.height <= 0)
        {
            cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
            goodInput = false;
        }
        if (squareSize <= 10e-6)
        {
            cerr << "Invalid square size " << squareSize << endl;
            goodInput = false;
        }
        if (nrFrames <= 0)
        {
            cerr << "Invalid number of frames " << nrFrames << endl;
            goodInput = false;
        }

		cout << "connecting the cam\n";
		error = camera.Connect( 0 );
		if ( error != PGRERROR_OK ){
			cout << "Failed to connect to camera" << endl;
			error.PrintErrorTrace();
		}
		// Get the camera info and print it out
		error = camera.GetCameraInfo( &camInfo );
		if ( error != PGRERROR_OK ){
			cout << "Failed to get camera info from camera" << endl; 
			error.PrintErrorTrace();
		}
		fmt7ImageSettings.offsetX = 320;
		fmt7ImageSettings.offsetY = 272;
		fmt7ImageSettings.width = 640;
		fmt7ImageSettings.height = 480;
		fmt7ImageSettings.pixelFormat = PIXEL_FORMAT_RAW8;
		percentageSpeed = 100.0;
		error = camera.SetFormat7Configuration(&fmt7ImageSettings,percentageSpeed);
		error = camera.StartCapture();
		if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED ) {
			cout << "Bandwidth exceeded" << endl;  
			error.PrintErrorTrace();
		}
		else if ( error != PGRERROR_OK ) {
			cout << "Failed to start image capture" << endl;    
			error.PrintErrorTrace();
				}
        if (inputType == INVALID)
        {
            cerr << " Inexistent input: " << input;
            goodInput = false;
        }

        flag = 0;
        if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;


        calibrationPattern = NOT_EXISTING;
        if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
        if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
        if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
        if (calibrationPattern == NOT_EXISTING)
            {
                cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
                goodInput = false;
            }
        atImageList = 0;

    }
    Mat nextImage() {
		Mat result;
		Mat view0;
        //inputCapture >> view0;
		Image rawImage;
		error = camera.RetrieveBuffer( &rawImage );
		if ( error != PGRERROR_OK ) {
			cout << "Failed to start image capture" << endl;    
			error.PrintErrorTrace();
		} 
		Image rgbImage;
		rawImage.Convert( FlyCapture2::PIXEL_FORMAT_BGR, &rgbImage );
		// convert to OpenCV Mat
		unsigned int rowBytes = (double)rgbImage.GetReceivedDataSize()/(double)rgbImage.GetRows();
		view0 = Mat(rgbImage.GetRows(), rgbImage.GetCols(), CV_8UC3, rgbImage.GetData(),rowBytes);
        view0.copyTo(result);
        return result;
    }
public:
    Size boardSize;            // The size of the board -> Number of items by width and height
    Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
    float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;              // The number of frames to use from the input for calibration
    float aspectRatio;         // The aspect ratio
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;          // Flip the captured images around the horizontal axis
    string outputFileName;      // The name of the file where to write
    bool showUndistorsed;       // Show undistorted images after calibration
    string input;               // The input ->

    int cameraID;
    vector<string> imageList;
    int atImageList;
    InputType inputType;
    bool goodInput;
    int flag;
	Error error;
	CameraInfo camInfo;
	Format7ImageSettings fmt7ImageSettings;
	float percentageSpeed;

private:
    string patternToUse;

};

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,
                           vector<vector<Point2f> > imagePoints );

int pgcalib ()
{
    Settings s;
	s.init();

    vector<vector<Point2f> > imagePoints;
    Mat cameraMatrix, distCoeffs;
    Size imageSize;
    int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
    clock_t prevTimestamp = 0;
    const Scalar RED(0,0,255), GREEN(0,255,0);
    const char ESC_KEY = 27;

	Mat calibrationImages[MAX_IMAGES];
	int index=0;
    for(int i = 0;;++i)
    {
      Mat view;
      bool blinkOutput = false;

      view = s.nextImage();
	  imshow("blah",view);
	  
      if( s.flipVertical )    flip( view, view, 0 );
	  imageSize = view.size();  // Format input image.

	  if(cvWaitKey(30) == 'c') {
		  calibrationImages[index] = view;
		  printf("Captured Image %d\n",index);
		  index++;
	  }
	  if (index==MAX_IMAGES) {
		  break;
	  }
	}
	cvDestroyWindow("blah");
	printf("Calibration time!\n");
	
	for (int i=0;i<MAX_IMAGES;i++){
		Mat view2 = calibrationImages[i];
		vector<Point2f> pointBuf;

        bool found;
        switch( s.calibrationPattern ) // Find feature points on the input format
        {
        case Settings::CHESSBOARD:
            found = findChessboardCorners( view2, s.boardSize, pointBuf,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case Settings::CIRCLES_GRID:
            found = findCirclesGrid( view2, s.boardSize, pointBuf );
            break;
        case Settings::ASYMMETRIC_CIRCLES_GRID:
			found = findCirclesGrid( view2, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
        }
		if ( found)                // If done with success,
        {
            cout << "calibrating image " << i << endl; 
			// improve the found corners' coordinate accuracy for chessboard
            if( s.calibrationPattern == Settings::CHESSBOARD)
            {	
				Mat viewGray;
				cvtColor(view2, viewGray, CV_BGR2GRAY);
				cornerSubPix( viewGray, pointBuf, Size(11,11),
				Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
            }
			imagePoints.push_back(pointBuf);
            prevTimestamp = clock();
            //blinkOutput = s.inputCapture.isOpened();
            // Draw the corners.
            drawChessboardCorners( view2, s.boardSize, Mat(pointBuf), found );
        }
		calibrationImages[i] = view2; 
	}
	runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints);
	printf("Show this stuff!\n");
	Mat view3, rview, map1, map2;
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
    getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
    imageSize, CV_16SC2, map1, map2);
	for(int i = 0; i < MAX_IMAGES; i++ )
    {
        view3 = calibrationImages[i];
        remap(view3, rview, map1, map2, INTER_LINEAR);
        imshow("calibrated", rview);
		calibrationImages[i] = rview;
        char c = (char)waitKey();
        if( c  == ESC_KEY || c == 'q' || c == 'Q' )
            break;
    }
	camera.Disconnect();
	cvDestroyWindow("calibrated");
    return 0;
}

static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                         const vector<vector<Point2f> >& imagePoints,
                                         const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                         const Mat& cameraMatrix , const Mat& distCoeffs,
                                         vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                     Settings::Pattern patternType /*= Settings::CHESSBOARD*/)
{
    corners.clear();

    switch(patternType)
    {
    case Settings::CHESSBOARD:
    case Settings::CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; ++i )
            for( int j = 0; j < boardSize.width; ++j )
                corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
        break;

    case Settings::ASYMMETRIC_CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
            for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float((2*j + i % 2)*squareSize), float(i*squareSize), 0));
        break;
    default:
        break;
    }
}

static bool runCalibration( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                            vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                            vector<float>& reprojErrs,  double& totalAvgErr)
{

    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
                                 distCoeffs, rvecs, tvecs, s.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                             rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

// Print camera parameters to the output file
static void saveCameraParams( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                              const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                              const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
                              double totalAvgErr )
{
    FileStorage fs( s.outputFileName, FileStorage::WRITE );

    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_Time" << buf;

    if( !rvecs.empty() || !reprojErrs.empty() )
        fs << "nrOfFrames" << (int)std::max(rvecs.size(), reprojErrs.size());
    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << s.boardSize.width;
    fs << "board_Height" << s.boardSize.height;
    fs << "square_Size" << s.squareSize;

    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        fs << "FixAspectRatio" << s.aspectRatio;

    if( s.flag )
    {
        sprintf( buf, "flags: %s%s%s%s",
            s.flag & CV_CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
            s.flag & CV_CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
            s.flag & CV_CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
            s.flag & CV_CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "" );
        cvWriteComment( *fs, buf, 0 );

    }

    fs << "flagValue" << s.flag;

    fs << "Camera_Matrix" << cameraMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;

    fs << "Avg_Reprojection_Error" << totalAvgErr;
    if( !reprojErrs.empty() )
        fs << "Per_View_Reprojection_Errors" << Mat(reprojErrs);

    if( !rvecs.empty() && !tvecs.empty() )
    {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
        for( int i = 0; i < (int)rvecs.size(); i++ )
        {
            Mat r = bigmat(Range(i, i+1), Range(0,3));
            Mat t = bigmat(Range(i, i+1), Range(3,6));

            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            //*.t() is MatExpr (not Mat) so we can use assignment operator
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
        fs << "Extrinsic_Parameters" << bigmat;
    }

    if( !imagePoints.empty() )
    {
        Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
        for( int i = 0; i < (int)imagePoints.size(); i++ )
        {
            Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
            Mat imgpti(imagePoints[i]);
            imgpti.copyTo(r);
        }
        fs << "Image_points" << imagePtMat;
    }
}

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints )
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;

    bool ok = runCalibration(s,imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs,
                             reprojErrs, totalAvgErr);
    cout << (ok ? "Calibration succeeded" : "Calibration failed")
        << ". avg re projection error = "  << totalAvgErr ;

    if( ok )
        saveCameraParams( s, imageSize, cameraMatrix, distCoeffs, rvecs ,tvecs, reprojErrs,
                            imagePoints, totalAvgErr);
    return ok;
}
