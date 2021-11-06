//
//  main.cpp
//  Horizon_xpl
//
//  Created by 冀宸 on 2021/10/15.
//

#include <sstream>
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

/// Macros
#define PID_LOOP_INTERVAL (1.f / 30.f) // 30 fps

/// Global Variables
// Widgets
static XPWidgetID gHorizonWidgetMainWindow;
static XPWidgetID gHorizonWidgetRTAttitude[3];
static XPWidgetID gHorizonWidgetTGAttitude[3];
static XPWidgetID gHorizonWidgetYoke[3];
static XPWidgetID gHorizonWidgetTrim[3];

// DataRefs
static XPLMDataRef gHorizonDataRefYoke[3];
static XPLMDataRef gHorizonDataRefTrim[3];
static const std::string gHorizonDataRefYokePrefix = "sim/joystick/";
static const std::string gHorizonDataRefTrimPrefix = "sim/flightmodel/controls/";
static const char *gHorizonDataRefYokeDescs[3] = {"yoke_roll_ratio", "yoke_pitch_ratio", "yoke_heading_ratio"};
static const char *gHorizonDataRefTrimDescs[3] = {"ail_trim", "elv_trim", "rud_trim"};

/// Prototypes
static void horizonMenuHandler(void*, void*);
static int horizonWidgetHandler(XPWidgetMessage, XPWidgetID, long, long);
static void horizonCreateMainWindow(int, int);
static float horizonPIDLoop(float, float, int, void*);

/// Functions
PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    XPLMMenuID menuId;
    int subMenuItem;

    strcpy(outName, "Horizon_xpl");
    strcpy(outSig, "jichen.Horizon_xpl");
    strcpy(outDesc, "An autopilot plugin for X-Plane 11.");

    // Menu
    subMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Horizon_xpl", NULL, 1);
    menuId = XPLMCreateMenu("Horizon_xpl", XPLMFindPluginsMenu(), subMenuItem,
                            horizonMenuHandler, NULL);
    XPLMAppendMenuItem(menuId, "AP", (void*) "AP", 1);

    // DataRef
    for (int i = 0; i < 3; ++i) {
        gHorizonDataRefYoke[i] = XPLMFindDataRef((gHorizonDataRefYokePrefix + gHorizonDataRefYokeDescs[i]).c_str());
        gHorizonDataRefTrim[i] = XPLMFindDataRef((gHorizonDataRefTrimPrefix + gHorizonDataRefTrimDescs[i]).c_str());
    }

    // PID loop
    XPLMRegisterFlightLoopCallback(horizonPIDLoop, PID_LOOP_INTERVAL, NULL);

    return 1;
}

PLUGIN_API void XPluginStop(void) {
    // Unregister PID loop
    XPLMUnregisterFlightLoopCallback(horizonPIDLoop, NULL);

    // Destroy menu
    if (gHorizonWidgetMainWindow) {
        XPDestroyWidget(gHorizonWidgetMainWindow, 1);
        gHorizonWidgetMainWindow = NULL;
    }
}

PLUGIN_API void XPluginDisable(void) {
    return;
}

PLUGIN_API int XPluginEnable(void) {
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID /*inFrom*/, int /*inMsg*/, void* /*inParam*/) {
    return;
}

static void horizonMenuHandler(void* /*inMenuRef*/, void *inItemRef) {
    if (strcmp((char*) inItemRef, "AP") == 0) {
        if (!gHorizonWidgetMainWindow) {
            horizonCreateMainWindow(320, 560);
        } else {
            if (!XPIsWidgetVisible(gHorizonWidgetMainWindow)) {
                XPShowWidget(gHorizonWidgetMainWindow);
            }
        }
    }
}

static int horizonWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget,
                                long /*inParam1*/, long /*inParam2*/) {
    if (inMessage == xpMessage_CloseButtonPushed) {
        if (inWidget == gHorizonWidgetMainWindow) {
            XPHideWidget(gHorizonWidgetMainWindow);
        }
        return 1;
    }
    return 0;
}

static void horizonCreateMainWindow(int width, int height) {
    static const char *attitude[3] = {" ROLL", " PITCH", " YAW"};
    int left, top, right, bottom;
    int row, column;

    // calculate Main Window location
    XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
    top -= 50;
    right -= 50;
    bottom = top - height;
    left = right - width;
    // create Main Window
    gHorizonWidgetMainWindow = XPCreateWidget(left, top, right, bottom,
                                              1, // visible
                                              "Horizon_xpl AP", // description
                                              1, // root
                                              NULL, // must be NULL for root
                                              xpWidgetClass_MainWindow);
    XPSetWidgetProperty(gHorizonWidgetMainWindow, xpProperty_MainWindowType,
                        xpMainWindowStyle_Translucent);
    XPSetWidgetProperty(gHorizonWidgetMainWindow, xpProperty_MainWindowHasCloseBoxes, 1);

    // ROLL PITCH YAW
    row = top - 30;
    column = left;
    for (int i = 0; i < 3; ++i) {
        int step = width / 4.f;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        XPCreateWidget(lb, tb, rb, bb,
                       1, // visible
                       attitude[i], // description
                       0, // not root
                       gHorizonWidgetMainWindow, // belongs to Main Window
                       xpWidgetClass_Caption);
    }

    // Realtime Attitude
    row -= 20;
    column = left;
    for (int i = -1; i < 3; ++i) {
        int step = width / 4.f;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        char *desc = NULL;
        XPWidgetID widgetID = NULL;
        if (i < 0) {
            desc = (char*) "Realtime";
        } else {
            desc = (char*) " 0.000";
        }
        widgetID = XPCreateWidget(lb, tb, rb, bb,
                                  1, // visible
                                  desc, // description
                                  0, // not root
                                  gHorizonWidgetMainWindow, // belongs to Main Window
                                  xpWidgetClass_Caption);
        if (i >= 0) {
            XPSetWidgetProperty(widgetID, xpProperty_CaptionLit, 1);
            gHorizonWidgetRTAttitude[i] = widgetID;
        }
    }

    // Target Attitude
    row -= 20;
    column = left;
    for (int i = -1; i < 3; ++i) {
        int step = width / 4.f;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        char *desc = NULL;
        XPWidgetID widgetID = NULL;
        if (i < 0) {
            desc = (char*) "Target";
        } else if (i < 2) {
            desc = (char*) " 0.0000";
        } else {
            desc = (char*) " n/a";
        }
        widgetID = XPCreateWidget(lb, tb, rb, bb,
                                  1, // visible
                                  desc, // description
                                  0, // not root
                                  gHorizonWidgetMainWindow, // belongs to Main Window
                                  xpWidgetClass_Caption);
        if (i >= 0) {
            gHorizonWidgetTGAttitude[i] = widgetID;
        }
    }

    // Yoke
    row -= 20;
    column = left;
    for (int i = -1; i < 3; ++i) {
        int step = width / 4.f;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        char *desc = NULL;
        XPWidgetID widgetID = NULL;
        if (i < 0) {
            desc = (char*) "Yoke";
        } else {
            desc = (char*) " 0.000";
        }
        widgetID = XPCreateWidget(lb, tb, rb, bb,
                                  1, // visible
                                  desc, // description
                                  0, // not root
                                  gHorizonWidgetMainWindow, // belongs to Main Window
                                  xpWidgetClass_Caption);
        if (i >= 0) {
            gHorizonWidgetYoke[i] = widgetID;
        }
    }

    // Trim
    row -= 20;
    column = left;
    for (int i = -1; i < 3; ++i) {
        int step = width / 4.f;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        char *desc = NULL;
        XPWidgetID widgetID = NULL;
        if (i < 0) {
            desc = (char*) "Trim";
        } else {
            desc = (char*) " 0.000";
        }
        widgetID = XPCreateWidget(lb, tb, rb, bb,
                                  1, // visible
                                  desc, // description
                                  0, // not root
                                  gHorizonWidgetMainWindow, // belongs to Main Window
                                  xpWidgetClass_Caption);
        if (i >= 0) {
            gHorizonWidgetTrim[i] = widgetID;
        }
    }

    XPAddWidgetCallback(gHorizonWidgetMainWindow, horizonWidgetHandler);
}

static float horizonPIDLoop(float /*inElapsedSinceLastCall*/,
                            float /*inElapsedTimeSinceLastFlightLoop*/,
                            int /*inCounter*/,
                            void* /*inRefcon*/) {
    float floatData = 0.f;
    char buf[32] = {0};

    for (int i = 0; i < 3; ++i) {
        floatData = XPLMGetDataf(gHorizonDataRefYoke[i]);
        snprintf(buf, sizeof(buf), "% .3f", floatData);
        XPSetWidgetDescriptor(gHorizonWidgetYoke[i], buf);

        floatData = XPLMGetDataf(gHorizonDataRefTrim[i]);
        snprintf(buf, sizeof(buf), "% .3f", floatData);
        XPSetWidgetDescriptor(gHorizonWidgetTrim[i], buf);
    }

    return PID_LOOP_INTERVAL;
}
