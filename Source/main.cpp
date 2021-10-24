//
//  main.cpp
//  Horizon_xpl
//
//  Created by 冀宸 on 2021/10/15.
//

#include <cstring>
#include "XPLMMenus.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

static XPWidgetID gHorizonXplMainWindow = NULL;
static XPWidgetID gHorizonXplRTAttitude[3];
static XPWidgetID gHorizonXplTGAttitude[3];

static void horizonXplCreateMainWindow(int, int);
static void horizonXplMenuHandler(void*, void*);
static int horizonXplWidgetHandler(XPWidgetMessage, XPWidgetID, long, long);

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    XPLMMenuID menuId;
    int subMenuItem;

    strcpy(outName, "Horizon_xpl");
    strcpy(outSig, "jichen.Horizon_xpl");
    strcpy(outDesc, "An autopilot plugin for X-Plane 11.");

    subMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Horizon_xpl", NULL, 1);
    menuId = XPLMCreateMenu("Horizon_xpl", XPLMFindPluginsMenu(), subMenuItem,
                            horizonXplMenuHandler, NULL);
    XPLMAppendMenuItem(menuId, "AP", (void*) "AP", 1);

    return 1;
}

PLUGIN_API void XPluginStop(void) {
    if (gHorizonXplMainWindow) {
        XPDestroyWidget(gHorizonXplMainWindow, 1);
        gHorizonXplMainWindow = NULL;
        memset(gHorizonXplRTAttitude, NULL, 3);
        memset(gHorizonXplTGAttitude, NULL, 3);
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

static void horizonXplCreateMainWindow(int width, int height) {
    static const char *attitude[3] = {"roll", "Pitch", "YAW"};
    int left, top, right, bottom;
    int row, column;

    // calculate Main Window location
    XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
    top -= 50;
    right -= 50;
    bottom = top - height;
    left = right - width;
    // create Main Window
    gHorizonXplMainWindow = XPCreateWidget(left, top, right, bottom,
                                           1, // visible
                                           "Horizon_xpl AP", // description
                                           1, // root
                                           NULL, // must be NULL for root
                                           xpWidgetClass_MainWindow);
    XPSetWidgetProperty(gHorizonXplMainWindow, xpProperty_MainWindowType,
                        xpMainWindowStyle_Translucent);
    XPSetWidgetProperty(gHorizonXplMainWindow, xpProperty_MainWindowHasCloseBoxes, 1);

    // roll Pitch YAW
    row = top - 30;
    column = left;
    for (int i = 0; i < 3; ++i) {
        int step = width / 4;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        XPCreateWidget(lb, tb, rb, bb,
                       1, // visible
                       attitude[i], // description
                       0, // not root
                       gHorizonXplMainWindow, // belongs to Main Window
                       xpWidgetClass_Caption);
    }

    // Realtime
    row -= 20;
    column = left;
    for (int i = -1; i < 3; ++i) {
        int step = width / 4;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        char *desc = NULL;
        XPWidgetID tmp = NULL;
        if (i < 0) {
            desc = (char*) "Realtime";
        } else {
            desc = (char*) "   0.0";
        }
        tmp = XPCreateWidget(lb, tb, rb, bb,
                             1, // visible
                             desc, // description
                             0, // not root
                             gHorizonXplMainWindow, // belongs to Main Window
                             xpWidgetClass_Caption);
        if (i >= 0) {
            XPSetWidgetProperty(tmp, xpProperty_CaptionLit, 1);
            gHorizonXplRTAttitude[i] = tmp;
        }
    }

    // Target
    row -= 20;
    column = left;
    for (int i = -1; i < 3; ++i) {
        int step = width / 4;
        int lb = column + (i + 1) * step + 10;
        int rb = column + (i + 2) * step - 10;
        int tb = row;
        int bb = tb - 12;
        char *desc = NULL;
        XPWidgetID tmp = NULL;
        if (i < 0) {
            desc = (char*) "Target";
        } else if (i < 2) {
            desc = (char*) "   0.0";
        } else {
            desc = (char*) " n/a";
        }
        tmp = XPCreateWidget(lb, tb, rb, bb,
                             1, // visible
                             desc, // description
                             0, // not root
                             gHorizonXplMainWindow, // belongs to Main Window
                             xpWidgetClass_Caption);
        if (i >= 0) {
            gHorizonXplTGAttitude[i] = tmp;
        }
    }

    XPAddWidgetCallback(gHorizonXplMainWindow, horizonXplWidgetHandler);
}

static void horizonXplMenuHandler(void* /*inMenuRef*/, void *inItemRef) {
    if (strcmp((char*) inItemRef, "AP") == 0) {
        if (!gHorizonXplMainWindow) {
            horizonXplCreateMainWindow(320, 560);
        } else {
            if (!XPIsWidgetVisible(gHorizonXplMainWindow)) {
                XPShowWidget(gHorizonXplMainWindow);
            }
        }
    }
}

static int horizonXplWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget,
                                   long /*inParam1*/, long /*inParam2*/) {
    if (inMessage == xpMessage_CloseButtonPushed) {
        if (inWidget == gHorizonXplMainWindow) {
            XPHideWidget(gHorizonXplMainWindow);
        }
        return 1;
    }
    return 0;
}
