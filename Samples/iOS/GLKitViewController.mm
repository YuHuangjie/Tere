//
//  GLKitViewController.h
//  LFRender
//
//  Created by Yu Huangjie on 2017/12/28.
//  Copyright © 2017年 Plex-vr. All rights reserved.
//

#import "GLKitViewController.h"
#include "LFEngine.h"
#include <string>

@interface GLKitViewController () {
    LFEngine *myEngine;
    int drawFlag;
    int drawWidth;
    int drawHeight;
}

@property (strong, nonatomic) EAGLContext *context;

@end

@implementation GLKitViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    // Create an OpenGL ES context and assign it to the view loaded from storyboard
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    [EAGLContext setCurrentContext:self.context];
    
    // Set animation frame rate
    self.preferredFramesPerSecond = 30;
    
    // Initialize
    NSString *path = [[NSBundle mainBundle] pathForResource:@"model/wuwangfuchai/profile" ofType:@"txt"];
    myEngine = new LFEngine([path UTF8String]);
    myEngine->StartFPSThread();
    
    drawFlag = true;
    drawWidth = 0;
    drawHeight = 0;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
    
    // Release resources
    delete myEngine;
    myEngine = nullptr;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - GLKViewDelegate

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    if (drawWidth != ((GLKView*)self.view).drawableWidth ||
        drawHeight != ((GLKView*)self.view).drawableHeight) {
        drawWidth = ((GLKView*)self.view).drawableWidth;
        drawHeight = ((GLKView*)self.view).drawableHeight;
        myEngine->Resize(drawWidth, drawHeight);
    }
    
    if (drawFlag) {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        myEngine->Draw();
        drawFlag = false;
    }
}

#pragma mark - GLKViewControllerDelegate

- (void)update {
 
}

#pragma mark - UIVIewTouchEvent
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    // We only support single touches, so anyObject retrieves just that touch from touches.
    UITouch *touch = [touches anyObject];
    
    if ([touch view] == self.view) {
        CGPoint touchPoint = [touch locationInView:self.view];
        myEngine->SetUI(1, touchPoint.x, touchPoint.y);
    }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    
    if ([touch view] == self.view) {
        CGPoint touchPoint = [touch locationInView:self.view];
        myEngine->SetUI(0, touchPoint.x, touchPoint.y);
        drawFlag = true;
    }
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    
    if ([touch view] == self.view) {
        // Notify the finger leaving
        CGPoint touchPoint = [touch locationInView:self.view];
        myEngine->SetUI(2, touchPoint.x, touchPoint.y);
        drawFlag = true;
    }
}

@end

