//
//  main.cpp
//  Horizon_xpl
//
//  Created by 冀宸 on 2021/10/15.
//

#include <cstring>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"

// An opaque handle to the window we will create.
static XPLMWindowID g_window;

// Callbacks we will register when we create our window.
void draw_hello_world(XPLMWindowID in_window_id, void *in_refcon);
inline int dummy_mouse_handler(XPLMWindowID in_window_id,
                               int x, int y, int is_down,
                               void *in_refcon) {
    return 0;
}
inline void dummy_key_handler(XPLMWindowID in_window_id,
                              char key, XPLMKeyFlags flags, char virtual_key,
                              void *in_refcon, int losing_focus) {
    return;
}
inline XPLMCursorStatus dummy_cursor_status_handler(XPLMWindowID in_window_id,
                                                    int x, int y,
                                                    void *in_refcon) {
    return xplm_CursorDefault;
}
inline int dummy_wheel_handler(XPLMWindowID in_window_id,
                               int x, int y, int wheel, int clicks,
                               void *in_refcon) {
    return 0;
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
    strcpy(outName, "Horizon_xpl");
    strcpy(outSig, "jichen.Horizon_xpl");
    strcpy(outDesc, "An auto pilot plug-in for X-Plane 11.");

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    params.visible = 1;
    params.drawWindowFunc = draw_hello_world;
    // Note on "dummy" handlers:
    // Even if we don't want to handle these events, we have to register a
    // "do-nothing" callback for them.
    params.handleMouseClickFunc = dummy_mouse_handler;
#if defined(XPLM300)
    params.handleRightClickFunc = dummy_mouse_handler;
#endif /* XPLM300 */
    params.handleKeyFunc = dummy_key_handler;
    params.handleCursorFunc = dummy_cursor_status_handler;
    params.handleMouseWheelFunc = dummy_wheel_handler;
    params.refcon = nullptr;
#if defined(XPLM301)
    params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
#endif /* XPLM301 */
#if defined(XPLM300)
    params.layer = xplm_WindowLayerFloatingWindows;
#endif /* XPLM300 */
    // Set the window's initial bounds
    // Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
    // We'll need to query for the global desktop bounds!
    int left, top, right, bottom;
    XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
    params.top = top - 50;
    params.right = right - 50;
    params.bottom = params.top - 200;
    params.left = params.right - 200;

    g_window = XPLMCreateWindowEx(&params);

    // Position the window as a "free" floating window, which the user can drag around.
    XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, -1);
    // Limit resizing our window: maintain a minimum width/height of 100 boxels
    // and a max width/height of 300 boxels.
    XPLMSetWindowResizingLimits(g_window, 100, 100, 300, 300);
    XPLMSetWindowTitle(g_window, "Sample Window");

    return g_window != nullptr;
}

PLUGIN_API void XPluginStop(void) {
    // Since we created the window, we'll be good citizens and clean it up.
    XPLMDestroyWindow(g_window);
    g_window = nullptr;
}

PLUGIN_API void XPluginDisable(void) {
    return;
}

PLUGIN_API int XPluginEnable(void) {
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void *inParam) {
    return;
}

void draw_hello_world(XPLMWindowID in_window_id, void *in_refcon) {
    // Mandatory: We *must* set the OpenGL state before drawing
    // (we can't make any assumptions about it).
    XPLMSetGraphicsState(0, // no fog
                         0, // 0 texture units
                         0, // no lighting
                         0, // no alpha testing
                         1, // do alpha blend
                         1, // do depth testing
                         0); // no depth writing

    int l, t, r, b;
    XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

    float col_white[] = {1.0, 1.0, 1.0}; // red, green, blue

    XPLMDrawString(col_white, l + 10, t - 20,
                   (char *) "Hello World!",
                   nullptr, xplmFont_Proportional);
}
