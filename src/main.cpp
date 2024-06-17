#include "SDFCreatorGui.h"
int main()
{
    SDFCreatorGui gui;
    gui.run();
    // TODO: Gamepad
    /*if (fabsf(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X)) > 0 ||
           fabsf(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y)) > 0 ||
           fabsf(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X)) > 0 ||
           fabsf(GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y)) > 0)
    {
        ui_mode_gamepad = true;
    }

    if (Vector2Length(GetMouseDelta()) > 0)
    {
        ui_mode_gamepad = false;
    }*/

    // #if DEMO_VIDEO_FEATURES
    //         if (IsKeyPressed(KEY_F))
    //         {
    //             false_color_mode = !false_color_mode;
    //             needs_rebuild = true;
    //         }
    // #endif
    //
    // #ifdef PLATFORM_WEB
    //         int w, h, dpi;
    //
    //         EM_ASM(
    //             {
    //                 var pixelsPerPoint = 1; // window.devicePixelRatio;
    //                 var canvas = document.getElementById('canvas');
    //                 var width = Math.floor(pixelsPerPoint * canvas.clientWidth);
    //                 var height = Math.floor(pixelsPerPoint * canvas.clientHeight);
    //                 canvas.width = width;
    //                 canvas.height = height;
    //                 setValue($0, width, "i32");
    //                 setValue($1, height, "i32");
    //                 setValue($2, pixelsPerPoint, "i32");
    //             },
    //             &w, &h, &dpi);
    //
    //         if (GetScreenWidth() != w || GetScreenHeight() != h)
    //         {
    //             SetWindowSize(w, h);
    //         }
    // #endif

    // TODO: autosave
    //  if (GetTime() - lastSave > 60)
    //  {
    //      save("snapshot");
    //  }

    // TODO:: Drag n Drop
    //  if (IsFileDropped())
    //  {
    //      FilePathList droppedFiles = LoadDroppedFiles();
    //      if (droppedFiles.count > 0)
    //      {
    //          load(droppedFiles.paths[0]);
    //      }
    //      UnloadDroppedFiles(droppedFiles);
    //  }

    return 0;
}
