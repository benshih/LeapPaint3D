/******************************************************************************\
* Modified by Benjamin Shih.
* Initially from EPFL Hackathon May 2014.
* Based off of FingerVisualizer and SampleListener examples.


* Copyright (C) Leap Motion, Inc. 2011-2013.                                   *
* Leap Motion proprietary and  confidential.  Not for distribution.            *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement between *
* Leap Motion and you, your company or other organization.                     *
\******************************************************************************/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Leap.h"
#include "LeapUtilGL.h"
#include <cctype>
#include <vector>

using namespace Leap;

class FingerVisualizerWindow;
class OpenGLCanvas;

// intermediate class for convenient conversion from JUCE color
// to float vector argument passed to GL functions
struct GLColor 
{
  GLColor() : r(1), g(1), b(1), a(1) {}

  GLColor( float _r, float _g, float _b, float _a=1 )
    : r(_r), g(_g), b(_b), a(_a)
  {}

  explicit GLColor( const Colour& juceColor ) 
    : r(juceColor.getFloatRed()), 
      g(juceColor.getFloatGreen()),
      b(juceColor.getFloatBlue()),
      a(juceColor.getFloatAlpha())
  {}

  operator const GLfloat*() const { return &r; }

  GLfloat r, g, b, a; 
};


class SampleListener : public Listener {
public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
};

void SampleListener::onInit(const Controller& controller) {
    std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
    controller.enableGesture(Gesture::TYPE_CIRCLE);
    controller.enableGesture(Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
    
}

void SampleListener::onDisconnect(const Controller& controller) {
    //Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
    std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();
    std::cout << "Frame id: " << frame.id()
    << ", timestamp: " << frame.timestamp()
    << ", hands: " << frame.hands().count()
    << ", fingers: " << frame.fingers().count()
    << ", tools: " << frame.tools().count()
    << ", gestures: " << frame.gestures().count() << std::endl;
    
    if (!frame.hands().isEmpty()) {
        // Get the first hand
        const Hand hand = frame.hands()[0];
        
        // Check if the hand has any fingers
        const FingerList fingers = hand.fingers();
        if (!fingers.isEmpty()) {
            // Calculate the hand's average finger tip position
            Vector avgPos;
            for (int i = 0; i < fingers.count(); ++i) {
                avgPos += fingers[i].tipPosition();
            }
            avgPos /= (float)fingers.count();
            std::cout << "Hand has " << fingers.count()
            << " fingers, average finger tip position" << avgPos << std::endl;
        }
        
        // Get the hand's sphere radius and palm position
        std::cout << "Hand sphere radius: " << hand.sphereRadius()
        << " mm, palm position: " << hand.palmPosition() << std::endl;
        
        // Get the hand's normal vector and direction
        const Vector normal = hand.palmNormal();
        const Vector direction = hand.direction();
        
        // Calculate the hand's pitch, roll, and yaw angles
        std::cout << "Hand pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
        << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
        << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;
    }
    
    // Get gestures
    const GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        Gesture gesture = gestures[g];
        
        switch (gesture.type()) {
            case Gesture::TYPE_CIRCLE:
            {
                CircleGesture circle = gesture;
                std::string clockwiseness;
                
                if (circle.pointable().direction().angleTo(circle.normal()) <= PI/4) {
                    clockwiseness = "clockwise";
                } else {
                    clockwiseness = "counterclockwise";
                }
                
                // Calculate angle swept since last frame
                float sweptAngle = 0;
                if (circle.state() != Gesture::STATE_START) {
                    CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
                    sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
                }
                std::cout << "Circle id: " << gesture.id()
                << ", state: " << gesture.state()
                << ", progress: " << circle.progress()
                << ", radius: " << circle.radius()
                << ", angle " << sweptAngle * RAD_TO_DEG
                <<  ", " << clockwiseness << std::endl;
                break;
            }
            case Gesture::TYPE_SWIPE:
            {
                SwipeGesture swipe = gesture;
                std::cout << "Swipe id: " << gesture.id()
                << ", state: " << gesture.state()
                << ", direction: " << swipe.direction()
                << ", speed: " << swipe.speed() << std::endl;
                break;
            }
            case Gesture::TYPE_KEY_TAP:
            {
                KeyTapGesture tap = gesture;
                std::cout << "Key Tap id: " << gesture.id()
                << ", state: " << gesture.state()
                << ", position: " << tap.position()
                << ", direction: " << tap.direction()<< std::endl;
                break;
            }
            case Gesture::TYPE_SCREEN_TAP:
            {
                ScreenTapGesture screentap = gesture;
                std::cout << "Screen Tap id: " << gesture.id()
                << ", state: " << gesture.state()
                << ", position: " << screentap.position()
                << ", direction: " << screentap.direction()<< std::endl;
                break;
            }
            default:
                std::cout << "Unknown gesture type." << std::endl;
                break;
        }
    }
    
    if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
        std::cout << std::endl;
    }
}

void SampleListener::onFocusGained(const Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
}
//==============================================================================
class FingerVisualizerApplication  : public JUCEApplication
{
public:
    //==============================================================================
    
    // Create a sample listener and controller
    SampleListener listener;
    Controller controller;
    
//    // Allocate point array.
//    size_t size = 100000;
////    std::vector<std::vector<int>> points;
////    for(int i = 0; i < size; i++)
////    {
////        vector<int> row;
////        points.push_back(row);
////    }
    
    FingerVisualizerApplication()
    {
        // Have the sample listener receive events from the controller
        controller.addListener(listener);
    }

    ~FingerVisualizerApplication()
    {
    }

    //==============================================================================
    void initialise (const String& commandLine);

    void shutdown()
    {
        // Do your application's shutdown code here..
        // Remove the sample listener when done
        controller.removeListener(listener);
    }

    //==============================================================================
    void systemRequestedQuit()
    {
        quit();
    }

    //==============================================================================
    const String getApplicationName()
    {
        return "Leap Finger Visualizer";
    }

    const String getApplicationVersion()
    {
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }

    void anotherInstanceStarted (const String& commandLine)
    {
      (void)commandLine;        
    }

    static Leap::Controller& getController() 
    {
        static Leap::Controller s_controller;

        return  s_controller;
    }

private:
    ScopedPointer<FingerVisualizerWindow>  m_pMainWindow; 
};

//==============================================================================
class OpenGLCanvas  : public Component,
                      public OpenGLRenderer,
                      Leap::Listener
{
public:
    OpenGLCanvas()
      : Component( "OpenGLCanvas" )
    {
        m_openGLContext.setRenderer (this);
        m_openGLContext.setComponentPaintingEnabled (true);
        m_openGLContext.attachTo (*this);
        setBounds( 0, 0, 1024, 768 );

        m_fLastUpdateTimeSeconds = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks());
        m_fLastRenderTimeSeconds = m_fLastUpdateTimeSeconds;

        FingerVisualizerApplication::getController().addListener( *this );

        initColors();

        resetCamera();

        setWantsKeyboardFocus( true );

        m_bPaused = false;

        m_fFrameScale = 0.0075f;
        m_mtxFrameTransform.origin = Leap::Vector( 0.0f, -2.0f, 0.5f );
        m_fPointableRadius = 0.05f;

        m_bShowHelp = false;

        m_strHelp = "ESC - quit\n"
                    "h - Toggle help and frame rate display\n"
                    "p - Toggle pause\n"
                    "Mouse Drag  - Rotate camera\n"
                    "Mouse Wheel - Zoom camera\n"
                    "Arrow Keys  - Rotate camera\n"
                    "Space       - Reset camera";

        //m_strPrompt = "Press 'h' for help";
        m_strPrompt = "";
        
    }

    ~OpenGLCanvas()
    {
        FingerVisualizerApplication::getController().removeListener( *this );
        m_openGLContext.detach();
    }

    void newOpenGLContextCreated()
    {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDepthFunc(GL_LESS);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glShadeModel(GL_SMOOTH);

        glEnable(GL_LIGHTING);

        m_fixedFont = Font("Courier New", 24, Font::plain );
    }

    void openGLContextClosing()
    {
    }

    bool keyPressed( const KeyPress& keyPress )
    {
      int iKeyCode = toupper(keyPress.getKeyCode());

      if ( iKeyCode == KeyPress::escapeKey )
      {
        JUCEApplication::quit();
        return true;
      }

      if ( iKeyCode == KeyPress::upKey )
      {
        m_camera.RotateOrbit( 0, 0, LeapUtil::kfHalfPi * -0.05f );
        return true;
      }

      if ( iKeyCode == KeyPress::downKey )
      {
        m_camera.RotateOrbit( 0, 0, LeapUtil::kfHalfPi * 0.05f );
        return true;
      }

      if ( iKeyCode == KeyPress::leftKey )
      {
        m_camera.RotateOrbit( 0, LeapUtil::kfHalfPi * -0.05f, 0 );
        return true;
      }

      if ( iKeyCode == KeyPress::rightKey )
      {
        m_camera.RotateOrbit( 0, LeapUtil::kfHalfPi * 0.05f, 0 );
        return true;
      }

      switch( iKeyCode )
      {
      case ' ':
        resetCamera();
        break;
      case 'C': // clear canvas
        for(int i = 0; i < count; i++)
        {
            std::cout << "clear" << std::endl;
        }
        count = 0;
        break;
      case 'H':
        m_bShowHelp = !m_bShowHelp;
        break;
      case 'P':
        m_bPaused = !m_bPaused;
        break;
      default:
        return false;
      }

      return true;
    }

    void mouseDown (const MouseEvent& e)
    {
        m_camera.OnMouseDown( LeapUtil::FromVector2( e.getPosition() ) );
    }

    void mouseDrag (const MouseEvent& e)
    {
        m_camera.OnMouseMoveOrbit( LeapUtil::FromVector2( e.getPosition() ) );
        m_openGLContext.triggerRepaint();
    }

    void mouseWheelMove ( const MouseEvent& e,
                          const MouseWheelDetails& wheel )
    {
      (void)e;
      m_camera.OnMouseWheel( wheel.deltaY );
      m_openGLContext.triggerRepaint();
    }

    void resized()
    {
    }

    void paint(Graphics&)
    {
    }

    
    // BAD: Initialize points here.
    int count = 0;
    Leap::Vector points[1000000];
    
    
    void renderOpenGL2D()
    {
        //count += 1;
        LeapUtilGL::GLAttribScope attribScope( GL_ENABLE_BIT );

        // when enabled text draws poorly.
        glDisable(GL_CULL_FACE);

        ScopedPointer<LowLevelGraphicsContext> glRenderer (createOpenGLGraphicsContext (m_openGLContext, getWidth(), getHeight()));

        if (glRenderer != nullptr)
        {
            Graphics g(*glRenderer.get());

            int iMargin   = 10;
            int iFontSize = static_cast<int>(m_fixedFont.getHeight());
            int iLineStep = iFontSize + (iFontSize >> 2);
            int iBaseLine = 20;
            Font origFont = g.getCurrentFont();

            const Rectangle<int>& rectBounds = getBounds();

            if ( m_bShowHelp )
            {
                g.setColour( Colours::seagreen );
                g.setFont( static_cast<float>(iFontSize) );

                if ( !m_bPaused )
                {
                  g.drawSingleLineText( m_strUpdateFPS, iMargin, iBaseLine );
                }

                g.drawSingleLineText( m_strRenderFPS, iMargin, iBaseLine + iLineStep );

                g.setFont( m_fixedFont );
                g.setColour( Colours::slateblue );

                g.drawMultiLineText(  m_strHelp,
                                      iMargin,
                                      iBaseLine + iLineStep * 3,
                                      rectBounds.getWidth() - iMargin*2 );
            }

            g.setFont( origFont );
            g.setFont( static_cast<float>(iFontSize) );

            g.setColour( Colours::salmon );
            g.drawMultiLineText(  m_strPrompt,
                                  iMargin,
                                  rectBounds.getBottom() - (iFontSize + iFontSize + iLineStep),
                                  rectBounds.getWidth()/4 );
            
            // Ben modification
            //g.drawLine(5, 5, 500, 500);
            // Get the most recent frame and report some basic information
            const Frame frame = FingerVisualizerApplication::getController().frame();
            std::cout << "Frame id: " << frame.id()
            << ", timestamp: " << frame.timestamp()
            << ", hands: " << frame.hands().count()
            << ", fingers: " << frame.fingers().count()
            << ", tools: " << frame.tools().count()
            << ", gestures: " << frame.gestures().count() << std::endl;
            
            if (!frame.hands().isEmpty()) {
                // Get the first hand
                const Hand hand = frame.hands()[0];
                
                // Check if the hand has any fingers
                const FingerList fingers = hand.fingers();
                if (!fingers.isEmpty()) {
                    // Calculate the hand's average finger tip position
                    Vector avgPos;
                    for (int i = 0; i < fingers.count(); ++i) {
                        avgPos += fingers[i].tipPosition();
                    }
                    avgPos /= (float)fingers.count();
                    std::cout << "Hand has " << fingers.count()
                    << " fingers, average finger tip position" << avgPos << std::endl;
                }
                
                // Get the hand's sphere radius and palm position
                std::cout << "Hand sphere radius: " << hand.sphereRadius()
                << " mm, palm position: " << hand.palmPosition() << std::endl;
                
                // Get the hand's normal vector and direction
                const Vector normal = hand.palmNormal();
                const Vector direction = hand.direction();
                
                // Calculate the hand's pitch, roll, and yaw angles
                std::cout << "Hand pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
                << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
                << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;
                
                
                // 3DPaint: If one finger is detected, record those points.
                if(1 == fingers.count())
                {
                    points[count] = fingers.leftmost().tipPosition(); //bad
                    count += 1;
                }
//                else if(2 == fingers.count())//needed in order to 'lift pen'
//                {
//                    points[count-1].x = -1; // bad.
//                }
                
                
            }
            

        }
    }

    //
    // calculations that should only be done once per leap data frame but may be drawn many times should go here.
    //   
    void update( Leap::Frame frame )
    {
        ScopedLock sceneLock(m_renderMutex);

        double curSysTimeSeconds = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks());

        float deltaTimeSeconds = static_cast<float>(curSysTimeSeconds - m_fLastUpdateTimeSeconds);
      
        m_fLastUpdateTimeSeconds = curSysTimeSeconds;
        float fUpdateDT = m_avgUpdateDeltaTime.AddSample( deltaTimeSeconds );
        float fUpdateFPS = (fUpdateDT > 0) ? 1.0f/fUpdateDT : 0.0f;
        m_strUpdateFPS = String::formatted( "UpdateFPS: %4.2f", fUpdateFPS );
    }

    /// affects model view matrix.  needs to be inside a glPush/glPop matrix block!
    void setupScene()
    {
        OpenGLHelpers::clear (Colours::black.withAlpha (1.0f));

        m_camera.SetAspectRatio( getWidth() / static_cast<float>(getHeight()) );

        m_camera.SetupGLProjection();

        m_camera.ResetGLView();

        // left, high, near - corner light
        LeapUtilGL::GLVector4fv vLight0Position( -3.0f, 3.0f, -3.0f, 1.0f );
        // right, near - side light
        LeapUtilGL::GLVector4fv vLight1Position(  3.0f, 0.0f, -1.5f, 1.0f );
        // near - head light
        LeapUtilGL::GLVector4fv vLight2Position( 0.0f, 0.0f,  -3.0f, 1.0f );

        /// JUCE turns off the depth test every frame when calling paint.
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GLColor(Colours::darkgrey));

        glLightfv(GL_LIGHT0, GL_POSITION, vLight0Position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, GLColor(Colour(0.5f, 0.40f, 0.40f, 1.0f)));
        glLightfv(GL_LIGHT0, GL_AMBIENT, GLColor(Colours::black));

        glLightfv(GL_LIGHT1, GL_POSITION, vLight1Position);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, GLColor(Colour(0.0f, 0.0f, 0.25f, 1.0f)));
        glLightfv(GL_LIGHT1, GL_AMBIENT, GLColor(Colours::black));

        glLightfv(GL_LIGHT2, GL_POSITION, vLight2Position);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, GLColor(Colour(0.15f, 0.15f, 0.15f, 1.0f)));
        glLightfv(GL_LIGHT2, GL_AMBIENT, GLColor(Colours::black));

        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);

        m_camera.SetupGLView();
    }

    // data should be drawn here but no heavy calculations done.
    // any major calculations that only need to be updated per leap data frame
    // should be handled in update and cached in members.
    void renderOpenGL()
    {
		    {
			      MessageManagerLock mm (Thread::getCurrentThread());
			      if (! mm.lockWasGained())
				      return;
		    }

        Leap::Frame frame = m_lastFrame;

        double  curSysTimeSeconds = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks());
        float   fRenderDT = static_cast<float>(curSysTimeSeconds - m_fLastRenderTimeSeconds);
        fRenderDT = m_avgRenderDeltaTime.AddSample( fRenderDT );
        m_fLastRenderTimeSeconds = curSysTimeSeconds;

        float fRenderFPS = (fRenderDT > 0) ? 1.0f/fRenderDT : 0.0f;

        m_strRenderFPS = String::formatted( "RenderFPS: %4.2f", fRenderFPS );

        LeapUtilGL::GLMatrixScope sceneMatrixScope;

        setupScene();
        
        
        // Draw the points.
        Leap::Vector vStartPos = m_mtxFrameTransform.transformPoint(points[0] * m_fFrameScale);
        Leap::Vector vEndPos;
        for(int i = 1; i < count; i++)
        {
            vEndPos = m_mtxFrameTransform.transformPoint( points[i] * m_fFrameScale);
            
            //glTranslatef( vStartPos.x, vStartPos.y, vStartPos.z );
            

            glBegin(GL_LINES);
            
            glVertex3f( vStartPos.x, vStartPos.y, vStartPos.z);
            glVertex3f( vEndPos.x, vEndPos.y, vEndPos.z );
            
            glEnd();
            
            
            
            std::cout << "Number of points being rendered: " << count << std::endl;
            vStartPos = vEndPos;
        }
        
//        // draw the grid background
//        {
//            LeapUtilGL::GLAttribScope colorScope(GL_CURRENT_BIT);
//
//            glColor3f( 0, 0, 1 );
//
//            {
//                LeapUtilGL::GLMatrixScope gridMatrixScope;
//
//                glTranslatef( 0, 0.0f, -1.5f );
//
//                glScalef( 3, 3, 3 );
//
//                LeapUtilGL::drawGrid( LeapUtilGL::kPlane_XY, 20, 20 );
//            }
//
//            {
//                LeapUtilGL::GLMatrixScope gridMatrixScope;
//
//                glTranslatef( 0, -1.5f, 0.0f );
//
//                glScalef( 3, 3, 3 );
//
//                LeapUtilGL::drawGrid( LeapUtilGL::kPlane_ZX, 20, 20 );
//            }
//        }
        
        {
            
        }

        // draw fingers/tools as lines with sphere at the tip.
        drawPointables( frame );

        {
          ScopedLock renderLock(m_renderMutex);

          // draw the text overlay
          renderOpenGL2D();
        }
    }

    void drawPointables( Leap::Frame frame )
    {
        LeapUtilGL::GLAttribScope colorScope( GL_CURRENT_BIT | GL_LINE_BIT );

        const Leap::PointableList& pointables = frame.pointables();

        const float fScale = m_fPointableRadius;

        glLineWidth( 3.0f );

        for ( size_t i = 0, e = pointables.count(); i < e; i++ )
        {
            const Leap::Pointable&  pointable   = pointables[i];
            Leap::Vector            vStartPos   = m_mtxFrameTransform.transformPoint( pointable.tipPosition() * m_fFrameScale );
            Leap::Vector            vEndPos     = m_mtxFrameTransform.transformDirection( pointable.direction() ) * -0.25f;
            const uint32_t          colorIndex  = static_cast<uint32_t>(pointable.id()) % kNumColors;

            glColor3fv( m_avColors[colorIndex].toFloatPointer() );

            {
                LeapUtilGL::GLMatrixScope matrixScope;

                glTranslatef( vStartPos.x, vStartPos.y, vStartPos.z );

                glBegin(GL_LINES);

                glVertex3f( 0, 0, 0 );
                glVertex3fv( vEndPos.toFloatPointer() );

                glEnd();

                glScalef( fScale, fScale, fScale );

                LeapUtilGL::drawSphere( LeapUtilGL::kStyle_Solid );
            }
        }
    }

    virtual void onInit(const Leap::Controller&) 
    {
    }

    virtual void onConnect(const Leap::Controller&) 
    {
    }

    virtual void onDisconnect(const Leap::Controller&) 
    {
    }

    virtual void onFrame(const Leap::Controller& controller)
    {
        if ( !m_bPaused )
        {
          Leap::Frame frame = controller.frame();
          update( frame );
          m_lastFrame = frame;
          m_openGLContext.triggerRepaint();
        }
    }

    void resetCamera()
    {
        m_camera.SetOrbitTarget( Leap::Vector::zero() );
        m_camera.SetPOVLookAt( Leap::Vector( 0, 2, 4 ), m_camera.GetOrbitTarget() );
    }

    void initColors()
    {
      float fMin      = 0.0f;
      float fMax      = 1.0f;
      float fNumSteps = static_cast<float>(pow( kNumColors, 1.0/3.0 ));
      float fStepSize = (fMax - fMin)/fNumSteps;
      float fR = fMin, fG = fMin, fB = fMin;

      for ( uint32_t i = 0; i < kNumColors; i++ )
      {
        m_avColors[i] = Leap::Vector( fR, fG, LeapUtil::Min(fB, fMax) );

        fR += fStepSize;

        if ( fR > fMax )
        {
          fR = fMin;
          fG += fStepSize;

          if ( fG > fMax )
          {
            fG = fMin;
            fB += fStepSize;
          }
        }
      }

      Random rng(0x13491349);

      for ( uint32_t i = 0; i < kNumColors; i++ )
      {
        uint32_t      uiRandIdx    = i + (rng.nextInt() % (kNumColors - i));
        Leap::Vector  vSwapTemp    = m_avColors[i];

        m_avColors[i]          = m_avColors[uiRandIdx];
        m_avColors[uiRandIdx]  = vSwapTemp;
      }
    }

private:
    OpenGLContext               m_openGLContext;
    LeapUtilGL::CameraGL        m_camera;
    Leap::Frame                 m_lastFrame;
    double                      m_fLastUpdateTimeSeconds;
    double                      m_fLastRenderTimeSeconds;
    Leap::Matrix                m_mtxFrameTransform;
    float                       m_fFrameScale;
    float                       m_fPointableRadius;
    LeapUtil::RollingAverage<>  m_avgUpdateDeltaTime;
    LeapUtil::RollingAverage<>  m_avgRenderDeltaTime;
    String                      m_strUpdateFPS;
    String                      m_strRenderFPS;
    String                      m_strPrompt;
    String                      m_strHelp;
    Font                        m_fixedFont;
    CriticalSection             m_renderMutex;
    bool                        m_bShowHelp;
    bool                        m_bPaused;

    enum  { kNumColors = 256 };
    Leap::Vector            m_avColors[kNumColors];
};

//==============================================================================
/**
    This is the top-level window that we'll pop up. Inside it, we'll create and
    show a component from the MainComponent.cpp file (you can open this file using
    the Jucer to edit it).
*/
class FingerVisualizerWindow  : public DocumentWindow
{
public:
    //==============================================================================
    FingerVisualizerWindow()
        : DocumentWindow ("Leap Finger Visualizer",
                          Colours::lightgrey,
                          DocumentWindow::allButtons,
                          true)
    {
        setContentOwned (new OpenGLCanvas(), true);

        // Centre the window on the screen
        centreWithSize (getWidth(), getHeight());

        // And show it!
        setVisible (true);

        getChildComponent(0)->grabKeyboardFocus();
    }

    ~FingerVisualizerWindow()
    {
        // (the content component will be deleted automatically, so no need to do it here)
    }

    //==============================================================================
    void closeButtonPressed()
    {
        // When the user presses the close button, we'll tell the app to quit. This
        JUCEApplication::quit();
    }
};

void FingerVisualizerApplication::initialise (const String& commandLine)
{
    (void) commandLine;
    // Do your application's initialisation code here..
    m_pMainWindow = new FingerVisualizerWindow();
}

//==============================================================================
// This macro generates the main() routine that starts the app.
START_JUCE_APPLICATION(FingerVisualizerApplication)
