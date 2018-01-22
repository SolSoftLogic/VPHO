/*
 *  video.mm
 *  vpstack
 */
#import <sys/sysctl.h>
#import <stdio.h>
#import "video.h"

#define MAX_FRAME_SIZE 400*304*4 

NSString *const VPVideoDidStartNotification   = @"VPVideoDidStartNotification";  
NSString *const VPVideoDidFinishNotification  = @"VPVideoDidFinishNotification";

#if !TARGET_IPHONE_SIMULATOR

@implementation Camcorder;

static Camcorder *camcorderSharedInstance = nil;

@synthesize previewView;

#pragma mark ---- Camcorder singleton object methods ----

+ (Camcorder*)sharedInstance 
{
	@synchronized(self) 
	{
		if (camcorderSharedInstance == nil) 
		{
			[[self alloc] init]; // assignment not done here
		}
	}
	return camcorderSharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone 
{
	@synchronized(self) 
	{
		if (camcorderSharedInstance == nil) 
		{
			camcorderSharedInstance= [super allocWithZone:zone];
			return camcorderSharedInstance;  // assignment and return on first allocation
		}
	}
	return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone
{
	return self;
}

- (id)retain 
{
	return self;
}

- (unsigned)retainCount 
{
	return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release 
{
	//do nothing
}

- (id)autorelease 
{
	return self;
}

#pragma mark ---- Camcorder object methods ----

- (BOOL) isFrontCameraAvailable
{
	//  look at all the video devices and get the first one that's on the front
    NSArray *videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in videoDevices)
    {
        if (device.position == AVCaptureDevicePositionFront)
        {
			return YES;
        }
    }
	
	return NO;
}

- (AVCaptureDevice *)frontFacingCameraIfAvailable
{
    //  look at all the video devices and get the first one that's on the front
    NSArray *videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    AVCaptureDevice *captureDevice = nil;
    for (AVCaptureDevice *device in videoDevices)
    {
        if (device.position == AVCaptureDevicePositionFront)
        {
            captureDevice = device;
            break;
        }
    }
	
    //  couldn't find one on the front, so just get the default video device.
    if ( !captureDevice)
    {
        captureDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }
	
    return captureDevice;
}

- (id)init
{	
    self = [super init ];	
	
    previewView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 436)];
    
	char hw_machine[256] = {0};
	size_t hw_machine_size = sizeof(hw_machine) - 1; 
	if (sysctlbyname("hw.machine", hw_machine, &hw_machine_size, 0, 0) == -1)
	{
		NSLog(@"sysctlbyname() fails");
		exit(1);
	}
	if (strcmp(hw_machine, "iPhone1,2") == 0 || strcmp(hw_machine, "iPhone1,1") == 0 ) // 2G or 3G
		pixelFormat = kCVPixelFormatType_32BGRA;
	else // all others modern devices
		pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;		
	
	NSError *error = nil;
	/*AVCaptureDeviceInput *captureInput = [AVCaptureDeviceInput 
										  deviceInputWithDevice:[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] 
										  error:nil];
	*/

	captureInputFrontCamera = [AVCaptureDeviceInput 
							   deviceInputWithDevice:[self frontFacingCameraIfAvailable] 
							   error:nil];
	
	captureInputDefaultCamera = [AVCaptureDeviceInput 
								 deviceInputWithDevice:[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] 
								 error:nil];
	
	captureInput = captureInputFrontCamera?captureInputFrontCamera:captureInputDefaultCamera;
	
	if (!captureInput)
    {
        NSLog(@"Could not get video input: %@", error);
        return self;
    }
	
	/* Setup the output */
	AVCaptureVideoDataOutput *captureOutput = [[AVCaptureVideoDataOutput alloc] init];
	
	/*While a frame is processes in -captureOutput:didOutputSampleBuffer:fromConnection: delegate methods no other frames are added in the queue.
	 If you don't want this behaviour set the property to NO */
	captureOutput.alwaysDiscardsLateVideoFrames = YES; 
	/*We specify a minimum duration for each frame (play with this settings to avoid having too many frames waiting
	 in the queue because it can cause memory issues). It is similar to the inverse of the maximum framerate.
	 In this example we set a min frame duration of 1/10 seconds so a maximum framerate of 10fps. We say that
	 we are not able to process more than 10 frames per second.*/
	captureOutput.minFrameDuration = CMTimeMake(1, 10);
	
	/*We create a serial queue to handle the processing of our frames*/
	dispatch_queue_t queue;
	queue = dispatch_queue_create("vphone.cameraQueue", NULL);
	[captureOutput setSampleBufferDelegate:self queue:queue];
	dispatch_release(queue);
	
	NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey; 
	NSNumber* value = [NSNumber numberWithUnsignedInt:pixelFormat]; 
	
	NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:value forKey:key]; 
	
	[captureOutput setVideoSettings:videoSettings]; 
	
	/* Create a capture session */
	captureSession = [[AVCaptureSession alloc] init];	
    captureSession.sessionPreset = AVCaptureSessionPresetLow; // 192x144 - 3GS/4G, 400x304 - 3G

	/* Add input and output */
    if (![captureSession canAddInput:captureInput])
    {
        NSLog(@"Cannot add an input device");
        return self;
    }
    [captureSession addInput:captureInput];
    
    if (![captureSession canAddOutput:captureOutput])
    {
        NSLog(@"Cannot add an output device");
        return self;
    }
    [captureSession addOutput:captureOutput];
	
	AVCaptureVideoPreviewLayer *previewLayer = [AVCaptureVideoPreviewLayer layerWithSession:captureSession]; 
	previewLayer.frame = previewView.bounds; // Assume you want the preview layer to fil the view. 
	previewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
//    previewLayer.automaticallyAdjustsMirroring = NO;
//    previewLayer.mirrored = NO;
//    previewLayer.orientation =  AVCaptureVideoOrientationPortrait;
	[previewView.layer addSublayer:previewLayer];
    
    
	srcFrame     = new uint8_t[MAX_FRAME_SIZE];
	encodedFrame = new uint8_t[MAX_FRAME_SIZE];
	
//	NSLog(@"Camcorder initialized");
	/*
    NSLog(@"connections: %d", [captureOutput.connections count]);
    AVCaptureConnection *videoConnection = [captureOutput.connections objectAtIndex:0];
    if ([videoConnection isVideoOrientationSupported])
    {
        NSLog(@"Changing orientation to portrain");
        [videoConnection setVideoOrientation:UIDeviceOrientationPortrait];  //[UIDevice currentDevice].orientation];
    }
    */
    
    return self;
}

/* not working */
- (void) switchCamera
{
	@synchronized(self)
	{
		if (![self isFrontCameraAvailable]) return; // nothing to switch
	
		[captureSession beginConfiguration];
		[captureSession removeInput:captureInput];
	
		if (captureInput == captureInputFrontCamera)
		{
			NSLog(@"switch to default camera");
			captureInput = captureInputDefaultCamera;
		}
		else
		{
			NSLog(@"switch to front camera");
			captureInput = captureInputFrontCamera;
		}

		if ([captureSession canAddInput:captureInput]) {
			[captureSession addInput:captureInput];
		}
		else {
			NSLog(@"Cannot add input");
		}

		[captureSession commitConfiguration];

	}
}

- (void) setVPSTACK:(IVPSTACK*)vpstack
{
	@synchronized(self)
	{
		vps = vpstack;
	}
}

- (int) start
{
	@synchronized(self)
	{
		if (captureSession == nil) return -1;
		
		int ret = 0;
		if (clientsCount++ == 0)
		{
			encoder = NewAVC();
			if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)
				ret = encoder->StartEncode(192, 144, F_I420, 10, 53/*??*/);
			else 
				ret = encoder->StartEncode(192, 144, F_BGR24, 10, 53/*??*/);
			
			if (ret < 0)
				NSLog(@"StartEncode() failed");
			else	
			{
				NSLog(@"Capture session started");
				[captureSession startRunning];
			}
		}
									   
		return ret;
	}
}

- (void) stop
{
	@synchronized(self)
	{
		if (captureSession == nil) return;
		
		if (clientsCount && --clientsCount == 0)
		{
			NSLog(@"Capture session finished");
			[captureSession stopRunning];
			encoder->End();
			delete encoder;
		}
	}
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput 
		didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
		fromConnection:(AVCaptureConnection *)connection 
{	
	@synchronized(self)
	{
		if (!clientsCount) return;

		CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer); 
		CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
	
//		NSLog(@"Got image %dx%d", CVPixelBufferGetWidth(pixelBuffer), CVPixelBufferGetHeight(pixelBuffer));
		
		uint8_t *frameToEncode = (uint8_t*)CVPixelBufferGetBaseAddress(pixelBuffer);
		
		if (CVPixelBufferGetPixelFormatType(pixelBuffer) == kCVPixelFormatType_32BGRA)
		{
			// downscale to 192x144 and convert to BGR24
			uint8_t *src = (uint8_t*)CVPixelBufferGetBaseAddress(pixelBuffer);
			uint8_t *dst = srcFrame;
			unsigned i = 0;
			unsigned src_line_size = 400 * 4;
			unsigned dst_line_size = 384 * 4;
//			for (int line = 0; line < 288; line += 2)
			for (int line = 288; line > 0; line -= 2)
			{
				unsigned pos = line * src_line_size;
				for (unsigned c = 0; c < dst_line_size; c += 8)
				{
					*((uint32_t*)&dst[i]) = *((uint32_t*)&src[pos + c]);
					i += 3;
				}
			}

			frameToEncode = srcFrame;
		}
		else if (CVPixelBufferGetPixelFormatType(pixelBuffer) == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)
		{
			unsigned yPlaneSize = CVPixelBufferGetWidth(pixelBuffer) * CVPixelBufferGetHeight(pixelBuffer);
			memcpy(srcFrame, (unsigned char*)CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0), yPlaneSize);
			
			// Copying UV components
			unsigned sizeOfUVPlane  = (CVPixelBufferGetWidth(pixelBuffer)/2)*CVPixelBufferGetHeight(pixelBuffer);
			unsigned char *uvPlanePtr = (unsigned char*)CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);	
			unsigned char *dst[2] = {srcFrame+yPlaneSize, srcFrame+yPlaneSize+sizeOfUVPlane/2};
			unsigned pos[2] = {0};
			for (unsigned i = 0; i < sizeOfUVPlane; i++)
			{
				int idx = (i & 1);	
				dst[idx][pos[idx]++] = uvPlanePtr[i];
			}
			
			frameToEncode = srcFrame;
		}

		
		unsigned encodedFrameSize = MAX_FRAME_SIZE;
		int iskey;
		int ret = encoder->Encode(frameToEncode, 
								  encodedFrame,
								  &encodedFrameSize, &iskey);
		
		CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);

		if (ret < 0 || encodedFrameSize == 0) return;
//#define DEBUG_ENCODED_VIDEO
#ifdef DEBUG_ENCODED_VIDEO
		NSLog(@"encoded frame: ret=%d, size=%d, iskey=%d\n", ret, encodedFrameSize, iskey);
		{
			NSString *tmp = NSTemporaryDirectory();
			char fname[256];
			sprintf(fname, "%s/video.h264", [tmp cStringUsingEncoding:1]);		
				
			static FILE *fh = 0;
			if (fh == 0)
				fh = fopen(fname, "wb");
			if (fh == 0)
				NSLog(@"fh==0");
			else
			{
				if (encodedFrame[0] == 0) // NAL_IDR_SLICE or NAL_SLICE
				{
					fwrite("\0\0\0\1", 4, 1, fh);
					fwrite(encodedFrame+1, encodedFrameSize-1, 1, fh);
				}
				else  // SPS_PPS and NAL_SLICE
				{
					unsigned sps_size = encodedFrame[0] >> 4;
					unsigned pps_size = encodedFrame[0] & 0x0F;
					// write SPS
					fwrite("\0\0\0\1", 4, 1, fh);
					fwrite(encodedFrame+1, sps_size, 1, fh);
					// write PPS
					fwrite("\0\0\0\1", 4, 1, fh);
					fwrite(encodedFrame+1+sps_size, pps_size, 1, fh);
					// write NAL_SLICE
					fwrite("\0\0\0\1", 4, 1, fh);
					fwrite(encodedFrame+1+sps_size+pps_size, 
						   encodedFrameSize-1-sps_size-pps_size, 1, fh);
				}
				fflush(fh);
			}			
		}
#endif		
		
		if(vps)
        {
			VPCALL vpcalls[100];
			int ncalls = vps->EnumCalls(vpcalls, 100, (1<<BC_VIDEO) | (1<<BC_AUDIOVIDEO));
			for(int i = 0; i < ncalls; i++)
			{
				vps->SendVideo(vpcalls[i], 
							   (unsigned short)(/*SampleTime*/0 * 1000), 
							   encodedFrame, 
							   encodedFrameSize, 
							   !!iskey);
			}
        }
	}
}


@end;

#endif /*!TARGET_IPHONE_SIMULATOR*/

#pragma mark ---- ImageDrawer object methods ----

@implementation ImageDrawer;
- (id) initWithImageView:(UIImageView*)imageView
{
    self = [super init];
    
    view = [imageView retain];
    
    return self;
}
- (void) draw:(UIImage*)img
{
    [view setImage:img];
    [img release];
}

- (void) dealloc
{
    [view release];
    
    [super dealloc];
}
@end;


#pragma mark ---- VPIOSVIDEO object methods ----

VPIOSVIDEO::VPIOSVIDEO()
: vps_(0), vpcall_(0), decoder_(0)
 , imageView_(0)
 , frame_(0)
 , colorspaceRef_(0)
 , dataProviderRef_(0)
 , drawer_(0)
{
}

/*virtual*/VPIOSVIDEO::~VPIOSVIDEO()
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
//	NSLog(@"VPIOSVIDEO::~VPIOSVIDEO)");

	// Post notification (on main thread!)
	NSMutableDictionary *userInfo = [[NSMutableDictionary alloc] initWithCapacity:1];
	[userInfo setObject:[NSNumber numberWithUnsignedLong:(unsigned long)vpcall_] forKey:@"VPCALL"];
	NSNotification *note = [NSNotification notificationWithName:VPVideoDidFinishNotification  object:nil userInfo:userInfo];
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread:@selector(postNotification:) withObject:note waitUntilDone:NO];
	
	
	
#if !TARGET_IPHONE_SIMULATOR
	[[Camcorder sharedInstance] performSelectorOnMainThread:@selector(stop) 
	 							withObject:nil waitUntilDone:NO];
#endif
	
	if (decoder_)
	{
		decoder_->End();
		delete decoder_;
	}
	
	if (colorspaceRef_)
		CGColorSpaceRelease(colorspaceRef_);
	
	if (dataProviderRef_)
		CGDataProviderRelease(dataProviderRef_);
	
	delete[] frame_;
		
	if (imageView_) 
		[imageView_ release];
	
    if (drawer_) 
        [drawer_ performSelectorOnMainThread:@selector(release) withObject:nil waitUntilDone:NO];
    
	[pool release];
	
}


/*virtual*/ int VPIOSVIDEO::Create(IVPSTACK *vps, 
								   VPCALL vpcall, 
								   unsigned fourccrx, 
								   unsigned xresrx, 
								   unsigned yresrx, 
								   unsigned fourcc, 
								   unsigned xres, 
								   unsigned yres, 
								   unsigned framerate, 
								   unsigned quality)
{
	int ret = 0;
	vps_         = vps;
	vpcall_      = vpcall;
	frameWidth_  = xresrx;
	frameHeight_ = yresrx;
	
//	NSLog(@"VPIOSVIDEO::Create()");
	
	// Create and init decoder
	frame_ = new uint8_t[frameWidth_*frameHeight_*3];
	colorspaceRef_  = CGColorSpaceCreateDeviceRGB();
	dataProviderRef_ = CGDataProviderCreateWithData(0, 
												    frame_, 
												    frameWidth_*frameHeight_*3,
												    0);
	decoder_   = NewAVC();
	ret = decoder_->StartDecode(frameWidth_, frameHeight_, F_BGR24);
	if (ret < 0)
	{
		NSLog(@"StartDecode() failed");
		return ret;
	}
	
	imageView_ = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, frameWidth_, frameHeight_)];
	//imageView_.transform = CGAffineTransformScale(imageView_.transform, -1.0, 1.0);
	drawer_ = [[ImageDrawer alloc] initWithImageView:imageView_];
    
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	
#if !TARGET_IPHONE_SIMULATOR	
	NSInvocation *inv = [NSInvocation invocationWithMethodSignature:[[Camcorder sharedInstance] methodSignatureForSelector:@selector(start)]];
	[inv setTarget:[Camcorder sharedInstance]];
	[inv setSelector:@selector(start)];
	// if you have arguments, set them up here
	// [inv setArgument:&addressOfSomeArgument atIndex:2]; // starting at 2, since 0 is the target and 1 is the selector
	
	[inv performSelectorOnMainThread:@selector(invoke) withObject:nil waitUntilDone:YES];	
	[inv getReturnValue:&ret];
#endif
	
	
	// Post notification (on main thread!)
	NSMutableDictionary *userInfo = [[[NSMutableDictionary alloc] initWithCapacity:2] autorelease];
	[userInfo setObject:[NSNumber numberWithUnsignedLong:(unsigned long)vpcall_] forKey:@"VPCALL"];
	[userInfo setObject:imageView_ forKey:@"IMAGEVIEW"];
	NSNotification *note = [NSNotification notificationWithName:VPVideoDidStartNotification object:nil userInfo:userInfo];
	[[NSNotificationCenter defaultCenter] performSelectorOnMainThread:@selector(postNotification:) withObject:note waitUntilDone:YES];
	
	[pool release];
	
	return ret;
}


/*virtual*/ void VPIOSVIDEO::VideoFrame(unsigned short timestamp, 
										const unsigned char *buf, 
										int buflen, 
										bool keyframe)
{
//	NSLog(@"VPIOSVIDEO::VideoFrame() timestamp: %d, size=%d, key=%d", timestamp, buflen, keyframe);
	
	int ret = decoder_->Decode(buf, buflen, frame_);
	
	if (ret < 0)
	{
		NSLog(@"Decode() failed");
		return;
	}
	
	CGImageRef imageRef = CGImageCreate (
										 frameWidth_,
										 frameHeight_,
							 			 8,
										 24,
										 frameWidth_ * 3,
										 colorspaceRef_,
										 kCGBitmapByteOrderDefault,
										 dataProviderRef_,
										 NULL,
										 0,
										 kCGRenderingIntentDefault);
	if (imageRef == 0)
	{
		NSLog(@"imageRef == 0");
		return;
	}
	
	UIImage *image = [[UIImage alloc] initWithCGImage:imageRef];
	CGImageRelease(imageRef);
	
    [drawer_ performSelectorOnMainThread:@selector(draw:) withObject:image waitUntilDone:NO];
    
//	[imageView_ performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:NO];
//	[image release];
}


#pragma mark ---- VPIOSVIDEODATAFACTORY object methods ----

VPIOSVIDEODATAFACTORY::VPIOSVIDEODATAFACTORY()
{
}

VPIOSVIDEODATAFACTORY::~VPIOSVIDEODATAFACTORY()
{
}

/*virtual*/ VPVIDEODATA *VPIOSVIDEODATAFACTORY::New(IVPSTACK *vps, 
													VPCALL vpcall, 
													unsigned fourccrx, 
													unsigned xresrx, 
													unsigned yresrx, 
													unsigned fourcc, 
													unsigned xres, 
													unsigned yres, 
													unsigned framerate, 
													unsigned videoquality)
{
	NSLog(@"VPIOSVIDEODATAFACTORY::New(). fourccrx=0x%08x, xresrx=%d, yresrx=%d, fourcc=0x%08x, xres=%d, yres=%d, framerate=%d, videoquality=%d\n"
			, fourccrx, xresrx, yresrx, fourcc, xres, yres, framerate, videoquality);


	VPIOSVIDEO *videodata = new VPIOSVIDEO();
	if (videodata->Create(vps, vpcall, fourccrx, xresrx, yresrx, 
						  fourcc, xres, yres, framerate, videoquality) < 0)
	{
		delete videodata;
		return 0;
	}
	return videodata;
}

