//
//  CanvasLayer.cpp
//  drawstory
//
//  Created by 张 靖宇 on 12-4-3.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "CanvasLayer.h"

using namespace cocos2d;


enum { kTagRenderTexture = 1000 };
enum { kCanvasHeight = 333 };

CanvasLayer::CanvasLayer() : brush_(NULL), drawing_(false), target_(NULL) {    
}


CanvasLayer::~CanvasLayer() {
    CC_SAFE_RELEASE(target_);
    delete brush_;
}


bool CanvasLayer::init(){
    bool result = false; 
    do {
        CC_BREAK_IF(!CCLayer::init());
        
        CCSize winSize = CCDirector::sharedDirector()->getWinSize();
        
        target_ = CCRenderTexture::renderTextureWithWidthAndHeight(winSize.width, kCanvasHeight);
        CC_BREAK_IF(!target_);
        target_->retain();
        target_->setPosition(CCPointMake(winSize.width * 0.5f, kCanvasHeight * 0.5f));
        addChild(target_,0,kTagRenderTexture);
        
        brush_ = new Brush;
        CC_BREAK_IF(!brush_ || !brush_->init());
        
        // initialize texture to white
        target_->beginWithClear(255, 255, 255, 255);
        target_->end(true);

        
        setContentSize(CCSizeMake(winSize.width,kCanvasHeight));
        layerRect_ = CCRectMake(0, 0, winSize.width, kCanvasHeight);
        
        result = true;
    } while (0);
    return result;
}



void CanvasLayer::onEnter() {
    CCLayer::onEnter();
    CCTouchDispatcher::sharedDispatcher()->addTargetedDelegate(this, kTouchPriorityCanvasLayer, true);
}

void CanvasLayer::onExit() {
    CCLayer::onExit();
    CCTouchDispatcher::sharedDispatcher()->removeDelegate(this);
}


bool CanvasLayer::ccTouchBegan(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) {
    CCPoint localPos = convertTouchToNodeSpace(touch);
    if (CCRect::CCRectContainsPoint(layerRect_, localPos)) {
        
        drawing_ = true;
        previousLocalPosition_ = localPos;
        commandQueue_.beginCommand(new DrawCommand(brush_->width(),brush_->color()));
        
        return true;
    }
    
    return false;
}

void CanvasLayer::ccTouchMoved(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) {
    if(drawing_){
        CCPoint localPos = convertTouchToNodeSpace(touch);
        if(CCRect::CCRectContainsPoint(layerRect_, localPos)) {
            
            float distance = ccpDistance(previousLocalPosition_, localPos);
            if(distance > 1){
                // push to command
                static_cast<DrawCommand*>(commandQueue_.current())->push(localPos);
                
                target_->begin();
                
                int d = static_cast<int>(distance);
                for(int i = 0; i < d; ++i){
                    float difx = localPos.x - previousLocalPosition_.x;
                    float dify = localPos.y - previousLocalPosition_.y;
                    float delta = static_cast<float>(i) / distance;
                    brush_->setPosition(CCPointMake(previousLocalPosition_.x + (difx * delta),
                                                    previousLocalPosition_.y + (dify * delta)));
                    brush_->visit();
                }
                target_->end(false);
                previousLocalPosition_ = localPos;
            }
        } else {
            drawing_ = false;
            
            target_->begin();
            target_->end(true);
        }
    }
}



void CanvasLayer::ccTouchEnded(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) {
    if(drawing_){
        CCPoint localPos = convertTouchToNodeSpace(touch);
        if(CCRect::CCRectContainsPoint(layerRect_, localPos)) {
            
            static_cast<DrawCommand*>(commandQueue_.current())->push(localPos);
            
            target_->begin();
            brush_->setPosition(localPos);
            brush_->visit();
            target_->end(true);
        }
        
        drawing_ = false;
    }
}

void CanvasLayer::ccTouchCancelled(cocos2d::CCTouch *touch, cocos2d::CCEvent *event) {
    if(drawing_){
        drawing_ = false;
        
        target_->begin();
        target_->end(true);
    }
}