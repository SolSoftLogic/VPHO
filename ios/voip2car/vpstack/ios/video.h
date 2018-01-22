/*
 *  video.h
 *  vpstack
 */
#ifndef _VIDEO_H_INCLUDED
#define _VIDEO_H_INCLUDED

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

#import "../vpstack.h"
#import "../../codecs/videocodecs.h"

UIKIT_EXTERN NSString *const VPVideoDidStartNotification; // always sent on main thread. userInfo contains VPCALL and UIImageView* 
UIKIT_EXTERN NSString *const VPVideoDidFinishNotification; // always sent on main thread. userInfo contains contains VPCALL 

#if !TARGET_IPHONE_SIMULATOR
/**
 * Camcorder captures frames from camera, encodes and sends to clients connected to videoconference
 * Uses singleton pattern and shared accross whole application.
 */
@interface Camcorder : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> 
{
	IVPSTACK *vps;
	VIDEOCODEC *encoder;
	AVCaptureSession *captureSession;
	AVCaptureDeviceInput *captureInputFrontCamera;
	AVCaptureDeviceInput *captureInputDefaultCamera;	
	AVCaptureDeviceInput *captureInput;
	
	UIView *previewView; /// can be used by application to display actual image from camera
	
	unsigned int pixelFormat;	
	unsigned int clientsCount;

	uint8_t *srcFrame;
	uint8_t *encodedFrame;
}

@property (nonatomic, assign) UIView *previewView;

+ (Camcorder*) sharedInstance;

- (void) setVPSTACK:(IVPSTACK*)vpstack;

- (int) start;

- (void) stop;

- (BOOL) isFrontCameraAvailable;

- (void)captureOutput:(AVCaptureOutput *)captureOutput 
		didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
	    fromConnection:(AVCaptureConnection *)connection;

@end

#endif  /*!TARGET_IPHONE_SIMULATOR*/

@interface ImageDrawer : NSObject
{
    UIImageView *view;
}
- (id) initWithImageView:(UIImageView*)view;
- (void) draw:(UIImage*)img;
@end

/*
 * Implements receiving and decoding frames from one of videoconference member
 */
class VPIOSVIDEO : public VPVIDEODATA {
public:
	VPIOSVIDEO();
	virtual ~VPIOSVIDEO();
	
	virtual void VideoFrame(unsigned short timestamp, const unsigned char *buf, int buflen, bool keyframe);
	virtual int Create(IVPSTACK *vps, 
					   VPCALL vpcall, 
					   unsigned fourccrx, 
					   unsigned xresrx, 
					   unsigned yresrx, 
					   unsigned fourcc, 
					   unsigned xres, 
					   unsigned yres, 
					   unsigned framerate, 
					   unsigned quality);	
private:
	IVPSTACK *vps_;
	VPCALL vpcall_;
	VIDEOCODEC *decoder_;
	UIImageView *imageView_;
	uint8_t *frame_;
	unsigned frameWidth_;
	unsigned frameHeight_;
	CGColorSpaceRef colorspaceRef_;  /// cached colorspace
	CGDataProviderRef dataProviderRef_; /// cached dataprovider
    ImageDrawer *drawer_;
};


class VPIOSVIDEODATAFACTORY : public VPVIDEODATAFACTORY
{
public:
	VPIOSVIDEODATAFACTORY();
	~VPIOSVIDEODATAFACTORY();
	
	virtual VPVIDEODATA *New(IVPSTACK *vps, 
							 VPCALL vpcall, 
							 unsigned fourccrx, 
							 unsigned xresrx, 
							 unsigned yresrx, 
							 unsigned fourcc, 
							 unsigned xres, 
							 unsigned yres, 
							 unsigned framerate, 
							 unsigned videoquality);
};




#endif