

// double lastSave;

// void append(char** str1, const char* str2)
// {
//     assert(str1);
//     assert(str2);
//     size_t len = (*str1 ? strlen(*str1) : 0) + strlen(str2);
//
//     char* concatenated = (char*)std::malloc(len + 1);
//     concatenated[0] = 0;
//     assert(concatenated);
//     if (*str1)
//     {
//         strcpy(concatenated, *str1);
//         std::free(*str1);
//     }
//     strcat(concatenated, str2);
//
//     *str1 = concatenated;
// }

/*int RayPlaneIntersection(const Vector3 RayOrigin, const Vector3 RayDirection, const Vector3 PlanePoint,
                         const Vector3 PlaneNormal, Vector3* IntersectionPoint)
{
    float dotProduct =
        (PlaneNormal.x * RayDirection.x) + (PlaneNormal.y * RayDirection.y) + (PlaneNormal.z * RayDirection.z);

    // Check if the ray is parallel to the plane
    if (dotProduct == 0.0f)
    {
        return 0;
    }

    float t = ((PlanePoint.x - RayOrigin.x) * PlaneNormal.x + (PlanePoint.y - RayOrigin.y) * PlaneNormal.y +
               (PlanePoint.z - RayOrigin.z) * PlaneNormal.z) /
              dotProduct;

    IntersectionPoint->x = RayOrigin.x + t * RayDirection.x;
    IntersectionPoint->y = RayOrigin.y + t * RayDirection.y;
    IntersectionPoint->z = RayOrigin.z + t * RayDirection.z;

    return 1;
}

Vector3 WorldToCamera(Vector3 worldPos, Matrix cameraMatrix)
{
    return Vector3Transform(worldPos, cameraMatrix);
}

Vector3 CameraToWorld(Vector3 worldPos, Matrix cameraMatrix)
{
    return Vector3Transform(worldPos, MatrixInvert(cameraMatrix));
}

Vector3 VectorProjection(const Vector3 vectorToProject, const Vector3 targetVector)
{
    float dotProduct = (vectorToProject.x * targetVector.x) + (vectorToProject.y * targetVector.y) +
                       (vectorToProject.z * targetVector.z);

    float targetVectorLengthSquared =
        (targetVector.x * targetVector.x) + (targetVector.y * targetVector.y) + (targetVector.z * targetVector.z);

    float scale = dotProduct / targetVectorLengthSquared;

    Vector3 projection;
    projection.x = targetVector.x * scale;
    projection.y = targetVector.y * scale;
    projection.z = targetVector.z * scale;

    return projection;
}

// Find the point on line p1 to p2 nearest to line p2 to p4
Vector3 NearestPointOnLine(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)
{
    float mua;

    Vector3 p13, p43, p21;
    float d1343, d4321, d1321, d4343, d2121;
    float numer, denom;

    const float EPS = 0.001;

    p13.x = p1.x - p3.x;
    p13.y = p1.y - p3.y;
    p13.z = p1.z - p3.z;
    p43.x = p4.x - p3.x;
    p43.y = p4.y - p3.y;
    p43.z = p4.z - p3.z;
    if (fabs(p43.x) < EPS && fabs(p43.y) < EPS && fabs(p43.z) < EPS)
        return (Vector3){};
    p21.x = p2.x - p1.x;
    p21.y = p2.y - p1.y;
    p21.z = p2.z - p1.z;
    if (fabs(p21.x) < EPS && fabs(p21.y) < EPS && fabs(p21.z) < EPS)
        return (Vector3){};

    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    denom = d2121 * d4343 - d4321 * d4321;
    if (fabs(denom) < EPS)
        return (Vector3){};
    numer = d1343 * d4321 - d1321 * d4343;

    mua = numer / denom;

    return (Vector3){p1.x + mua * p21.x, p1.y + mua * p21.y, p1.z + mua * p21.z};
}

BoundingBox boundingBoxSized(Vector3 center, float size)
{
    return (BoundingBox){
        Vector3SubtractValue(center, size / 2),
        Vector3AddValue(center, size / 2),
    };
}

BoundingBox shapeBoundingBox(Sphere s)
{
    // const float radius = sqrtf(powf(s.size.x, 2) + powf(s.size.y, 2) + powf(s.size.z, 2));
    return (BoundingBox){
        Vector3Subtract(s.pos, s.size),
        Vector3Add(s.pos, s.size),
    };
}*/

// int GuiFloatValueBox(Rectangle bounds, const char* text, float* value, float minValue, float maxValue, bool editMode)
// {
// #if !defined(RAYGUI_VALUEBOX_MAX_CHARS)
// #define RAYGUI_VALUEBOX_MAX_CHARS 32
// #endif
//
//     int result = 0;
//     GuiState state = guiState;
//
//     static char editingTextValue[RAYGUI_VALUEBOX_MAX_CHARS + 1] = "\0";
//     char textValue[RAYGUI_VALUEBOX_MAX_CHARS + 1] = "\0";
//     static float original_value;
//     static int key_delay = 0;
//     snprintf(textValue, RAYGUI_VALUEBOX_MAX_CHARS, "%g", *value);
//
//     Rectangle textBounds = {0};
//     if (text != NULL)
//     {
//         textBounds.width = (float)(GetTextWidth(text) + 2);
//         textBounds.height = (float)(GuiGetStyle(DEFAULT, TEXT_SIZE));
//         textBounds.x = bounds.x + bounds.width + GuiGetStyle(VALUEBOX, TEXT_PADDING);
//         textBounds.y = bounds.y + bounds.height / 2 - GuiGetStyle(DEFAULT, TEXT_SIZE) / 2;
//         if (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_LEFT)
//             textBounds.x = bounds.x - textBounds.width - GuiGetStyle(VALUEBOX, TEXT_PADDING);
//     }
//
//     char* text_to_display = textValue;
//
//     // Update control
//     //--------------------------------------------------------------------
//     if ((state != STATE_DISABLED) && !guiLocked)
//     {
//         Vector2 mousePoint = GetMousePosition();
//
//         bool valueHasChanged = false;
//
//         if (editMode)
//         {
//             state = STATE_PRESSED;
//
//             int keyCount = (int)strlen(editingTextValue);
//
//             // Only allow keys in range [48..57]
//             if (keyCount < RAYGUI_VALUEBOX_MAX_CHARS)
//             {
//                 int key = GetCharPressed();
//                 if (((key >= 48) && (key <= 57)) || key == 46)
//                 {
//                     editingTextValue[keyCount] = (char)key;
//                     keyCount++;
//                     valueHasChanged = true;
//                 }
//             }
//
//             // Delete text
//             if (keyCount > 0)
//             {
//                 if (IsKeyPressed(KEY_BACKSPACE))
//                 {
//                     keyCount--;
//                     editingTextValue[keyCount] = '\0';
//                     valueHasChanged = true;
//                 }
//             }
//
//             if (!valueHasChanged && IsKeyDown(KEY_UP) && key_delay <= 0)
//             {
//                 *value += fabs(*value * 0.1) + 0.1;
//                 if (*value > maxValue)
//                     *value = maxValue;
//                 key_delay = 9;
//                 sprintf(editingTextValue, "%g", *value);
//             }
//             else if (!valueHasChanged && IsKeyDown(KEY_DOWN) && key_delay <= 0)
//             {
//                 *value -= fabs(*value * 0.1) + 0.1;
//                 if (*value < minValue)
//                     *value = minValue;
//                 key_delay = 9;
//                 sprintf(editingTextValue, "%g", *value);
//             }
//
//             if (key_delay > 0)
//                 key_delay--;
//             if (!IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN))
//                 key_delay = 0;
//             text_to_display = editingTextValue;
//             if (IsKeyPressed(KEY_ESCAPE))
//             {
//                 *value = original_value;
//                 result = 1;
//             }
//             else
//             {
//                 if (valueHasChanged)
//                     *value = (float)strtod(editingTextValue, NULL);
//                 if (IsKeyPressed(KEY_ENTER) ||
//                     (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
//                     result = 1;
//             }
//         }
//         else
//         {
//             if (*value > maxValue)
//                 *value = maxValue;
//             else if (*value < minValue)
//                 *value = minValue;
//
//             if (CheckCollisionPointRec(mousePoint, bounds))
//             {
//                 state = STATE_FOCUSED;
//                 if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
//                 {
//                     original_value = *value;
//                     memcpy(editingTextValue, textValue, RAYGUI_VALUEBOX_MAX_CHARS + 1);
//                     result = 1;
//                 }
//             }
//         }
//     }
//     //--------------------------------------------------------------------
//
//     // Draw control
//     //--------------------------------------------------------------------
//     Color baseColor = BLANK;
//     if (state == STATE_PRESSED)
//         baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED));
//     else if (state == STATE_DISABLED)
//         baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED));
//     else if (state == STATE_FOCUSED)
//         baseColor = WHITE;
//
//     GuiDrawRectangle(bounds, 0, GetColor(GuiGetStyle(VALUEBOX, BORDER + (state * 3))), baseColor);
//     GuiDrawText(text_to_display, GetTextBounds(VALUEBOX, bounds), TEXT_ALIGN_LEFT,
//                 GetColor(GuiGetStyle(VALUEBOX, TEXT)));
//
//     // Draw cursor
//     if (editMode)
//     {
//         Rectangle cursor = {bounds.x + GetTextWidth(text_to_display) + 1,
//                             bounds.y + 2 * GuiGetStyle(VALUEBOX, BORDER_WIDTH), 4,
//                             bounds.height - 4 * GuiGetStyle(VALUEBOX, BORDER_WIDTH)};
//         GuiDrawRectangle(cursor, 0, BLANK, GetColor(GuiGetStyle(LABEL, TEXT + (state * 3))));
//     }
//
//     // Draw text label if provided
//     GuiDrawText(text, textBounds,
//                 (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_RIGHT) ? TEXT_ALIGN_LEFT : TEXT_ALIGN_RIGHT,
//                 GetColor(GuiGetStyle(LABEL, TEXT)));
//     //--------------------------------------------------------------------
//
//     return result;
// }


/*__attribute__((format(printf, 4, 5))) void append_format(char** data, int* size, int* capacity, const char* format,
...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    int added = vsnprintf(*data + *size, *capacity - *size, format, arg_ptr);

    *size += MIN(added, *capacity - *size);
    assert(*size < *capacity);

    va_end(arg_ptr);
}*/

/*uint64_t FNV1a_64_hash(uint8_t* data, int len)
{
    uint64_t hash = 0xcbf29ce484222325;
    for (int i = 0; i < len; i++)
    {
        hash = (hash ^ data[i]) * 0x00000100000001B3;
    }

    return hash;
}*/

/*RenderTexture2D LoadFloatRenderTexture(int width, int height)
{
    RenderTexture2D target = {0};

    target = LoadRenderTexture(width, height); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32;
        target.texture.id = rlLoadTexture(NULL, width, height, target.texture.format, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

        if (rlFramebufferComplete(target.id))
        {
            print("FBO: [ID %i] Framebuffer object created successfully", target.id);
        }
        else
        {
            print("FBO: [ID %i] Framebuffer object FAILED", target.id);
        }

        rlDisableFramebuffer();
    }
    else
        TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}*/

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
    //          openSnapshot(droppedFiles.paths[0]);
    //      }
    //      UnloadDroppedFiles(droppedFiles);
    //  }

    return 0;
}
